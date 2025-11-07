/**
    @file luajit.c
    luajit: General-purpose Max external in Lua

    Allows writing complete Max externals in Lua with dynamic inlets/outlets.
    Inspired by pyext from the pktpy project.

    Features:
    - Dynamic inlets/outlets (1-16 each)
    - Message routing to Lua functions
    - Text editor integration
    - Hot reload support
    - Lua module path configuration
    - Attribute system
*/

#include "ext.h"
#include "ext_obex.h"
#include "ext_path.h"
#include "ext_sysfile.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// Include the shared Max API module for Lua
#include "luajit_api.h"

#define LUAJIT_MAX_INLETS 16
#define LUAJIT_MAX_OUTLETS 16

// Struct to represent the object's state
typedef struct _luajit {
    t_object ob;                    // Max object header

    // Lua instance
    lua_State* L;                   // Lua state (one per instance)
    char instance_name[128];        // Unique name for registry

    // Script info
    t_symbol* script_name;          // Script filename
    char script_path[MAX_PATH_CHARS];
    char script_filename[MAX_PATH_CHARS];
    short script_path_id;           // Max path ID

    // Dynamic I/O
    long num_inlets;                // Configurable (1-16)
    long num_outlets;               // Configurable (1-16)
    void* inlets[LUAJIT_MAX_INLETS];    // Proxy inlets
    void* outlets[LUAJIT_MAX_OUTLETS];  // Outlet pointers
    long inlet_num;                 // Current inlet (for proxies)

    // Text editor integration
    t_object* editor;
    char** code_buffer;
    long code_size;
    long run_on_save;
    long run_on_close;
} t_luajit;

// Method prototypes
void* luajit_new(t_symbol* s, long argc, t_atom* argv);
void luajit_free(t_luajit* x);
void luajit_assist(t_luajit* x, void* b, long m, long a, char* s);

// Lua initialization and script loading
bool luajit_init_lua(t_luajit* x);
bool luajit_load_script(t_luajit* x);
void luajit_configure_io(t_luajit* x);
bool luajit_create_io(t_luajit* x);
void luajit_inject_outlets(t_luajit* x);
void luajit_setup_lua_paths(t_luajit* x);

// Message handlers
void luajit_bang(t_luajit* x);
void luajit_int(t_luajit* x, long n);
void luajit_float(t_luajit* x, double f);
void luajit_list(t_luajit* x, t_symbol* s, long argc, t_atom* argv);
void luajit_anything(t_luajit* x, t_symbol* s, long argc, t_atom* argv);

// Text editor
void luajit_dblclick(t_luajit* x);
void luajit_edclose(t_luajit* x, char** text, long size);
t_max_err luajit_edsave(t_luajit* x, char** text, long size);
void luajit_okclose(t_luajit* x, char* s, short* result);
void luajit_read(t_luajit* x, t_symbol* s);
void luajit_doread(t_luajit* x, t_symbol* s, long argc, t_atom* argv);

// Utility
bool luajit_call_method(t_luajit* x, const char* method_name, long argc, t_atom* argv);
void luajit_reload(t_luajit* x);
t_max_err luajit_getvalue(t_luajit* x, t_symbol* key, long* argc, t_atom** argv);
t_max_err luajit_setvalue(t_luajit* x, t_symbol* key, long argc, t_atom* argv);

// Global class pointer
static t_class* luajit_class = NULL;

//-----------------------------------------------------------------------------------------------
// Main entry point
//-----------------------------------------------------------------------------------------------

void ext_main(void* r)
{
    t_class* c = class_new("luajit", (method)luajit_new, (method)luajit_free,
                          (long)sizeof(t_luajit), 0L, A_GIMME, 0);

    class_addmethod(c, (method)luajit_bang,     "bang",     0);
    class_addmethod(c, (method)luajit_int,      "int",      A_LONG, 0);
    class_addmethod(c, (method)luajit_float,    "float",    A_FLOAT, 0);
    class_addmethod(c, (method)luajit_list,     "list",     A_GIMME, 0);
    class_addmethod(c, (method)luajit_anything, "anything", A_GIMME, 0);
    class_addmethod(c, (method)luajit_assist,   "assist",   A_CANT, 0);

    // Text editor methods
    class_addmethod(c, (method)luajit_dblclick, "dblclick", A_CANT, 0);
    class_addmethod(c, (method)luajit_edclose,  "edclose",  A_CANT, 0);
    class_addmethod(c, (method)luajit_edsave,   "edsave",   A_CANT, 0);
    class_addmethod(c, (method)luajit_okclose,  "okclose",  A_CANT, 0);
    class_addmethod(c, (method)luajit_read,     "read",     A_DEFSYM, 0);

    // Attributes
    CLASS_ATTR_LONG(c, "run_on_save", 0, t_luajit, run_on_save);
    CLASS_ATTR_STYLE_LABEL(c, "run_on_save", 0, "onoff", "Reload on Save");
    CLASS_ATTR_SAVE(c, "run_on_save", 0);

    CLASS_ATTR_LONG(c, "run_on_close", 0, t_luajit, run_on_close);
    CLASS_ATTR_STYLE_LABEL(c, "run_on_close", 0, "onoff", "Reload on Close");
    CLASS_ATTR_SAVE(c, "run_on_close", 0);

    // Dynamic attribute support
    class_addmethod(c, (method)luajit_getvalue, "getvalue", A_SYM, 0);
    class_addmethod(c, (method)luajit_setvalue, "setvalue", A_GIMME, 0);

    class_register(CLASS_BOX, c);
    luajit_class = c;
}

//-----------------------------------------------------------------------------------------------
// Object lifecycle
//-----------------------------------------------------------------------------------------------

void* luajit_new(t_symbol* s, long argc, t_atom* argv)
{
    t_luajit* x = (t_luajit*)object_alloc(luajit_class);

    if (!x) {
        error("luajit: failed to allocate object");
        return NULL;
    }

    // Generate unique instance name
    snprintf(x->instance_name, 128, "_luajit_inst_%p", x);

    // Default configuration
    x->num_inlets = 1;
    x->num_outlets = 1;
    x->editor = NULL;
    x->code_buffer = NULL;
    x->code_size = 0;
    x->run_on_save = 0;
    x->run_on_close = 1;

    // Get script filename (first argument)
    if (argc > 0 && atom_gettype(argv) == A_SYM) {
        x->script_name = atom_getsym(argv);
    } else {
        x->script_name = gensym("");
    }

    if (x->script_name == gensym("")) {
        error("luajit: requires script filename as first argument");
        object_free(x);
        return NULL;
    }

    // Initialize Lua
    if (!luajit_init_lua(x)) {
        error("luajit: failed to initialize Lua");
        object_free(x);
        return NULL;
    }

    // Load script
    if (!luajit_load_script(x)) {
        error("luajit: failed to load script %s", x->script_name->s_name);
        object_free(x);
        return NULL;
    }

    // Read inlet/outlet config from Lua
    luajit_configure_io(x);

    // Create inlets/outlets based on config
    if (!luajit_create_io(x)) {
        error("luajit: failed to create I/O");
        object_free(x);
        return NULL;
    }

    // Inject outlet wrappers into Lua instance
    luajit_inject_outlets(x);

    post("luajit: created with %ld inlets, %ld outlets", x->num_inlets, x->num_outlets);

    // Process remaining attributes
    attr_args_process(x, argc, argv);

    return x;
}

void luajit_free(t_luajit* x)
{
    // Free code buffer
    if (x->code_buffer) {
        sysmem_freehandle(x->code_buffer);
    }

    // Close Lua state (handles all Lua cleanup)
    if (x->L) {
        lua_close(x->L);
        x->L = NULL;
    }

    // Free inlet proxies (skip first inlet, which is automatic)
    for (long i = 1; i < x->num_inlets; i++) {
        if (x->inlets[i]) {
            object_free(x->inlets[i]);
        }
    }

    // Outlets are freed automatically by Max
}

void luajit_assist(t_luajit* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "Inlet %ld", a);
    } else {
        sprintf(s, "Outlet %ld", a);
    }
}

//-----------------------------------------------------------------------------------------------
// Lua initialization and script loading
//-----------------------------------------------------------------------------------------------

bool luajit_init_lua(t_luajit* x)
{
    // Create Lua state
    x->L = luaL_newstate();
    if (!x->L) {
        error("luajit: failed to create Lua state");
        return false;
    }

    // Open standard libraries
    luaL_openlibs(x->L);

    // Setup Lua package paths for module loading
    luajit_setup_lua_paths(x);

    // Initialize the shared Max API module for Lua
    luajit_api_init(x->L);

    post("luajit: Lua initialized");
    return true;
}

void luajit_setup_lua_paths(t_luajit* x)
{
    // Get the path to the Max package
    short path_id = class_getpath(luajit_class);
    char package_path[MAX_PATH_CHARS];
    char lua_path[MAX_PATH_CHARS * 2];

    // Get absolute path to package
    path_toabsolutesystempath(path_id, "", package_path);

    // Setup package.path to include:
    // 1. Current directory (where the script is)
    // 2. Package examples directory
    // 3. Package lua_modules directory (if exists)
    snprintf(lua_path, sizeof(lua_path),
            "./?.lua;%s/examples/?.lua;%s/lua_modules/?.lua;%s/lua_modules/?/init.lua",
            package_path, package_path, package_path);

    // Set package.path
    lua_getglobal(x->L, "package");
    lua_pushstring(x->L, lua_path);
    lua_setfield(x->L, -2, "path");
    lua_pop(x->L, 1);

    post("luajit: Lua package.path set");
}

bool luajit_load_script(t_luajit* x)
{
    char filename[MAX_PATH_CHARS];
    char filepath[MAX_PATH_CHARS];
    t_fourcc outtype = 0; // outtype: 1248621921 -> 'Jlua'
    t_fourcc filetype = FOUR_CHAR_CODE('Jlua');
    short path_id;
    t_max_err err;

    // post("scriptname: %s", x->script_name->s_name);
    strncpy_zero(filename, x->script_name->s_name, MAX_PATH_CHARS);

    // Locate file in Max search path
    if (locatefile_extended(filename, &path_id, &outtype, &filetype, 1)) {
        error("luajit: cannot find script %s", x->script_name->s_name);
        return false;
    }

    // this works too (NULL,0 mean any type)
    // if (locatefile_extended(filename, &path_id, &outtype, NULL, 0)) {
    //     error("luajit: cannot find script %s", x->script_name->s_name);
    //     return false;
    // }

    // post("outtype: %d", outtype);

    // Get absolute path
    filepath[0] = '\0';
    err = path_toabsolutesystempath(path_id, filename, filepath);
    if (err != MAX_ERR_NONE) {
        error("luajit: cannot convert %s to absolute path", x->script_name->s_name);
        return false;
    }

    // Store paths
    x->script_path_id = path_id;
    strncpy(x->script_path, filepath, MAX_PATH_CHARS - 1);
    x->script_path[MAX_PATH_CHARS - 1] = '\0';
    strncpy(x->script_filename, filename, MAX_PATH_CHARS - 1);
    x->script_filename[MAX_PATH_CHARS - 1] = '\0';

    // Execute script
    if (luaL_dofile(x->L, filepath) != LUA_OK) {
        const char* err_msg = lua_tostring(x->L, -1);
        error("luajit: %s", err_msg);
        lua_pop(x->L, 1);
        return false;
    }

    post("luajit: loaded %s", filepath);
    return true;
}

void luajit_configure_io(t_luajit* x)
{
    // Get 'external' table from global scope
    lua_getglobal(x->L, "external");

    if (!lua_istable(x->L, -1)) {
        lua_pop(x->L, 1);
        return;  // Use defaults
    }

    // Read inlets
    lua_getfield(x->L, -1, "inlets");
    if (lua_isnumber(x->L, -1)) {
        long n = lua_tointeger(x->L, -1);
        x->num_inlets = CLAMP(n, 1, LUAJIT_MAX_INLETS);
    }
    lua_pop(x->L, 1);

    // Read outlets
    lua_getfield(x->L, -1, "outlets");
    if (lua_isnumber(x->L, -1)) {
        long n = lua_tointeger(x->L, -1);
        x->num_outlets = CLAMP(n, 1, LUAJIT_MAX_OUTLETS);
    }
    lua_pop(x->L, 1);

    lua_pop(x->L, 1);  // Pop external table
}

bool luajit_create_io(t_luajit* x)
{
    // Create inlets (first inlet is automatic, others use proxies)
    for (long i = 1; i < x->num_inlets; i++) {
        x->inlets[i] = proxy_new(x, i, &x->inlet_num);
        if (!x->inlets[i]) {
            error("luajit: failed to create inlet %ld", i);
            return false;
        }
    }

    // Create outlets (right to left)
    for (long i = x->num_outlets - 1; i >= 0; i--) {
        x->outlets[i] = outlet_new(x, NULL);
        if (!x->outlets[i]) {
            error("luajit: failed to create outlet %ld", i);
            return false;
        }
    }

    return true;
}

void luajit_inject_outlets(t_luajit* x)
{
    lua_newtable(x->L);  // Create _outlets table

    for (long i = 0; i < x->num_outlets; i++) {
        // Create lightweight outlet wrapper userdata
        void** outlet_ptr = (void**)lua_newuserdata(x->L, sizeof(void*));
        *outlet_ptr = x->outlets[i];

        // Set metatable with outlet methods
        luaL_getmetatable(x->L, "api.OutletWrapper");
        lua_setmetatable(x->L, -2);

        lua_rawseti(x->L, -2, i + 1);  // 1-indexed
    }

    lua_setglobal(x->L, "_outlets");
}

//-----------------------------------------------------------------------------------------------
// Message dispatch
//-----------------------------------------------------------------------------------------------

bool luajit_call_method(t_luajit* x, const char* method_name, long argc, t_atom* argv)
{
    // Store current inlet number in registry for api.proxy_getinlet()
    lua_pushinteger(x->L, proxy_getinlet((t_object*)x));
    lua_setfield(x->L, LUA_REGISTRYINDEX, "_luajit_current_inlet");

    // Get external table
    lua_getglobal(x->L, "external");
    if (!lua_istable(x->L, -1)) {
        lua_pop(x->L, 1);
        return false;  // Silent if no external table
    }

    // Get method
    lua_getfield(x->L, -1, method_name);
    if (!lua_isfunction(x->L, -1)) {
        lua_pop(x->L, 2);
        return false;  // Silent if method doesn't exist
    }

    // Push arguments
    for (long i = 0; i < argc; i++) {
        switch (atom_gettype(&argv[i])) {
            case A_LONG:
                lua_pushinteger(x->L, atom_getlong(&argv[i]));
                break;
            case A_FLOAT:
                lua_pushnumber(x->L, atom_getfloat(&argv[i]));
                break;
            case A_SYM:
                lua_pushstring(x->L, atom_getsym(&argv[i])->s_name);
                break;
            default:
                lua_pushnil(x->L);
        }
    }

    // Call method
    if (lua_pcall(x->L, argc, 0, 0) != LUA_OK) {
        error("luajit: %s", lua_tostring(x->L, -1));
        lua_pop(x->L, 1);
        lua_pop(x->L, 1);  // Pop external table
        return false;
    }

    lua_pop(x->L, 1);  // Pop external table
    return true;
}

void luajit_bang(t_luajit* x)
{
    luajit_call_method(x, "bang", 0, NULL);
}

void luajit_int(t_luajit* x, long n)
{
    t_atom atom;
    atom_setlong(&atom, n);
    luajit_call_method(x, "int", 1, &atom);
}

void luajit_float(t_luajit* x, double f)
{
    t_atom atom;
    atom_setfloat(&atom, f);
    luajit_call_method(x, "float", 1, &atom);
}

void luajit_list(t_luajit* x, t_symbol* s, long argc, t_atom* argv)
{
    luajit_call_method(x, "list", argc, argv);
}

void luajit_anything(t_luajit* x, t_symbol* s, long argc, t_atom* argv)
{
    luajit_call_method(x, s->s_name, argc, argv);
}

//-----------------------------------------------------------------------------------------------
// Text Editor Integration
//-----------------------------------------------------------------------------------------------

void luajit_dblclick(t_luajit* x)
{
    if (x->editor) {
        // Editor already exists, make it visible
        object_attr_setchar(x->editor, gensym("visible"), 1);
    } else {
        // Create new editor
        x->editor = object_new(CLASS_NOBOX, gensym("jed"), x, 0);

        // Load the script file content if we have a loaded script
        if (x->script_name != gensym("") && x->script_path[0] != 0) {
            // Read the file
            luajit_doread(x, x->script_name, 0, NULL);
            // Set the text in editor
            if (x->code_buffer && *x->code_buffer) {
                object_method(x->editor, gensym("settext"), *x->code_buffer, gensym("utf-8"));
            }
        }

        object_attr_setchar(x->editor, gensym("scratch"), 1);
        char title[256];
        snprintf(title, sizeof(title), "luajit: %s", x->script_name->s_name);
        object_attr_setsym(x->editor, gensym("title"), gensym(title));
    }
}

void luajit_edclose(t_luajit* x, char** text, long size)
{
    // Free old code buffer
    if (x->code_buffer) {
        sysmem_freehandle(x->code_buffer);
    }

    // Save new text
    x->code_buffer = sysmem_newhandleclear(size + 1);
    sysmem_copyptr((char*)*text, *x->code_buffer, size);
    x->code_size = size + 1;
    x->editor = NULL;

    // Optionally reload on close
    if (x->run_on_close && x->code_size > 2) {
        // Write back to file
        if (x->script_path[0] != 0) {
            t_filehandle fh;
            t_max_err err = path_createsysfile(x->script_filename, x->script_path_id,
                                              'TEXT', &fh);
            if (err == MAX_ERR_NONE) {
                t_ptr_size write_size = (t_ptr_size)size;
                sysfile_write(fh, &write_size, *x->code_buffer);
                sysfile_close(fh);
            }
        }
        // Reload the script
        luajit_reload(x);
    }
}

t_max_err luajit_edsave(t_luajit* x, char** text, long size)
{
    if (x->run_on_save) {
        post("luajit: run-on-save: reloading script");

        // Save text to code buffer
        if (x->code_buffer) {
            sysmem_freehandle(x->code_buffer);
        }
        x->code_buffer = sysmem_newhandleclear(size + 1);
        sysmem_copyptr((char*)*text, *x->code_buffer, size);
        x->code_size = size + 1;

        // Write back to file
        if (x->script_path[0] != 0) {
            t_filehandle fh;
            t_max_err err = path_createsysfile(x->script_filename, x->script_path_id,
                                              'TEXT', &fh);
            if (err == MAX_ERR_NONE) {
                t_ptr_size write_size = (t_ptr_size)size;
                sysfile_write(fh, &write_size, *x->code_buffer);
                sysfile_close(fh);
                // Reload the script
                luajit_reload(x);
            } else {
                error("luajit: failed to save script");
                return MAX_ERR_GENERIC;
            }
        }
    }
    return MAX_ERR_NONE;
}

void luajit_okclose(t_luajit* x, char* s, short* result)
{
    *result = 3; // Don't put up a dialog
}

void luajit_read(t_luajit* x, t_symbol* s)
{
    defer((t_object*)x, (method)luajit_doread, s, 0, NULL);
}

void luajit_doread(t_luajit* x, t_symbol* s, long argc, t_atom* argv)
{
    t_max_err err;
    t_filehandle fh;
    char filename[MAX_PATH_CHARS];
    short path;

    // Use provided symbol or current script
    if (s == gensym("") || s == NULL) {
        if (x->script_path[0] == 0) {
            error("luajit: no script loaded");
            return;
        }
        strncpy(filename, x->script_filename, MAX_PATH_CHARS - 1);
        filename[MAX_PATH_CHARS - 1] = '\0';
        path = x->script_path_id;
    } else {
        t_fourcc outtype = 0;
        t_fourcc filetype = FOUR_CHAR_CODE('TEXT');
        strncpy_zero(filename, s->s_name, MAX_PATH_CHARS);
        if (locatefile_extended(filename, &path, &outtype, &filetype, 1)) {
            error("luajit: can't find file: %s", s->s_name);
            return;
        }
    }

    // Read the file
    err = path_opensysfile(filename, path, &fh, READ_PERM);
    if (err == MAX_ERR_NONE) {
        sysfile_readtextfile(fh, x->code_buffer, 0, TEXT_LB_UNIX | TEXT_NULL_TERMINATE);
        sysfile_close(fh);
        x->code_size = sysmem_handlesize(x->code_buffer);
    } else {
        error("luajit: error reading file");
    }
}

//-----------------------------------------------------------------------------------------------
// Utility
//-----------------------------------------------------------------------------------------------

void luajit_reload(t_luajit* x)
{
    // Clear current state
    lua_pushnil(x->L);
    lua_setglobal(x->L, "external");

    lua_pushnil(x->L);
    lua_setglobal(x->L, "_outlets");

    // Reload script
    if (!luajit_load_script(x)) {
        error("luajit: reload failed");
        return;
    }

    // Re-configure (may change I/O counts)
    long old_inlets = x->num_inlets;
    long old_outlets = x->num_outlets;

    luajit_configure_io(x);

    // Check if I/O changed
    if (x->num_inlets != old_inlets || x->num_outlets != old_outlets) {
        error("luajit: inlet/outlet count changed - requires recreating object");
        // Restore old config
        x->num_inlets = old_inlets;
        x->num_outlets = old_outlets;
    }

    // Re-inject outlets
    luajit_inject_outlets(x);

    post("luajit: reloaded %s", x->script_path);
}

//-----------------------------------------------------------------------------------------------
// Dynamic Attribute System
//-----------------------------------------------------------------------------------------------

t_max_err luajit_getvalue(t_luajit* x, t_symbol* key, long* argc, t_atom** argv)
{
    // Get value from Lua global or external table
    lua_getglobal(x->L, "external");
    if (!lua_istable(x->L, -1)) {
        lua_pop(x->L, 1);
        return MAX_ERR_GENERIC;
    }

    lua_getfield(x->L, -1, key->s_name);

    if (lua_isnil(x->L, -1)) {
        lua_pop(x->L, 2);
        return MAX_ERR_GENERIC;
    }

    // Allocate atom for return value
    *argc = 1;
    *argv = (t_atom*)sysmem_newptr(sizeof(t_atom));

    // Convert Lua value to atom
    if (lua_isnumber(x->L, -1)) {
        double d = lua_tonumber(x->L, -1);
        double intpart;
        if (modf(d, &intpart) == 0.0) {
            atom_setlong(*argv, (long)d);
        } else {
            atom_setfloat(*argv, d);
        }
    } else if (lua_isstring(x->L, -1)) {
        atom_setsym(*argv, gensym(lua_tostring(x->L, -1)));
    } else if (lua_isboolean(x->L, -1)) {
        atom_setlong(*argv, lua_toboolean(x->L, -1));
    } else {
        sysmem_freeptr(*argv);
        *argc = 0;
        *argv = NULL;
        lua_pop(x->L, 2);
        return MAX_ERR_GENERIC;
    }

    lua_pop(x->L, 2);
    return MAX_ERR_NONE;
}

t_max_err luajit_setvalue(t_luajit* x, t_symbol* key, long argc, t_atom* argv)
{
    if (argc < 2) {
        return MAX_ERR_GENERIC;
    }

    t_symbol* attr_name = atom_getsym(argv);
    t_atom* value = argv + 1;

    // Get external table
    lua_getglobal(x->L, "external");
    if (!lua_istable(x->L, -1)) {
        lua_pop(x->L, 1);
        return MAX_ERR_GENERIC;
    }

    // Set value in external table
    switch (atom_gettype(value)) {
        case A_LONG:
            lua_pushinteger(x->L, atom_getlong(value));
            break;
        case A_FLOAT:
            lua_pushnumber(x->L, atom_getfloat(value));
            break;
        case A_SYM:
            lua_pushstring(x->L, atom_getsym(value)->s_name);
            break;
        default:
            lua_pop(x->L, 1);
            return MAX_ERR_GENERIC;
    }

    lua_setfield(x->L, -2, attr_name->s_name);
    lua_pop(x->L, 1);

    return MAX_ERR_NONE;
}

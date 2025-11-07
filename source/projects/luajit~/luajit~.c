/**
    @file
    luajit~: luajit for Max
*/

#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <libgen.h>
#include <unistd.h>

// Include the shared Max API module for Lua
#include "luajit_api.h"

#define USE_LUA 1


// struct to represent the object's state
typedef struct _mlj {
    t_pxobject ob;      // the object itself (t_pxobject in MSP instead of t_object)
    lua_State *L;       // lua state
    t_symbol* filename; // filename of lua file in Max search path
    t_symbol* funcname; // name of lua dsp function to use
    int func_ref;       // cached function reference in LUA_REGISTRYINDEX
    double param1;      // the value of a property of our object
    double prev_sample; // previous sample value (for feedback)
    double samplerate;  // current sample rate
    long vectorsize;    // current vector size
    char in_error_state; // flag to indicate Lua error state
} t_mlj;


// method prototypes
void *mlj_new(t_symbol *s, long argc, t_atom *argv);
void mlj_init_lua(t_mlj *x);
void mlj_free(t_mlj *x);
void mlj_assist(t_mlj *x, void *b, long m, long a, char *s);
void mlj_bang(t_mlj *x);
void mlj_anything(t_mlj* x, t_symbol* s, long argc, t_atom* argv);
void mlj_float(t_mlj *x, double f);
void mlj_dsp64(t_mlj *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void mlj_perform64(t_mlj *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

t_string* get_path_from_package(t_class* c, char* subpath);

// global class pointer variable
static t_class *mlj_class = NULL;


//-----------------------------------------------------------------------------------------------


int run_lua_string(t_mlj *x, const char* code)
{
    int err;
    err = luaL_dostring(x->L, code);
    if (err) {
        error("%s", lua_tostring(x->L, -1));
        lua_pop(x->L, 1);  /* pop error message from the stack */
    }
    return 0; 
}

int run_lua_file(t_mlj *x, const char* path)
{
    int err;
    err = luaL_dofile(x->L, path);
    if (err) {
        error("%s", lua_tostring(x->L, -1));
        lua_pop(x->L, 1);  /* pop error message from the stack */
    }
    return 0; 
}

float lua_dsp(t_mlj *x, float audio_in, float audio_prev, float n_samples, float param1) {
   // If in error state, return silence
   if (x->in_error_state) {
       return 0.0f;
   }

   // Get cached function reference
   if (x->func_ref == LUA_REFNIL || x->func_ref == LUA_NOREF) {
       x->in_error_state = 1;
       error("luajit~: no Lua function loaded");
       return 0.0f;
   }

   lua_rawgeti(x->L, LUA_REGISTRYINDEX, x->func_ref);

   // Verify it's still a function
   if (!lua_isfunction(x->L, -1)) {
       lua_pop(x->L, 1);
       x->in_error_state = 1;
       error("luajit~: cached reference is not a function");
       return 0.0f;
   }

   // Push arguments
   lua_pushnumber(x->L, audio_in);
   lua_pushnumber(x->L, audio_prev);
   lua_pushnumber(x->L, n_samples);
   lua_pushnumber(x->L, param1);

   // Call the function with 4 arguments, returning 1 result (protected call)
   int status = lua_pcall(x->L, 4, 1, 0);

   if (status != LUA_OK) {
       const char* err_msg = lua_tostring(x->L, -1);
       error("luajit~: Lua error: %s", err_msg);
       lua_pop(x->L, 1);
       x->in_error_state = 1;
       return 0.0f;
   }

   // Verify return value is a number
   if (!lua_isnumber(x->L, -1)) {
       error("luajit~: Lua function must return a number");
       lua_pop(x->L, 1);
       x->in_error_state = 1;
       return 0.0f;
   }

   // Get the result
   float result = (float)lua_tonumber(x->L, -1);
   lua_pop(x->L, 1);

   // Check for invalid values (NaN, Inf)
   if (isnan(result) || isinf(result)) {
       error("luajit~: Lua returned invalid value (NaN or Inf)");
       x->in_error_state = 1;
       return 0.0f;
   }

   // Clamp to safe range to prevent clipping/damage
   if (result > 1.0f) result = 1.0f;
   if (result < -1.0f) result = -1.0f;

   return result;
}


t_string* get_path_from_external(t_class* c, char* subpath)
{
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
    char conform_path[MAX_PATH_CHARS];
    short path_id = class_getpath(c);
    t_string* result;

#ifdef __APPLE__
    const char* ext_filename = "%s.mxo";
#else
    const char* ext_filename = "%s.mxe64";
#endif
    snprintf_zero(external_name, MAX_PATH_CHARS, ext_filename, c->c_sym->s_name);
    path_toabsolutesystempath(path_id, external_name, external_path);
    path_nameconform(external_path, conform_path, PATH_STYLE_MAX, PATH_TYPE_BOOT);
    result = string_new(external_path);
    if (subpath != NULL) {
        string_append(result, subpath);
    }
    return result;
}


t_string* get_path_from_package(t_class* c, char* subpath)
{
    t_string* result;
    t_string* external_path = get_path_from_external(c, NULL);

    const char* ext_path_c = string_getptr(external_path);

    // dirname() modifies its input, so we need to make a copy
    char* path_copy = strdup(ext_path_c);
    if (!path_copy) {
        object_free(external_path);
        return NULL;
    }

    char* dir1 = dirname(path_copy);
    char* dir1_copy = strdup(dir1);
    free(path_copy);

    if (!dir1_copy) {
        object_free(external_path);
        return NULL;
    }

    char* dir2 = dirname(dir1_copy);
    result = string_new(dir2);
    free(dir1_copy);

    if (subpath != NULL) {
        string_append(result, subpath);
    }

    object_free(external_path);  // Free the temporary path

    return result;
}

//-----------------------------------------------------------------------------------------------

void ext_main(void *r)
{
    // object initialization, note the use of dsp_free for the freemethod, which is required
    // unless you need to free allocated memory, in which case you should call dsp_free from
    // your custom free function.

    t_class *c = class_new("luajit~", (method)mlj_new, (method)mlj_free, (long)sizeof(t_mlj), 0L, A_GIMME, 0);

    class_addmethod(c, (method)mlj_float,    "float",    A_FLOAT, 0);
    class_addmethod(c, (method)mlj_anything, "anything", A_GIMME, 0);
    class_addmethod(c, (method)mlj_bang,     "bang",              0);
    class_addmethod(c, (method)mlj_dsp64,    "dsp64",    A_CANT,  0);
    class_addmethod(c, (method)mlj_assist,   "assist",   A_CANT,  0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    mlj_class = c;
}


void mlj_run_file(t_mlj *x)
{
    if (x->filename != gensym("")) {
        char norm_path[MAX_PATH_CHARS];
        path_nameconform(x->filename->s_name, norm_path,
            PATH_STYLE_MAX, PATH_TYPE_BOOT);
        if (access(norm_path, F_OK) == 0) { // file exists in path
            post("run %s", norm_path);
            run_lua_file(x, norm_path);
        } else { // try in the example folder
            t_string* path = get_path_from_package(mlj_class, "/examples/");
            if (!path) {
                error("luajit~: failed to get package path");
                return;
            }
            string_append(path, x->filename->s_name);
            const char* lua_file = string_getptr(path);
            post("run %s", lua_file);
            run_lua_file(x, lua_file);
            object_free(path);  // Free the path string
        }
    }
}


// Max API wrapper functions for Lua
static int lua_max_post(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    post("%s", msg);
    return 0;
}

static int lua_max_error(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    error("%s", msg);
    return 0;
}

void mlj_init_lua(t_mlj *x)
{
    x->L = luaL_newstate();
    luaL_openlibs(x->L);  /* opens the standard libraries */

    // Configure LuaJIT for real-time use
    // Stop the GC initially - we'll run it manually
    lua_gc(x->L, LUA_GCSTOP, 0);

    // Set initial sample rate (will be updated in dsp64)
    x->samplerate = 44100.0;
    lua_pushnumber(x->L, x->samplerate);
    lua_setglobal(x->L, "SAMPLE_RATE");

    // Initialize function reference to invalid
    x->func_ref = LUA_NOREF;
    x->in_error_state = 0;

    // Initialize the shared Max API module for Lua
    luajit_api_init(x->L);

    mlj_run_file(x);

    // Restart GC in incremental mode with conservative settings
    lua_gc(x->L, LUA_GCRESTART, 0);
    // Set incremental GC: smaller steps, longer pauses between steps
    lua_gc(x->L, LUA_GCSETPAUSE, 200);    // wait 2x memory before next GC
    lua_gc(x->L, LUA_GCSETSTEPMUL, 100);  // slower collection
}


void *mlj_new(t_symbol *s, long argc, t_atom *argv)
{
    t_mlj *x = (t_mlj *)object_alloc(mlj_class);

    if (x) {
        dsp_setup((t_pxobject *)x, 1);  // MSP inlets: arg is # of inlets and is REQUIRED!
        // use 0 if you don't need inlets

        outlet_new(x, "signal");        // signal outlet (note "signal" rather than NULL)
        x->param1 = 0.0;
        x->prev_sample = 0.0;
        x->samplerate = 44100.0;
        x->vectorsize = 64;
        x->func_ref = LUA_NOREF;
        x->in_error_state = 0;
        x->filename = atom_getsymarg(0, argc, argv); // 1st arg of object
        x->funcname = gensym("base");
        post("filename: %s", x->filename->s_name);

        // init lua
        mlj_init_lua(x);
    }
    return (x);
}


void mlj_free(t_mlj *x)
{
    // Release cached function reference if it exists
    if (x->func_ref != LUA_NOREF && x->func_ref != LUA_REFNIL) {
        luaL_unref(x->L, LUA_REGISTRYINDEX, x->func_ref);
    }
    lua_close(x->L);
    dsp_free((t_pxobject *)x);
}


void mlj_assist(t_mlj *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { //inlet
        sprintf(s, "I am inlet %ld", a);
    }
    else {  // outlet
        sprintf(s, "I am outlet %ld", a);
    }
}

void mlj_bang(t_mlj *x)
{
    // Release old function reference
    if (x->func_ref != LUA_NOREF && x->func_ref != LUA_REFNIL) {
        luaL_unref(x->L, LUA_REGISTRYINDEX, x->func_ref);
        x->func_ref = LUA_NOREF;
    }

    mlj_run_file(x);

    // Re-cache the current function
    lua_getglobal(x->L, x->funcname->s_name);
    if (lua_isfunction(x->L, -1)) {
        x->func_ref = luaL_ref(x->L, LUA_REGISTRYINDEX);
        x->in_error_state = 0;
        post("reloaded and cached function: %s", x->funcname->s_name);
    } else {
        lua_pop(x->L, 1);
        error("function '%s' not found after reload", x->funcname->s_name);
        x->in_error_state = 1;
    }
}

void mlj_anything(t_mlj* x, t_symbol* s, long argc, t_atom* argv)
{
    if (s != gensym("")) {
        // Release old function reference
        if (x->func_ref != LUA_NOREF && x->func_ref != LUA_REFNIL) {
            luaL_unref(x->L, LUA_REGISTRYINDEX, x->func_ref);
        }

        // Get the new function and cache its reference
        lua_getglobal(x->L, s->s_name);
        if (lua_isfunction(x->L, -1)) {
            x->func_ref = luaL_ref(x->L, LUA_REGISTRYINDEX);
            x->funcname = s;
            x->in_error_state = 0;  // Clear error state on successful function change
            post("funcname: %s", s->s_name);
        } else {
            lua_pop(x->L, 1);
            error("'%s' is not a function", s->s_name);
            x->func_ref = LUA_NOREF;
            x->in_error_state = 1;
        }
    }
}


void mlj_float(t_mlj *x, double f)
{
    x->param1 = f;
    // Removed post() - too noisy for RT parameter changes
}


void mlj_dsp64(t_mlj *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    post("sample rate: %f", samplerate);
    post("maxvectorsize: %d", maxvectorsize);

    // Store sample rate and vector size
    x->samplerate = samplerate;
    x->vectorsize = maxvectorsize;

    // Update Lua global
    lua_pushnumber(x->L, samplerate);
    lua_setglobal(x->L, "SAMPLE_RATE");

    object_method(dsp64, gensym("dsp_add64"), x, mlj_perform64, 0, NULL);
}


#if defined USE_LUA

void mlj_perform64(t_mlj *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    t_double *inL = ins[0];     // we get audio for each inlet of the object from the **ins argument
    t_double *outL = outs[0];   // we get audio for each outlet of the object from the **outs argument
    int n = sampleframes;
    double prev = x->prev_sample;

    // If in error state, output silence
    if (x->in_error_state) {
        while (n--) {
            *outL++ = 0.0;
        }
        return;
    }

    while (n--) {
        prev = lua_dsp(x, *inL++, prev, n, x->param1);
        *outL++ = prev;
    }

    x->prev_sample = prev;
}

#else

// experiment in c here

void mlj_perform64(t_mlj *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    t_double *inL = ins[0];     // we get audio for each inlet of the object from the **ins argument
    t_double *outL = outs[0];   // we get audio for each outlet of the object from the **outs argument
    int n = sampleframes;       // n = 64
    double v0;
    double b = 1 - x->param1;
    double v1 = x->v1;

    while (n--) {
        v0 = (*inL++);
        v1 += b * (v0 - v1);
        *outL++ = v1;
    }
    x->v1 = v1;
}

#endif

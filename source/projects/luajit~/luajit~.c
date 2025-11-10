/**
    @file
    luajit~: luajit for Max
*/

#include <ctype.h>

#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"

// Shared library - single header with all common functionality
#include "luajit_external.h"

// struct to represent the object's state
typedef struct _mlj {
    t_pxobject ob;           // the object itself (t_pxobject in MSP instead of t_object)
    luajit_engine* engine;   // Lua engine (allocated separately)
    double param1;           // legacy single parameter support
} t_mlj;


// method prototypes
void *mlj_new(t_symbol *s, long argc, t_atom *argv);
void mlj_init_lua(t_mlj *x);
void mlj_free(t_mlj *x);
void mlj_assist(t_mlj *x, void *b, long m, long a, char *s);
void mlj_bang(t_mlj *x);
void mlj_anything(t_mlj* x, t_symbol* s, long argc, t_atom* argv);
void mlj_list(t_mlj* x, t_symbol* s, long argc, t_atom* argv);
void mlj_float(t_mlj *x, double f);
void mlj_dsp64(t_mlj *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void mlj_perform64(t_mlj *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

// global class pointer variable
static t_class *mlj_class = NULL;


//-----------------------------------------------------------------------------------------------

// Adapter for mxh_load_lua_file
static int load_lua_file_adapter(void* context, const char* path) {
    t_mlj* x = (t_mlj*)context;
    return lua_engine_run_file(x->engine->L, path);
}

void mlj_run_file(t_mlj *x) {
    mxh_load_lua_file(mlj_class, x->engine->filename, load_lua_file_adapter, x);
}

//-----------------------------------------------------------------------------------------------

void ext_main(void *r)
{
    t_class *c = class_new("luajit~", (method)mlj_new, (method)mlj_free, (long)sizeof(t_mlj), 0L, A_GIMME, 0);

    class_addmethod(c, (method)mlj_float,    "float",    A_FLOAT, 0);
    class_addmethod(c, (method)mlj_list,     "list",     A_GIMME, 0);
    class_addmethod(c, (method)mlj_anything, "anything", A_GIMME, 0);
    class_addmethod(c, (method)mlj_bang,     "bang",              0);
    class_addmethod(c, (method)mlj_dsp64,    "dsp64",    A_CANT,  0);
    class_addmethod(c, (method)mlj_assist,   "assist",   A_CANT,  0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    mlj_class = c;
}


void mlj_init_lua(t_mlj *x)
{
    // Allocate and initialize Lua engine (no custom bindings)
    x->engine = luajit_new(NULL, "luajit~");
    // Note: Don't load file here - filename needs to be set first
}


void *mlj_new(t_symbol *s, long argc, t_atom *argv)
{
    t_mlj *x = (t_mlj *)object_alloc(mlj_class);

    if (x) {
        dsp_setup((t_pxobject *)x, 1);  // MSP inlets: arg is # of inlets and is REQUIRED!
        outlet_new(x, "signal");         // signal outlet (note "signal" rather than NULL)

        // Initialize legacy parameter
        x->param1 = 0.0;
        x->engine = NULL;

        // Allocate and initialize Lua engine
        mlj_init_lua(x);

        // Set filename and funcname if engine was created successfully
        if (x->engine) {
            x->engine->filename = atom_getsymarg(0, argc, argv); // 1st arg of object
            x->engine->funcname = gensym("base");
            post("filename: %s", x->engine->filename->s_name);

            // Now load the Lua file
            mlj_run_file(x);
        }
    }
    return (x);
}


void mlj_free(t_mlj *x)
{
    luajit_free(x->engine);
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
    if (x->engine) {
        luajit_handle_bang(x->engine, x, (luajit_run_file_func)mlj_run_file, "luajit~");
    }
}

void mlj_list(t_mlj* x, t_symbol* s, long argc, t_atom* argv)
{
    if (x->engine) {
        luajit_handle_list(x->engine, x, s, argc, argv, NULL, "luajit~");
    }
}

void mlj_anything(t_mlj* x, t_symbol* s, long argc, t_atom* argv)
{
    if (x->engine) {
        luajit_handle_anything(x->engine, x, s, argc, argv, NULL, "luajit~");
    }
}

void mlj_float(t_mlj *x, double f)
{
    // Update legacy parameter
    x->param1 = f;
    // Use standard handler for engine parameters
    if (x->engine) {
        luajit_handle_float(x->engine, f);
    }
}


void mlj_dsp64(t_mlj *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    if (x->engine) {
        luajit_handle_dsp64(x->engine, x, dsp64, count, samplerate, maxvectorsize, flags, mlj_perform64);
    }
}

void mlj_perform64(t_mlj *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    if (x->engine) {
        luajit_handle_perform64(x->engine, dsp64, ins, numins, outs, numouts, sampleframes, flags, userparam);
    }
}

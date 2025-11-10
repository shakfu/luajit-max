/**
    @file
    luajit~: luajit for Max
*/

#include <ctype.h>

#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"

// Shared libraries
#include "lua_engine.h"
#include "max_helpers.h"

// Lua API module
#include "luajit_api.h"

// Maximum number of dynamic parameters
#define MAX_PARAMS 32

// struct to represent the object's state
typedef struct _mlj {
    t_pxobject ob;           // the object itself (t_pxobject in MSP instead of t_object)
    lua_State *L;            // lua state
    t_symbol* filename;      // filename of lua file in Max search path
    t_symbol* funcname;      // name of lua dsp function to use
    int func_ref;            // cached function reference in LUA_REGISTRYINDEX
    double param1;           // the value of a property of our object (legacy support)
    double params[MAX_PARAMS]; // dynamic parameter array
    int num_params;          // number of active parameters
    double prev_sample;      // previous sample value (for feedback)
    double samplerate;       // current sample rate
    long vectorsize;         // current vector size
    char in_error_state;     // flag to indicate Lua error state
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
    return lua_engine_run_file(x->L, path);
}

void mlj_run_file(t_mlj *x) {
    mxh_load_lua_file(mlj_class, x->filename, load_lua_file_adapter, x);
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
    // Create Lua state with RT-safe configuration
    x->L = lua_engine_init();
    if (!x->L) {
        error("luajit~: failed to initialize Lua engine");
        x->in_error_state = 1;
        return;
    }

    // Set initial sample rate (will be updated in dsp64)
    x->samplerate = 44100.0;
    lua_engine_set_samplerate(x->L, x->samplerate);

    // Initialize function reference to invalid
    x->func_ref = LUA_NOREF;
    x->in_error_state = 0;

    // Initialize the shared Max API module for Lua
    luajit_api_init(x->L);

    // Load Lua file
    mlj_run_file(x);
}


void *mlj_new(t_symbol *s, long argc, t_atom *argv)
{
    t_mlj *x = (t_mlj *)object_alloc(mlj_class);

    if (x) {
        dsp_setup((t_pxobject *)x, 1);  // MSP inlets: arg is # of inlets and is REQUIRED!
        outlet_new(x, "signal");         // signal outlet (note "signal" rather than NULL)

        x->param1 = 0.0;

        // Initialize dynamic parameter array
        x->num_params = 1;
        for (int i = 0; i < MAX_PARAMS; i++) {
            x->params[i] = 0.0;
        }

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
    lua_engine_release_function(x->L, x->func_ref);
    lua_engine_free(x->L);
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
    lua_engine_release_function(x->L, x->func_ref);
    x->func_ref = LUA_NOREF;

    mlj_run_file(x);

    // Re-cache the current function
    x->func_ref = lua_engine_cache_function(x->L, x->funcname->s_name);
    if (x->func_ref == LUA_NOREF) {
        x->in_error_state = 1;
        error("function '%s' not found after reload", x->funcname->s_name);
    } else {
        x->in_error_state = 0;
        post("reloaded and cached function: %s", x->funcname->s_name);
    }
}

void mlj_list(t_mlj* x, t_symbol* s, long argc, t_atom* argv)
{
    // Handle list messages as positional numeric parameters
    if (argc == 0) return;

    // Check if all arguments are numeric (positional parameters)
    int all_numeric = 1;
    for (long i = 0; i < argc; i++) {
        if (atom_gettype(argv + i) != A_FLOAT && atom_gettype(argv + i) != A_LONG) {
            all_numeric = 0;
            break;
        }
    }

    if (all_numeric) {
        // Positional numeric list: "10 0.1 4"
        x->num_params = (argc > MAX_PARAMS) ? MAX_PARAMS : argc;
        for (long i = 0; i < x->num_params; i++) {
            x->params[i] = atom_getfloat(argv + i);
        }
        post("set %d params: positional", x->num_params);
    } else {
        // Parse as named parameters: "delay 2 feedback 0.5 dry_wet 0.5"
        if (argc % 2 != 0) {
            error("named parameters must be in pairs: name value");
            return;
        }

        // Clear existing named parameters
        lua_engine_clear_named_params(x->L);

        for (long i = 0; i < argc; i += 2) {
            if (atom_gettype(argv + i) != A_SYM) {
                error("parameter names must be symbols");
                return;
            }

            t_symbol* param_name = atom_getsym(argv + i);
            const char* name = param_name->s_name;
            double value = atom_getfloat(argv + i + 1);

            // Set named parameter in Lua PARAMS table
            lua_engine_set_named_param(x->L, name, value);
        }
        post("set %ld named params", argc / 2);
    }
}

void mlj_anything(t_mlj* x, t_symbol* s, long argc, t_atom* argv)
{
    if (s != gensym("")) {
        // Check if this is a named parameter message with arguments
        if (argc > 0) {
            // Check if 's' is a valid function name
            // Try to cache it - if it succeeds, this is a combined function+params message
            int test_ref = lua_engine_cache_function(x->L, s->s_name);

            if (test_ref != LUA_NOREF) {
                // Valid function - this is combined syntax: "funcname param1 val1 param2 val2"
                // Release old function reference and use new one
                lua_engine_release_function(x->L, x->func_ref);
                x->func_ref = test_ref;
                x->funcname = s;
                x->in_error_state = 0;
                post("funcname: %s", s->s_name);

                // Now process the parameters
                mlj_list(x, s, argc, argv);
            } else {
                // Not a function - treat as named parameters only
                mlj_list(x, s, argc, argv);
            }
        } else {
            // No arguments - just switch function
            // Release old function reference
            lua_engine_release_function(x->L, x->func_ref);

            // Get the new function and cache its reference
            x->func_ref = lua_engine_cache_function(x->L, s->s_name);
            if (x->func_ref == LUA_NOREF) {
                x->in_error_state = 1;
                error("'%s' is not a function", s->s_name);
            } else {
                x->funcname = s;
                x->in_error_state = 0;  // Clear error state on successful function change
                post("funcname: %s", s->s_name);
            }
        }
    }
}


void mlj_float(t_mlj *x, double f)
{
    x->param1 = f;
    // Also set first parameter in array for consistency
    x->params[0] = f;
    if (x->num_params == 0) {
        x->num_params = 1;
    }
}


void mlj_dsp64(t_mlj *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    post("sample rate: %f", samplerate);
    post("maxvectorsize: %d", maxvectorsize);

    // Store sample rate and vector size
    x->samplerate = samplerate;
    x->vectorsize = maxvectorsize;

    // Update Lua global
    lua_engine_set_samplerate(x->L, samplerate);

    object_method(dsp64, gensym("dsp_add64"), x, mlj_perform64, 0, NULL);
}


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

    // Convert params to float array for lua_engine
    float float_params[MAX_PARAMS];
    for (int i = 0; i < x->num_params; i++) {
        float_params[i] = (float)x->params[i];
    }

    while (n--) {
        // Use dynamic parameter version
        prev = lua_engine_call_dsp_dynamic(x->L, x->func_ref, &x->in_error_state,
                                           *inL++, prev, n, float_params, x->num_params);
        *outL++ = prev;
    }

    x->prev_sample = prev;
}

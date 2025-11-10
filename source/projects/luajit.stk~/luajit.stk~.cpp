/**
    @file
    luajit.stk~: luajit+stk for Max
*/

#include <cstdlib>
#include <cctype>

#include "stk_bindings.h"  // STK bindings (includes lua.hpp, LuaBridge, and all STK headers)

#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"

// Shared libraries
extern "C" {
#include "lua_engine.h"
#include "max_helpers.h"
#include "luajit_api.h"
}

// Maximum number of dynamic parameters
#define MAX_PARAMS 32

enum {
    PARAM0 = 0,
    PARAM1,
    PARAM2,
    PARAM3,
    MAX_INLET_INDEX // -> maximum number of inlets (0-based)
};

// struct to represent the object's state
typedef struct _lstk {
    t_pxobject ob;           // the object itself (t_pxobject in MSP instead of t_object)
    lua_State *L;            // lua state
    t_symbol* filename;      // filename of lua file in Max search path
    t_symbol* funcname;      // name of lua dsp function to use
    int func_ref;            // cached function reference in LUA_REGISTRYINDEX
    double param0;           // parameter 0 (leftmost) - legacy support
    double param1;           // parameter 1 - legacy support
    double param2;           // parameter 2 - legacy support
    double param3;           // parameter 3 (rightmost) - legacy support
    double params[MAX_PARAMS]; // dynamic parameter array
    int num_params;          // number of active parameters
    double prev_sample;      // previous sample value (for feedback)
    double samplerate;       // current sample rate
    long vectorsize;         // current vector size
    char in_error_state;     // flag to indicate Lua error state
    long m_in;               // space for the inlet number used by all of the proxies
    void *inlets[MAX_INLET_INDEX];
} t_lstk;


// method prototypes
void *lstk_new(t_symbol *s, long argc, t_atom *argv);
void lstk_init_lua(t_lstk *x);
void lstk_free(t_lstk *x);
void lstk_assist(t_lstk* x, void* b, long io, long idx, char* s);
void lstk_bang(t_lstk *x);
void lstk_anything(t_lstk* x, t_symbol* s, long argc, t_atom* argv);
void lstk_list(t_lstk* x, t_symbol* s, long argc, t_atom* argv);
void lstk_float(t_lstk *x, double f);
void lstk_dsp64(t_lstk *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void lstk_perform64(t_lstk *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

// global class pointer variable
static t_class *lstk_class = NULL;


//-----------------------------------------------------------------------------------------------

// Adapter for mxh_load_lua_file
static int load_lua_file_adapter(void* context, const char* path) {
    t_lstk* x = (t_lstk*)context;
    return lua_engine_run_file(x->L, path);
}

void lstk_run_file(t_lstk *x) {
    mxh_load_lua_file(lstk_class, x->filename, load_lua_file_adapter, x);
}

//-----------------------------------------------------------------------------------------------

void ext_main(void *r)
{
    t_class *c = class_new("luajit.stk~", (method)lstk_new, (method)lstk_free, (long)sizeof(t_lstk), 0L, A_GIMME, 0);

    class_addmethod(c, (method)lstk_float,    "float",    A_FLOAT, 0);
    class_addmethod(c, (method)lstk_list,     "list",     A_GIMME, 0);
    class_addmethod(c, (method)lstk_anything, "anything", A_GIMME, 0);
    class_addmethod(c, (method)lstk_bang,     "bang",              0);
    class_addmethod(c, (method)lstk_dsp64,    "dsp64",    A_CANT,  0);
    class_addmethod(c, (method)lstk_assist,   "assist",   A_CANT,  0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    lstk_class = c;
}


void *lstk_new(t_symbol *s, long argc, t_atom *argv)
{
    t_lstk *x = (t_lstk *)object_alloc(lstk_class);

    if (x) {
        dsp_setup((t_pxobject *)x, 1);  // MSP inlets: arg is # of inlets and is REQUIRED!
        outlet_new(x, "signal");         // signal outlet (note "signal" rather than NULL)

        x->param0 = 0.0;
        x->param1 = 0.0;
        x->param2 = 0.0;
        x->param3 = 0.0;

        // Initialize dynamic parameter array
        x->num_params = 4;  // Default to 4 params for backward compatibility
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
        post("load: %s", x->filename->s_name);

        for(int i = (MAX_INLET_INDEX - 1); i > 0; i--) {
            x->inlets[i] = proxy_new((t_object *)x, i, &x->m_in);
        }

        // init lua
        lstk_init_lua(x);
    }
    return (x);
}


void lstk_free(t_lstk *x)
{
    // Release cached function reference if it exists
    lua_engine_release_function(x->L, x->func_ref);
    lua_engine_free(x->L);
    dsp_free((t_pxobject *)x);

    for(int i = (MAX_INLET_INDEX - 1); i > 0; i--) {
        if (x->inlets[i]) {
            object_free(x->inlets[i]);
            x->inlets[i] = NULL;
        }
    }
}


void lstk_assist(t_lstk* x, void* b, long io, long idx, char* s)
{
    /* Document inlets */
    if (io == ASSIST_INLET) {
        switch (idx) {
        case PARAM0:
            snprintf_zero(s, ASSIST_MAX_STRING_LEN, "%ld: input/param", idx);
            break;
        case PARAM1:
            snprintf_zero(s, ASSIST_MAX_STRING_LEN, "%ld: param", idx);
            break;
        case PARAM2:
            snprintf_zero(s, ASSIST_MAX_STRING_LEN, "%ld: param", idx);
            break;
        case PARAM3:
            snprintf_zero(s, ASSIST_MAX_STRING_LEN, "%ld: param", idx);
            break;
        }
    }

    /* Document outlet */
    else {  // outlet
        snprintf_zero(s, ASSIST_MAX_STRING_LEN, "outlet %ld", idx);
    }
}



void lstk_bang(t_lstk *x)
{
    // Release old function reference
    lua_engine_release_function(x->L, x->func_ref);
    x->func_ref = LUA_NOREF;

    lstk_run_file(x);

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

void lstk_list(t_lstk* x, t_symbol* s, long argc, t_atom* argv)
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
        // Also update legacy params for backward compatibility
        if (x->num_params > 0) x->param0 = x->params[0];
        if (x->num_params > 1) x->param1 = x->params[1];
        if (x->num_params > 2) x->param2 = x->params[2];
        if (x->num_params > 3) x->param3 = x->params[3];
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

void lstk_anything(t_lstk* x, t_symbol* s, long argc, t_atom* argv)
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
                lstk_list(x, s, argc, argv);
            } else {
                // Not a function - treat as named parameters only
                lstk_list(x, s, argc, argv);
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


void lstk_float(t_lstk *x, double f)
{
    long inlet = proxy_getinlet((t_object *)x);
    switch (inlet) {
        case 0:
            x->param0 = f;
            x->params[0] = f;
            break;
        case 1:
            x->param1 = f;
            x->params[1] = f;
            break;
        case 2:
            x->param2 = f;
            x->params[2] = f;
            break;
        case 3:
            x->param3 = f;
            x->params[3] = f;
            break;
    }
    // Ensure num_params covers the inlet that was used
    if (inlet >= x->num_params) {
        x->num_params = inlet + 1;
    }
}


void lstk_dsp64(t_lstk *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    post("sample rate: %f", samplerate);
    post("maxvectorsize: %d", maxvectorsize);

    // Store sample rate and vector size
    x->samplerate = samplerate;
    x->vectorsize = maxvectorsize;

    // Update Lua global
    lua_engine_set_samplerate(x->L, samplerate);

    object_method(dsp64, gensym("dsp_add64"), x, lstk_perform64, 0, NULL);
}


void lstk_perform64(t_lstk *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
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


void lstk_init_lua(t_lstk *x)
{
    // Create Lua state with RT-safe configuration
    x->L = lua_engine_init();
    if (!x->L) {
        error("luajit.stk~: failed to initialize Lua engine");
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

    // Wrap STK binding in try-catch to handle potential exceptions
    try {
        register_stk_bindings(x->L);
    } catch (std::exception& e) {
        error("luajit.stk~: STK initialization error: %s", e.what());
        x->in_error_state = 1;
    } catch (...) {
        error("luajit.stk~: Unknown STK initialization error");
        x->in_error_state = 1;
    }

    // Load Lua file
    lstk_run_file(x);
}

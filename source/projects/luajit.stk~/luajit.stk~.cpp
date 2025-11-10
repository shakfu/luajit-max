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

// Shared library - single header with all common functionality
extern "C" {
#include "luajit_external.h"
}

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
    luajit_engine* engine;   // Lua engine (allocated separately)
    double param0;           // parameter 0 (leftmost) - legacy support
    double param1;           // parameter 1 - legacy support
    double param2;           // parameter 2 - legacy support
    double param3;           // parameter 3 (rightmost) - legacy support
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

// STK custom bindings callback
static int stk_bindings_callback(lua_State* L) {
    try {
        register_stk_bindings(L);
        return 0;  // Success
    } catch (std::exception& e) {
        error("luajit.stk~: STK initialization error: %s", e.what());
        return -1;  // Failure
    } catch (...) {
        error("luajit.stk~: Unknown STK initialization error");
        return -1;  // Failure
    }
}

// Helper callback to sync legacy parameters after list processing
static void sync_legacy_params(void* context, long argc, t_atom* argv) {
    t_lstk* x = (t_lstk*)context;
    if (x->engine && x->engine->num_params > 0) {
        if (x->engine->num_params > 0) x->param0 = x->engine->params[0];
        if (x->engine->num_params > 1) x->param1 = x->engine->params[1];
        if (x->engine->num_params > 2) x->param2 = x->engine->params[2];
        if (x->engine->num_params > 3) x->param3 = x->engine->params[3];
    }
}

// Adapter for mxh_load_lua_file
static int load_lua_file_adapter(void* context, const char* path) {
    t_lstk* x = (t_lstk*)context;
    return lua_engine_run_file(x->engine->L, path);
}

void lstk_run_file(t_lstk *x) {
    mxh_load_lua_file(lstk_class, x->engine->filename, load_lua_file_adapter, x);
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

        // Initialize legacy parameters
        x->param0 = 0.0;
        x->param1 = 0.0;
        x->param2 = 0.0;
        x->param3 = 0.0;
        x->engine = NULL;

        // Create proxy inlets
        for(int i = (MAX_INLET_INDEX - 1); i > 0; i--) {
            x->inlets[i] = proxy_new((t_object *)x, i, &x->m_in);
        }

        // Allocate and initialize Lua engine with STK bindings
        lstk_init_lua(x);

        // Set filename and funcname if engine was created successfully
        if (x->engine) {
            x->engine->filename = atom_getsymarg(0, argc, argv); // 1st arg of object
            x->engine->funcname = gensym("base");
            x->engine->num_params = 4;  // Default to 4 params for backward compatibility
            post("load: %s", x->engine->filename->s_name);

            // Now load the Lua file
            lstk_run_file(x);
        }
    }
    return (x);
}


void lstk_free(t_lstk *x)
{
    luajit_free(x->engine);
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
    if (x->engine) {
        luajit_handle_bang(x->engine, x, (luajit_run_file_func)lstk_run_file, "luajit.stk~");
    }
}

void lstk_list(t_lstk* x, t_symbol* s, long argc, t_atom* argv)
{
    if (x->engine) {
        luajit_handle_list(x->engine, x, s, argc, argv, sync_legacy_params, "luajit.stk~");
    }
}

void lstk_anything(t_lstk* x, t_symbol* s, long argc, t_atom* argv)
{
    if (x->engine) {
        luajit_handle_anything(x->engine, x, s, argc, argv, sync_legacy_params, "luajit.stk~");
    }
}


void lstk_float(t_lstk *x, double f)
{
    if (!x->engine) return;

    long inlet = proxy_getinlet((t_object *)x);

    // Update engine params
    if (inlet < LUAJIT_MAX_PARAMS) {
        x->engine->params[inlet] = f;

        // Ensure num_params covers the inlet that was used
        if (inlet >= x->engine->num_params) {
            x->engine->num_params = inlet + 1;
        }
    }

    // Update legacy params
    switch (inlet) {
        case 0:
            x->param0 = f;
            break;
        case 1:
            x->param1 = f;
            break;
        case 2:
            x->param2 = f;
            break;
        case 3:
            x->param3 = f;
            break;
    }
}


void lstk_dsp64(t_lstk *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    if (x->engine) {
        luajit_handle_dsp64(x->engine, x, dsp64, count, samplerate, maxvectorsize, flags, (void*)lstk_perform64);
    }
}

void lstk_perform64(t_lstk *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    if (x->engine) {
        luajit_handle_perform64(x->engine, dsp64, ins, numins, outs, numouts, sampleframes, flags, userparam);
    }
}

void lstk_init_lua(t_lstk *x)
{
    // Allocate and initialize Lua engine with STK bindings
    x->engine = luajit_new(stk_bindings_callback, "luajit.stk~");
    // Note: Don't load file here - filename needs to be set first
}

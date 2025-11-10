# luajit_external.h Usage Guide

This single-header library provides **complete** functionality for luajit-max externals, including:
- Lua engine management (initialization, file loading, function caching)
- Max/MSP path resolution and file loading
- Message handlers (bang, list, anything, float)
- DSP callbacks (dsp64, perform64)
- Named and positional parameter systems

## Basic Usage

### 1. Include the header

```c
#include "luajit_external.h"
```

### 2. Add luajit_engine pointer to your struct

```c
typedef struct _mlj {
    t_pxobject ob;           // Max object (must be first)
    luajit_engine* engine;   // Engine pointer (allocated separately)
    // ... external-specific fields ...
} t_mlj;
```

### 3. Allocate and free the engine

```c
void mlj_init_lua(t_mlj *x) {
    // Allocate and initialize (returns NULL on failure)
    x->engine = luajit_new(NULL, "luajit~");
    if (x->engine) {
        // Set filename/funcname after allocation
        x->engine->filename = atom_getsymarg(0, argc, argv);
        x->engine->funcname = gensym("base");
        mlj_run_file(x);
    }
}

void mlj_free(t_mlj *x) {
    luajit_free(x->engine);  // Handles NULL safely
    dsp_free((t_pxobject *)x);
}
```

### 4. Use the provided handlers (with NULL checks)

#### Message Handlers

```c
void mlj_bang(t_mlj *x) {
    if (x->engine) {
        luajit_handle_bang(x->engine, x, mlj_run_file, "luajit~");
    }
}

void mlj_list(t_mlj* x, t_symbol* s, long argc, t_atom* argv) {
    if (x->engine) {
        luajit_handle_list(x->engine, x, s, argc, argv, NULL, "luajit~");
    }
}

void mlj_anything(t_mlj* x, t_symbol* s, long argc, t_atom* argv) {
    if (x->engine) {
        luajit_handle_anything(x->engine, x, s, argc, argv, NULL, "luajit~");
    }
}

void mlj_float(t_mlj *x, double f) {
    if (x->engine) {
        luajit_handle_float(x->engine, f);
    }
}
```

#### DSP Callbacks

```c
void mlj_dsp64(t_mlj *x, t_object *dsp64, short *count,
               double samplerate, long maxvectorsize, long flags) {
    if (x->engine) {
        luajit_handle_dsp64(x->engine, x, dsp64, count, samplerate,
                            maxvectorsize, flags, mlj_perform64);
    }
}

void mlj_perform64(t_mlj *x, t_object *dsp64, double **ins, long numins,
                   double **outs, long numouts, long sampleframes,
                   long flags, void *userparam) {
    if (x->engine) {
        luajit_handle_perform64(x->engine, dsp64, ins, numins, outs, numouts,
                                sampleframes, flags, userparam);
    }
}
```

### 5. With Custom Bindings (e.g., STK)

```c
int stk_bindings(lua_State* L) {
    // Register STK bindings
    register_stk_bindings(L);
    return 0;  // Return 0 on success
}

void lstk_init_lua(t_lstk *x) {
    x->engine = luajit_new(stk_bindings, "luajit.stk~");
    if (x->engine) {
        lstk_run_file(x);
    }
}
```

### 6. Optional: External-Specific List Processing

```c
void my_list_extra(void* context, long argc, t_atom* argv) {
    t_mlj* x = (t_mlj*)context;
    // Sync legacy parameters
    if (x->engine && x->engine->num_params > 0) {
        x->param1 = x->engine->params[0];
    }
}

void mlj_list(t_mlj* x, t_symbol* s, long argc, t_atom* argv) {
    if (x->engine) {
        luajit_handle_list(x->engine, x, s, argc, argv, my_list_extra, "luajit~");
    }
}
```

## Accessing Engine Fields

All engine state is accessible through the `luajit_engine` pointer:

```c
x->engine->L              // Lua state
x->engine->filename       // Current file name
x->engine->funcname       // Current function name
x->engine->func_ref       // Cached function reference
x->engine->params[]       // Parameter array
x->engine->num_params     // Number of parameters
x->engine->prev_sample    // Previous output sample
x->engine->samplerate     // Current sample rate
x->engine->vectorsize     // Current vector size
x->engine->in_error_state // Error flag
```

## Benefits

- **No code duplication** - Identical handlers shared between externals
- **Type-safe** - Concrete `luajit_engine` struct, no pointer casting
- **Header-only** - No compilation/linking complexity
- **Extensible** - Callbacks for external-specific behavior
- **Clean API** - All engine state in one struct

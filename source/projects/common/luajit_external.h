/**
    @file luajit_external.h
    @brief Single-header library for luajit-max externals

    This header provides common functionality shared between luajit~ and luajit.stk~
    including message handlers, DSP callbacks, and initialization patterns.
*/

#ifndef LUAJIT_EXTERNAL_H
#define LUAJIT_EXTERNAL_H

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <libgen.h>
#include <unistd.h>
#include "ext.h"
#include "ext_obex.h"
#include "ext_strings.h"
#include "z_dsp.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "luajit_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of dynamic parameters
#define LUAJIT_MAX_PARAMS 32

//------------------------------------------------------------------------------
// Engine State Structure
//------------------------------------------------------------------------------

/**
 * Core Lua engine state for luajit externals.
 * External-specific structs should embed this struct and access it
 * through a member variable (e.g., x->engine).
 */
typedef struct {
    lua_State *L;               // Lua state
    t_symbol* filename;         // Lua file name
    t_symbol* funcname;         // Current DSP function name
    int func_ref;               // Cached function reference
    double params[LUAJIT_MAX_PARAMS]; // Dynamic parameter array
    int num_params;             // Number of active parameters
    double prev_sample;         // Previous output sample (for feedback)
    double samplerate;          // Current sample rate
    long vectorsize;            // Current vector size
    char in_error_state;        // Error flag (1 = in error, 0 = ok)
} luajit_engine;

//------------------------------------------------------------------------------
// Core Lua Engine Functions (from lua_engine.c)
//------------------------------------------------------------------------------

/**
 * Initialize Lua state with RT-safe GC settings
 */
static inline lua_State* lua_engine_init(void) {
    lua_State* L = luaL_newstate();
    if (!L) {
        error("lua_engine: failed to create Lua state");
        return NULL;
    }

    luaL_openlibs(L);

    // Configure GC for real-time use
    lua_gc(L, LUA_GCSTOP, 0);
    lua_gc(L, LUA_GCRESTART, 0);
    lua_gc(L, LUA_GCSETPAUSE, 200);    // wait 2x memory before next GC
    lua_gc(L, LUA_GCSETSTEPMUL, 100);  // slower collection

    return L;
}

/**
 * Cleanup Lua state
 */
static inline void lua_engine_free(lua_State* L) {
    if (L) {
        lua_close(L);
    }
}

/**
 * Run Lua code from string
 */
static inline int lua_engine_run_string(lua_State* L, const char* code) {
    int err = luaL_dostring(L, code);
    if (err) {
        error("lua_engine: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return -1;
    }
    return 0;
}

/**
 * Run Lua file from path
 */
static inline int lua_engine_run_file(lua_State* L, const char* path) {
    int err = luaL_dofile(L, path);
    if (err) {
        error("lua_engine: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return -1;
    }
    return 0;
}

/**
 * Cache a Lua function reference for fast lookup
 * Returns LUA_NOREF on failure
 */
static inline int lua_engine_cache_function(lua_State* L, const char* func_name) {
    lua_getglobal(L, func_name);

    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        error("lua_engine: '%s' is not a function", func_name);
        return LUA_NOREF;
    }

    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return ref;
}

/**
 * Release a cached function reference
 */
static inline void lua_engine_release_function(lua_State* L, int func_ref) {
    if (func_ref != LUA_NOREF && func_ref != LUA_REFNIL) {
        luaL_unref(L, LUA_REGISTRYINDEX, func_ref);
    }
}

/**
 * Validate that a function reference is still valid
 */
static inline int lua_engine_validate_function(lua_State* L, int func_ref) {
    if (func_ref == LUA_REFNIL || func_ref == LUA_NOREF) {
        return 0;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);
    int is_func = lua_isfunction(L, -1);
    lua_pop(L, 1);

    return is_func;
}

/**
 * Set sample rate global in Lua
 */
static inline void lua_engine_set_samplerate(lua_State* L, double samplerate) {
    lua_pushnumber(L, samplerate);
    lua_setglobal(L, "SAMPLE_RATE");
}

/**
 * Configure GC for real-time use (legacy - now done in lua_engine_init)
 */
static inline void lua_engine_configure_gc(lua_State* L) {
    lua_gc(L, LUA_GCSTOP, 0);
    lua_gc(L, LUA_GCRESTART, 0);
    lua_gc(L, LUA_GCSETPAUSE, 200);
    lua_gc(L, LUA_GCSETSTEPMUL, 100);
}

/**
 * Set a named parameter in the global PARAMS table
 */
static inline void lua_engine_set_named_param(lua_State* L, const char* name, double value) {
    lua_getglobal(L, "PARAMS");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "PARAMS");
    }

    lua_pushstring(L, name);
    lua_pushnumber(L, value);
    lua_settable(L, -3);

    lua_pop(L, 1);
}

/**
 * Clear all named parameters
 */
static inline void lua_engine_clear_named_params(lua_State* L) {
    lua_newtable(L);
    lua_setglobal(L, "PARAMS");
}

/**
 * Helper: Validate and clamp DSP result
 */
static inline float validate_and_clamp_result(lua_State* L, char* error_flag) {
    if (!lua_isnumber(L, -1)) {
        error("lua_engine: function must return a number");
        lua_pop(L, 1);
        *error_flag = 1;
        return 0.0f;
    }

    float result = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    if (isnan(result) || isinf(result)) {
        error("lua_engine: function returned invalid value (NaN or Inf)");
        *error_flag = 1;
        return 0.0f;
    }

    if (result > 1.0f) result = 1.0f;
    if (result < -1.0f) result = -1.0f;

    return result;
}

/**
 * Execute cached Lua DSP function with 4 parameters
 * Returns 0.0f on error and sets error_flag
 */
static inline float lua_engine_call_dsp4(lua_State* L, int func_ref, char* error_flag,
                                         float audio_in, float audio_prev,
                                         float n_samples, float param1)
{
    if (*error_flag) {
        return 0.0f;
    }

    if (func_ref == LUA_REFNIL || func_ref == LUA_NOREF) {
        *error_flag = 1;
        error("lua_engine: no Lua function loaded");
        return 0.0f;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);

    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        *error_flag = 1;
        error("lua_engine: cached reference is not a function");
        return 0.0f;
    }

    lua_pushnumber(L, audio_in);
    lua_pushnumber(L, audio_prev);
    lua_pushnumber(L, n_samples);
    lua_pushnumber(L, param1);

    int status = lua_pcall(L, 4, 1, 0);

    if (status != LUA_OK) {
        const char* err_msg = lua_tostring(L, -1);
        error("lua_engine: Lua error: %s", err_msg);
        lua_pop(L, 1);
        *error_flag = 1;
        return 0.0f;
    }

    return validate_and_clamp_result(L, error_flag);
}

/**
 * Execute cached Lua DSP function with 7 parameters (for luajit.stk~)
 * Returns 0.0f on error and sets error_flag
 */
static inline float lua_engine_call_dsp7(lua_State* L, int func_ref, char* error_flag,
                                         float audio_in, float audio_prev, float n_samples,
                                         float param0, float param1, float param2, float param3)
{
    if (*error_flag) {
        return 0.0f;
    }

    if (func_ref == LUA_REFNIL || func_ref == LUA_NOREF) {
        *error_flag = 1;
        error("lua_engine: no Lua function loaded");
        return 0.0f;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);

    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        *error_flag = 1;
        error("lua_engine: cached reference is not a function");
        return 0.0f;
    }

    lua_pushnumber(L, audio_in);
    lua_pushnumber(L, audio_prev);
    lua_pushnumber(L, n_samples);
    lua_pushnumber(L, param0);
    lua_pushnumber(L, param1);
    lua_pushnumber(L, param2);
    lua_pushnumber(L, param3);

    int status = lua_pcall(L, 7, 1, 0);

    if (status != LUA_OK) {
        const char* err_msg = lua_tostring(L, -1);
        error("lua_engine: Lua error: %s", err_msg);
        lua_pop(L, 1);
        *error_flag = 1;
        return 0.0f;
    }

    return validate_and_clamp_result(L, error_flag);
}

/**
 * Execute cached Lua DSP function with dynamic parameter array
 * Returns 0.0f on error and sets error_flag
 */
static inline float lua_engine_call_dsp_dynamic(lua_State* L, int func_ref, char* error_flag,
                                                float audio_in, float audio_prev, float n_samples,
                                                float* params, int num_params)
{
    if (*error_flag) {
        return 0.0f;
    }

    if (func_ref == LUA_REFNIL || func_ref == LUA_NOREF) {
        *error_flag = 1;
        error("lua_engine: no Lua function loaded");
        return 0.0f;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);

    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        *error_flag = 1;
        error("lua_engine: cached reference is not a function");
        return 0.0f;
    }

    lua_pushnumber(L, audio_in);
    lua_pushnumber(L, audio_prev);
    lua_pushnumber(L, n_samples);

    for (int i = 0; i < num_params; i++) {
        lua_pushnumber(L, params[i]);
    }

    int total_args = 3 + num_params;
    int status = lua_pcall(L, total_args, 1, 0);

    if (status != LUA_OK) {
        const char* err_msg = lua_tostring(L, -1);
        error("lua_engine: Lua error: %s", err_msg);
        lua_pop(L, 1);
        *error_flag = 1;
        return 0.0f;
    }

    return validate_and_clamp_result(L, error_flag);
}

//------------------------------------------------------------------------------
// Max Helpers (from max_helpers.c)
//------------------------------------------------------------------------------

/**
 * Get absolute path to the external itself
 * Returns t_string that must be freed with object_free()
 */
static inline t_string* mxh_get_external_path(t_class* c, const char* subpath) {
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

/**
 * Get path to package directory (two levels up from external)
 * Returns t_string that must be freed with object_free()
 */
static inline t_string* mxh_get_package_path(t_class* c, const char* subpath) {
    t_string* result;
    t_string* external_path = mxh_get_external_path(c, NULL);

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

    object_free(external_path);

    return result;
}

/**
 * Resolve and load a Lua file, checking both absolute path and package examples folder
 * Returns 0 on success, -1 on failure
 */
static inline int mxh_load_lua_file(t_class* c, t_symbol* filename,
                                    int (*load_func)(void*, const char*),
                                    void* context)
{
    if (filename == gensym("")) {
        return 0;
    }

    char norm_path[MAX_PATH_CHARS];
    path_nameconform(filename->s_name, norm_path, PATH_STYLE_MAX, PATH_TYPE_BOOT);

    // Try absolute path first
    if (access(norm_path, F_OK) == 0) {
        post("loading: %s", norm_path);
        return load_func(context, norm_path);
    }

    // Try in the examples folder
    t_string* path = mxh_get_package_path(c, "/examples/");
    if (!path) {
        error("max_helpers: failed to get package path");
        return -1;
    }

    string_append(path, filename->s_name);
    const char* lua_file = string_getptr(path);
    post("loading: %s", lua_file);

    int result = load_func(context, lua_file);
    object_free(path);

    return result;
}

//------------------------------------------------------------------------------
// Message Handler Callbacks
//------------------------------------------------------------------------------

/**
 * Callback type for running/reloading Lua file.
 * @param context - External-specific context (e.g., t_mlj* or t_lstk*)
 */
typedef void (*luajit_run_file_func)(void* context);

/**
 * Callback type for external-specific list processing.
 * Called after standard list parameter processing.
 * @param context - External-specific context
 * @param argc - Number of atoms
 * @param argv - Atom array
 */
typedef void (*luajit_list_extra_func)(void* context, long argc, t_atom* argv);

//------------------------------------------------------------------------------
// Message Handlers (Inline Implementations)
//------------------------------------------------------------------------------

/**
 * Standard bang handler: reload Lua file and re-cache function.
 *
 * @param engine - Lua engine instance
 * @param context - External-specific context for run_file callback
 * @param run_file - Function to run/reload Lua file
 * @param error_prefix - Prefix for error messages (e.g., "luajit~")
 */
static inline void luajit_handle_bang(luajit_engine* engine,
                                      void* context,
                                      luajit_run_file_func run_file,
                                      const char* error_prefix)
{
    // Save old reference for later cleanup
    int old_ref = engine->func_ref;

    // Temporarily set to NOREF to prevent audio thread from using stale reference
    engine->func_ref = LUA_NOREF;
    engine->in_error_state = 1;  // Silence audio during reload

    // Reload file
    run_file(context);

    // Re-cache the current function
    int new_ref = lua_engine_cache_function(engine->L, engine->funcname->s_name);
    if (new_ref == LUA_NOREF) {
        engine->in_error_state = 1;
        error("%s: function '%s' not found after reload",
              error_prefix, engine->funcname->s_name);
        // Keep func_ref as NOREF, stay in error state
    } else {
        engine->func_ref = new_ref;  // Swap to new reference
        engine->in_error_state = 0;
        post("reloaded and cached function: %s", engine->funcname->s_name);
    }

    // Release old reference after swap
    lua_engine_release_function(engine->L, old_ref);
}

/**
 * Standard list handler: parse positional or named parameters.
 *
 * Handles two formats:
 * 1. Positional numeric: "10 0.1 4" -> params[0]=10, params[1]=0.1, params[2]=4
 * 2. Named pairs: "delay 2 feedback 0.5" -> PARAMS.delay=2, PARAMS.feedback=0.5
 *
 * @param engine - Lua engine instance
 * @param context - External-specific context for extra callback
 * @param s - Message selector (unused)
 * @param argc - Number of atoms
 * @param argv - Atom array
 * @param extra - Optional callback for external-specific processing
 * @param error_prefix - Prefix for error messages
 */
static inline void luajit_handle_list(luajit_engine* engine,
                                      void* context,
                                      t_symbol* s,
                                      long argc,
                                      t_atom* argv,
                                      luajit_list_extra_func extra,
                                      const char* error_prefix)
{
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
        engine->num_params = (argc > LUAJIT_MAX_PARAMS) ? LUAJIT_MAX_PARAMS : argc;
        for (long i = 0; i < engine->num_params; i++) {
            engine->params[i] = atom_getfloat(argv + i);
        }
        post("set %d params: positional", engine->num_params);

        // Call external-specific extra processing if provided
        if (extra) {
            extra(context, argc, argv);
        }
    } else {
        // Parse as named parameters: "delay 2 feedback 0.5 dry_wet 0.5"
        if (argc % 2 != 0) {
            error("%s: named parameters must be in pairs: name value", error_prefix);
            return;
        }

        // Clear existing named parameters
        lua_engine_clear_named_params(engine->L);

        for (long i = 0; i < argc; i += 2) {
            if (atom_gettype(argv + i) != A_SYM) {
                error("%s: parameter names must be symbols", error_prefix);
                return;
            }

            t_symbol* param_name = atom_getsym(argv + i);
            const char* name = param_name->s_name;
            double value = atom_getfloat(argv + i + 1);

            // Set named parameter in Lua PARAMS table
            lua_engine_set_named_param(engine->L, name, value);
        }
        post("set %ld named params", argc / 2);
    }
}

/**
 * Standard anything handler: switch function or set named params.
 *
 * Handles three cases:
 * 1. Function switch with params: "funcname param1 val1 param2 val2"
 * 2. Named params only: "param1 val1 param2 val2"
 * 3. Function switch only: "funcname"
 *
 * @param engine - Lua engine instance
 * @param context - External-specific context for extra callback
 * @param s - Message selector (potential function name or param name)
 * @param argc - Number of atoms
 * @param argv - Atom array
 * @param extra - Optional callback for external-specific processing
 * @param error_prefix - Prefix for error messages
 */
static inline void luajit_handle_anything(luajit_engine* engine,
                                          void* context,
                                          t_symbol* s,
                                          long argc,
                                          t_atom* argv,
                                          luajit_list_extra_func extra,
                                          const char* error_prefix)
{
    if (s != gensym("")) {
        // Check if this is a named parameter message with arguments
        if (argc > 0) {
            // Silence audio before touching Lua state (thread safety)
            char was_in_error = engine->in_error_state;
            engine->in_error_state = 1;

            // Check if 's' is a valid function name
            // Try to cache it - if it succeeds, this is a combined function+params message
            int test_ref = lua_engine_cache_function(engine->L, s->s_name);

            if (test_ref != LUA_NOREF) {
                // Valid function - this is combined syntax: "funcname param1 val1 param2 val2"
                // Swap to new function reference (cache first, then release old)
                int old_ref = engine->func_ref;
                engine->func_ref = test_ref;  // Atomic swap
                engine->funcname = s;
                lua_engine_release_function(engine->L, old_ref);  // Release after swap
                post("funcname: %s", s->s_name);

                // Now process the parameters
                luajit_handle_list(engine, context, s, argc, argv, extra, error_prefix);

                // Re-enable audio
                engine->in_error_state = 0;
            } else {
                // Not a function - treat as named parameters only
                engine->in_error_state = was_in_error;  // Restore state
                luajit_handle_list(engine, context, s, argc, argv, extra, error_prefix);
            }
        } else {
            // No arguments - just switch function
            // Silence audio before touching Lua state (thread safety)
            engine->in_error_state = 1;

            // Get the new function and cache its reference first
            int new_ref = lua_engine_cache_function(engine->L, s->s_name);
            if (new_ref == LUA_NOREF) {
                error("%s: '%s' is not a function", error_prefix, s->s_name);
                // Stay in error state
            } else {
                // Swap to new function reference (atomic swap, then release old)
                int old_ref = engine->func_ref;
                engine->func_ref = new_ref;  // Atomic swap
                engine->funcname = s;
                lua_engine_release_function(engine->L, old_ref);  // Release after swap
                post("funcname: %s", s->s_name);

                // Re-enable audio
                engine->in_error_state = 0;
            }
        }
    }
}

/**
 * Standard float handler: set first parameter.
 *
 * @param engine - Lua engine instance
 * @param f - Float value
 */
static inline void luajit_handle_float(luajit_engine* engine, double f)
{
    if (engine->num_params > 0) {
        engine->params[0] = f;
    } else {
        engine->num_params = 1;
        engine->params[0] = f;
    }
}

//------------------------------------------------------------------------------
// DSP Callbacks (Inline Implementations)
//------------------------------------------------------------------------------

/**
 * Standard dsp64 handler: called when DSP is compiled.
 *
 * @param engine - Lua engine instance
 * @param context - External-specific context (for object_method)
 * @param dsp64 - DSP object
 * @param count - Inlet connection count
 * @param samplerate - Current sample rate
 * @param maxvectorsize - Maximum vector size
 * @param flags - DSP flags
 * @param perform_func - perform64 function to register
 */
static inline void luajit_handle_dsp64(luajit_engine* engine,
                                       void* context,
                                       t_object *dsp64,
                                       short *count,
                                       double samplerate,
                                       long maxvectorsize,
                                       long flags,
                                       void* perform_func)
{
    post("sample rate: %f", samplerate);
    post("maxvectorsize: %d", maxvectorsize);

    // Store sample rate and vector size
    engine->samplerate = samplerate;
    engine->vectorsize = maxvectorsize;

    // Update Lua global
    lua_engine_set_samplerate(engine->L, samplerate);

    object_method(dsp64, gensym("dsp_add64"), context, perform_func, 0, NULL);
}

/**
 * Standard perform64 handler: audio processing callback.
 *
 * @param engine - Lua engine instance
 * @param dsp64 - DSP object
 * @param ins - Input audio buffers
 * @param numins - Number of inputs
 * @param outs - Output audio buffers
 * @param numouts - Number of outputs
 * @param sampleframes - Number of samples to process
 * @param flags - DSP flags
 * @param userparam - User parameter (unused)
 */
static inline void luajit_handle_perform64(luajit_engine* engine,
                                           t_object *dsp64,
                                           double **ins,
                                           long numins,
                                           double **outs,
                                           long numouts,
                                           long sampleframes,
                                           long flags,
                                           void *userparam)
{
    t_double *inL = ins[0];     // Input from first inlet
    t_double *outL = outs[0];   // Output to first outlet
    int n = sampleframes;
    double prev = engine->prev_sample;

    // If in error state, output silence
    if (engine->in_error_state) {
        while (n--) {
            *outL++ = 0.0;
        }
        return;
    }

    // Convert params to float array for lua_engine
    float float_params[LUAJIT_MAX_PARAMS];
    for (int i = 0; i < engine->num_params; i++) {
        float_params[i] = (float)engine->params[i];
    }

    while (n--) {
        // Use dynamic parameter version
        prev = lua_engine_call_dsp_dynamic(engine->L, engine->func_ref, &engine->in_error_state,
                                           *inL++, prev, n, float_params, engine->num_params);
        *outL++ = prev;
    }

    engine->prev_sample = prev;
}

//------------------------------------------------------------------------------
// Initialization Helpers
//------------------------------------------------------------------------------

/**
 * Callback type for custom Lua bindings (e.g., STK registration).
 * @param L - Lua state
 * @return 0 on success, non-zero on error
 */
typedef int (*luajit_custom_bindings_func)(lua_State* L);

/**
 * Allocate and initialize a new Lua engine.
 *
 * @param custom_bindings - Optional callback for custom bindings (can be NULL)
 * @param error_prefix - Prefix for error messages
 * @return Allocated engine instance, or NULL on failure
 */
static inline luajit_engine* luajit_new(luajit_custom_bindings_func custom_bindings,
                                        const char* error_prefix)
{
    // Allocate engine
    luajit_engine* engine = (luajit_engine*)malloc(sizeof(luajit_engine));
    if (!engine) {
        error("%s: failed to allocate engine", error_prefix);
        return NULL;
    }

    // Initialize all fields to zero
    memset(engine, 0, sizeof(luajit_engine));

    // Create Lua state with RT-safe configuration
    engine->L = lua_engine_init();
    if (!engine->L) {
        error("%s: failed to initialize Lua engine", error_prefix);
        free(engine);
        return NULL;
    }

    // Set initial sample rate (will be updated in dsp64)
    engine->samplerate = 44100.0;
    lua_engine_set_samplerate(engine->L, engine->samplerate);

    // Initialize function reference to invalid
    engine->func_ref = LUA_NOREF;
    engine->in_error_state = 0;
    engine->num_params = 0;
    engine->prev_sample = 0.0;
    engine->vectorsize = 0;

    // Initialize the shared Max API module for Lua
    luajit_api_init(engine->L);

    // Call custom bindings if provided
    if (custom_bindings) {
        if (custom_bindings(engine->L) != 0) {
            error("%s: custom bindings initialization failed", error_prefix);
            lua_engine_free(engine->L);
            free(engine);
            return NULL;
        }
    }

    return engine;
}

/**
 * Free Lua engine and deallocate.
 *
 * @param engine - Lua engine instance to free (can be NULL)
 */
static inline void luajit_free(luajit_engine* engine)
{
    if (engine) {
        if (engine->L) {
            // Release cached function reference
            if (engine->func_ref != LUA_NOREF) {
                lua_engine_release_function(engine->L, engine->func_ref);
                engine->func_ref = LUA_NOREF;
            }

            // Free Lua state
            lua_engine_free(engine->L);
            engine->L = NULL;
        }

        // Free the engine itself
        free(engine);
    }
}

#ifdef __cplusplus
}
#endif

#endif // LUAJIT_EXTERNAL_H

// lua_engine.c
// Implementation of shared Lua engine management

#include "lua_engine.h"
#include "ext.h"  // For Max error/post functions
#include <math.h>

// ----------------------------------------------------------------------------
// Core Lua Engine Functions

lua_State* lua_engine_init(void) {
    lua_State* L = luaL_newstate();
    if (!L) {
        error("lua_engine: failed to create Lua state");
        return NULL;
    }

    luaL_openlibs(L);
    lua_engine_configure_gc(L);

    return L;
}

void lua_engine_free(lua_State* L) {
    if (L) {
        lua_close(L);
    }
}

int lua_engine_run_string(lua_State* L, const char* code) {
    int err = luaL_dostring(L, code);
    if (err) {
        error("lua_engine: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return -1;
    }
    return 0;
}

int lua_engine_run_file(lua_State* L, const char* path) {
    int err = luaL_dofile(L, path);
    if (err) {
        error("lua_engine: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return -1;
    }
    return 0;
}

// ----------------------------------------------------------------------------
// Function Reference Management

int lua_engine_cache_function(lua_State* L, const char* func_name) {
    lua_getglobal(L, func_name);

    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        error("lua_engine: '%s' is not a function", func_name);
        return LUA_NOREF;
    }

    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return ref;
}

void lua_engine_release_function(lua_State* L, int func_ref) {
    if (func_ref != LUA_NOREF && func_ref != LUA_REFNIL) {
        luaL_unref(L, LUA_REGISTRYINDEX, func_ref);
    }
}

int lua_engine_validate_function(lua_State* L, int func_ref) {
    if (func_ref == LUA_REFNIL || func_ref == LUA_NOREF) {
        return 0;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);
    int is_func = lua_isfunction(L, -1);
    lua_pop(L, 1);

    return is_func;
}

// ----------------------------------------------------------------------------
// RT-Safe DSP Execution

static float validate_and_clamp_result(lua_State* L, char* error_flag) {
    // Verify return value is a number
    if (!lua_isnumber(L, -1)) {
        error("lua_engine: function must return a number");
        lua_pop(L, 1);
        *error_flag = 1;
        return 0.0f;
    }

    float result = (float)lua_tonumber(L, -1);
    lua_pop(L, 1);

    // Check for invalid values (NaN, Inf)
    if (isnan(result) || isinf(result)) {
        error("lua_engine: function returned invalid value (NaN or Inf)");
        *error_flag = 1;
        return 0.0f;
    }

    // Clamp to safe audio range
    if (result > 1.0f) result = 1.0f;
    if (result < -1.0f) result = -1.0f;

    return result;
}

float lua_engine_call_dsp4(lua_State* L, int func_ref, char* error_flag,
                           float audio_in, float audio_prev,
                           float n_samples, float param1)
{
    // If already in error state, return silence
    if (*error_flag) {
        return 0.0f;
    }

    // Validate function reference
    if (func_ref == LUA_REFNIL || func_ref == LUA_NOREF) {
        *error_flag = 1;
        error("lua_engine: no Lua function loaded");
        return 0.0f;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);

    // Verify it's still a function
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        *error_flag = 1;
        error("lua_engine: cached reference is not a function");
        return 0.0f;
    }

    // Push arguments
    lua_pushnumber(L, audio_in);
    lua_pushnumber(L, audio_prev);
    lua_pushnumber(L, n_samples);
    lua_pushnumber(L, param1);

    // Call with 4 arguments, returning 1 result (protected call)
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

float lua_engine_call_dsp7(lua_State* L, int func_ref, char* error_flag,
                           float audio_in, float audio_prev, float n_samples,
                           float param0, float param1, float param2, float param3)
{
    // If already in error state, return silence
    if (*error_flag) {
        return 0.0f;
    }

    // Validate function reference
    if (func_ref == LUA_REFNIL || func_ref == LUA_NOREF) {
        *error_flag = 1;
        error("lua_engine: no Lua function loaded");
        return 0.0f;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);

    // Verify it's still a function
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        *error_flag = 1;
        error("lua_engine: cached reference is not a function");
        return 0.0f;
    }

    // Push arguments
    lua_pushnumber(L, audio_in);
    lua_pushnumber(L, audio_prev);
    lua_pushnumber(L, n_samples);
    lua_pushnumber(L, param0);
    lua_pushnumber(L, param1);
    lua_pushnumber(L, param2);
    lua_pushnumber(L, param3);

    // Call with 7 arguments, returning 1 result (protected call)
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

float lua_engine_call_dsp_dynamic(lua_State* L, int func_ref, char* error_flag,
                                  float audio_in, float audio_prev, float n_samples,
                                  float* params, int num_params)
{
    // If already in error state, return silence
    if (*error_flag) {
        return 0.0f;
    }

    // Validate function reference
    if (func_ref == LUA_REFNIL || func_ref == LUA_NOREF) {
        *error_flag = 1;
        error("lua_engine: no Lua function loaded");
        return 0.0f;
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);

    // Verify it's still a function
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        *error_flag = 1;
        error("lua_engine: cached reference is not a function");
        return 0.0f;
    }

    // Push fixed arguments
    lua_pushnumber(L, audio_in);
    lua_pushnumber(L, audio_prev);
    lua_pushnumber(L, n_samples);

    // Push dynamic parameters
    for (int i = 0; i < num_params; i++) {
        lua_pushnumber(L, params[i]);
    }

    // Call with 3 + num_params arguments, returning 1 result
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

// ----------------------------------------------------------------------------
// Configuration

void lua_engine_set_samplerate(lua_State* L, double samplerate) {
    lua_pushnumber(L, samplerate);
    lua_setglobal(L, "SAMPLE_RATE");
}

void lua_engine_configure_gc(lua_State* L) {
    // Stop the GC initially - we'll run it manually
    lua_gc(L, LUA_GCSTOP, 0);

    // Restart GC in incremental mode with conservative settings
    lua_gc(L, LUA_GCRESTART, 0);
    // Set incremental GC: smaller steps, longer pauses between steps
    lua_gc(L, LUA_GCSETPAUSE, 200);    // wait 2x memory before next GC
    lua_gc(L, LUA_GCSETSTEPMUL, 100);  // slower collection
}

// ----------------------------------------------------------------------------
// Named Parameters

void lua_engine_set_named_param(lua_State* L, const char* name, double value) {
    // Get or create global PARAMS table
    lua_getglobal(L, "PARAMS");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);  // pop the non-table value
        lua_newtable(L);
        lua_pushvalue(L, -1);  // duplicate table
        lua_setglobal(L, "PARAMS");  // set as global
    }

    // Set PARAMS[name] = value
    lua_pushstring(L, name);
    lua_pushnumber(L, value);
    lua_settable(L, -3);

    lua_pop(L, 1);  // pop PARAMS table
}

void lua_engine_clear_named_params(lua_State* L) {
    // Create a new empty table and set it as PARAMS
    lua_newtable(L);
    lua_setglobal(L, "PARAMS");
}

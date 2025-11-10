// lua_engine.h
// Shared Lua engine management for luajit-max externals
// Handles Lua state, file loading, and DSP function execution

#ifndef LUA_ENGINE_H
#define LUA_ENGINE_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Core Lua Engine Functions

// Initialize Lua state with RT-safe GC settings
lua_State* lua_engine_init(void);

// Cleanup Lua state
void lua_engine_free(lua_State* L);

// Run Lua code from string
int lua_engine_run_string(lua_State* L, const char* code);

// Run Lua file from path
int lua_engine_run_file(lua_State* L, const char* path);

// ----------------------------------------------------------------------------
// Function Reference Management

// Cache a Lua function reference for fast lookup
// Returns LUA_NOREF on failure
int lua_engine_cache_function(lua_State* L, const char* func_name);

// Release a cached function reference
void lua_engine_release_function(lua_State* L, int func_ref);

// Validate that a function reference is still valid
int lua_engine_validate_function(lua_State* L, int func_ref);

// ----------------------------------------------------------------------------
// RT-Safe DSP Execution

// Execute cached Lua DSP function with 4 parameters
// Returns 0.0f on error and sets error_flag
float lua_engine_call_dsp4(lua_State* L, int func_ref, char* error_flag,
                           float audio_in, float audio_prev,
                           float n_samples, float param1);

// Execute cached Lua DSP function with 7 parameters (for luajit.stk~)
// Returns 0.0f on error and sets error_flag
float lua_engine_call_dsp7(lua_State* L, int func_ref, char* error_flag,
                           float audio_in, float audio_prev, float n_samples,
                           float param0, float param1, float param2, float param3);

// Execute cached Lua DSP function with dynamic parameter array
// Returns 0.0f on error and sets error_flag
// params: array of parameters
// num_params: number of parameters in the array
float lua_engine_call_dsp_dynamic(lua_State* L, int func_ref, char* error_flag,
                                  float audio_in, float audio_prev, float n_samples,
                                  float* params, int num_params);

// ----------------------------------------------------------------------------
// Configuration

// Set sample rate global in Lua
void lua_engine_set_samplerate(lua_State* L, double samplerate);

// Configure GC for real-time use
void lua_engine_configure_gc(lua_State* L);

// ----------------------------------------------------------------------------
// Named Parameters

// Set a named parameter in the global PARAMS table
void lua_engine_set_named_param(lua_State* L, const char* name, double value);

// Clear all named parameters
void lua_engine_clear_named_params(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif // LUA_ENGINE_H

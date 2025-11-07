// luajit_api.h
// Main header for luajit-max API module
// This wraps a subset of the Max C API for use from Lua scripts

#ifndef LUAJIT_API_H
#define LUAJIT_API_H

// Include common infrastructure
#include "api_common.h"

// Include API wrappers
#include "api_symbol.h"
#include "api_atom.h"
#include "api_clock.h"
#include "api_outlet.h"
#include "api_table.h"
#include "api_atomarray.h"
#include "api_buffer.h"
#include "api_dictionary.h"
#include "api_object.h"
#include "api_patcher.h"
#include "api_inlet.h"
#include "api_box.h"
#include "api_patchline.h"

// Forward declarations for future API modules

// ----------------------------------------------------------------------------
// Basic Max console functions

static int api_post(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    post("%s", msg);
    return 0;
}

static int api_error(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    error("%s", msg);
    return 0;
}

// ----------------------------------------------------------------------------
// API module initialization
// Call this function during external initialization to register the API module

static void luajit_api_init(lua_State* L) {
    // Create api module table
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "api");

    // Register basic console functions
    lua_pushcfunction(L, api_post);
    lua_setfield(L, -2, "post");

    lua_pushcfunction(L, api_error);
    lua_setfield(L, -2, "error");

    lua_pop(L, 1);  // Pop api table

    // Register types (these will add to the api table)
    register_symbol_type(L);
    register_atom_type(L);
    register_clock_type(L);
    register_outlet_type(L);
    register_table_type(L);
    register_atomarray_type(L);
    register_buffer_type(L);
    register_dictionary_type(L);
    register_object_type(L);
    register_patcher_type(L);
    register_inlet_type(L);
    register_box_type(L);
    register_patchline_type(L);

    // Future type registrations (LOW priority):
}

// ----------------------------------------------------------------------------
// Example usage in externals:
//
// In your external's init function:
//   lua_State* L = luaL_newstate();
//   luaL_openlibs(L);
//   luajit_api_init(L);  // Initialize the api module
//
// In Lua scripts:
//   local sym = api.gensym("foo")
//   local atom = api.Atom(42)
//   api.post("Hello from Lua!")
//

#endif // LUAJIT_API_H

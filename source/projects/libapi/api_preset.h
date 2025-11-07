// api_preset.h
// Preset and Pattr wrapper for luajit-max API
// Provides state management integration with Max's preset system

#ifndef LUAJIT_API_PRESET_H
#define LUAJIT_API_PRESET_H

#include "api_common.h"

// ----------------------------------------------------------------------------
// Preset module functions (module-level only, no class)

// api.preset_store(format_string)
static int api_preset_store(lua_State* L) {
    const char* fmt = luaL_checkstring(L, 1);
    preset_store((char*)fmt);
    return 0;
}

// api.preset_set(object_ptr, value)
static int api_preset_set(lua_State* L) {
    lua_Number obj_ptr = luaL_checknumber(L, 1);
    lua_Number val = luaL_checknumber(L, 2);

    t_object* obj = (t_object*)(intptr_t)obj_ptr;
    preset_set(obj, (t_atom_long)val);

    return 0;
}

// api.preset_int(object_ptr, value)
static int api_preset_int(lua_State* L) {
    lua_Number obj_ptr = luaL_checknumber(L, 1);
    lua_Number n = luaL_checknumber(L, 2);

    t_object* obj = (t_object*)(intptr_t)obj_ptr;
    preset_int(obj, (t_atom_long)n);

    return 0;
}

// api.preset_get_data_symbol() -> "preset_data"
static int api_preset_get_data_symbol(lua_State* L) {
    lua_pushstring(L, "preset_data");
    return 1;
}

// Register preset functions
static void register_preset_type(lua_State* L) {
    // Get api module
    lua_getglobal(L, "api");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "api");
    }

    // Register preset functions in api module
    lua_pushcfunction(L, api_preset_store);
    lua_setfield(L, -2, "preset_store");

    lua_pushcfunction(L, api_preset_set);
    lua_setfield(L, -2, "preset_set");

    lua_pushcfunction(L, api_preset_int);
    lua_setfield(L, -2, "preset_int");

    lua_pushcfunction(L, api_preset_get_data_symbol);
    lua_setfield(L, -2, "preset_get_data_symbol");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_PRESET_H

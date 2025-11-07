// api_symbol.h
// Symbol wrapper for luajit-max API

#ifndef LUAJIT_API_SYMBOL_H
#define LUAJIT_API_SYMBOL_H

#include "api_common.h"

// Metatable name for Symbol userdata
#define SYMBOL_MT "Max.Symbol"

// Symbol userdata structure
typedef struct {
    t_symbol* sym;
} SymbolUD;

// Create a new Symbol userdata
static int Symbol_new(lua_State* L) {
    const char* name = luaL_optstring(L, 1, "");

    SymbolUD* ud = (SymbolUD*)lua_newuserdata(L, sizeof(SymbolUD));
    ud->sym = gensym(name);

    luaL_getmetatable(L, SYMBOL_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Get symbol name
static int Symbol_name(lua_State* L) {
    SymbolUD* ud = (SymbolUD*)luaL_checkudata(L, 1, SYMBOL_MT);
    lua_pushstring(L, ud->sym->s_name);
    return 1;
}

// __tostring metamethod
static int Symbol_tostring(lua_State* L) {
    SymbolUD* ud = (SymbolUD*)luaL_checkudata(L, 1, SYMBOL_MT);
    lua_pushfstring(L, "Symbol('%s')", ud->sym->s_name);
    return 1;
}

// __eq metamethod
static int Symbol_eq(lua_State* L) {
    SymbolUD* ud1 = (SymbolUD*)luaL_checkudata(L, 1, SYMBOL_MT);

    // Compare with another Symbol
    if (lua_isuserdata(L, 2)) {
        SymbolUD* ud2 = (SymbolUD*)luaL_checkudata(L, 2, SYMBOL_MT);
        lua_pushboolean(L, ud1->sym == ud2->sym);
        return 1;
    }

    // Compare with string
    if (lua_isstring(L, 2)) {
        const char* str = lua_tostring(L, 2);
        lua_pushboolean(L, strcmp(ud1->sym->s_name, str) == 0);
        return 1;
    }

    lua_pushboolean(L, 0);
    return 1;
}

// Module-level gensym function
static int api_gensym(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);

    SymbolUD* ud = (SymbolUD*)lua_newuserdata(L, sizeof(SymbolUD));
    ud->sym = gensym(name);

    luaL_getmetatable(L, SYMBOL_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Register Symbol type
static void register_symbol_type(lua_State* L) {
    // Create metatable for Symbol
    luaL_newmetatable(L, SYMBOL_MT);

    // Set __index to itself (metatable is also method table)
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    // Register methods
    lua_pushcfunction(L, Symbol_name);
    lua_setfield(L, -2, "name");

    // Register metamethods
    lua_pushcfunction(L, Symbol_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, Symbol_eq);
    lua_setfield(L, -2, "__eq");

    lua_pop(L, 1);  // Pop metatable

    // Register constructor in api module
    lua_getglobal(L, "api");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "api");
    }

    lua_pushcfunction(L, Symbol_new);
    lua_setfield(L, -2, "Symbol");

    lua_pushcfunction(L, api_gensym);
    lua_setfield(L, -2, "gensym");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_SYMBOL_H

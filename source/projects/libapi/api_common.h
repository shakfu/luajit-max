// api_common.h
// Common infrastructure for luajit-max API wrappers

#ifndef LUAJIT_API_COMMON_H
#define LUAJIT_API_COMMON_H

// Max and MSP includes
#include "ext.h"
#include "ext_obex.h"
#include "ext_atomarray.h"
#include "ext_dictionary.h"
#include "jpatcher_api.h"
#include "ext_hashtab.h"
#include "ext_linklist.h"

// Lua includes
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// Standard C includes
#include <math.h>

// Utility macros for Lua C functions
#define LUA_CHECK_ARGS(L, n) \
    if (lua_gettop(L) != (n)) { \
        return luaL_error(L, "expected %d arguments, got %d", (n), lua_gettop(L)); \
    }

#define LUA_CHECK_MIN_ARGS(L, n) \
    if (lua_gettop(L) < (n)) { \
        return luaL_error(L, "expected at least %d arguments, got %d", (n), lua_gettop(L)); \
    }

// Conversion utilities between Lua and Max atoms
static inline void lua_pushatomvalue(lua_State* L, t_atom* atom) {
    switch (atom_gettype(atom)) {
        case A_LONG:
            lua_pushinteger(L, atom_getlong(atom));
            break;
        case A_FLOAT:
            lua_pushnumber(L, atom_getfloat(atom));
            break;
        case A_SYM:
            lua_pushstring(L, atom_getsym(atom)->s_name);
            break;
        default:
            lua_pushnil(L);
            break;
    }
}

static inline bool lua_toatom(lua_State* L, int idx, t_atom* atom) {
    int type = lua_type(L, idx);
    switch (type) {
        case LUA_TNUMBER: {
            // In LuaJIT/Lua 5.1, all numbers are doubles
            // Check if the value is a whole number to decide int vs float
            double d = lua_tonumber(L, idx);
            double intpart;
            if (modf(d, &intpart) == 0.0) {
                atom_setlong(atom, (long)d);
            } else {
                atom_setfloat(atom, d);
            }
            return true;
        }
        case LUA_TSTRING:
            atom_setsym(atom, gensym(lua_tostring(L, idx)));
            return true;
        case LUA_TBOOLEAN:
            atom_setlong(atom, lua_toboolean(L, idx));
            return true;
        default:
            return false;
    }
}

// Push atom array to Lua table
static inline void lua_pushatomarray(lua_State* L, long argc, t_atom* argv) {
    lua_createtable(L, argc, 0);
    for (long i = 0; i < argc; i++) {
        lua_pushatomvalue(L, argv + i);
        lua_rawseti(L, -2, i + 1);  // Lua tables are 1-indexed
    }
}

#endif // LUAJIT_API_COMMON_H

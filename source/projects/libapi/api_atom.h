// api_atom.h
// Atom wrapper for luajit-max API

#ifndef LUAJIT_API_ATOM_H
#define LUAJIT_API_ATOM_H

#include "api_common.h"
#include "api_symbol.h"

// Metatable name for Atom userdata
#define ATOM_MT "Max.Atom"

// Atom userdata structure
typedef struct {
    t_atom atom;
} AtomUD;

// Create a new Atom userdata
static int Atom_new(lua_State* L) {
    int nargs = lua_gettop(L);

    AtomUD* ud = (AtomUD*)lua_newuserdata(L, sizeof(AtomUD));

    if (nargs == 0) {
        // Atom() - default to long 0
        atom_setlong(&ud->atom, 0);
    } else {
        // Atom(value)
        if (!lua_toatom(L, 1, &ud->atom)) {
            return luaL_error(L, "Atom() argument must be number, string, or boolean");
        }
    }

    luaL_getmetatable(L, ATOM_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Get atom type as string
static int Atom_type(lua_State* L) {
    AtomUD* ud = (AtomUD*)luaL_checkudata(L, 1, ATOM_MT);

    switch (atom_gettype(&ud->atom)) {
        case A_LONG:
            lua_pushstring(L, "long");
            break;
        case A_FLOAT:
            lua_pushstring(L, "float");
            break;
        case A_SYM:
            lua_pushstring(L, "symbol");
            break;
        default:
            lua_pushstring(L, "unknown");
            break;
    }
    return 1;
}

// Get atom value
static int Atom_value(lua_State* L) {
    AtomUD* ud = (AtomUD*)luaL_checkudata(L, 1, ATOM_MT);
    lua_pushatomvalue(L, &ud->atom);
    return 1;
}

// Set atom value
static int Atom_setvalue(lua_State* L) {
    AtomUD* ud = (AtomUD*)luaL_checkudata(L, 1, ATOM_MT);

    if (!lua_toatom(L, 2, &ud->atom)) {
        return luaL_error(L, "value must be number, string, or boolean");
    }

    return 0;
}

// Check if atom is long
static int Atom_is_long(lua_State* L) {
    AtomUD* ud = (AtomUD*)luaL_checkudata(L, 1, ATOM_MT);
    lua_pushboolean(L, atom_gettype(&ud->atom) == A_LONG);
    return 1;
}

// Check if atom is float
static int Atom_is_float(lua_State* L) {
    AtomUD* ud = (AtomUD*)luaL_checkudata(L, 1, ATOM_MT);
    lua_pushboolean(L, atom_gettype(&ud->atom) == A_FLOAT);
    return 1;
}

// Check if atom is symbol
static int Atom_is_symbol(lua_State* L) {
    AtomUD* ud = (AtomUD*)luaL_checkudata(L, 1, ATOM_MT);
    lua_pushboolean(L, atom_gettype(&ud->atom) == A_SYM);
    return 1;
}

// Get atom as long
static int Atom_getlong(lua_State* L) {
    AtomUD* ud = (AtomUD*)luaL_checkudata(L, 1, ATOM_MT);
    lua_pushinteger(L, atom_getlong(&ud->atom));
    return 1;
}

// Get atom as float
static int Atom_getfloat(lua_State* L) {
    AtomUD* ud = (AtomUD*)luaL_checkudata(L, 1, ATOM_MT);
    lua_pushnumber(L, atom_getfloat(&ud->atom));
    return 1;
}

// Get atom as symbol
static int Atom_getsym(lua_State* L) {
    AtomUD* ud = (AtomUD*)luaL_checkudata(L, 1, ATOM_MT);
    t_symbol* sym = atom_getsym(&ud->atom);

    SymbolUD* sym_ud = (SymbolUD*)lua_newuserdata(L, sizeof(SymbolUD));
    sym_ud->sym = sym;

    luaL_getmetatable(L, SYMBOL_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// __tostring metamethod
static int Atom_tostring(lua_State* L) {
    AtomUD* ud = (AtomUD*)luaL_checkudata(L, 1, ATOM_MT);

    switch (atom_gettype(&ud->atom)) {
        case A_LONG:
            lua_pushfstring(L, "Atom(%d)", (int)atom_getlong(&ud->atom));
            break;
        case A_FLOAT:
            lua_pushfstring(L, "Atom(%f)", (double)atom_getfloat(&ud->atom));
            break;
        case A_SYM:
            lua_pushfstring(L, "Atom('%s')", atom_getsym(&ud->atom)->s_name);
            break;
        default:
            lua_pushstring(L, "Atom(<unknown>)");
            break;
    }
    return 1;
}

// Module-level parse function
static int api_parse(lua_State* L) {
    const char* parsestr = luaL_checkstring(L, 1);

    // Parse string into atoms
    t_atom* av = NULL;
    long ac = 0;
    t_max_err err = atom_setparse(&ac, &av, parsestr);

    if (err != MAX_ERR_NONE) {
        if (av) sysmem_freeptr(av);
        return luaL_error(L, "Failed to parse string");
    }

    // Return as Lua table
    lua_createtable(L, ac, 0);
    for (long i = 0; i < ac; i++) {
        AtomUD* ud = (AtomUD*)lua_newuserdata(L, sizeof(AtomUD));
        ud->atom = av[i];
        luaL_getmetatable(L, ATOM_MT);
        lua_setmetatable(L, -2);
        lua_rawseti(L, -2, i + 1);
    }

    // Free temporary atoms
    if (av) sysmem_freeptr(av);

    return 1;
}

// Module-level atom_gettext function
static int api_atom_gettext(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    // LuaJIT uses lua_objlen instead of lua_rawlen (Lua 5.1 compatible)
    long ac = (long)lua_objlen(L, 1);
    t_atom* av = (t_atom*)sysmem_newptr(ac * sizeof(t_atom));

    if (!av) {
        return luaL_error(L, "Failed to allocate memory for atoms");
    }

    // Convert Lua table to atom array
    for (long i = 0; i < ac; i++) {
        lua_rawgeti(L, 1, i + 1);
        if (lua_isuserdata(L, -1)) {
            AtomUD* ud = (AtomUD*)luaL_checkudata(L, -1, ATOM_MT);
            av[i] = ud->atom;
        } else if (!lua_toatom(L, -1, &av[i])) {
            sysmem_freeptr(av);
            return luaL_error(L, "Table element %d is not a valid atom type", (int)(i + 1));
        }
        lua_pop(L, 1);
    }

    // Convert to text
    long textsize = 0;
    char* text = NULL;
    t_max_err err = atom_gettext(ac, av, &textsize, &text, 0);

    sysmem_freeptr(av);

    if (err != MAX_ERR_NONE || text == NULL) {
        lua_pushstring(L, "");
    } else {
        lua_pushstring(L, text);
        if (text) sysmem_freeptr(text);
    }

    return 1;
}

// Register Atom type
static void register_atom_type(lua_State* L) {
    // Create metatable for Atom
    luaL_newmetatable(L, ATOM_MT);

    // Set __index to itself
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    // Register methods
    lua_pushcfunction(L, Atom_type);
    lua_setfield(L, -2, "type");

    lua_pushcfunction(L, Atom_value);
    lua_setfield(L, -2, "value");

    lua_pushcfunction(L, Atom_setvalue);
    lua_setfield(L, -2, "setvalue");

    lua_pushcfunction(L, Atom_is_long);
    lua_setfield(L, -2, "is_long");

    lua_pushcfunction(L, Atom_is_float);
    lua_setfield(L, -2, "is_float");

    lua_pushcfunction(L, Atom_is_symbol);
    lua_setfield(L, -2, "is_symbol");

    lua_pushcfunction(L, Atom_getlong);
    lua_setfield(L, -2, "getlong");

    lua_pushcfunction(L, Atom_getfloat);
    lua_setfield(L, -2, "getfloat");

    lua_pushcfunction(L, Atom_getsym);
    lua_setfield(L, -2, "getsym");

    // Register metamethods
    lua_pushcfunction(L, Atom_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_pop(L, 1);  // Pop metatable

    // Register in api module
    lua_getglobal(L, "api");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "api");
    }

    lua_pushcfunction(L, Atom_new);
    lua_setfield(L, -2, "Atom");

    lua_pushcfunction(L, api_parse);
    lua_setfield(L, -2, "parse");

    lua_pushcfunction(L, api_atom_gettext);
    lua_setfield(L, -2, "atom_gettext");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_ATOM_H

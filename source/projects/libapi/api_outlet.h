// api_outlet.h
// Outlet wrapper for luajit-max API

#ifndef LUAJIT_API_OUTLET_H
#define LUAJIT_API_OUTLET_H

#include "api_common.h"

// Metatable name for Outlet userdata
#define OUTLET_MT "Max.Outlet"

// Outlet userdata structure
typedef struct {
    void* outlet;
    bool owns_outlet;
} OutletUD;

// Outlet constructor: Outlet(owner_ptr, type_string)
static int Outlet_new(lua_State* L) {
    if (lua_gettop(L) < 2) {
        return luaL_error(L, "Outlet() requires 2 arguments: owner_ptr, type_string");
    }

    void* owner = (void*)(intptr_t)luaL_checknumber(L, 1);
    const char* type_str = luaL_optstring(L, 2, NULL);

    // Create userdata
    OutletUD* ud = (OutletUD*)lua_newuserdata(L, sizeof(OutletUD));
    ud->outlet = outlet_new(owner, type_str);
    ud->owns_outlet = true;

    // Set metatable
    luaL_getmetatable(L, OUTLET_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Outlet.bang()
static int Outlet_bang(lua_State* L) {
    OutletUD* ud = (OutletUD*)luaL_checkudata(L, 1, OUTLET_MT);

    if (ud->outlet == NULL) {
        return luaL_error(L, "Outlet is null");
    }

    outlet_bang((t_outlet*)ud->outlet);
    return 0;
}

// Outlet.int(value)
static int Outlet_int(lua_State* L) {
    OutletUD* ud = (OutletUD*)luaL_checkudata(L, 1, OUTLET_MT);

    if (ud->outlet == NULL) {
        return luaL_error(L, "Outlet is null");
    }

    t_atom_long value = (t_atom_long)luaL_checknumber(L, 2);
    outlet_int((t_outlet*)ud->outlet, value);

    return 0;
}

// Outlet.float(value)
static int Outlet_float(lua_State* L) {
    OutletUD* ud = (OutletUD*)luaL_checkudata(L, 1, OUTLET_MT);

    if (ud->outlet == NULL) {
        return luaL_error(L, "Outlet is null");
    }

    double value = luaL_checknumber(L, 2);
    outlet_float((t_outlet*)ud->outlet, value);

    return 0;
}

// Outlet.symbol(sym_string)
static int Outlet_symbol(lua_State* L) {
    OutletUD* ud = (OutletUD*)luaL_checkudata(L, 1, OUTLET_MT);

    if (ud->outlet == NULL) {
        return luaL_error(L, "Outlet is null");
    }

    const char* str = luaL_checkstring(L, 2);
    t_symbol* sym = gensym(str);
    outlet_anything((t_outlet*)ud->outlet, sym, 0, NULL);

    return 0;
}

// Outlet.list(table_of_values)
static int Outlet_list(lua_State* L) {
    OutletUD* ud = (OutletUD*)luaL_checkudata(L, 1, OUTLET_MT);

    if (ud->outlet == NULL) {
        return luaL_error(L, "Outlet is null");
    }

    luaL_checktype(L, 2, LUA_TTABLE);

    // Convert Lua table to atom array
    long ac = (long)lua_objlen(L, 2);
    t_atom* av = (t_atom*)sysmem_newptr(ac * sizeof(t_atom));

    if (!av) {
        return luaL_error(L, "Failed to allocate memory for atoms");
    }

    for (long i = 0; i < ac; i++) {
        lua_rawgeti(L, 2, i + 1);
        if (!lua_toatom(L, -1, &av[i])) {
            sysmem_freeptr(av);
            return luaL_error(L, "Table element %d is not a valid atom type", (int)(i + 1));
        }
        lua_pop(L, 1);
    }

    outlet_list((t_outlet*)ud->outlet, NULL, (short)ac, av);

    sysmem_freeptr(av);

    return 0;
}

// Outlet.anything(symbol, table_of_values)
static int Outlet_anything(lua_State* L) {
    OutletUD* ud = (OutletUD*)luaL_checkudata(L, 1, OUTLET_MT);

    if (ud->outlet == NULL) {
        return luaL_error(L, "Outlet is null");
    }

    const char* sym_str = luaL_checkstring(L, 2);
    t_symbol* sym = gensym(sym_str);

    luaL_checktype(L, 3, LUA_TTABLE);

    // Convert Lua table to atom array
    long ac = (long)lua_objlen(L, 3);
    t_atom* av = (t_atom*)sysmem_newptr(ac * sizeof(t_atom));

    if (!av) {
        return luaL_error(L, "Failed to allocate memory for atoms");
    }

    for (long i = 0; i < ac; i++) {
        lua_rawgeti(L, 3, i + 1);
        if (!lua_toatom(L, -1, &av[i])) {
            sysmem_freeptr(av);
            return luaL_error(L, "Table element %d is not a valid atom type", (int)(i + 1));
        }
        lua_pop(L, 1);
    }

    outlet_anything((t_outlet*)ud->outlet, sym, (short)ac, av);

    sysmem_freeptr(av);

    return 0;
}

// Outlet.pointer()
static int Outlet_pointer(lua_State* L) {
    OutletUD* ud = (OutletUD*)luaL_checkudata(L, 1, OUTLET_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->outlet);
    return 1;
}

// __gc metamethod (destructor)
static int Outlet_gc(lua_State* L) {
    OutletUD* ud = (OutletUD*)luaL_checkudata(L, 1, OUTLET_MT);
    // Note: Outlets are typically freed by Max when object is freed
    // We don't manually free them
    ud->outlet = NULL;
    return 0;
}

// __tostring metamethod
static int Outlet_tostring(lua_State* L) {
    OutletUD* ud = (OutletUD*)luaL_checkudata(L, 1, OUTLET_MT);
    lua_pushfstring(L, "Outlet(ptr=%p)", ud->outlet);
    return 1;
}

// Register Outlet type
static void register_outlet_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, OUTLET_MT);

    // Set __index to itself
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    // Register methods
    lua_pushcfunction(L, Outlet_bang);
    lua_setfield(L, -2, "bang");

    lua_pushcfunction(L, Outlet_int);
    lua_setfield(L, -2, "int");

    lua_pushcfunction(L, Outlet_float);
    lua_setfield(L, -2, "float");

    lua_pushcfunction(L, Outlet_symbol);
    lua_setfield(L, -2, "symbol");

    lua_pushcfunction(L, Outlet_list);
    lua_setfield(L, -2, "list");

    lua_pushcfunction(L, Outlet_anything);
    lua_setfield(L, -2, "anything");

    lua_pushcfunction(L, Outlet_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Outlet_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Outlet_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_pop(L, 1);  // Pop metatable

    // Register constructor in api module
    lua_getglobal(L, "api");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "api");
    }

    lua_pushcfunction(L, Outlet_new);
    lua_setfield(L, -2, "Outlet");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_OUTLET_H

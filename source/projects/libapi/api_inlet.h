// api_inlet.h
// Inlet and Proxy wrapper for luajit-max API
// Provides dynamic inlet creation and management

#ifndef LUAJIT_API_INLET_H
#define LUAJIT_API_INLET_H

#include "api_common.h"

// Metatable name for Inlet userdata
#define INLET_MT "Max.Inlet"

// Inlet userdata structure
typedef struct {
    void* inlet;
    long inlet_num;  // For tracking inlet number
    bool is_proxy;
    bool owns_inlet;
} InletUD;

// Inlet constructor: Inlet()
static int Inlet_new(lua_State* L) {
    // Create userdata
    InletUD* ud = (InletUD*)lua_newuserdata(L, sizeof(InletUD));
    ud->inlet = NULL;
    ud->inlet_num = 0;
    ud->is_proxy = false;
    ud->owns_inlet = false;

    // Set metatable
    luaL_getmetatable(L, INLET_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Module-level: api.inlet_new(owner_ptr, message_name)
// Create a general-purpose inlet
static int api_inlet_new(lua_State* L) {
    lua_Number owner_ptr = luaL_checknumber(L, 1);
    const char* msg = NULL;

    if (lua_gettop(L) >= 2 && lua_isstring(L, 2)) {
        msg = lua_tostring(L, 2);
    }

    void* inlet = inlet_new((void*)(intptr_t)owner_ptr, msg);

    if (!inlet) {
        return luaL_error(L, "Failed to create inlet");
    }

    // Create Inlet userdata
    InletUD* ud = (InletUD*)lua_newuserdata(L, sizeof(InletUD));
    ud->inlet = inlet;
    ud->inlet_num = 0;
    ud->is_proxy = false;
    ud->owns_inlet = true;

    luaL_getmetatable(L, INLET_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Module-level: api.intin(owner_ptr, inlet_number)
// Create integer-typed inlet (sends in1, in2, etc.)
static int api_intin(lua_State* L) {
    lua_Number owner_ptr = luaL_checknumber(L, 1);
    long inlet_num = (long)luaL_checknumber(L, 2);

    if (inlet_num < 1 || inlet_num > 9) {
        return luaL_error(L, "Inlet number must be between 1 and 9");
    }

    void* inlet = intin((void*)(intptr_t)owner_ptr, (short)inlet_num);

    if (!inlet) {
        return luaL_error(L, "Failed to create integer inlet");
    }

    // Create Inlet userdata
    InletUD* ud = (InletUD*)lua_newuserdata(L, sizeof(InletUD));
    ud->inlet = inlet;
    ud->inlet_num = inlet_num;
    ud->is_proxy = false;
    ud->owns_inlet = true;

    luaL_getmetatable(L, INLET_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Module-level: api.floatin(owner_ptr, inlet_number)
// Create float-typed inlet (sends ft1, ft2, etc.)
static int api_floatin(lua_State* L) {
    lua_Number owner_ptr = luaL_checknumber(L, 1);
    long inlet_num = (long)luaL_checknumber(L, 2);

    if (inlet_num < 1 || inlet_num > 9) {
        return luaL_error(L, "Inlet number must be between 1 and 9");
    }

    void* inlet = floatin((void*)(intptr_t)owner_ptr, (short)inlet_num);

    if (!inlet) {
        return luaL_error(L, "Failed to create float inlet");
    }

    // Create Inlet userdata
    InletUD* ud = (InletUD*)lua_newuserdata(L, sizeof(InletUD));
    ud->inlet = inlet;
    ud->inlet_num = inlet_num;
    ud->is_proxy = false;
    ud->owns_inlet = true;

    luaL_getmetatable(L, INLET_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Module-level: api.proxy_new(owner_ptr, inlet_id, stuffloc_ptr)
// Create a proxy inlet (modern approach)
static int api_proxy_new(lua_State* L) {
    lua_Number owner_ptr = luaL_checknumber(L, 1);
    long inlet_id = (long)luaL_checknumber(L, 2);
    lua_Number stuffloc_ptr = luaL_checknumber(L, 3);

    void* proxy = proxy_new((void*)(intptr_t)owner_ptr, inlet_id, (long*)(intptr_t)stuffloc_ptr);

    if (!proxy) {
        return luaL_error(L, "Failed to create proxy inlet");
    }

    // Create Inlet userdata
    InletUD* ud = (InletUD*)lua_newuserdata(L, sizeof(InletUD));
    ud->inlet = proxy;
    ud->inlet_num = inlet_id;
    ud->is_proxy = true;
    ud->owns_inlet = true;

    luaL_getmetatable(L, INLET_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Module-level: api.proxy_getinlet(owner_ptr) -> number
// Get the inlet number where a message was received
static int api_proxy_getinlet(lua_State* L) {
    lua_Number owner_ptr = luaL_checknumber(L, 1);
    long inlet_num = proxy_getinlet((t_object*)(intptr_t)owner_ptr);
    lua_pushnumber(L, inlet_num);
    return 1;
}

// Module-level: api.inlet_count(owner_ptr) -> number
// Get the number of inlets for an object
static int api_inlet_count(lua_State* L) {
    lua_Number owner_ptr = luaL_checknumber(L, 1);
    long count = inlet_count((t_object*)(intptr_t)owner_ptr);
    lua_pushnumber(L, count);
    return 1;
}

// Module-level: api.inlet_nth(owner_ptr, index) -> Inlet or nil
// Get the nth inlet (0-indexed)
static int api_inlet_nth(lua_State* L) {
    lua_Number owner_ptr = luaL_checknumber(L, 1);
    long index = (long)luaL_checknumber(L, 2);

    void* inlet = inlet_nth((t_object*)(intptr_t)owner_ptr, index);

    if (!inlet) {
        lua_pushnil(L);
        return 1;
    }

    // Create Inlet userdata (non-owning)
    InletUD* ud = (InletUD*)lua_newuserdata(L, sizeof(InletUD));
    ud->inlet = inlet;
    ud->inlet_num = index;
    ud->is_proxy = false;
    ud->owns_inlet = false;  // We don't own it, just wrapping existing

    luaL_getmetatable(L, INLET_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Inlet:delete() - Delete this inlet (if we own it)
static int Inlet_delete(lua_State* L) {
    InletUD* ud = (InletUD*)luaL_checkudata(L, 1, INLET_MT);

    if (!ud->inlet) {
        return luaL_error(L, "Inlet is null");
    }

    if (!ud->owns_inlet) {
        return luaL_error(L, "Cannot delete inlet we don't own");
    }

    inlet_delete(ud->inlet);
    ud->inlet = NULL;
    ud->owns_inlet = false;

    return 0;
}

// Inlet:pointer() - Get pointer to inlet
static int Inlet_pointer(lua_State* L) {
    InletUD* ud = (InletUD*)luaL_checkudata(L, 1, INLET_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->inlet);
    return 1;
}

// Inlet:get_num() - Get inlet number
static int Inlet_get_num(lua_State* L) {
    InletUD* ud = (InletUD*)luaL_checkudata(L, 1, INLET_MT);
    lua_pushnumber(L, ud->inlet_num);
    return 1;
}

// Inlet:is_proxy() - Check if this is a proxy inlet
static int Inlet_is_proxy_method(lua_State* L) {
    InletUD* ud = (InletUD*)luaL_checkudata(L, 1, INLET_MT);
    lua_pushboolean(L, ud->is_proxy);
    return 1;
}

// Inlet:is_null() - Check if inlet pointer is null
static int Inlet_is_null(lua_State* L) {
    InletUD* ud = (InletUD*)luaL_checkudata(L, 1, INLET_MT);
    lua_pushboolean(L, ud->inlet == NULL);
    return 1;
}

// __gc metamethod (destructor)
static int Inlet_gc(lua_State* L) {
    InletUD* ud = (InletUD*)luaL_checkudata(L, 1, INLET_MT);
    // Only delete if we own it
    if (ud->owns_inlet && ud->inlet) {
        inlet_delete(ud->inlet);
        ud->inlet = NULL;
    }
    return 0;
}

// __tostring metamethod
static int Inlet_tostring(lua_State* L) {
    InletUD* ud = (InletUD*)luaL_checkudata(L, 1, INLET_MT);

    if (ud->is_proxy) {
        lua_pushfstring(L, "Inlet(proxy, num=%d, %p)", (int)ud->inlet_num, ud->inlet);
    } else if (ud->inlet) {
        lua_pushfstring(L, "Inlet(%p)", ud->inlet);
    } else {
        lua_pushstring(L, "Inlet(null)");
    }
    return 1;
}

// Register Inlet type
static void register_inlet_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, INLET_MT);

    // Register methods
    lua_pushcfunction(L, Inlet_delete);
    lua_setfield(L, -2, "delete");

    lua_pushcfunction(L, Inlet_pointer);
    lua_setfield(L, -2, "pointer");

    lua_pushcfunction(L, Inlet_get_num);
    lua_setfield(L, -2, "get_num");

    lua_pushcfunction(L, Inlet_is_proxy_method);
    lua_setfield(L, -2, "is_proxy");

    lua_pushcfunction(L, Inlet_is_null);
    lua_setfield(L, -2, "is_null");

    // Register metamethods
    lua_pushcfunction(L, Inlet_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Inlet_tostring);
    lua_setfield(L, -2, "__tostring");

    // __index points to metatable itself for method lookup
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);  // Pop metatable

    // Register module-level functions in api module
    lua_getglobal(L, "api");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "api");
    }

    lua_pushcfunction(L, Inlet_new);
    lua_setfield(L, -2, "Inlet");

    lua_pushcfunction(L, api_inlet_new);
    lua_setfield(L, -2, "inlet_new");

    lua_pushcfunction(L, api_intin);
    lua_setfield(L, -2, "intin");

    lua_pushcfunction(L, api_floatin);
    lua_setfield(L, -2, "floatin");

    lua_pushcfunction(L, api_proxy_new);
    lua_setfield(L, -2, "proxy_new");

    lua_pushcfunction(L, api_proxy_getinlet);
    lua_setfield(L, -2, "proxy_getinlet");

    lua_pushcfunction(L, api_inlet_count);
    lua_setfield(L, -2, "inlet_count");

    lua_pushcfunction(L, api_inlet_nth);
    lua_setfield(L, -2, "inlet_nth");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_INLET_H

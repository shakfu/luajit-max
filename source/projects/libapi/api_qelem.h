// api_qelem.h
// Qelem wrapper for luajit-max API
// Provides queue-based deferred execution for UI updates

#ifndef LUAJIT_API_QELEM_H
#define LUAJIT_API_QELEM_H

#include "api_common.h"

// Metatable name for Qelem userdata
#define QELEM_MT "Max.Qelem"

// Qelem userdata structure
typedef struct {
    t_qelem* qelem;
    lua_State* L;           // Lua state for callback
    int callback_ref;       // Lua registry reference to callback
    int userdata_ref;       // Lua registry reference to userdata (optional)
    bool is_set;
} QelemUD;

// Callback wrapper that executes Lua callback
static void qelem_callback_wrapper(QelemUD* ud) {
    if (!ud || !ud->L || ud->callback_ref == LUA_NOREF) {
        return;
    }

    lua_State* L = ud->L;

    // Get callback function from registry
    lua_rawgeti(L, LUA_REGISTRYINDEX, ud->callback_ref);

    // Push userdata if available
    int nargs = 0;
    if (ud->userdata_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, ud->userdata_ref);
        nargs = 1;
    }

    // Call the callback
    if (lua_pcall(L, nargs, 0, 0) != 0) {
        // Error in callback
        const char* err = lua_tostring(L, -1);
        error("Qelem callback error: %s", err);
        lua_pop(L, 1);
    }

    // Clear set flag after execution
    ud->is_set = false;
}

// Qelem constructor: Qelem(callback, userdata)
static int Qelem_new(lua_State* L) {
    if (lua_gettop(L) < 1) {
        return luaL_error(L, "Qelem() requires at least 1 argument (callback)");
    }

    if (!lua_isfunction(L, 1)) {
        return luaL_error(L, "Qelem(): first argument must be a function");
    }

    QelemUD* ud = (QelemUD*)lua_newuserdata(L, sizeof(QelemUD));
    ud->qelem = NULL;
    ud->L = L;
    ud->callback_ref = LUA_NOREF;
    ud->userdata_ref = LUA_NOREF;
    ud->is_set = false;

    // Store callback in registry
    lua_pushvalue(L, 1);
    ud->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    // Store optional userdata in registry
    if (lua_gettop(L) >= 3) {
        lua_pushvalue(L, 2);
        ud->userdata_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    // Create qelem
    ud->qelem = qelem_new(ud, (method)qelem_callback_wrapper);
    if (!ud->qelem) {
        // Clean up references if qelem creation fails
        if (ud->callback_ref != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, ud->callback_ref);
        }
        if (ud->userdata_ref != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, ud->userdata_ref);
        }
        return luaL_error(L, "Failed to create qelem");
    }

    luaL_getmetatable(L, QELEM_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Qelem:set()
static int Qelem_set(lua_State* L) {
    QelemUD* ud = (QelemUD*)luaL_checkudata(L, 1, QELEM_MT);

    if (!ud->qelem) {
        return luaL_error(L, "Qelem is null");
    }

    qelem_set(ud->qelem);
    ud->is_set = true;

    return 0;
}

// Qelem:unset()
static int Qelem_unset(lua_State* L) {
    QelemUD* ud = (QelemUD*)luaL_checkudata(L, 1, QELEM_MT);

    if (!ud->qelem) {
        return luaL_error(L, "Qelem is null");
    }

    qelem_unset(ud->qelem);
    ud->is_set = false;

    return 0;
}

// Qelem:front()
static int Qelem_front(lua_State* L) {
    QelemUD* ud = (QelemUD*)luaL_checkudata(L, 1, QELEM_MT);

    if (!ud->qelem) {
        return luaL_error(L, "Qelem is null");
    }

    qelem_front(ud->qelem);
    ud->is_set = true;

    return 0;
}

// Qelem:is_set() -> bool
static int Qelem_is_set(lua_State* L) {
    QelemUD* ud = (QelemUD*)luaL_checkudata(L, 1, QELEM_MT);
    lua_pushboolean(L, ud->is_set);
    return 1;
}

// Qelem:is_null() -> bool
static int Qelem_is_null(lua_State* L) {
    QelemUD* ud = (QelemUD*)luaL_checkudata(L, 1, QELEM_MT);
    lua_pushboolean(L, ud->qelem == NULL);
    return 1;
}

// Qelem:pointer() -> pointer
static int Qelem_pointer(lua_State* L) {
    QelemUD* ud = (QelemUD*)luaL_checkudata(L, 1, QELEM_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->qelem);
    return 1;
}

// __gc metamethod
static int Qelem_gc(lua_State* L) {
    QelemUD* ud = (QelemUD*)luaL_checkudata(L, 1, QELEM_MT);

    if (ud->qelem) {
        qelem_unset(ud->qelem);
        qelem_free(ud->qelem);
        ud->qelem = NULL;
    }

    // Free Lua registry references
    if (ud->callback_ref != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, ud->callback_ref);
        ud->callback_ref = LUA_NOREF;
    }

    if (ud->userdata_ref != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, ud->userdata_ref);
        ud->userdata_ref = LUA_NOREF;
    }

    return 0;
}

// __tostring metamethod
static int Qelem_tostring(lua_State* L) {
    QelemUD* ud = (QelemUD*)luaL_checkudata(L, 1, QELEM_MT);

    if (ud->qelem) {
        lua_pushfstring(L, "Qelem(%p, set=%s)",
                        ud->qelem, ud->is_set ? "true" : "false");
    } else {
        lua_pushstring(L, "Qelem(null)");
    }
    return 1;
}

// Register Qelem type
static void register_qelem_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, QELEM_MT);

    // Register methods
    lua_pushcfunction(L, Qelem_set);
    lua_setfield(L, -2, "set");

    lua_pushcfunction(L, Qelem_unset);
    lua_setfield(L, -2, "unset");

    lua_pushcfunction(L, Qelem_front);
    lua_setfield(L, -2, "front");

    lua_pushcfunction(L, Qelem_is_set);
    lua_setfield(L, -2, "is_set");

    lua_pushcfunction(L, Qelem_is_null);
    lua_setfield(L, -2, "is_null");

    lua_pushcfunction(L, Qelem_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Qelem_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Qelem_tostring);
    lua_setfield(L, -2, "__tostring");

    // __index points to metatable itself
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);  // Pop metatable

    // Register constructor in api module
    lua_getglobal(L, "api");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "api");
    }

    lua_pushcfunction(L, Qelem_new);
    lua_setfield(L, -2, "Qelem");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_QELEM_H

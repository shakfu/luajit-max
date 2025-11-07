// api_clock.h
// Clock wrapper for luajit-max API

#ifndef LUAJIT_API_CLOCK_H
#define LUAJIT_API_CLOCK_H

#include "api_common.h"

// Metatable name for Clock userdata
#define CLOCK_MT "Max.Clock"

// Clock userdata structure
typedef struct {
    t_clock* clock;
    bool owns_clock;
    lua_State* L;           // Store Lua state for callback
    int callback_ref;       // Lua registry reference to callback
    void* owner;            // Store owner object pointer
} ClockUD;

// Clock callback bridge - called by Max scheduler
static void clock_callback_bridge(ClockUD* clock_ud) {
    if (clock_ud == NULL || clock_ud->L == NULL || clock_ud->callback_ref == LUA_NOREF) {
        return;
    }

    lua_State* L = clock_ud->L;

    // Get callback function from registry
    lua_rawgeti(L, LUA_REGISTRYINDEX, clock_ud->callback_ref);

    // Call the callback with no arguments
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        error("Clock callback error: %s", err);
        lua_pop(L, 1);
    }
}

// Clock constructor: Clock(owner_ptr, callback)
static int Clock_new(lua_State* L) {
    if (lua_gettop(L) < 2) {
        return luaL_error(L, "Clock() requires 2 arguments: owner_ptr, callback");
    }

    if (!lua_isnumber(L, 1)) {
        return luaL_error(L, "Clock() arg 1 must be owner pointer (number)");
    }

    if (!lua_isfunction(L, 2)) {
        return luaL_error(L, "Clock() arg 2 must be a function");
    }

    // Create userdata
    ClockUD* ud = (ClockUD*)lua_newuserdata(L, sizeof(ClockUD));
    ud->clock = NULL;
    ud->owns_clock = false;
    ud->L = L;
    ud->callback_ref = LUA_NOREF;
    ud->owner = (void*)(intptr_t)lua_tonumber(L, 1);

    // Store callback in registry
    lua_pushvalue(L, 2);  // Push callback function
    ud->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    // Create Max clock
    ud->clock = clock_new(ud->owner, (method)clock_callback_bridge);
    ud->owns_clock = true;

    // Set metatable
    luaL_getmetatable(L, CLOCK_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Clock.delay(milliseconds)
static int Clock_delay(lua_State* L) {
    ClockUD* ud = (ClockUD*)luaL_checkudata(L, 1, CLOCK_MT);

    if (ud->clock == NULL) {
        return luaL_error(L, "Clock is null");
    }

    long ms = (long)luaL_checknumber(L, 2);
    clock_delay(ud->clock, ms);

    return 0;
}

// Clock.fdelay(milliseconds_float)
static int Clock_fdelay(lua_State* L) {
    ClockUD* ud = (ClockUD*)luaL_checkudata(L, 1, CLOCK_MT);

    if (ud->clock == NULL) {
        return luaL_error(L, "Clock is null");
    }

    double ms = luaL_checknumber(L, 2);
    clock_fdelay(ud->clock, ms);

    return 0;
}

// Clock.unset()
static int Clock_unset(lua_State* L) {
    ClockUD* ud = (ClockUD*)luaL_checkudata(L, 1, CLOCK_MT);

    if (ud->clock == NULL) {
        return luaL_error(L, "Clock is null");
    }

    clock_unset(ud->clock);

    return 0;
}

// Clock.pointer()
static int Clock_pointer(lua_State* L) {
    ClockUD* ud = (ClockUD*)luaL_checkudata(L, 1, CLOCK_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->clock);
    return 1;
}

// __gc metamethod (destructor)
static int Clock_gc(lua_State* L) {
    ClockUD* ud = (ClockUD*)luaL_checkudata(L, 1, CLOCK_MT);

    if (ud->clock && ud->owns_clock) {
        clock_unset(ud->clock);
        freeobject((t_object*)ud->clock);
        ud->clock = NULL;
    }

    // Release callback reference
    if (ud->callback_ref != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, ud->callback_ref);
        ud->callback_ref = LUA_NOREF;
    }

    return 0;
}

// __tostring metamethod
static int Clock_tostring(lua_State* L) {
    ClockUD* ud = (ClockUD*)luaL_checkudata(L, 1, CLOCK_MT);
    lua_pushfstring(L, "Clock(active=%s)", ud->clock ? "true" : "false");
    return 1;
}

// Register Clock type
static void register_clock_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, CLOCK_MT);

    // Set __index to itself
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    // Register methods
    lua_pushcfunction(L, Clock_delay);
    lua_setfield(L, -2, "delay");

    lua_pushcfunction(L, Clock_fdelay);
    lua_setfield(L, -2, "fdelay");

    lua_pushcfunction(L, Clock_unset);
    lua_setfield(L, -2, "unset");

    lua_pushcfunction(L, Clock_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Clock_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Clock_tostring);
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

    lua_pushcfunction(L, Clock_new);
    lua_setfield(L, -2, "Clock");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_CLOCK_H

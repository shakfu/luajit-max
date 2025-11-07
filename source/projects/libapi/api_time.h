// api_time.h
// Time (ITM) wrapper for luajit-max API
// Provides transport and tempo-aware timing beyond basic Clock

#ifndef LUAJIT_API_TIME_H
#define LUAJIT_API_TIME_H

#include "api_common.h"

// Metatable name for ITM userdata
#define ITM_MT "Max.ITM"

// ITM userdata structure
typedef struct {
    t_itm* itm;
    bool owns_itm;
} ITMUD;

// ITM constructor: ITM(name) or ITM() for global
static int ITM_new(lua_State* L) {
    ITMUD* ud = (ITMUD*)lua_newuserdata(L, sizeof(ITMUD));
    ud->itm = NULL;
    ud->owns_itm = false;

    int nargs = lua_gettop(L);

    if (nargs == 0) {
        // ITM() - get global ITM
        ud->itm = (t_itm*)itm_getglobal();
        ud->owns_itm = false;
    } else if (nargs == 1 && lua_isstring(L, 1)) {
        // ITM(name) - get named ITM
        const char* name = lua_tostring(L, 1);
        t_symbol* s = gensym(name);
        ud->itm = (t_itm*)itm_getnamed(s, NULL, NULL, 1);
        ud->owns_itm = true;
    } else if (nargs == 1 && lua_isnumber(L, 1)) {
        // ITM(ptr) - wrap existing ITM
        lua_Number ptr_num = lua_tonumber(L, 1);
        ud->itm = (t_itm*)(intptr_t)ptr_num;
        ud->owns_itm = false;
    } else {
        return luaL_error(L, "ITM() takes 0 or 1 argument (name or pointer)");
    }

    luaL_getmetatable(L, ITM_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// ITM:getticks() -> ticks
static int ITM_getticks(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    double ticks = itm_getticks(ud->itm);
    lua_pushnumber(L, ticks);
    return 1;
}

// ITM:gettime() -> time_ms
static int ITM_gettime(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    double time = itm_gettime(ud->itm);
    lua_pushnumber(L, time);
    return 1;
}

// ITM:getstate() -> state
static int ITM_getstate(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    long state = itm_getstate(ud->itm);
    lua_pushnumber(L, state);
    return 1;
}

// ITM:tickstoms(ticks) -> ms
static int ITM_tickstoms(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);
    double ticks = luaL_checknumber(L, 2);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    double ms = itm_tickstoms(ud->itm, ticks);
    lua_pushnumber(L, ms);
    return 1;
}

// ITM:mstoticks(ms) -> ticks
static int ITM_mstoticks(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);
    double ms = luaL_checknumber(L, 2);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    double ticks = itm_mstoticks(ud->itm, ms);
    lua_pushnumber(L, ticks);
    return 1;
}

// ITM:mstosamps(ms) -> samples
static int ITM_mstosamps(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);
    double ms = luaL_checknumber(L, 2);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    double samps = itm_mstosamps(ud->itm, ms);
    lua_pushnumber(L, samps);
    return 1;
}

// ITM:sampstoms(samples) -> ms
static int ITM_sampstoms(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);
    double samps = luaL_checknumber(L, 2);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    double ms = itm_sampstoms(ud->itm, samps);
    lua_pushnumber(L, ms);
    return 1;
}

// ITM:bbutoticsk(bars, beats, units) -> ticks
static int ITM_bbutoticsk(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);
    long bars = (long)luaL_checknumber(L, 2);
    long beats = (long)luaL_checknumber(L, 3);
    double units = luaL_checknumber(L, 4);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    double ticks = 0;
    itm_barbeatunitstoticks(ud->itm, bars, beats, units, &ticks, 0);
    lua_pushnumber(L, ticks);
    return 1;
}

// ITM:tickstobbu(ticks) -> {bars, beats, units}
static int ITM_tickstobbu(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);
    double ticks = luaL_checknumber(L, 2);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    long bars, beats;
    double units;
    itm_tickstobarbeatunits(ud->itm, ticks, &bars, &beats, &units, 0);

    // Return table {bars, beats, units}
    lua_createtable(L, 3, 0);
    lua_pushnumber(L, bars);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, beats);
    lua_rawseti(L, -2, 2);
    lua_pushnumber(L, units);
    lua_rawseti(L, -2, 3);

    return 1;
}

// ITM:pause()
static int ITM_pause(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    itm_pause(ud->itm);
    return 0;
}

// ITM:resume()
static int ITM_resume(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    itm_resume(ud->itm);
    return 0;
}

// ITM:seek(oldticks, newticks, chase)
static int ITM_seek(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);
    double oldticks = luaL_checknumber(L, 2);
    double newticks = luaL_checknumber(L, 3);
    long chase = 1;

    if (lua_gettop(L) >= 4) {
        chase = (long)lua_tonumber(L, 4);
    }

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    itm_seek(ud->itm, oldticks, newticks, chase);
    return 0;
}

// ITM:settimesignature(numerator, denominator, flags)
static int ITM_settimesignature(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);
    long num = (long)luaL_checknumber(L, 2);
    long denom = (long)luaL_checknumber(L, 3);
    long flags = 0;

    if (lua_gettop(L) >= 4) {
        flags = (long)lua_tonumber(L, 4);
    }

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    itm_settimesignature(ud->itm, num, denom, flags);
    return 0;
}

// ITM:gettimesignature() -> {numerator, denominator}
static int ITM_gettimesignature(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    long num, denom;
    itm_gettimesignature(ud->itm, &num, &denom);

    // Return table {numerator, denominator}
    lua_createtable(L, 2, 0);
    lua_pushnumber(L, num);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, denom);
    lua_rawseti(L, -2, 2);

    return 1;
}

// ITM:dump()
static int ITM_dump(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    itm_dump(ud->itm);
    return 0;
}

// ITM:sync()
static int ITM_sync(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);

    if (!ud->itm) {
        return luaL_error(L, "ITM is null");
    }

    itm_sync(ud->itm);
    return 0;
}

// ITM:pointer() -> pointer
static int ITM_pointer(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->itm);
    return 1;
}

// ITM:is_valid() -> bool
static int ITM_is_valid(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);
    lua_pushboolean(L, ud->itm != NULL);
    return 1;
}

// __gc metamethod
static int ITM_gc(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);

    if (ud->itm && ud->owns_itm) {
        itm_dereference(ud->itm);
        ud->itm = NULL;
    }

    return 0;
}

// __tostring metamethod
static int ITM_tostring(lua_State* L) {
    ITMUD* ud = (ITMUD*)luaL_checkudata(L, 1, ITM_MT);

    if (ud->itm) {
        lua_pushfstring(L, "ITM(%p)", ud->itm);
    } else {
        lua_pushstring(L, "ITM(null)");
    }
    return 1;
}

// Module-level functions

// api.itm_getglobal() -> pointer
static int api_itm_getglobal(lua_State* L) {
    void* itm = itm_getglobal();
    lua_pushnumber(L, (lua_Number)(intptr_t)itm);
    return 1;
}

// api.itm_setresolution(res)
static int api_itm_setresolution(lua_State* L) {
    double res = luaL_checknumber(L, 1);
    itm_setresolution(res);
    return 0;
}

// api.itm_getresolution() -> res
static int api_itm_getresolution(lua_State* L) {
    double res = itm_getresolution();
    lua_pushnumber(L, res);
    return 1;
}

// Register ITM type
static void register_time_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, ITM_MT);

    // Register methods
    lua_pushcfunction(L, ITM_getticks);
    lua_setfield(L, -2, "getticks");

    lua_pushcfunction(L, ITM_gettime);
    lua_setfield(L, -2, "gettime");

    lua_pushcfunction(L, ITM_getstate);
    lua_setfield(L, -2, "getstate");

    lua_pushcfunction(L, ITM_tickstoms);
    lua_setfield(L, -2, "tickstoms");

    lua_pushcfunction(L, ITM_mstoticks);
    lua_setfield(L, -2, "mstoticks");

    lua_pushcfunction(L, ITM_mstosamps);
    lua_setfield(L, -2, "mstosamps");

    lua_pushcfunction(L, ITM_sampstoms);
    lua_setfield(L, -2, "sampstoms");

    lua_pushcfunction(L, ITM_bbutoticsk);
    lua_setfield(L, -2, "bbutoticsk");

    lua_pushcfunction(L, ITM_tickstobbu);
    lua_setfield(L, -2, "tickstobbu");

    lua_pushcfunction(L, ITM_pause);
    lua_setfield(L, -2, "pause");

    lua_pushcfunction(L, ITM_resume);
    lua_setfield(L, -2, "resume");

    lua_pushcfunction(L, ITM_seek);
    lua_setfield(L, -2, "seek");

    lua_pushcfunction(L, ITM_settimesignature);
    lua_setfield(L, -2, "settimesignature");

    lua_pushcfunction(L, ITM_gettimesignature);
    lua_setfield(L, -2, "gettimesignature");

    lua_pushcfunction(L, ITM_dump);
    lua_setfield(L, -2, "dump");

    lua_pushcfunction(L, ITM_sync);
    lua_setfield(L, -2, "sync");

    lua_pushcfunction(L, ITM_pointer);
    lua_setfield(L, -2, "pointer");

    lua_pushcfunction(L, ITM_is_valid);
    lua_setfield(L, -2, "is_valid");

    // Register metamethods
    lua_pushcfunction(L, ITM_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, ITM_tostring);
    lua_setfield(L, -2, "__tostring");

    // __index points to metatable itself
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);  // Pop metatable

    // Register constructor and module functions in api module
    lua_getglobal(L, "api");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "api");
    }

    lua_pushcfunction(L, ITM_new);
    lua_setfield(L, -2, "ITM");

    lua_pushcfunction(L, api_itm_getglobal);
    lua_setfield(L, -2, "itm_getglobal");

    lua_pushcfunction(L, api_itm_setresolution);
    lua_setfield(L, -2, "itm_setresolution");

    lua_pushcfunction(L, api_itm_getresolution);
    lua_setfield(L, -2, "itm_getresolution");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_TIME_H

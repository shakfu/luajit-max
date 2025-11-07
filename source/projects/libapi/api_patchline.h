// api_patchline.h
// Patchline wrapper for luajit-max API
// Provides access to Max patchline (patch cord) objects

#ifndef LUAJIT_API_PATCHLINE_H
#define LUAJIT_API_PATCHLINE_H

#include "api_common.h"

// Metatable name for Patchline userdata
#define PATCHLINE_MT "Max.Patchline"

// Patchline userdata structure
typedef struct {
    t_object* patchline;
    bool owns_patchline;  // Always false - patchlines are owned by patcher
} PatchlineUD;

// Patchline constructor: Patchline()
static int Patchline_new(lua_State* L) {
    // Create userdata
    PatchlineUD* ud = (PatchlineUD*)lua_newuserdata(L, sizeof(PatchlineUD));
    ud->patchline = NULL;
    ud->owns_patchline = false;

    // Set metatable
    luaL_getmetatable(L, PATCHLINE_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Patchline:wrap(pointer) - Wrap existing patchline pointer
static int Patchline_wrap(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);
    lua_Number ptr_num = luaL_checknumber(L, 2);

    if (ptr_num == 0) {
        return luaL_error(L, "Cannot wrap null pointer");
    }

    ud->patchline = (t_object*)(intptr_t)ptr_num;
    ud->owns_patchline = false;

    return 0;
}

// Patchline:is_null() - Check if patchline is null
static int Patchline_is_null(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);
    lua_pushboolean(L, ud->patchline == NULL);
    return 1;
}

// Patchline:get_box1() - Get source box
static int Patchline_get_box1(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);

    if (!ud->patchline) {
        return luaL_error(L, "Patchline is null");
    }

    t_object* box = jpatchline_get_box1(ud->patchline);

    if (!box) {
        lua_pushnil(L);
        return 1;
    }

    // Create Box userdata (non-owning)
    BoxUD* box_ud = (BoxUD*)lua_newuserdata(L, sizeof(BoxUD));
    box_ud->box = box;
    box_ud->owns_box = false;

    luaL_getmetatable(L, BOX_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Patchline:get_box2() - Get destination box
static int Patchline_get_box2(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);

    if (!ud->patchline) {
        return luaL_error(L, "Patchline is null");
    }

    t_object* box = jpatchline_get_box2(ud->patchline);

    if (!box) {
        lua_pushnil(L);
        return 1;
    }

    // Create Box userdata (non-owning)
    BoxUD* box_ud = (BoxUD*)lua_newuserdata(L, sizeof(BoxUD));
    box_ud->box = box;
    box_ud->owns_box = false;

    luaL_getmetatable(L, BOX_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Patchline:get_outletnum() - Get outlet number of source box
static int Patchline_get_outletnum(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);

    if (!ud->patchline) {
        return luaL_error(L, "Patchline is null");
    }

    long outletnum = jpatchline_get_outletnum(ud->patchline);
    lua_pushnumber(L, outletnum);
    return 1;
}

// Patchline:get_inletnum() - Get inlet number of destination box
static int Patchline_get_inletnum(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);

    if (!ud->patchline) {
        return luaL_error(L, "Patchline is null");
    }

    long inletnum = jpatchline_get_inletnum(ud->patchline);
    lua_pushnumber(L, inletnum);
    return 1;
}

// Patchline:get_startpoint() - Get start point {x, y}
static int Patchline_get_startpoint(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);

    if (!ud->patchline) {
        return luaL_error(L, "Patchline is null");
    }

    double x, y;
    t_max_err err = jpatchline_get_startpoint(ud->patchline, &x, &y);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get startpoint");
    }

    // Return as table {x, y}
    lua_createtable(L, 2, 0);
    lua_pushnumber(L, x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, y);
    lua_rawseti(L, -2, 2);

    return 1;
}

// Patchline:get_endpoint() - Get end point {x, y}
static int Patchline_get_endpoint(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);

    if (!ud->patchline) {
        return luaL_error(L, "Patchline is null");
    }

    double x, y;
    t_max_err err = jpatchline_get_endpoint(ud->patchline, &x, &y);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get endpoint");
    }

    // Return as table {x, y}
    lua_createtable(L, 2, 0);
    lua_pushnumber(L, x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, y);
    lua_rawseti(L, -2, 2);

    return 1;
}

// Patchline:get_hidden() - Get hidden state
static int Patchline_get_hidden(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);

    if (!ud->patchline) {
        return luaL_error(L, "Patchline is null");
    }

    char hidden = jpatchline_get_hidden(ud->patchline);
    lua_pushboolean(L, hidden != 0);
    return 1;
}

// Patchline:set_hidden(hidden) - Set hidden state
static int Patchline_set_hidden(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);
    int hidden = lua_toboolean(L, 2);

    if (!ud->patchline) {
        return luaL_error(L, "Patchline is null");
    }

    t_max_err err = jpatchline_set_hidden(ud->patchline, hidden ? 1 : 0);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to set hidden");
    }

    return 0;
}

// Patchline:get_nextline() - Get next patchline in linked list
static int Patchline_get_nextline(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);

    if (!ud->patchline) {
        return luaL_error(L, "Patchline is null");
    }

    t_object* nextline = jpatchline_get_nextline(ud->patchline);

    if (!nextline) {
        lua_pushnil(L);
        return 1;
    }

    // Create Patchline userdata (non-owning)
    PatchlineUD* next_ud = (PatchlineUD*)lua_newuserdata(L, sizeof(PatchlineUD));
    next_ud->patchline = nextline;
    next_ud->owns_patchline = false;

    luaL_getmetatable(L, PATCHLINE_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Patchline:pointer() - Get raw pointer value
static int Patchline_pointer(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->patchline);
    return 1;
}

// __gc metamethod (destructor)
static int Patchline_gc(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);
    // Patchlines are owned by patcher, never free them
    (void)ud;
    return 0;
}

// __tostring metamethod
static int Patchline_tostring(lua_State* L) {
    PatchlineUD* ud = (PatchlineUD*)luaL_checkudata(L, 1, PATCHLINE_MT);

    if (ud->patchline) {
        lua_pushfstring(L, "Patchline(%p)", ud->patchline);
    } else {
        lua_pushstring(L, "Patchline(null)");
    }
    return 1;
}

// Register Patchline type
static void register_patchline_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, PATCHLINE_MT);

    // Register methods
    lua_pushcfunction(L, Patchline_wrap);
    lua_setfield(L, -2, "wrap");

    lua_pushcfunction(L, Patchline_is_null);
    lua_setfield(L, -2, "is_null");

    lua_pushcfunction(L, Patchline_get_box1);
    lua_setfield(L, -2, "get_box1");

    lua_pushcfunction(L, Patchline_get_box2);
    lua_setfield(L, -2, "get_box2");

    lua_pushcfunction(L, Patchline_get_outletnum);
    lua_setfield(L, -2, "get_outletnum");

    lua_pushcfunction(L, Patchline_get_inletnum);
    lua_setfield(L, -2, "get_inletnum");

    lua_pushcfunction(L, Patchline_get_startpoint);
    lua_setfield(L, -2, "get_startpoint");

    lua_pushcfunction(L, Patchline_get_endpoint);
    lua_setfield(L, -2, "get_endpoint");

    lua_pushcfunction(L, Patchline_get_hidden);
    lua_setfield(L, -2, "get_hidden");

    lua_pushcfunction(L, Patchline_set_hidden);
    lua_setfield(L, -2, "set_hidden");

    lua_pushcfunction(L, Patchline_get_nextline);
    lua_setfield(L, -2, "get_nextline");

    lua_pushcfunction(L, Patchline_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Patchline_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Patchline_tostring);
    lua_setfield(L, -2, "__tostring");

    // __index points to metatable itself for method lookup
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

    lua_pushcfunction(L, Patchline_new);
    lua_setfield(L, -2, "Patchline");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_PATCHLINE_H

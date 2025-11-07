// api_box.h
// Box wrapper for luajit-max API
// Provides access to Max patcher box objects

#ifndef LUAJIT_API_BOX_H
#define LUAJIT_API_BOX_H

#include "api_common.h"

// Metatable name for Box userdata
#define BOX_MT "Max.Box"

// Box userdata structure
typedef struct {
    t_object* box;
    bool owns_box;  // Always false - boxes are owned by patcher
} BoxUD;

// Box constructor: Box()
static int Box_new(lua_State* L) {
    // Create userdata
    BoxUD* ud = (BoxUD*)lua_newuserdata(L, sizeof(BoxUD));
    ud->box = NULL;
    ud->owns_box = false;

    // Set metatable
    luaL_getmetatable(L, BOX_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Box:wrap(pointer) - Wrap existing box pointer
static int Box_wrap(lua_State* L) {
    BoxUD* ud = (BoxUD*)luaL_checkudata(L, 1, BOX_MT);
    lua_Number ptr_num = luaL_checknumber(L, 2);

    if (ptr_num == 0) {
        return luaL_error(L, "Cannot wrap null pointer");
    }

    ud->box = (t_object*)(intptr_t)ptr_num;
    ud->owns_box = false;  // Never own boxes

    return 0;
}

// Box:is_null() - Check if box is null
static int Box_is_null(lua_State* L) {
    BoxUD* ud = (BoxUD*)luaL_checkudata(L, 1, BOX_MT);
    lua_pushboolean(L, ud->box == NULL);
    return 1;
}

// Box:classname() - Get object class name
static int Box_classname(lua_State* L) {
    BoxUD* ud = (BoxUD*)luaL_checkudata(L, 1, BOX_MT);

    if (!ud->box) {
        return luaL_error(L, "Box is null");
    }

    t_object* obj = jbox_get_object(ud->box);
    t_symbol* classname = object_classname(obj);
    lua_pushstring(L, classname->s_name);
    return 1;
}

// Box:get_object() - Get underlying Max object pointer
static int Box_get_object(lua_State* L) {
    BoxUD* ud = (BoxUD*)luaL_checkudata(L, 1, BOX_MT);

    if (!ud->box) {
        return luaL_error(L, "Box is null");
    }

    t_object* obj = jbox_get_object(ud->box);
    lua_pushnumber(L, (lua_Number)(intptr_t)obj);
    return 1;
}

// Box:get_rect() - Get box rectangle {x, y, width, height}
static int Box_get_rect(lua_State* L) {
    BoxUD* ud = (BoxUD*)luaL_checkudata(L, 1, BOX_MT);

    if (!ud->box) {
        return luaL_error(L, "Box is null");
    }

    t_rect rect;
    t_max_err err = jbox_get_rect_for_view(ud->box, NULL, &rect);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get box rectangle");
    }

    // Return as table: {x, y, width, height}
    lua_createtable(L, 4, 0);
    lua_pushnumber(L, rect.x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, rect.y);
    lua_rawseti(L, -2, 2);
    lua_pushnumber(L, rect.width);
    lua_rawseti(L, -2, 3);
    lua_pushnumber(L, rect.height);
    lua_rawseti(L, -2, 4);

    return 1;
}

// Box:set_rect(x, y, width, height) - Set box rectangle
static int Box_set_rect(lua_State* L) {
    BoxUD* ud = (BoxUD*)luaL_checkudata(L, 1, BOX_MT);

    if (!ud->box) {
        return luaL_error(L, "Box is null");
    }

    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);
    double width = luaL_checknumber(L, 4);
    double height = luaL_checknumber(L, 5);

    t_rect rect;
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;

    t_max_err err = jbox_set_rect_for_view(ud->box, NULL, &rect);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to set box rectangle");
    }

    return 0;
}

// Box:pointer() - Get raw pointer value
static int Box_pointer(lua_State* L) {
    BoxUD* ud = (BoxUD*)luaL_checkudata(L, 1, BOX_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->box);
    return 1;
}

// __gc metamethod (destructor)
static int Box_gc(lua_State* L) {
    BoxUD* ud = (BoxUD*)luaL_checkudata(L, 1, BOX_MT);
    // Boxes are owned by patcher, never free them
    (void)ud;
    return 0;
}

// __tostring metamethod
static int Box_tostring(lua_State* L) {
    BoxUD* ud = (BoxUD*)luaL_checkudata(L, 1, BOX_MT);

    if (ud->box) {
        t_object* obj = jbox_get_object(ud->box);
        t_symbol* classname = object_classname(obj);
        lua_pushfstring(L, "Box(%s, %p)", classname->s_name, ud->box);
    } else {
        lua_pushstring(L, "Box(null)");
    }
    return 1;
}

// Register Box type
static void register_box_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, BOX_MT);

    // Register methods
    lua_pushcfunction(L, Box_wrap);
    lua_setfield(L, -2, "wrap");

    lua_pushcfunction(L, Box_is_null);
    lua_setfield(L, -2, "is_null");

    lua_pushcfunction(L, Box_classname);
    lua_setfield(L, -2, "classname");

    lua_pushcfunction(L, Box_get_object);
    lua_setfield(L, -2, "get_object");

    lua_pushcfunction(L, Box_get_rect);
    lua_setfield(L, -2, "get_rect");

    lua_pushcfunction(L, Box_set_rect);
    lua_setfield(L, -2, "set_rect");

    lua_pushcfunction(L, Box_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Box_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Box_tostring);
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

    lua_pushcfunction(L, Box_new);
    lua_setfield(L, -2, "Box");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_BOX_H

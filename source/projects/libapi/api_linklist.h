// api_linklist.h
// Linklist wrapper for luajit-max API
// Provides access to Max's linked list data structure

#ifndef LUAJIT_API_LINKLIST_H
#define LUAJIT_API_LINKLIST_H

#include "api_common.h"

// Metatable name for Linklist userdata
#define LINKLIST_MT "Max.Linklist"

// Linklist userdata structure
typedef struct {
    t_linklist* linklist;
    bool owns_linklist;
} LinklistUD;

// Linklist constructor: Linklist()
static int Linklist_new(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)lua_newuserdata(L, sizeof(LinklistUD));
    ud->linklist = linklist_new();
    ud->owns_linklist = true;

    if (!ud->linklist) {
        return luaL_error(L, "Failed to create linklist");
    }

    luaL_getmetatable(L, LINKLIST_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Linklist.wrap(pointer) - Wrap existing linklist
static int Linklist_wrap(lua_State* L) {
    lua_Number ptr_num = luaL_checknumber(L, 1);

    if (ptr_num == 0) {
        return luaL_error(L, "Cannot wrap null pointer");
    }

    LinklistUD* ud = (LinklistUD*)lua_newuserdata(L, sizeof(LinklistUD));
    ud->linklist = (t_linklist*)(intptr_t)ptr_num;
    ud->owns_linklist = false;

    luaL_getmetatable(L, LINKLIST_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Linklist:is_null() -> bool
static int Linklist_is_null(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);
    lua_pushboolean(L, ud->linklist == NULL);
    return 1;
}

// Linklist:append(item) -> index
static int Linklist_append(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);

    if (!ud->linklist) {
        return luaL_error(L, "Linklist is null");
    }

    void* item = NULL;

    // Accept number as pointer
    if (lua_isnumber(L, 2)) {
        lua_Number item_num = lua_tonumber(L, 2);
        item = (void*)(intptr_t)item_num;
    } else {
        return luaL_error(L, "append() requires a number (pointer)");
    }

    t_atom_long index = linklist_append(ud->linklist, item);
    lua_pushnumber(L, index);
    return 1;
}

// Linklist:insertindex(item, index) -> result_index
static int Linklist_insertindex(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);
    long index = (long)luaL_checknumber(L, 3);

    if (!ud->linklist) {
        return luaL_error(L, "Linklist is null");
    }

    void* item = NULL;

    // Accept number as pointer
    if (lua_isnumber(L, 2)) {
        lua_Number item_num = lua_tonumber(L, 2);
        item = (void*)(intptr_t)item_num;
    } else {
        return luaL_error(L, "insertindex() requires a number (pointer) for item");
    }

    t_atom_long result_index = linklist_insertindex(ud->linklist, item, index);
    lua_pushnumber(L, result_index);
    return 1;
}

// Linklist:getindex(index) -> item_pointer or nil
static int Linklist_getindex(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);
    long index = (long)luaL_checknumber(L, 2);

    if (!ud->linklist) {
        return luaL_error(L, "Linklist is null");
    }

    void* item = linklist_getindex(ud->linklist, index);

    if (!item) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushnumber(L, (lua_Number)(intptr_t)item);
    return 1;
}

// Linklist:chuckindex(index) -> result
static int Linklist_chuckindex(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);
    long index = (long)luaL_checknumber(L, 2);

    if (!ud->linklist) {
        return luaL_error(L, "Linklist is null");
    }

    long result = linklist_chuckindex(ud->linklist, index);
    lua_pushnumber(L, result);
    return 1;
}

// Linklist:deleteindex(index) -> result (alias for chuckindex)
static int Linklist_deleteindex(lua_State* L) {
    return Linklist_chuckindex(L);
}

// Linklist:clear()
static int Linklist_clear(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);

    if (!ud->linklist) {
        return luaL_error(L, "Linklist is null");
    }

    linklist_clear(ud->linklist);
    return 0;
}

// Linklist:getsize() -> size
static int Linklist_getsize(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);

    if (!ud->linklist) {
        return luaL_error(L, "Linklist is null");
    }

    t_atom_long size = linklist_getsize(ud->linklist);
    lua_pushnumber(L, size);
    return 1;
}

// Linklist:reverse()
static int Linklist_reverse(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);

    if (!ud->linklist) {
        return luaL_error(L, "Linklist is null");
    }

    linklist_reverse(ud->linklist);
    return 0;
}

// Linklist:rotate(n)
static int Linklist_rotate(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);
    long n = (long)luaL_checknumber(L, 2);

    if (!ud->linklist) {
        return luaL_error(L, "Linklist is null");
    }

    linklist_rotate(ud->linklist, n);
    return 0;
}

// Linklist:shuffle()
static int Linklist_shuffle(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);

    if (!ud->linklist) {
        return luaL_error(L, "Linklist is null");
    }

    linklist_shuffle(ud->linklist);
    return 0;
}

// Linklist:swap(a, b)
static int Linklist_swap(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);
    long a = (long)luaL_checknumber(L, 2);
    long b = (long)luaL_checknumber(L, 3);

    if (!ud->linklist) {
        return luaL_error(L, "Linklist is null");
    }

    linklist_swap(ud->linklist, a, b);
    return 0;
}

// Linklist:pointer() -> pointer
static int Linklist_pointer(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->linklist);
    return 1;
}

// __len metamethod
static int Linklist_len(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);

    if (!ud->linklist) {
        return luaL_error(L, "Linklist is null");
    }

    t_atom_long size = linklist_getsize(ud->linklist);
    lua_pushnumber(L, size);
    return 1;
}

// __index metamethod (for linklist[index] access)
static int Linklist_index(lua_State* L) {
    // Check if key is a number (linklist index access)
    if (lua_isnumber(L, 2)) {
        // First try method lookup
        luaL_getmetatable(L, LINKLIST_MT);
        lua_pushvalue(L, 2);
        lua_rawget(L, -2);

        if (!lua_isnil(L, -1)) {
            // Found a method
            return 1;
        }
        lua_pop(L, 2);  // Pop nil and metatable

        // Try linklist access
        LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);
        if (!ud->linklist) {
            return luaL_error(L, "Linklist is null");
        }

        long index = (long)lua_tonumber(L, 2);
        t_atom_long size = linklist_getsize(ud->linklist);

        // Handle negative indices
        if (index < 0) {
            index = size + index;
        }

        if (index < 0 || index >= size) {
            return luaL_error(L, "List index out of range");
        }

        void* item = linklist_getindex(ud->linklist, index);

        if (!item) {
            lua_pushnil(L);
            return 1;
        }

        lua_pushnumber(L, (lua_Number)(intptr_t)item);
        return 1;
    }

    // Default to metatable lookup for methods
    luaL_getmetatable(L, LINKLIST_MT);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

// __gc metamethod
static int Linklist_gc(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);

    if (ud->linklist && ud->owns_linklist) {
        object_free(ud->linklist);
        ud->linklist = NULL;
    }

    return 0;
}

// __tostring metamethod
static int Linklist_tostring(lua_State* L) {
    LinklistUD* ud = (LinklistUD*)luaL_checkudata(L, 1, LINKLIST_MT);

    if (ud->linklist) {
        t_atom_long size = linklist_getsize(ud->linklist);
        lua_pushfstring(L, "Linklist(%p, size=%d)", ud->linklist, (int)size);
    } else {
        lua_pushstring(L, "Linklist(null)");
    }
    return 1;
}

// Register Linklist type
static void register_linklist_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, LINKLIST_MT);

    // Register methods
    lua_pushcfunction(L, Linklist_is_null);
    lua_setfield(L, -2, "is_null");

    lua_pushcfunction(L, Linklist_append);
    lua_setfield(L, -2, "append");

    lua_pushcfunction(L, Linklist_insertindex);
    lua_setfield(L, -2, "insertindex");

    lua_pushcfunction(L, Linklist_getindex);
    lua_setfield(L, -2, "getindex");

    lua_pushcfunction(L, Linklist_chuckindex);
    lua_setfield(L, -2, "chuckindex");

    lua_pushcfunction(L, Linklist_deleteindex);
    lua_setfield(L, -2, "deleteindex");

    lua_pushcfunction(L, Linklist_clear);
    lua_setfield(L, -2, "clear");

    lua_pushcfunction(L, Linklist_getsize);
    lua_setfield(L, -2, "getsize");

    lua_pushcfunction(L, Linklist_reverse);
    lua_setfield(L, -2, "reverse");

    lua_pushcfunction(L, Linklist_rotate);
    lua_setfield(L, -2, "rotate");

    lua_pushcfunction(L, Linklist_shuffle);
    lua_setfield(L, -2, "shuffle");

    lua_pushcfunction(L, Linklist_swap);
    lua_setfield(L, -2, "swap");

    lua_pushcfunction(L, Linklist_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Linklist_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Linklist_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, Linklist_len);
    lua_setfield(L, -2, "__len");

    lua_pushcfunction(L, Linklist_index);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);  // Pop metatable

    // Register constructor and wrap function in api module
    lua_getglobal(L, "api");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "api");
    }

    lua_pushcfunction(L, Linklist_new);
    lua_setfield(L, -2, "Linklist");

    lua_pushcfunction(L, Linklist_wrap);
    lua_setfield(L, -2, "linklist_wrap");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_LINKLIST_H

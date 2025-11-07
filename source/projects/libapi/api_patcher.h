// api_patcher.h
// Max patcher wrapper for luajit-max API
// Provides patcher manipulation and navigation

#ifndef LUAJIT_API_PATCHER_H
#define LUAJIT_API_PATCHER_H

#include "api_common.h"

// Metatable name for Patcher userdata
#define PATCHER_MT "Max.Patcher"

// Patcher userdata structure
typedef struct {
    t_object* patcher;
    bool owns_patcher;  // Whether we should free it (typically false for patchers)
} PatcherUD;

// Patcher constructor: Patcher()
static int Patcher_new(lua_State* L) {
    // Create userdata
    PatcherUD* ud = (PatcherUD*)lua_newuserdata(L, sizeof(PatcherUD));
    ud->patcher = NULL;
    ud->owns_patcher = false;  // Patchers are typically not owned

    // Set metatable
    luaL_getmetatable(L, PATCHER_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Patcher:wrap(pointer) - Wrap existing patcher pointer
static int Patcher_wrap(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);
    lua_Number ptr_num = luaL_checknumber(L, 2);

    if (ptr_num == 0) {
        return luaL_error(L, "Cannot wrap null pointer");
    }

    ud->patcher = (t_object*)(intptr_t)ptr_num;
    ud->owns_patcher = false;  // Never own wrapped patchers

    return 0;
}

// Patcher:is_null() - Check if patcher is null
static int Patcher_is_null(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);
    lua_pushboolean(L, ud->patcher == NULL);
    return 1;
}

// Patcher:newobject(text) - Create object from text string
static int Patcher_newobject(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);
    const char* text = luaL_checkstring(L, 2);

    if (!ud->patcher) {
        return luaL_error(L, "Patcher is null");
    }

    t_object* box = NULL;
    t_max_err err = object_method_typed(ud->patcher, gensym("newobject"), 1, NULL, NULL);

    // Try using newdefault with text
    t_atom a;
    atom_setsym(&a, gensym(text));
    box = (t_object*)object_method_typed(ud->patcher, gensym("newdefault"), 1, &a, NULL);

    if (!box) {
        lua_pushnil(L);
    } else {
        lua_pushnumber(L, (lua_Number)(intptr_t)box);
    }
    return 1;
}

// Patcher:locked(state) - Set or get locked state
static int Patcher_locked(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);

    if (!ud->patcher) {
        return luaL_error(L, "Patcher is null");
    }

    int nargs = lua_gettop(L);

    if (nargs >= 2) {
        // Set locked state
        long locked = lua_toboolean(L, 2);
        t_atom a;
        atom_setlong(&a, locked);
        object_method_typed(ud->patcher, gensym("locked"), 1, &a, NULL);
        return 0;
    } else {
        // Get locked state
        long locked = 0;
        t_atom result;
        object_method_typed(ud->patcher, gensym("locked"), 0, NULL, &result);
        if (atom_gettype(&result) == A_LONG) {
            locked = atom_getlong(&result);
        }
        lua_pushboolean(L, locked != 0);
        return 1;
    }
}

// Patcher:title(new_title) - Set or get patcher title
static int Patcher_title(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);

    if (!ud->patcher) {
        return luaL_error(L, "Patcher is null");
    }

    int nargs = lua_gettop(L);

    if (nargs >= 2) {
        // Set title
        const char* title = luaL_checkstring(L, 2);
        t_atom a;
        atom_setsym(&a, gensym(title));
        object_method_typed(ud->patcher, gensym("title"), 1, &a, NULL);
        return 0;
    } else {
        // Get title
        t_atom result;
        object_method_typed(ud->patcher, gensym("title"), 0, NULL, &result);
        if (atom_gettype(&result) == A_SYM) {
            t_symbol* title = atom_getsym(&result);
            lua_pushstring(L, title->s_name);
        } else {
            lua_pushnil(L);
        }
        return 1;
    }
}

// Patcher:rect(x, y, w, h) - Set or get patcher rectangle
static int Patcher_rect(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);

    if (!ud->patcher) {
        return luaL_error(L, "Patcher is null");
    }

    int nargs = lua_gettop(L);

    if (nargs >= 5) {
        // Set rect
        double x = luaL_checknumber(L, 2);
        double y = luaL_checknumber(L, 3);
        double w = luaL_checknumber(L, 4);
        double h = luaL_checknumber(L, 5);

        t_atom args[4];
        atom_setfloat(&args[0], x);
        atom_setfloat(&args[1], y);
        atom_setfloat(&args[2], w);
        atom_setfloat(&args[3], h);

        object_method_typed(ud->patcher, gensym("rect"), 4, args, NULL);
        return 0;
    } else {
        // Get rect
        t_atom result[4];
        long ac = 4;
        t_atom* av = result;

        object_method_typed(ud->patcher, gensym("rect"), 0, NULL, NULL);
        t_object* patcherview = NULL;
        object_obex_lookup(ud->patcher, gensym("#P"), &patcherview);

        if (patcherview) {
            t_rect rect;
            object_attr_get_rect(patcherview, gensym("rect"), &rect);
            lua_pushnumber(L, rect.x);
            lua_pushnumber(L, rect.y);
            lua_pushnumber(L, rect.width);
            lua_pushnumber(L, rect.height);
            return 4;
        }

        return 0;
    }
}

// Patcher:parentpatcher() - Get parent patcher
static int Patcher_parentpatcher(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);

    if (!ud->patcher) {
        return luaL_error(L, "Patcher is null");
    }

    t_object* parent = NULL;
    object_method_typed(ud->patcher, gensym("parentpatcher"), 0, NULL, NULL);
    parent = (t_object*)object_method(ud->patcher, gensym("parentpatcher"));

    if (!parent) {
        lua_pushnil(L);
    } else {
        // Create new Patcher userdata wrapping parent
        PatcherUD* parent_ud = (PatcherUD*)lua_newuserdata(L, sizeof(PatcherUD));
        parent_ud->patcher = parent;
        parent_ud->owns_patcher = false;
        luaL_getmetatable(L, PATCHER_MT);
        lua_setmetatable(L, -2);
    }
    return 1;
}

// Patcher:toppatcher() - Get top-level patcher
static int Patcher_toppatcher(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);

    if (!ud->patcher) {
        return luaL_error(L, "Patcher is null");
    }

    t_object* top = (t_object*)object_method(ud->patcher, gensym("toppatcher"));

    if (!top) {
        lua_pushnil(L);
    } else {
        // Create new Patcher userdata wrapping top
        PatcherUD* top_ud = (PatcherUD*)lua_newuserdata(L, sizeof(PatcherUD));
        top_ud->patcher = top;
        top_ud->owns_patcher = false;
        luaL_getmetatable(L, PATCHER_MT);
        lua_setmetatable(L, -2);
    }
    return 1;
}

// Patcher:dirty(state) - Set or get dirty flag
static int Patcher_dirty(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);

    if (!ud->patcher) {
        return luaL_error(L, "Patcher is null");
    }

    int nargs = lua_gettop(L);

    if (nargs >= 2) {
        // Set dirty state
        long dirty = lua_toboolean(L, 2);
        t_atom a;
        atom_setlong(&a, dirty);
        object_method_typed(ud->patcher, gensym("dirty"), 1, &a, NULL);
        return 0;
    } else {
        // Get dirty state - not typically available as getter
        lua_pushnil(L);
        return 1;
    }
}

// Patcher:count() - Count objects in patcher
static int Patcher_count(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);

    if (!ud->patcher) {
        return luaL_error(L, "Patcher is null");
    }

    long count = 0;
    t_atom result;
    object_method_typed(ud->patcher, gensym("count"), 0, NULL, &result);

    if (atom_gettype(&result) == A_LONG) {
        count = atom_getlong(&result);
    }

    lua_pushnumber(L, count);
    return 1;
}

// Patcher:name() - Get patcher name
static int Patcher_name(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);

    if (!ud->patcher) {
        return luaL_error(L, "Patcher is null");
    }

    t_symbol* name = (t_symbol*)object_method(ud->patcher, gensym("name"));

    if (name) {
        lua_pushstring(L, name->s_name);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// Patcher:filepath() - Get patcher file path
static int Patcher_filepath(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);

    if (!ud->patcher) {
        return luaL_error(L, "Patcher is null");
    }

    t_symbol* filepath = (t_symbol*)object_method(ud->patcher, gensym("filepath"));

    if (filepath) {
        lua_pushstring(L, filepath->s_name);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// Patcher:filename() - Get patcher filename
static int Patcher_filename(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);

    if (!ud->patcher) {
        return luaL_error(L, "Patcher is null");
    }

    t_symbol* filename = (t_symbol*)object_method(ud->patcher, gensym("filename"));

    if (filename) {
        lua_pushstring(L, filename->s_name);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// Patcher:pointer() - Get raw pointer value
static int Patcher_pointer(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->patcher);
    return 1;
}

// __gc metamethod (destructor)
static int Patcher_gc(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);
    // Typically don't free patchers as they're managed by Max
    if (ud->owns_patcher && ud->patcher) {
        object_free(ud->patcher);
        ud->patcher = NULL;
    }
    return 0;
}

// __tostring metamethod
static int Patcher_tostring(lua_State* L) {
    PatcherUD* ud = (PatcherUD*)luaL_checkudata(L, 1, PATCHER_MT);

    if (ud->patcher) {
        t_symbol* name = (t_symbol*)object_method(ud->patcher, gensym("name"));
        if (name) {
            lua_pushfstring(L, "Patcher(%s, %p)", name->s_name, ud->patcher);
        } else {
            lua_pushfstring(L, "Patcher(%p)", ud->patcher);
        }
    } else {
        lua_pushstring(L, "Patcher(null)");
    }
    return 1;
}

// Register Patcher type
static void register_patcher_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, PATCHER_MT);

    // Register methods
    lua_pushcfunction(L, Patcher_wrap);
    lua_setfield(L, -2, "wrap");

    lua_pushcfunction(L, Patcher_is_null);
    lua_setfield(L, -2, "is_null");

    lua_pushcfunction(L, Patcher_newobject);
    lua_setfield(L, -2, "newobject");

    lua_pushcfunction(L, Patcher_locked);
    lua_setfield(L, -2, "locked");

    lua_pushcfunction(L, Patcher_title);
    lua_setfield(L, -2, "title");

    lua_pushcfunction(L, Patcher_rect);
    lua_setfield(L, -2, "rect");

    lua_pushcfunction(L, Patcher_parentpatcher);
    lua_setfield(L, -2, "parentpatcher");

    lua_pushcfunction(L, Patcher_toppatcher);
    lua_setfield(L, -2, "toppatcher");

    lua_pushcfunction(L, Patcher_dirty);
    lua_setfield(L, -2, "dirty");

    lua_pushcfunction(L, Patcher_count);
    lua_setfield(L, -2, "count");

    lua_pushcfunction(L, Patcher_name);
    lua_setfield(L, -2, "name");

    lua_pushcfunction(L, Patcher_filepath);
    lua_setfield(L, -2, "filepath");

    lua_pushcfunction(L, Patcher_filename);
    lua_setfield(L, -2, "filename");

    lua_pushcfunction(L, Patcher_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Patcher_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Patcher_tostring);
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

    lua_pushcfunction(L, Patcher_new);
    lua_setfield(L, -2, "Patcher");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_PATCHER_H

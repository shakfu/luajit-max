// api_hashtab.h
// Hashtab wrapper for luajit-max API
// Provides access to Max hashtable objects

#ifndef LUAJIT_API_HASHTAB_H
#define LUAJIT_API_HASHTAB_H

#include "api_common.h"

// Metatable name for Hashtab userdata
#define HASHTAB_MT "Max.Hashtab"

// Hashtab userdata structure
typedef struct {
    t_hashtab* hashtab;
    bool owns_hashtab;
} HashtabUD;

// Hashtab constructor: Hashtab(slotcount)
static int Hashtab_new(lua_State* L) {
    long slotcount = 0;  // 0 = use default

    if (lua_gettop(L) >= 1 && lua_isnumber(L, 1)) {
        slotcount = (long)lua_tonumber(L, 1);
    }

    HashtabUD* ud = (HashtabUD*)lua_newuserdata(L, sizeof(HashtabUD));
    ud->hashtab = hashtab_new(slotcount);
    ud->owns_hashtab = true;

    luaL_getmetatable(L, HASHTAB_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Hashtab:wrap(pointer)
static int Hashtab_wrap(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);
    lua_Number ptr_num = luaL_checknumber(L, 2);

    if (ptr_num == 0) {
        return luaL_error(L, "Cannot wrap null pointer");
    }

    // Free old hashtab if we owned it
    if (ud->owns_hashtab && ud->hashtab) {
        object_free(ud->hashtab);
    }

    ud->hashtab = (t_hashtab*)(intptr_t)ptr_num;
    ud->owns_hashtab = false;

    return 0;
}

// Hashtab:is_null()
static int Hashtab_is_null(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);
    lua_pushboolean(L, ud->hashtab == NULL);
    return 1;
}

// Hashtab:store(key, value) or hashtab[key] = value
static int Hashtab_store(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);
    const char* key_str = luaL_checkstring(L, 2);

    if (!ud->hashtab) {
        return luaL_error(L, "Hashtab is null");
    }

    t_symbol* key = gensym(key_str);
    t_max_err err = MAX_ERR_GENERIC;

    // Store based on type
    if (lua_isnumber(L, 3)) {
        // Check if it's an integer or float
        double val = lua_tonumber(L, 3);
        double intpart;
        if (modf(val, &intpart) == 0.0) {
            // Store as long
            err = hashtab_storelong(ud->hashtab, key, (t_atom_long)val);
        } else {
            // Store float as long (hashtab doesn't support floats directly)
            err = hashtab_storelong(ud->hashtab, key, (t_atom_long)val);
        }
    } else if (lua_isstring(L, 3)) {
        err = hashtab_storesym(ud->hashtab, key, gensym(lua_tostring(L, 3)));
    } else if (lua_isuserdata(L, 3)) {
        // Try to get pointer from userdata
        void* ptr = lua_touserdata(L, 3);
        err = hashtab_store(ud->hashtab, key, (t_object*)ptr);
    } else {
        return luaL_error(L, "Unsupported value type for hashtab");
    }

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to store value in hashtab");
    }

    return 0;
}

// Hashtab:lookup(key, default)
static int Hashtab_lookup(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);
    const char* key_str = luaL_checkstring(L, 2);

    if (!ud->hashtab) {
        return luaL_error(L, "Hashtab is null");
    }

    t_symbol* key = gensym(key_str);

    t_object* obj_val = NULL;
    t_max_err err = hashtab_lookup(ud->hashtab, key, &obj_val);

    if (err != MAX_ERR_NONE) {
        // Key not found, return default
        if (lua_gettop(L) >= 3) {
            lua_pushvalue(L, 3);
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // Found - try different types
    t_atom_long long_val = 0;
    if (hashtab_lookuplong(ud->hashtab, key, &long_val) == MAX_ERR_NONE) {
        lua_pushnumber(L, long_val);
        return 1;
    }

    t_symbol* sym_val = NULL;
    if (hashtab_lookupsym(ud->hashtab, key, &sym_val) == MAX_ERR_NONE && sym_val) {
        lua_pushstring(L, sym_val->s_name);
        return 1;
    }

    // Return object pointer as number
    lua_pushnumber(L, (lua_Number)(intptr_t)obj_val);
    return 1;
}

// Hashtab:delete(key)
static int Hashtab_delete(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);
    const char* key_str = luaL_checkstring(L, 2);

    if (!ud->hashtab) {
        return luaL_error(L, "Hashtab is null");
    }

    t_symbol* key = gensym(key_str);
    t_max_err err = hashtab_delete(ud->hashtab, key);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to delete key '%s'", key_str);
    }

    return 0;
}

// Hashtab:clear()
static int Hashtab_clear(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);

    if (!ud->hashtab) {
        return luaL_error(L, "Hashtab is null");
    }

    t_max_err err = hashtab_clear(ud->hashtab);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to clear hashtab");
    }

    return 0;
}

// Hashtab:keys() -> {key1, key2, ...}
static int Hashtab_keys(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);

    if (!ud->hashtab) {
        return luaL_error(L, "Hashtab is null");
    }

    long keycount = 0;
    t_symbol** keys = NULL;

    t_max_err err = hashtab_getkeys(ud->hashtab, &keycount, &keys);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get hashtab keys");
    }

    // Create Lua table
    lua_createtable(L, keycount, 0);

    for (long i = 0; i < keycount; i++) {
        if (keys[i]) {
            lua_pushstring(L, keys[i]->s_name);
        } else {
            lua_pushstring(L, "");
        }
        lua_rawseti(L, -2, i + 1);
    }

    // Note: keys array is owned by hashtab, don't free

    return 1;
}

// Hashtab:has_key(key) -> bool
static int Hashtab_has_key(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);
    const char* key_str = luaL_checkstring(L, 2);

    if (!ud->hashtab) {
        return luaL_error(L, "Hashtab is null");
    }

    t_symbol* key = gensym(key_str);

    t_object* val = NULL;
    t_max_err err = hashtab_lookup(ud->hashtab, key, &val);

    lua_pushboolean(L, err == MAX_ERR_NONE);
    return 1;
}

// Hashtab:getsize() -> size
static int Hashtab_getsize(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);

    if (!ud->hashtab) {
        return luaL_error(L, "Hashtab is null");
    }

    t_atom_long size = hashtab_getsize(ud->hashtab);
    lua_pushnumber(L, size);
    return 1;
}

// Hashtab:pointer() -> pointer
static int Hashtab_pointer(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->hashtab);
    return 1;
}

// __index metamethod for hashtab[key] access
static int Hashtab_index(lua_State* L) {
    // Check if key is a string (hashtab key access)
    if (lua_isstring(L, 2)) {
        // First try method lookup
        luaL_getmetatable(L, HASHTAB_MT);
        lua_pushvalue(L, 2);
        lua_rawget(L, -2);

        if (!lua_isnil(L, -1)) {
            // Found a method
            return 1;
        }
        lua_pop(L, 2);  // Pop nil and metatable

        // Try hashtab lookup
        HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);
        if (!ud->hashtab) {
            return luaL_error(L, "Hashtab is null");
        }

        const char* key_str = lua_tostring(L, 2);
        t_symbol* key = gensym(key_str);

        t_object* obj_val = NULL;
        t_max_err err = hashtab_lookup(ud->hashtab, key, &obj_val);

        if (err != MAX_ERR_NONE) {
            lua_pushnil(L);
            return 1;
        }

        // Try different types
        t_atom_long long_val = 0;
        if (hashtab_lookuplong(ud->hashtab, key, &long_val) == MAX_ERR_NONE) {
            lua_pushnumber(L, long_val);
            return 1;
        }

        t_symbol* sym_val = NULL;
        if (hashtab_lookupsym(ud->hashtab, key, &sym_val) == MAX_ERR_NONE && sym_val) {
            lua_pushstring(L, sym_val->s_name);
            return 1;
        }

        lua_pushnumber(L, (lua_Number)(intptr_t)obj_val);
        return 1;
    }

    // Default to metatable lookup for methods
    luaL_getmetatable(L, HASHTAB_MT);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

// __newindex metamethod for hashtab[key] = value
static int Hashtab_newindex(lua_State* L) {
    return Hashtab_store(L);
}

// __len metamethod
static int Hashtab_len(lua_State* L) {
    return Hashtab_getsize(L);
}

// __gc metamethod
static int Hashtab_gc(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);
    if (ud->owns_hashtab && ud->hashtab) {
        object_free(ud->hashtab);
        ud->hashtab = NULL;
    }
    return 0;
}

// __tostring metamethod
static int Hashtab_tostring(lua_State* L) {
    HashtabUD* ud = (HashtabUD*)luaL_checkudata(L, 1, HASHTAB_MT);

    if (ud->hashtab) {
        t_atom_long size = hashtab_getsize(ud->hashtab);
        lua_pushfstring(L, "Hashtab(size=%d)", (int)size);
    } else {
        lua_pushstring(L, "Hashtab(null)");
    }
    return 1;
}

// Register Hashtab type
static void register_hashtab_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, HASHTAB_MT);

    // Register methods
    lua_pushcfunction(L, Hashtab_wrap);
    lua_setfield(L, -2, "wrap");

    lua_pushcfunction(L, Hashtab_is_null);
    lua_setfield(L, -2, "is_null");

    lua_pushcfunction(L, Hashtab_store);
    lua_setfield(L, -2, "store");

    lua_pushcfunction(L, Hashtab_lookup);
    lua_setfield(L, -2, "lookup");

    lua_pushcfunction(L, Hashtab_delete);
    lua_setfield(L, -2, "delete");

    lua_pushcfunction(L, Hashtab_clear);
    lua_setfield(L, -2, "clear");

    lua_pushcfunction(L, Hashtab_keys);
    lua_setfield(L, -2, "keys");

    lua_pushcfunction(L, Hashtab_has_key);
    lua_setfield(L, -2, "has_key");

    lua_pushcfunction(L, Hashtab_getsize);
    lua_setfield(L, -2, "getsize");

    lua_pushcfunction(L, Hashtab_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Hashtab_index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Hashtab_newindex);
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, Hashtab_len);
    lua_setfield(L, -2, "__len");

    lua_pushcfunction(L, Hashtab_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Hashtab_tostring);
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

    lua_pushcfunction(L, Hashtab_new);
    lua_setfield(L, -2, "Hashtab");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_HASHTAB_H

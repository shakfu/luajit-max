// api_dictionary.h
// Dictionary wrapper for luajit-max API
// Provides access to Max dictionary objects for structured data storage

#ifndef LUAJIT_API_DICTIONARY_H
#define LUAJIT_API_DICTIONARY_H

#include "api_common.h"
#include "ext_dictionary.h"
#include "ext_dictobj.h"

// Metatable name for Dictionary userdata
#define DICTIONARY_MT "Max.Dictionary"

// Dictionary userdata structure
typedef struct {
    t_dictionary* dict;
    bool owns_dict;  // Whether we should free it
} DictionaryUD;

// Dictionary constructor: Dictionary()
static int Dictionary_new(lua_State* L) {
    // Create userdata
    DictionaryUD* ud = (DictionaryUD*)lua_newuserdata(L, sizeof(DictionaryUD));
    ud->dict = dictionary_new();
    ud->owns_dict = true;

    // Set metatable
    luaL_getmetatable(L, DICTIONARY_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Dictionary:getlong(key, default) - Get long value
static int Dictionary_getlong(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    t_symbol* key = gensym(key_str);

    t_atom_long value = 0;
    t_max_err err;

    if (lua_gettop(L) >= 3) {
        t_atom_long def = (t_atom_long)luaL_checknumber(L, 3);
        err = dictionary_getdeflong(ud->dict, key, &value, def);
    } else {
        err = dictionary_getlong(ud->dict, key, &value);
    }

    if (err != MAX_ERR_NONE && lua_gettop(L) < 3) {
        return luaL_error(L, "Key '%s' not found in dictionary", key_str);
    }

    lua_pushnumber(L, value);
    return 1;
}

// Dictionary:getfloat(key, default) - Get float value
static int Dictionary_getfloat(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    t_symbol* key = gensym(key_str);

    double value = 0.0;
    t_max_err err;

    if (lua_gettop(L) >= 3) {
        double def = luaL_checknumber(L, 3);
        err = dictionary_getdeffloat(ud->dict, key, &value, def);
    } else {
        err = dictionary_getfloat(ud->dict, key, &value);
    }

    if (err != MAX_ERR_NONE && lua_gettop(L) < 3) {
        return luaL_error(L, "Key '%s' not found in dictionary", key_str);
    }

    lua_pushnumber(L, value);
    return 1;
}

// Dictionary:getstring(key, default) - Get string value
static int Dictionary_getstring(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    t_symbol* key = gensym(key_str);

    const char* value = NULL;
    t_max_err err = dictionary_getstring(ud->dict, key, &value);

    if (err != MAX_ERR_NONE) {
        if (lua_gettop(L) >= 3) {
            value = luaL_checkstring(L, 3);
        } else {
            return luaL_error(L, "Key '%s' not found in dictionary", key_str);
        }
    }

    lua_pushstring(L, value ? value : "");
    return 1;
}

// Dictionary:getsym(key, default) - Get symbol value
static int Dictionary_getsym(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    t_symbol* key = gensym(key_str);

    t_symbol* value = NULL;
    t_max_err err = dictionary_getsym(ud->dict, key, &value);

    if (err != MAX_ERR_NONE) {
        if (lua_gettop(L) >= 3) {
            const char* def = luaL_checkstring(L, 3);
            value = gensym(def);
        } else {
            return luaL_error(L, "Key '%s' not found in dictionary", key_str);
        }
    }

    lua_pushstring(L, value ? value->s_name : "");
    return 1;
}

// Dictionary:get(key, default) - Generic get with type detection
static int Dictionary_get(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    t_symbol* key = gensym(key_str);

    // Check if key exists
    if (!dictionary_hasentry(ud->dict, key)) {
        if (lua_gettop(L) >= 3) {
            lua_pushvalue(L, 3);  // Return default
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // Try to get as atom first
    t_atom a;
    t_max_err err = dictionary_getatom(ud->dict, key, &a);

    if (err == MAX_ERR_NONE) {
        lua_pushatomvalue(L, &a);
        return 1;
    }

    // Check if it's a string
    if (dictionary_entryisstring(ud->dict, key)) {
        const char* value = NULL;
        err = dictionary_getstring(ud->dict, key, &value);
        if (err == MAX_ERR_NONE) {
            lua_pushstring(L, value);
            return 1;
        }
    }

    // Check if it's an atomarray
    if (dictionary_entryisatomarray(ud->dict, key)) {
        t_object* atomarray_obj = NULL;
        err = dictionary_getatomarray(ud->dict, key, &atomarray_obj);
        if (err == MAX_ERR_NONE && atomarray_obj) {
            // Convert atomarray to Lua table
            t_atomarray* aa = (t_atomarray*)atomarray_obj;
            long ac = 0;
            t_atom* av = NULL;
            atomarray_getatoms(aa, &ac, &av);

            lua_createtable(L, (int)ac, 0);
            for (long i = 0; i < ac; i++) {
                lua_pushatomvalue(L, &av[i]);
                lua_rawseti(L, -2, i + 1);
            }
            return 1;
        }
    }

    // Check if it's a nested dictionary
    if (dictionary_entryisdictionary(ud->dict, key)) {
        t_object* sub_dict = NULL;
        err = dictionary_getdictionary(ud->dict, key, &sub_dict);
        if (err == MAX_ERR_NONE && sub_dict) {
            // Create new DictionaryUD wrapper (non-owning)
            DictionaryUD* new_ud = (DictionaryUD*)lua_newuserdata(L, sizeof(DictionaryUD));
            new_ud->dict = (t_dictionary*)sub_dict;
            new_ud->owns_dict = false;  // Parent owns it

            luaL_getmetatable(L, DICTIONARY_MT);
            lua_setmetatable(L, -2);
            return 1;
        }
    }

    lua_pushnil(L);
    return 1;
}

// Dictionary:setlong(key, value) - Set long value
static int Dictionary_setlong(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    t_atom_long value = (t_atom_long)luaL_checknumber(L, 3);

    t_symbol* key = gensym(key_str);
    t_max_err err = dictionary_appendlong(ud->dict, key, value);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to set long value");
    }

    return 0;
}

// Dictionary:setfloat(key, value) - Set float value
static int Dictionary_setfloat(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    double value = luaL_checknumber(L, 3);

    t_symbol* key = gensym(key_str);
    t_max_err err = dictionary_appendfloat(ud->dict, key, value);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to set float value");
    }

    return 0;
}

// Dictionary:setstring(key, value) - Set string value
static int Dictionary_setstring(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    const char* value = luaL_checkstring(L, 3);

    t_symbol* key = gensym(key_str);
    t_max_err err = dictionary_appendstring(ud->dict, key, value);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to set string value");
    }

    return 0;
}

// Dictionary:setsym(key, value) - Set symbol value
static int Dictionary_setsym(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    const char* value_str = luaL_checkstring(L, 3);

    t_symbol* key = gensym(key_str);
    t_symbol* value = gensym(value_str);
    t_max_err err = dictionary_appendsym(ud->dict, key, value);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to set symbol value");
    }

    return 0;
}

// Dictionary:set(key, value) - Generic set with type detection
static int Dictionary_set(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    t_symbol* key = gensym(key_str);
    t_max_err err = MAX_ERR_GENERIC;

    int value_type = lua_type(L, 3);

    switch (value_type) {
        case LUA_TNUMBER: {
            double d = lua_tonumber(L, 3);
            double intpart;
            if (modf(d, &intpart) == 0.0) {
                err = dictionary_appendlong(ud->dict, key, (t_atom_long)d);
            } else {
                err = dictionary_appendfloat(ud->dict, key, d);
            }
            break;
        }
        case LUA_TSTRING: {
            const char* str = lua_tostring(L, 3);
            err = dictionary_appendstring(ud->dict, key, str);
            break;
        }
        case LUA_TBOOLEAN: {
            int b = lua_toboolean(L, 3);
            err = dictionary_appendlong(ud->dict, key, b);
            break;
        }
        case LUA_TTABLE: {
            // Convert Lua table to atoms
            long table_len = (long)lua_objlen(L, 3);
            t_atom* atoms = (t_atom*)sysmem_newptr(table_len * sizeof(t_atom));
            if (!atoms) {
                return luaL_error(L, "Failed to allocate memory");
            }

            for (long i = 0; i < table_len; i++) {
                lua_rawgeti(L, 3, i + 1);
                if (!lua_toatom(L, -1, &atoms[i])) {
                    sysmem_freeptr(atoms);
                    return luaL_error(L, "Table item %d cannot be converted to atom", (int)(i + 1));
                }
                lua_pop(L, 1);
            }

            err = dictionary_appendatoms(ud->dict, key, table_len, atoms);
            sysmem_freeptr(atoms);
            break;
        }
        case LUA_TUSERDATA: {
            // Check if it's a Dictionary
            if (lua_getmetatable(L, 3)) {
                luaL_getmetatable(L, DICTIONARY_MT);
                if (lua_rawequal(L, -1, -2)) {
                    DictionaryUD* sub_ud = (DictionaryUD*)lua_touserdata(L, 3);
                    err = dictionary_appenddictionary(ud->dict, key, (t_object*)sub_ud->dict);
                    sub_ud->owns_dict = false;  // Parent now owns it
                }
                lua_pop(L, 2);  // Pop both metatables
            }
            break;
        }
        default:
            return luaL_error(L, "Unsupported value type for dictionary");
    }

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to set dictionary value");
    }

    return 0;
}

// Dictionary:has(key) - Check if key exists
static int Dictionary_has(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    t_symbol* key = gensym(key_str);

    long has = dictionary_hasentry(ud->dict, key);
    lua_pushboolean(L, has != 0);
    return 1;
}

// Dictionary:delete(key) - Delete key
static int Dictionary_delete(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* key_str = luaL_checkstring(L, 2);
    t_symbol* key = gensym(key_str);

    t_max_err err = dictionary_deleteentry(ud->dict, key);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to delete key '%s'", key_str);
    }

    return 0;
}

// Dictionary:clear() - Clear all entries
static int Dictionary_clear(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);

    t_max_err err = dictionary_clear(ud->dict);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to clear dictionary");
    }

    return 0;
}

// Dictionary:keys() - Get list of keys
static int Dictionary_keys(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);

    long numkeys = 0;
    t_symbol** keys = NULL;

    t_max_err err = dictionary_getkeys(ud->dict, &numkeys, &keys);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get dictionary keys");
    }

    // Create Lua table
    lua_createtable(L, (int)numkeys, 0);

    for (long i = 0; i < numkeys; i++) {
        lua_pushstring(L, keys[i]->s_name);
        lua_rawseti(L, -2, i + 1);
    }

    if (keys) {
        dictionary_freekeys(ud->dict, numkeys, keys);
    }

    return 1;
}

// Dictionary:size() - Get entry count
static int Dictionary_size(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    t_atom_long count = dictionary_getentrycount(ud->dict);
    lua_pushnumber(L, count);
    return 1;
}

// Dictionary:read(filename, path) - Read from file
static int Dictionary_read(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* filename = luaL_checkstring(L, 2);
    short path = (short)luaL_checknumber(L, 3);

    // Clear existing dictionary and read new one
    if (ud->owns_dict && ud->dict) {
        object_free(ud->dict);
    }

    t_dictionary* new_dict = NULL;
    t_max_err err = dictionary_read(filename, path, &new_dict);

    if (err != MAX_ERR_NONE || !new_dict) {
        return luaL_error(L, "Failed to read dictionary from file '%s'", filename);
    }

    ud->dict = new_dict;
    ud->owns_dict = true;

    return 0;
}

// Dictionary:write(filename, path) - Write to file
static int Dictionary_write(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    const char* filename = luaL_checkstring(L, 2);
    short path = (short)luaL_checknumber(L, 3);

    t_max_err err = dictionary_write(ud->dict, filename, path);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to write dictionary to file '%s'", filename);
    }

    return 0;
}

// Dictionary:dump(recurse, console) - Dump to console
static int Dictionary_dump(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);

    long recurse = 1;
    long console = 0;

    if (lua_gettop(L) >= 2) {
        recurse = lua_toboolean(L, 2);
    }
    if (lua_gettop(L) >= 3) {
        console = lua_toboolean(L, 3);
    }

    t_max_err err = dictionary_dump(ud->dict, recurse, console);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to dump dictionary");
    }

    return 0;
}

// Dictionary:pointer() - Get raw pointer value
static int Dictionary_pointer(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->dict);
    return 1;
}

// __gc metamethod (destructor)
static int Dictionary_gc(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    if (ud->owns_dict && ud->dict) {
        object_free(ud->dict);
        ud->dict = NULL;
    }
    return 0;
}

// __tostring metamethod
static int Dictionary_tostring(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    t_atom_long count = dictionary_getentrycount(ud->dict);
    lua_pushfstring(L, "Dictionary(entries=%d)", (int)count);
    return 1;
}

// __len metamethod
static int Dictionary_len(lua_State* L) {
    DictionaryUD* ud = (DictionaryUD*)luaL_checkudata(L, 1, DICTIONARY_MT);
    t_atom_long count = dictionary_getentrycount(ud->dict);
    lua_pushnumber(L, count);
    return 1;
}

// __index metamethod (for dict[key] access and method lookup)
static int Dictionary_index(lua_State* L) {
    // Check if it's string key access
    if (lua_isstring(L, 2)) {
        const char* key_str = lua_tostring(L, 2);

        // First check if it's a method name
        luaL_getmetatable(L, DICTIONARY_MT);
        lua_pushvalue(L, 2);
        lua_rawget(L, -2);

        if (!lua_isnil(L, -1)) {
            // It's a method
            return 1;
        }
        lua_pop(L, 2);  // Pop nil and metatable

        // Not a method, treat as dictionary key access
        return Dictionary_get(L);
    }

    // For non-string keys, look up method in metatable
    luaL_getmetatable(L, DICTIONARY_MT);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

// __newindex metamethod (for dict[key] = value)
static int Dictionary_newindex(lua_State* L) {
    if (lua_isstring(L, 2)) {
        return Dictionary_set(L);
    }
    return luaL_error(L, "Dictionary keys must be strings");
}

// Register Dictionary type
static void register_dictionary_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, DICTIONARY_MT);

    // Register methods
    lua_pushcfunction(L, Dictionary_get);
    lua_setfield(L, -2, "get");

    lua_pushcfunction(L, Dictionary_getlong);
    lua_setfield(L, -2, "getlong");

    lua_pushcfunction(L, Dictionary_getfloat);
    lua_setfield(L, -2, "getfloat");

    lua_pushcfunction(L, Dictionary_getstring);
    lua_setfield(L, -2, "getstring");

    lua_pushcfunction(L, Dictionary_getsym);
    lua_setfield(L, -2, "getsym");

    lua_pushcfunction(L, Dictionary_set);
    lua_setfield(L, -2, "set");

    lua_pushcfunction(L, Dictionary_setlong);
    lua_setfield(L, -2, "setlong");

    lua_pushcfunction(L, Dictionary_setfloat);
    lua_setfield(L, -2, "setfloat");

    lua_pushcfunction(L, Dictionary_setstring);
    lua_setfield(L, -2, "setstring");

    lua_pushcfunction(L, Dictionary_setsym);
    lua_setfield(L, -2, "setsym");

    lua_pushcfunction(L, Dictionary_has);
    lua_setfield(L, -2, "has");

    lua_pushcfunction(L, Dictionary_delete);
    lua_setfield(L, -2, "delete");

    lua_pushcfunction(L, Dictionary_clear);
    lua_setfield(L, -2, "clear");

    lua_pushcfunction(L, Dictionary_keys);
    lua_setfield(L, -2, "keys");

    lua_pushcfunction(L, Dictionary_size);
    lua_setfield(L, -2, "size");

    lua_pushcfunction(L, Dictionary_read);
    lua_setfield(L, -2, "read");

    lua_pushcfunction(L, Dictionary_write);
    lua_setfield(L, -2, "write");

    lua_pushcfunction(L, Dictionary_dump);
    lua_setfield(L, -2, "dump");

    lua_pushcfunction(L, Dictionary_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Dictionary_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Dictionary_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, Dictionary_len);
    lua_setfield(L, -2, "__len");

    lua_pushcfunction(L, Dictionary_index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Dictionary_newindex);
    lua_setfield(L, -2, "__newindex");

    lua_pop(L, 1);  // Pop metatable

    // Register constructor in api module
    lua_getglobal(L, "api");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "api");
    }

    lua_pushcfunction(L, Dictionary_new);
    lua_setfield(L, -2, "Dictionary");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_DICTIONARY_H

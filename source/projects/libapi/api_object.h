// api_object.h
// Generic Max object wrapper for luajit-max API
// Provides dynamic object creation and manipulation

#ifndef LUAJIT_API_OBJECT_H
#define LUAJIT_API_OBJECT_H

#include "api_common.h"

// Metatable name for Object userdata
#define OBJECT_MT "Max.Object"

// Object userdata structure
typedef struct {
    t_object* obj;
    bool owns_obj;  // Whether we should free it
} ObjectUD;

// Object constructor: Object()
static int Object_new(lua_State* L) {
    // Create userdata
    ObjectUD* ud = (ObjectUD*)lua_newuserdata(L, sizeof(ObjectUD));
    ud->obj = NULL;
    ud->owns_obj = false;

    // Set metatable
    luaL_getmetatable(L, OBJECT_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Object:create(classname, ...) - Create Max object
static int Object_create(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);
    const char* classname_str = luaL_checkstring(L, 2);
    t_symbol* classname = gensym(classname_str);

    // Convert Lua args to atoms
    int num_args = lua_gettop(L) - 2;
    t_atom* atoms = NULL;

    if (num_args > 0) {
        atoms = (t_atom*)sysmem_newptr(num_args * sizeof(t_atom));
        if (!atoms) {
            return luaL_error(L, "Failed to allocate memory for arguments");
        }

        for (int i = 0; i < num_args; i++) {
            if (!lua_toatom(L, i + 3, &atoms[i])) {
                sysmem_freeptr(atoms);
                return luaL_error(L, "Argument %d cannot be converted to atom", i + 1);
            }
        }
    }

    // Create object
    t_object* obj = (t_object*)object_new_typed(CLASS_BOX, classname, num_args, atoms);

    if (atoms) {
        sysmem_freeptr(atoms);
    }

    if (!obj) {
        return luaL_error(L, "Failed to create object of class '%s'", classname_str);
    }

    // Free old object if we owned it
    if (ud->owns_obj && ud->obj) {
        object_free(ud->obj);
    }

    ud->obj = obj;
    ud->owns_obj = true;

    return 0;
}

// Object:wrap(pointer) - Wrap existing object pointer (no ownership)
static int Object_wrap(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);
    lua_Number ptr_num = luaL_checknumber(L, 2);

    if (ptr_num == 0) {
        return luaL_error(L, "Cannot wrap null pointer");
    }

    // Free old object if we owned it
    if (ud->owns_obj && ud->obj) {
        object_free(ud->obj);
    }

    ud->obj = (t_object*)(intptr_t)ptr_num;
    ud->owns_obj = false;  // Don't free wrapped objects

    return 0;
}

// Object:free() - Free object
static int Object_free_method(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);

    if (ud->owns_obj && ud->obj) {
        object_free(ud->obj);
        ud->obj = NULL;
        ud->owns_obj = false;
    }

    return 0;
}

// Object:is_null() - Check if object is null
static int Object_is_null(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);
    lua_pushboolean(L, ud->obj == NULL);
    return 1;
}

// Object:classname() - Get class name
static int Object_classname(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);

    if (!ud->obj) {
        return luaL_error(L, "Object is null");
    }

    t_symbol* classname = object_classname(ud->obj);
    lua_pushstring(L, classname->s_name);
    return 1;
}

// Object:method(name, ...) - Call method on object
static int Object_method(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);
    const char* method_name = luaL_checkstring(L, 2);

    if (!ud->obj) {
        return luaL_error(L, "Object is null");
    }

    t_symbol* method_sym = gensym(method_name);

    // Convert Lua args to atoms
    int num_args = lua_gettop(L) - 2;
    t_atom* atoms = NULL;
    t_atom result;

    if (num_args > 0) {
        atoms = (t_atom*)sysmem_newptr(num_args * sizeof(t_atom));
        if (!atoms) {
            return luaL_error(L, "Failed to allocate memory for arguments");
        }

        for (int i = 0; i < num_args; i++) {
            if (!lua_toatom(L, i + 3, &atoms[i])) {
                sysmem_freeptr(atoms);
                return luaL_error(L, "Argument %d cannot be converted to atom", i + 1);
            }
        }
    }

    // Call method
    atom_setsym(&result, gensym(""));  // Initialize result
    t_max_err err = object_method_typed(ud->obj, method_sym, num_args, atoms, &result);

    if (atoms) {
        sysmem_freeptr(atoms);
    }

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Method '%s' failed with error %d", method_name, (int)err);
    }

    // Convert result to Lua
    lua_pushatomvalue(L, &result);
    return 1;
}

// Object:getattr(name) - Get attribute value
static int Object_getattr(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);
    const char* attr_name = luaL_checkstring(L, 2);

    if (!ud->obj) {
        return luaL_error(L, "Object is null");
    }

    t_symbol* attr_sym = gensym(attr_name);

    long ac = 0;
    t_atom* av = NULL;

    t_max_err err = object_attr_getvalueof(ud->obj, attr_sym, &ac, &av);

    if (err != MAX_ERR_NONE || ac == 0 || !av) {
        lua_pushnil(L);
        return 1;
    }

    // If single value, return it directly
    if (ac == 1) {
        lua_pushatomvalue(L, &av[0]);
    } else {
        // Multiple values, return as table
        lua_createtable(L, (int)ac, 0);
        for (long i = 0; i < ac; i++) {
            lua_pushatomvalue(L, &av[i]);
            lua_rawseti(L, -2, i + 1);
        }
    }

    return 1;
}

// Object:setattr(name, value) - Set attribute value
static int Object_setattr(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);
    const char* attr_name = luaL_checkstring(L, 2);

    if (!ud->obj) {
        return luaL_error(L, "Object is null");
    }

    t_symbol* attr_sym = gensym(attr_name);
    t_max_err err = MAX_ERR_GENERIC;
    int value_type = lua_type(L, 3);

    // Handle different value types
    switch (value_type) {
        case LUA_TNUMBER: {
            double d = lua_tonumber(L, 3);
            double intpart;
            if (modf(d, &intpart) == 0.0) {
                err = object_attr_setlong(ud->obj, attr_sym, (long)d);
            } else {
                err = object_attr_setfloat(ud->obj, attr_sym, d);
            }
            break;
        }
        case LUA_TSTRING: {
            const char* str = lua_tostring(L, 3);
            err = object_attr_setsym(ud->obj, attr_sym, gensym(str));
            break;
        }
        case LUA_TTABLE: {
            // Convert table to atoms
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

            err = object_attr_setvalueof(ud->obj, attr_sym, table_len, atoms);
            sysmem_freeptr(atoms);
            break;
        }
        default:
            return luaL_error(L, "Unsupported value type for attribute");
    }

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to set attribute '%s'", attr_name);
    }

    return 0;
}

// Object:attrnames() - Get list of attribute names
static int Object_attrnames(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);

    if (!ud->obj) {
        return luaL_error(L, "Object is null");
    }

    long numattrs = 0;
    t_symbol** attrnames = NULL;

    t_max_err err = object_attr_getnames(ud->obj, &numattrs, &attrnames);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get attribute names");
    }

    // Create Lua table
    lua_createtable(L, (int)numattrs, 0);

    for (long i = 0; i < numattrs; i++) {
        lua_pushstring(L, attrnames[i]->s_name);
        lua_rawseti(L, -2, i + 1);
    }

    if (attrnames) {
        sysmem_freeptr(attrnames);
    }

    return 1;
}

// Object:pointer() - Get raw pointer value
static int Object_pointer(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->obj);
    return 1;
}

// __gc metamethod (destructor)
static int Object_gc(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);
    if (ud->owns_obj && ud->obj) {
        object_free(ud->obj);
        ud->obj = NULL;
    }
    return 0;
}

// __tostring metamethod
static int Object_tostring(lua_State* L) {
    ObjectUD* ud = (ObjectUD*)luaL_checkudata(L, 1, OBJECT_MT);

    if (ud->obj) {
        t_symbol* classname = object_classname(ud->obj);
        lua_pushfstring(L, "Object(%s, %p)", classname->s_name, ud->obj);
    } else {
        lua_pushstring(L, "Object(null)");
    }
    return 1;
}

// Register Object type
static void register_object_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, OBJECT_MT);

    // Register methods
    lua_pushcfunction(L, Object_create);
    lua_setfield(L, -2, "create");

    lua_pushcfunction(L, Object_wrap);
    lua_setfield(L, -2, "wrap");

    lua_pushcfunction(L, Object_free_method);
    lua_setfield(L, -2, "free");

    lua_pushcfunction(L, Object_is_null);
    lua_setfield(L, -2, "is_null");

    lua_pushcfunction(L, Object_classname);
    lua_setfield(L, -2, "classname");

    lua_pushcfunction(L, Object_method);
    lua_setfield(L, -2, "method");

    lua_pushcfunction(L, Object_getattr);
    lua_setfield(L, -2, "getattr");

    lua_pushcfunction(L, Object_setattr);
    lua_setfield(L, -2, "setattr");

    lua_pushcfunction(L, Object_attrnames);
    lua_setfield(L, -2, "attrnames");

    lua_pushcfunction(L, Object_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Object_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Object_tostring);
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

    lua_pushcfunction(L, Object_new);
    lua_setfield(L, -2, "Object");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_OBJECT_H

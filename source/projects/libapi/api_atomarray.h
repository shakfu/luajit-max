// api_atomarray.h
// AtomArray wrapper for luajit-max API

#ifndef LUAJIT_API_ATOMARRAY_H
#define LUAJIT_API_ATOMARRAY_H

#include "api_common.h"
#include "api_atom.h"

// Metatable name for AtomArray userdata
#define ATOMARRAY_MT "Max.AtomArray"

// AtomArray userdata structure
typedef struct {
    t_atomarray* atomarray;
    bool owns_atomarray;  // Whether we should free it
} AtomArrayUD;

// AtomArray constructor: AtomArray() or AtomArray({...})
static int AtomArray_new(lua_State* L) {
    int nargs = lua_gettop(L);

    // Create userdata
    AtomArrayUD* ud = (AtomArrayUD*)lua_newuserdata(L, sizeof(AtomArrayUD));
    ud->atomarray = atomarray_new(0, NULL);  // Empty array
    ud->owns_atomarray = true;

    // Set metatable
    luaL_getmetatable(L, ATOMARRAY_MT);
    lua_setmetatable(L, -2);

    // If table provided, initialize from it
    if (nargs >= 1 && lua_istable(L, 1)) {
        long table_len = (long)lua_objlen(L, 1);

        // Allocate temporary atom array
        t_atom* atoms = (t_atom*)sysmem_newptr(table_len * sizeof(t_atom));
        if (!atoms) {
            return luaL_error(L, "Failed to allocate memory for atoms");
        }

        // Convert Lua table to atoms
        for (long i = 0; i < table_len; i++) {
            lua_rawgeti(L, 1, i + 1);
            if (!lua_toatom(L, -1, &atoms[i])) {
                sysmem_freeptr(atoms);
                return luaL_error(L, "Table item %d cannot be converted to atom", (int)(i + 1));
            }
            lua_pop(L, 1);
        }

        // Set atoms in atomarray
        atomarray_setatoms(ud->atomarray, table_len, atoms);
        sysmem_freeptr(atoms);
    }

    return 1;
}

// AtomArray.getsize() or #atomarray
static int AtomArray_len(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);
    t_atom_long size = atomarray_getsize(ud->atomarray);
    lua_pushnumber(L, size);
    return 1;
}

// AtomArray[index] - get item
static int AtomArray_getitem(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);
    long index = (long)luaL_checknumber(L, 2);
    long size = atomarray_getsize(ud->atomarray);

    // Handle negative indices
    if (index < 0) {
        index += size;
    }

    // Lua tables are 1-indexed, convert to 0-indexed
    if (index >= 1 && index <= size) {
        index = index - 1;
    } else if (index < 0 || index >= size) {
        return luaL_error(L, "AtomArray index out of range");
    }

    // Get atom at index
    t_atom a;
    t_max_err err = atomarray_getindex(ud->atomarray, index, &a);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get atom at index %d", (int)index);
    }

    // Push atom value to Lua
    lua_pushatomvalue(L, &a);

    return 1;
}

// AtomArray[index] = value - set item
static int AtomArray_setitem(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);
    long index = (long)luaL_checknumber(L, 2);
    long size = atomarray_getsize(ud->atomarray);

    // Handle negative indices
    if (index < 0) {
        index += size;
    }

    // Lua tables are 1-indexed, convert to 0-indexed
    if (index >= 1 && index <= size) {
        index = index - 1;
    } else if (index < 0 || index >= size) {
        return luaL_error(L, "AtomArray index out of range");
    }

    // Convert value to atom
    t_atom a;
    if (!lua_toatom(L, 3, &a)) {
        return luaL_error(L, "Value cannot be converted to atom");
    }

    // atomarray_setindex is not exported, so we rebuild the array
    long ac = 0;
    t_atom* av = NULL;
    t_max_err err = atomarray_getatoms(ud->atomarray, &ac, &av);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get atoms");
    }

    // Modify the atom at index
    av[index] = a;

    // Rebuild atomarray with modified atoms
    err = atomarray_setatoms(ud->atomarray, ac, av);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to set atoms");
    }

    return 0;
}

// AtomArray:append(value)
static int AtomArray_append(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);

    t_atom a;
    if (!lua_toatom(L, 2, &a)) {
        return luaL_error(L, "Value cannot be converted to atom");
    }

    atomarray_appendatom(ud->atomarray, &a);

    return 0;
}

// AtomArray:clear()
static int AtomArray_clear(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);
    atomarray_clear(ud->atomarray);
    return 0;
}

// AtomArray:to_list() - Convert to Lua table
static int AtomArray_to_list(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);

    long ac = 0;
    t_atom* av = NULL;

    t_max_err err = atomarray_getatoms(ud->atomarray, &ac, &av);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get atoms from atomarray");
    }

    // Create Lua table
    lua_createtable(L, (int)ac, 0);

    for (long i = 0; i < ac; i++) {
        lua_pushatomvalue(L, &av[i]);
        lua_rawseti(L, -2, i + 1);  // Lua tables are 1-indexed
    }

    return 1;
}

// AtomArray:duplicate() - Create a copy
static int AtomArray_duplicate(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);

    t_atomarray* dup = (t_atomarray*)atomarray_duplicate(ud->atomarray);
    if (!dup) {
        return luaL_error(L, "Failed to duplicate atomarray");
    }

    // Create new AtomArrayUD wrapper
    AtomArrayUD* new_ud = (AtomArrayUD*)lua_newuserdata(L, sizeof(AtomArrayUD));
    new_ud->atomarray = dup;
    new_ud->owns_atomarray = true;

    luaL_getmetatable(L, ATOMARRAY_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// AtomArray:to_ints() - Convert to table of integers
static int AtomArray_to_ints(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);

    if (ud->atomarray == NULL) {
        return luaL_error(L, "AtomArray is null");
    }

    long ac;
    t_atom* av;
    atomarray_getatoms(ud->atomarray, &ac, &av);

    // Create Lua table
    lua_createtable(L, (int)ac, 0);

    // Extract longs
    t_atom_long* vals = (t_atom_long*)sysmem_newptr(ac * sizeof(t_atom_long));
    atom_getlong_array(ac, av, ac, vals);

    for (long i = 0; i < ac; i++) {
        lua_pushnumber(L, (lua_Number)vals[i]);
        lua_rawseti(L, -2, i + 1);
    }

    sysmem_freeptr(vals);
    return 1;
}

// AtomArray:to_floats() - Convert to table of floats
static int AtomArray_to_floats(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);

    if (ud->atomarray == NULL) {
        return luaL_error(L, "AtomArray is null");
    }

    long ac;
    t_atom* av;
    atomarray_getatoms(ud->atomarray, &ac, &av);

    // Create Lua table
    lua_createtable(L, (int)ac, 0);

    // Extract doubles
    double* vals = (double*)sysmem_newptr(ac * sizeof(double));
    atom_getdouble_array(ac, av, ac, vals);

    for (long i = 0; i < ac; i++) {
        lua_pushnumber(L, vals[i]);
        lua_rawseti(L, -2, i + 1);
    }

    sysmem_freeptr(vals);
    return 1;
}

// AtomArray:to_symbols() - Convert to table of symbol strings
static int AtomArray_to_symbols(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);

    if (ud->atomarray == NULL) {
        return luaL_error(L, "AtomArray is null");
    }

    long ac;
    t_atom* av;
    atomarray_getatoms(ud->atomarray, &ac, &av);

    // Create Lua table
    lua_createtable(L, (int)ac, 0);

    // Extract symbols
    t_symbol** vals = (t_symbol**)sysmem_newptr(ac * sizeof(t_symbol*));
    atom_getsym_array(ac, av, ac, vals);

    for (long i = 0; i < ac; i++) {
        if (vals[i] != NULL) {
            lua_pushstring(L, vals[i]->s_name);
        } else {
            lua_pushstring(L, "");
        }
        lua_rawseti(L, -2, i + 1);
    }

    sysmem_freeptr(vals);
    return 1;
}

// AtomArray:to_text() - Convert to formatted string
static int AtomArray_to_text(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);

    if (ud->atomarray == NULL) {
        return luaL_error(L, "AtomArray is null");
    }

    long ac;
    t_atom* av;
    atomarray_getatoms(ud->atomarray, &ac, &av);

    // Convert to text
    long textsize = 0;
    char* text = NULL;
    t_max_err err = atom_gettext(ac, av, &textsize, &text, 0);

    if (err != MAX_ERR_NONE || text == NULL) {
        lua_pushstring(L, "");
        return 1;
    }

    lua_pushstring(L, text);

    // Free text allocated by atom_gettext
    if (text) sysmem_freeptr(text);

    return 1;
}

// AtomArray:pointer() - Get raw pointer
static int AtomArray_pointer(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->atomarray);
    return 1;
}

// __gc metamethod (destructor)
static int AtomArray_gc(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);
    if (ud->owns_atomarray && ud->atomarray) {
        object_free(ud->atomarray);
        ud->atomarray = NULL;
    }
    return 0;
}

// __tostring metamethod
static int AtomArray_tostring(lua_State* L) {
    AtomArrayUD* ud = (AtomArrayUD*)luaL_checkudata(L, 1, ATOMARRAY_MT);
    long size = atomarray_getsize(ud->atomarray);
    lua_pushfstring(L, "AtomArray(size=%d)", (int)size);
    return 1;
}

// __index metamethod (for array-style access and methods)
static int AtomArray_index(lua_State* L) {
    // Check if it's numeric index
    if (lua_isnumber(L, 2)) {
        return AtomArray_getitem(L);
    }

    // Otherwise, look up method in metatable
    luaL_getmetatable(L, ATOMARRAY_MT);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

// __newindex metamethod (for array-style assignment)
static int AtomArray_newindex(lua_State* L) {
    if (lua_isnumber(L, 2)) {
        return AtomArray_setitem(L);
    }

    return luaL_error(L, "Cannot set non-numeric keys on AtomArray");
}

// Register AtomArray type
static void register_atomarray_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, ATOMARRAY_MT);

    // Register methods
    lua_pushcfunction(L, AtomArray_append);
    lua_setfield(L, -2, "append");

    lua_pushcfunction(L, AtomArray_clear);
    lua_setfield(L, -2, "clear");

    lua_pushcfunction(L, AtomArray_to_list);
    lua_setfield(L, -2, "to_list");

    lua_pushcfunction(L, AtomArray_duplicate);
    lua_setfield(L, -2, "duplicate");

    lua_pushcfunction(L, AtomArray_to_ints);
    lua_setfield(L, -2, "to_ints");

    lua_pushcfunction(L, AtomArray_to_floats);
    lua_setfield(L, -2, "to_floats");

    lua_pushcfunction(L, AtomArray_to_symbols);
    lua_setfield(L, -2, "to_symbols");

    lua_pushcfunction(L, AtomArray_to_text);
    lua_setfield(L, -2, "to_text");

    lua_pushcfunction(L, AtomArray_len);
    lua_setfield(L, -2, "getsize");

    lua_pushcfunction(L, AtomArray_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, AtomArray_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, AtomArray_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, AtomArray_len);
    lua_setfield(L, -2, "__len");

    lua_pushcfunction(L, AtomArray_index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, AtomArray_newindex);
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

    lua_pushcfunction(L, AtomArray_new);
    lua_setfield(L, -2, "AtomArray");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_ATOMARRAY_H

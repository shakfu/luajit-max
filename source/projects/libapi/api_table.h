// api_table.h
// Table wrapper for luajit-max API

#ifndef LUAJIT_API_TABLE_H
#define LUAJIT_API_TABLE_H

#include "api_common.h"

// Metatable name for Table userdata
#define TABLE_MT "Max.Table"

// Table userdata structure
// Max tables are named global arrays of long integers
typedef struct {
    t_symbol* name;
    long** handle;      // Pointer to pointer to array of longs
    long size;          // Number of elements
    bool is_bound;      // Whether we successfully retrieved the table
} TableUD;

// Table constructor: Table(name)
static int Table_new(lua_State* L) {
    const char* name_str = luaL_optstring(L, 1, NULL);

    // Create userdata
    TableUD* ud = (TableUD*)lua_newuserdata(L, sizeof(TableUD));
    ud->name = name_str ? gensym(name_str) : NULL;
    ud->handle = NULL;
    ud->size = 0;
    ud->is_bound = false;

    // Try to bind if name provided
    if (ud->name) {
        short result = table_get(ud->name, &ud->handle, &ud->size);
        if (result == 0) {
            ud->is_bound = true;
        }
    }

    // Set metatable
    luaL_getmetatable(L, TABLE_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Table.bind(name)
static int Table_bind(lua_State* L) {
    TableUD* ud = (TableUD*)luaL_checkudata(L, 1, TABLE_MT);
    const char* name_str = luaL_checkstring(L, 2);

    ud->name = gensym(name_str);

    // Try to get the table
    short result = table_get(ud->name, &ud->handle, &ud->size);

    if (result != 0) {
        ud->is_bound = false;
        ud->handle = NULL;
        ud->size = 0;
        lua_pushboolean(L, 0);
        return 1;
    }

    ud->is_bound = true;
    lua_pushboolean(L, 1);
    return 1;
}

// Table.refresh()
static int Table_refresh(lua_State* L) {
    TableUD* ud = (TableUD*)luaL_checkudata(L, 1, TABLE_MT);

    if (!ud->name) {
        return luaL_error(L, "No table name set - call bind() first");
    }

    // Re-get the table
    short result = table_get(ud->name, &ud->handle, &ud->size);

    if (result != 0) {
        ud->is_bound = false;
        ud->handle = NULL;
        ud->size = 0;
        lua_pushboolean(L, 0);
        return 1;
    }

    ud->is_bound = true;
    lua_pushboolean(L, 1);
    return 1;
}

// Table.get(index)
static int Table_get(lua_State* L) {
    TableUD* ud = (TableUD*)luaL_checkudata(L, 1, TABLE_MT);
    long index = (long)luaL_checknumber(L, 2);

    if (!ud->is_bound || !ud->handle) {
        return luaL_error(L, "Table not bound - call bind() first");
    }

    // Validate index
    if (index < 0 || index >= ud->size) {
        return luaL_error(L, "Table index out of range (0 to %ld)", ud->size - 1);
    }

    // Dereference handle to get array, then index into it
    long value = (*ud->handle)[index];
    lua_pushnumber(L, value);
    return 1;
}

// Table.set(index, value)
static int Table_set(lua_State* L) {
    TableUD* ud = (TableUD*)luaL_checkudata(L, 1, TABLE_MT);
    long index = (long)luaL_checknumber(L, 2);
    long value = (long)luaL_checknumber(L, 3);

    if (!ud->is_bound || !ud->handle) {
        return luaL_error(L, "Table not bound - call bind() first");
    }

    // Validate index
    if (index < 0 || index >= ud->size) {
        return luaL_error(L, "Table index out of range (0 to %ld)", ud->size - 1);
    }

    // Dereference handle to get array, then set value
    (*ud->handle)[index] = value;

    return 0;
}

// Table.size()
static int Table_size(lua_State* L) {
    TableUD* ud = (TableUD*)luaL_checkudata(L, 1, TABLE_MT);

    if (!ud->is_bound) {
        lua_pushnumber(L, 0);
    } else {
        lua_pushnumber(L, ud->size);
    }

    return 1;
}

// Table.name()
static int Table_name(lua_State* L) {
    TableUD* ud = (TableUD*)luaL_checkudata(L, 1, TABLE_MT);

    if (ud->name) {
        lua_pushstring(L, ud->name->s_name);
    } else {
        lua_pushnil(L);
    }

    return 1;
}

// Table.is_bound()
static int Table_is_bound(lua_State* L) {
    TableUD* ud = (TableUD*)luaL_checkudata(L, 1, TABLE_MT);
    lua_pushboolean(L, ud->is_bound);
    return 1;
}

// Table.to_list() - Export table contents to Lua table
static int Table_to_list(lua_State* L) {
    TableUD* ud = (TableUD*)luaL_checkudata(L, 1, TABLE_MT);

    if (!ud->is_bound || !ud->handle) {
        return luaL_error(L, "Table not bound - call bind() first");
    }

    // Create Lua table
    lua_createtable(L, (int)ud->size, 0);

    // Copy values
    for (long i = 0; i < ud->size; i++) {
        lua_pushnumber(L, (*ud->handle)[i]);
        lua_rawseti(L, -2, i + 1);  // Lua tables are 1-indexed
    }

    return 1;
}

// Table.from_list(lua_table) - Import from Lua table
static int Table_from_list(lua_State* L) {
    TableUD* ud = (TableUD*)luaL_checkudata(L, 1, TABLE_MT);
    luaL_checktype(L, 2, LUA_TTABLE);

    if (!ud->is_bound || !ud->handle) {
        return luaL_error(L, "Table not bound - call bind() first");
    }

    long table_len = (long)lua_objlen(L, 2);

    // Copy values (up to table size)
    long count = table_len < ud->size ? table_len : ud->size;
    for (long i = 0; i < count; i++) {
        lua_rawgeti(L, 2, i + 1);
        (*ud->handle)[i] = (long)lua_tonumber(L, -1);
        lua_pop(L, 1);
    }

    return 0;
}

// __gc metamethod (destructor)
static int Table_gc(lua_State* L) {
    TableUD* ud = (TableUD*)luaL_checkudata(L, 1, TABLE_MT);
    // Note: We don't own the table data, Max manages it
    ud->handle = NULL;
    ud->is_bound = false;
    return 0;
}

// __tostring metamethod
static int Table_tostring(lua_State* L) {
    TableUD* ud = (TableUD*)luaL_checkudata(L, 1, TABLE_MT);

    if (ud->is_bound && ud->name) {
        lua_pushfstring(L, "Table(name='%s', size=%d)",
                       ud->name->s_name, (int)ud->size);
    } else if (ud->name) {
        lua_pushfstring(L, "Table(name='%s', unbound)", ud->name->s_name);
    } else {
        lua_pushstring(L, "Table(null)");
    }

    return 1;
}

// __len metamethod
static int Table_len(lua_State* L) {
    return Table_size(L);
}

// __index metamethod (for array-style access)
static int Table_index(lua_State* L) {
    // Check if it's numeric index
    if (lua_isnumber(L, 2)) {
        return Table_get(L);
    }

    // Otherwise, look up method in metatable
    luaL_getmetatable(L, TABLE_MT);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    return 1;
}

// __newindex metamethod (for array-style assignment)
static int Table_newindex(lua_State* L) {
    if (lua_isnumber(L, 2)) {
        return Table_set(L);
    }

    return luaL_error(L, "Cannot set non-numeric keys on Table");
}

// Register Table type
static void register_table_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, TABLE_MT);

    // Register methods
    lua_pushcfunction(L, Table_bind);
    lua_setfield(L, -2, "bind");

    lua_pushcfunction(L, Table_refresh);
    lua_setfield(L, -2, "refresh");

    lua_pushcfunction(L, Table_get);
    lua_setfield(L, -2, "get");

    lua_pushcfunction(L, Table_set);
    lua_setfield(L, -2, "set");

    lua_pushcfunction(L, Table_size);
    lua_setfield(L, -2, "size");

    lua_pushcfunction(L, Table_name);
    lua_setfield(L, -2, "name");

    lua_pushcfunction(L, Table_is_bound);
    lua_setfield(L, -2, "is_bound");

    lua_pushcfunction(L, Table_to_list);
    lua_setfield(L, -2, "to_list");

    lua_pushcfunction(L, Table_from_list);
    lua_setfield(L, -2, "from_list");

    // Register metamethods
    lua_pushcfunction(L, Table_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Table_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, Table_len);
    lua_setfield(L, -2, "__len");

    lua_pushcfunction(L, Table_index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, Table_newindex);
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

    lua_pushcfunction(L, Table_new);
    lua_setfield(L, -2, "Table");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_TABLE_H

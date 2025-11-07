// api_database.h
// Database (SQLite) wrapper for luajit-max API
// Provides SQLite database access from Lua

#ifndef LUAJIT_API_DATABASE_H
#define LUAJIT_API_DATABASE_H

#include "api_common.h"

// Metatable names
#define DATABASE_MT "Max.Database"
#define DBRESULT_MT "Max.DBResult"

// ----------------------------------------------------------------------------
// Database userdata structure

typedef struct {
    t_database* db;
    t_symbol* dbname;
    bool owns_db;
} DatabaseUD;

// ----------------------------------------------------------------------------
// DBResult userdata structure

typedef struct {
    t_db_result* result;
    bool owns_result;
} DBResultUD;

// ----------------------------------------------------------------------------
// Database type methods

// Database constructor: Database()
static int Database_new(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)lua_newuserdata(L, sizeof(DatabaseUD));
    ud->db = NULL;
    ud->dbname = NULL;
    ud->owns_db = false;

    luaL_getmetatable(L, DATABASE_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Database:open(name, filepath)
static int Database_open(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);
    const char* name_str = luaL_checkstring(L, 2);
    const char* filepath = NULL;

    if (lua_gettop(L) >= 3 && lua_isstring(L, 3)) {
        filepath = lua_tostring(L, 3);
    }

    // Close existing if we own it
    if (ud->owns_db && ud->db) {
        db_close(&ud->db);
    }

    ud->dbname = gensym(name_str);
    t_max_err err = db_open(ud->dbname, filepath, &ud->db);

    if (err != MAX_ERR_NONE) {
        ud->db = NULL;
        ud->owns_db = false;
        return luaL_error(L, "Failed to open database");
    }

    ud->owns_db = true;
    return 0;
}

// Database:close()
static int Database_close(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);

    if (!ud->db) {
        return 0;
    }

    if (ud->owns_db) {
        t_max_err err = db_close(&ud->db);
        if (err != MAX_ERR_NONE) {
            return luaL_error(L, "Failed to close database");
        }
    }

    ud->db = NULL;
    ud->owns_db = false;

    return 0;
}

// Database:query(sql) -> DBResult
static int Database_query(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);
    const char* sql = luaL_checkstring(L, 2);

    if (!ud->db) {
        return luaL_error(L, "Database not open");
    }

    t_db_result* result = NULL;
    t_max_err err = db_query_direct(ud->db, &result, sql);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Query failed");
    }

    // Create DBResult wrapper
    DBResultUD* result_ud = (DBResultUD*)lua_newuserdata(L, sizeof(DBResultUD));
    result_ud->result = result;
    result_ud->owns_result = true;

    luaL_getmetatable(L, DBRESULT_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Database:transaction_start()
static int Database_transaction_start(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);

    if (!ud->db) {
        return luaL_error(L, "Database not open");
    }

    t_max_err err = db_transaction_start(ud->db);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to start transaction");
    }

    return 0;
}

// Database:transaction_end()
static int Database_transaction_end(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);

    if (!ud->db) {
        return luaL_error(L, "Database not open");
    }

    t_max_err err = db_transaction_end(ud->db);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to end transaction");
    }

    return 0;
}

// Database:transaction_flush()
static int Database_transaction_flush(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);

    if (!ud->db) {
        return luaL_error(L, "Database not open");
    }

    t_max_err err = db_transaction_flush(ud->db);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to flush transactions");
    }

    return 0;
}

// Database:get_last_insert_id() -> id
static int Database_get_last_insert_id(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);

    if (!ud->db) {
        return luaL_error(L, "Database not open");
    }

    long id;
    t_max_err err = db_query_getlastinsertid(ud->db, &id);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get last insert ID");
    }

    lua_pushnumber(L, id);
    return 1;
}

// Database:create_table(tablename)
static int Database_create_table(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);
    const char* tablename = luaL_checkstring(L, 2);

    if (!ud->db) {
        return luaL_error(L, "Database not open");
    }

    t_max_err err = db_query_table_new(ud->db, tablename);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to create table");
    }

    return 0;
}

// Database:add_column(tablename, columnname, columntype, flags)
static int Database_add_column(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);
    const char* tablename = luaL_checkstring(L, 2);
    const char* columnname = luaL_checkstring(L, 3);
    const char* columntype = luaL_checkstring(L, 4);
    const char* flags = NULL;

    if (lua_gettop(L) >= 5 && lua_isstring(L, 5)) {
        flags = lua_tostring(L, 5);
    }

    if (!ud->db) {
        return luaL_error(L, "Database not open");
    }

    t_max_err err = db_query_table_addcolumn(ud->db, tablename, columnname, columntype, flags);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to add column");
    }

    return 0;
}

// Database:is_open() -> bool
static int Database_is_open(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);
    lua_pushboolean(L, ud->db != NULL);
    return 1;
}

// Database:pointer() -> pointer
static int Database_pointer(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->db);
    return 1;
}

// __gc metamethod
static int Database_gc(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);
    if (ud->owns_db && ud->db) {
        db_close(&ud->db);
    }
    ud->db = NULL;
    return 0;
}

// __tostring metamethod
static int Database_tostring(lua_State* L) {
    DatabaseUD* ud = (DatabaseUD*)luaL_checkudata(L, 1, DATABASE_MT);

    if (ud->db && ud->dbname) {
        lua_pushfstring(L, "Database(name='%s', %p)", ud->dbname->s_name, ud->db);
    } else {
        lua_pushstring(L, "Database(closed)");
    }
    return 1;
}

// ----------------------------------------------------------------------------
// DBResult type methods

// DBResult constructor (internal use only)
static int DBResult_new(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)lua_newuserdata(L, sizeof(DBResultUD));
    ud->result = NULL;
    ud->owns_result = false;

    luaL_getmetatable(L, DBRESULT_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// DBResult:numrecords() -> count
static int DBResult_numrecords(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);

    if (!ud->result) {
        lua_pushnumber(L, 0);
        return 1;
    }

    long count = db_result_numrecords(ud->result);
    lua_pushnumber(L, count);
    return 1;
}

// DBResult:numfields() -> count
static int DBResult_numfields(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);

    if (!ud->result) {
        lua_pushnumber(L, 0);
        return 1;
    }

    long count = db_result_numfields(ud->result);
    lua_pushnumber(L, count);
    return 1;
}

// DBResult:fieldname(index) -> name
static int DBResult_fieldname(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);
    long index = (long)luaL_checknumber(L, 2);

    if (!ud->result) {
        return luaL_error(L, "Result is null");
    }

    char* name = db_result_fieldname(ud->result, index);

    if (name) {
        lua_pushstring(L, name);
    } else {
        lua_pushnil(L);
    }

    return 1;
}

// DBResult:get_string(record, field) -> value
static int DBResult_get_string(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);
    long record = (long)luaL_checknumber(L, 2);
    long field = (long)luaL_checknumber(L, 3);

    if (!ud->result) {
        return luaL_error(L, "Result is null");
    }

    char* value = db_result_string(ud->result, record, field);

    if (value) {
        lua_pushstring(L, value);
    } else {
        lua_pushnil(L);
    }

    return 1;
}

// DBResult:get_long(record, field) -> value
static int DBResult_get_long(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);
    long record = (long)luaL_checknumber(L, 2);
    long field = (long)luaL_checknumber(L, 3);

    if (!ud->result) {
        return luaL_error(L, "Result is null");
    }

    long value = db_result_long(ud->result, record, field);
    lua_pushnumber(L, value);
    return 1;
}

// DBResult:get_float(record, field) -> value
static int DBResult_get_float(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);
    long record = (long)luaL_checknumber(L, 2);
    long field = (long)luaL_checknumber(L, 3);

    if (!ud->result) {
        return luaL_error(L, "Result is null");
    }

    float value = db_result_float(ud->result, record, field);
    lua_pushnumber(L, value);
    return 1;
}

// DBResult:get_record(index) -> {values...}
static int DBResult_get_record(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);
    long record_index = (long)luaL_checknumber(L, 2);

    if (!ud->result) {
        return luaL_error(L, "Result is null");
    }

    long numfields = db_result_numfields(ud->result);

    // Create table
    lua_createtable(L, numfields, 0);

    for (long i = 0; i < numfields; i++) {
        char* value = db_result_string(ud->result, record_index, i);
        if (value) {
            lua_pushstring(L, value);
        } else {
            lua_pushnil(L);
        }
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

// DBResult:to_list() -> {{values...}, ...}
static int DBResult_to_list(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);

    if (!ud->result) {
        lua_newtable(L);
        return 1;
    }

    long numrecords = db_result_numrecords(ud->result);
    long numfields = db_result_numfields(ud->result);

    // Create outer table
    lua_createtable(L, numrecords, 0);

    for (long r = 0; r < numrecords; r++) {
        lua_createtable(L, numfields, 0);

        for (long f = 0; f < numfields; f++) {
            char* value = db_result_string(ud->result, r, f);
            if (value) {
                lua_pushstring(L, value);
            } else {
                lua_pushnil(L);
            }
            lua_rawseti(L, -2, f + 1);
        }

        lua_rawseti(L, -2, r + 1);
    }

    return 1;
}

// DBResult:reset()
static int DBResult_reset(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);

    if (!ud->result) {
        return luaL_error(L, "Result is null");
    }

    db_result_reset(ud->result);
    return 0;
}

// DBResult:clear()
static int DBResult_clear(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);

    if (!ud->result) {
        return 0;
    }

    db_result_clear(ud->result);
    return 0;
}

// __len metamethod
static int DBResult_len(lua_State* L) {
    return DBResult_numrecords(L);
}

// __gc metamethod
static int DBResult_gc(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);
    if (ud->owns_result && ud->result) {
        object_free((t_object*)ud->result);
    }
    ud->result = NULL;
    return 0;
}

// __tostring metamethod
static int DBResult_tostring(lua_State* L) {
    DBResultUD* ud = (DBResultUD*)luaL_checkudata(L, 1, DBRESULT_MT);

    if (ud->result) {
        long numrecords = db_result_numrecords(ud->result);
        long numfields = db_result_numfields(ud->result);
        lua_pushfstring(L, "DBResult(%d records, %d fields)", (int)numrecords, (int)numfields);
    } else {
        lua_pushstring(L, "DBResult(null)");
    }
    return 1;
}

// ----------------------------------------------------------------------------
// Register Database and DBResult types

static void register_database_type(lua_State* L) {
    // Create Database metatable
    luaL_newmetatable(L, DATABASE_MT);

    // Register methods
    lua_pushcfunction(L, Database_open);
    lua_setfield(L, -2, "open");

    lua_pushcfunction(L, Database_close);
    lua_setfield(L, -2, "close");

    lua_pushcfunction(L, Database_query);
    lua_setfield(L, -2, "query");

    lua_pushcfunction(L, Database_transaction_start);
    lua_setfield(L, -2, "transaction_start");

    lua_pushcfunction(L, Database_transaction_end);
    lua_setfield(L, -2, "transaction_end");

    lua_pushcfunction(L, Database_transaction_flush);
    lua_setfield(L, -2, "transaction_flush");

    lua_pushcfunction(L, Database_get_last_insert_id);
    lua_setfield(L, -2, "get_last_insert_id");

    lua_pushcfunction(L, Database_create_table);
    lua_setfield(L, -2, "create_table");

    lua_pushcfunction(L, Database_add_column);
    lua_setfield(L, -2, "add_column");

    lua_pushcfunction(L, Database_is_open);
    lua_setfield(L, -2, "is_open");

    lua_pushcfunction(L, Database_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Database_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Database_tostring);
    lua_setfield(L, -2, "__tostring");

    // __index points to metatable itself
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);  // Pop metatable

    // Create DBResult metatable
    luaL_newmetatable(L, DBRESULT_MT);

    // Register methods
    lua_pushcfunction(L, DBResult_numrecords);
    lua_setfield(L, -2, "numrecords");

    lua_pushcfunction(L, DBResult_numfields);
    lua_setfield(L, -2, "numfields");

    lua_pushcfunction(L, DBResult_fieldname);
    lua_setfield(L, -2, "fieldname");

    lua_pushcfunction(L, DBResult_get_string);
    lua_setfield(L, -2, "get_string");

    lua_pushcfunction(L, DBResult_get_long);
    lua_setfield(L, -2, "get_long");

    lua_pushcfunction(L, DBResult_get_float);
    lua_setfield(L, -2, "get_float");

    lua_pushcfunction(L, DBResult_get_record);
    lua_setfield(L, -2, "get_record");

    lua_pushcfunction(L, DBResult_to_list);
    lua_setfield(L, -2, "to_list");

    lua_pushcfunction(L, DBResult_reset);
    lua_setfield(L, -2, "reset");

    lua_pushcfunction(L, DBResult_clear);
    lua_setfield(L, -2, "clear");

    // Register metamethods
    lua_pushcfunction(L, DBResult_len);
    lua_setfield(L, -2, "__len");

    lua_pushcfunction(L, DBResult_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, DBResult_tostring);
    lua_setfield(L, -2, "__tostring");

    // __index points to metatable itself
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);  // Pop metatable

    // Register Database constructor in api module
    lua_getglobal(L, "api");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "api");
    }

    lua_pushcfunction(L, Database_new);
    lua_setfield(L, -2, "Database");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_DATABASE_H

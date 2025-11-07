// api_path.h
// Path and file I/O wrapper for luajit-max API
// Provides file system path operations and file I/O

#ifndef LUAJIT_API_PATH_H
#define LUAJIT_API_PATH_H

#include "api_common.h"

// ----------------------------------------------------------------------------
// Module-level path functions
// Max uses Path IDs (short integers) to identify file system locations

// api.path_getdefault() -> path_id
static int api_path_getdefault(lua_State* L) {
    short path_id = path_getdefault();
    lua_pushnumber(L, path_id);
    return 1;
}

// api.path_setdefault(path_id, recursive)
static int api_path_setdefault(lua_State* L) {
    short path_id = (short)luaL_checknumber(L, 1);
    short recursive = lua_toboolean(L, 2) ? 1 : 0;

    path_setdefault(path_id, recursive);
    return 0;
}

// api.path_getapppath() -> path_id
static int api_path_getapppath(lua_State* L) {
    short path_id = path_getapppath();
    lua_pushnumber(L, path_id);
    return 1;
}

// api.locatefile_extended(filename, typelist) -> {filename, path_id, filetype} or nil
static int api_locatefile_extended(lua_State* L) {
    const char* input_name = luaL_checkstring(L, 1);

    // Copy filename to mutable buffer (API may modify it)
    char filename[MAX_FILENAME_CHARS];
    strncpy(filename, input_name, MAX_FILENAME_CHARS - 1);
    filename[MAX_FILENAME_CHARS - 1] = '\0';

    short path_id;
    t_fourcc outtype;
    t_fourcc typelist[32];
    short numtypes = 0;

    // Parse optional typelist
    if (lua_istable(L, 2)) {
        int list_len = lua_objlen(L, 2);
        numtypes = (list_len > 32) ? 32 : list_len;

        for (int i = 0; i < numtypes; i++) {
            lua_rawgeti(L, 2, i + 1);
            if (lua_isstring(L, -1)) {
                const char* typestr = lua_tostring(L, -1);
                // Convert 4-char string to fourcc
                if (strlen(typestr) == 4) {
                    typelist[i] = ((t_fourcc)typestr[0] << 24) |
                                  ((t_fourcc)typestr[1] << 16) |
                                  ((t_fourcc)typestr[2] << 8) |
                                  ((t_fourcc)typestr[3]);
                } else {
                    typelist[i] = 0;
                }
            } else {
                typelist[i] = 0;
            }
            lua_pop(L, 1);
        }
    }

    // Search for file
    short result = locatefile_extended(filename, &path_id, &outtype,
                                       numtypes > 0 ? typelist : NULL, numtypes);

    if (result != 0) {
        // File not found
        lua_pushnil(L);
        return 1;
    }

    // Return table {filename, path_id, filetype}
    lua_createtable(L, 3, 0);

    lua_pushstring(L, filename);
    lua_rawseti(L, -2, 1);

    lua_pushnumber(L, path_id);
    lua_rawseti(L, -2, 2);

    // Convert fourcc to string
    char typebuf[5];
    typebuf[0] = (outtype >> 24) & 0xFF;
    typebuf[1] = (outtype >> 16) & 0xFF;
    typebuf[2] = (outtype >> 8) & 0xFF;
    typebuf[3] = outtype & 0xFF;
    typebuf[4] = '\0';
    lua_pushstring(L, typebuf);
    lua_rawseti(L, -2, 3);

    return 1;
}

// api.path_toabsolutesystempath(path_id, filename) -> absolute_path
static int api_path_toabsolutesystempath(lua_State* L) {
    short path_id = (short)luaL_checknumber(L, 1);
    const char* filename = luaL_checkstring(L, 2);

    char out_path[MAX_PATH_CHARS];
    t_max_err err = path_toabsolutesystempath(path_id, filename, out_path);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to convert to absolute path");
    }

    lua_pushstring(L, out_path);
    return 1;
}

// api.path_nameconform(src, style, type) -> conformed_path
static int api_path_nameconform(lua_State* L) {
    const char* src = luaL_checkstring(L, 1);
    long style = (long)luaL_checknumber(L, 2);
    long type = (long)luaL_checknumber(L, 3);

    char dst[MAX_PATH_CHARS];
    short result = path_nameconform(src, dst, style, type);

    if (result != 0) {
        return luaL_error(L, "Failed to conform path name");
    }

    lua_pushstring(L, dst);
    return 1;
}

// api.path_opensysfile(filename, path_id, perm) -> filehandle
static int api_path_opensysfile(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    short path_id = (short)luaL_checknumber(L, 2);
    short perm = (short)luaL_checknumber(L, 3);

    t_filehandle fh;
    short result = path_opensysfile(filename, path_id, &fh, perm);

    if (result != 0) {
        return luaL_error(L, "Failed to open file");
    }

    lua_pushnumber(L, (lua_Number)(intptr_t)fh);
    return 1;
}

// api.path_createsysfile(filename, path_id, filetype) -> filehandle
static int api_path_createsysfile(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    short path_id = (short)luaL_checknumber(L, 2);
    const char* typestr = luaL_checkstring(L, 3);

    // Convert 4-char string to fourcc
    t_fourcc filetype = 0;
    if (strlen(typestr) == 4) {
        filetype = ((t_fourcc)typestr[0] << 24) |
                   ((t_fourcc)typestr[1] << 16) |
                   ((t_fourcc)typestr[2] << 8) |
                   ((t_fourcc)typestr[3]);
    }

    t_filehandle fh;
    short result = path_createsysfile(filename, path_id, filetype, &fh);

    if (result != 0) {
        return luaL_error(L, "Failed to create file");
    }

    lua_pushnumber(L, (lua_Number)(intptr_t)fh);
    return 1;
}

// api.path_closesysfile(filehandle)
static int api_path_closesysfile(lua_State* L) {
    t_filehandle fh = (t_filehandle)(intptr_t)luaL_checknumber(L, 1);
    sysfile_close(fh);
    return 0;
}

// api.sysfile_read(filehandle, count) -> data_string
static int api_sysfile_read(lua_State* L) {
    t_filehandle fh = (t_filehandle)(intptr_t)luaL_checknumber(L, 1);
    t_ptr_size count = (t_ptr_size)luaL_checknumber(L, 2);

    // Allocate buffer
    char* buffer = (char*)sysmem_newptr(count + 1);
    if (!buffer) {
        return luaL_error(L, "Failed to allocate read buffer");
    }

    t_ptr_size actual = count;
    t_max_err err = sysfile_read(fh, &actual, buffer);

    if (err != MAX_ERR_NONE) {
        sysmem_freeptr(buffer);
        return luaL_error(L, "Failed to read from file");
    }

    buffer[actual] = '\0';
    lua_pushstring(L, buffer);

    sysmem_freeptr(buffer);
    return 1;
}

// api.sysfile_write(filehandle, data) -> bytes_written
static int api_sysfile_write(lua_State* L) {
    t_filehandle fh = (t_filehandle)(intptr_t)luaL_checknumber(L, 1);
    const char* data = luaL_checkstring(L, 2);
    t_ptr_size count = strlen(data);

    t_ptr_size actual = count;
    t_max_err err = sysfile_write(fh, &actual, data);

    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to write to file");
    }

    lua_pushnumber(L, (lua_Number)actual);
    return 1;
}

// api.sysfile_geteof(filehandle) -> eof_position
static int api_sysfile_geteof(lua_State* L) {
    t_filehandle fh = (t_filehandle)(intptr_t)luaL_checknumber(L, 1);
    t_ptr_size eof;

    t_max_err err = sysfile_geteof(fh, &eof);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get EOF position");
    }

    lua_pushnumber(L, (lua_Number)eof);
    return 1;
}

// api.sysfile_seteof(filehandle, pos)
static int api_sysfile_seteof(lua_State* L) {
    t_filehandle fh = (t_filehandle)(intptr_t)luaL_checknumber(L, 1);
    t_ptr_size eof = (t_ptr_size)luaL_checknumber(L, 2);

    t_max_err err = sysfile_seteof(fh, eof);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to set EOF position");
    }

    return 0;
}

// api.sysfile_getpos(filehandle) -> position
static int api_sysfile_getpos(lua_State* L) {
    t_filehandle fh = (t_filehandle)(intptr_t)luaL_checknumber(L, 1);
    t_ptr_size pos;

    t_max_err err = sysfile_getpos(fh, &pos);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to get file position");
    }

    lua_pushnumber(L, (lua_Number)pos);
    return 1;
}

// api.sysfile_setpos(filehandle, pos, mode)
static int api_sysfile_setpos(lua_State* L) {
    t_filehandle fh = (t_filehandle)(intptr_t)luaL_checknumber(L, 1);
    t_ptr_size pos = (t_ptr_size)luaL_checknumber(L, 2);
    t_sysfile_pos_mode mode = (t_sysfile_pos_mode)luaL_checknumber(L, 3);

    t_max_err err = sysfile_setpos(fh, mode, pos);
    if (err != MAX_ERR_NONE) {
        return luaL_error(L, "Failed to set file position");
    }

    return 0;
}

// api.sysfile_readtextfile(filehandle, maxsize) -> text
static int api_sysfile_readtextfile(lua_State* L) {
    t_filehandle fh = (t_filehandle)(intptr_t)luaL_checknumber(L, 1);
    t_ptr_size maxsize = 65536;

    if (lua_gettop(L) >= 2 && lua_isnumber(L, 2)) {
        maxsize = (t_ptr_size)lua_tonumber(L, 2);
    }

    t_handle h = sysmem_newhandle(0);
    t_max_err err = sysfile_readtextfile(fh, h, maxsize, TEXT_LB_NATIVE);

    if (err != MAX_ERR_NONE) {
        if (h) sysmem_freehandle(h);
        return luaL_error(L, "Failed to read text file");
    }

    if (h == NULL) {
        lua_pushstring(L, "");
        return 1;
    }

    t_ptr_size size = sysmem_handlesize(h);
    char* text = *h;

    lua_pushlstring(L, text, size);

    sysmem_freehandle(h);
    return 1;
}

// api.path_deletefile(filename, path_id)
static int api_path_deletefile(lua_State* L) {
    const char* filename = luaL_checkstring(L, 1);
    short path_id = (short)luaL_checknumber(L, 2);

    short result = path_deletefile(filename, path_id);

    if (result != 0) {
        return luaL_error(L, "Failed to delete file");
    }

    return 0;
}

// Register path functions in api module
static void register_path_type(lua_State* L) {
    // Get api module
    lua_getglobal(L, "api");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "api");
    }

    // Register all path functions in api module
    lua_pushcfunction(L, api_path_getdefault);
    lua_setfield(L, -2, "path_getdefault");

    lua_pushcfunction(L, api_path_setdefault);
    lua_setfield(L, -2, "path_setdefault");

    lua_pushcfunction(L, api_path_getapppath);
    lua_setfield(L, -2, "path_getapppath");

    lua_pushcfunction(L, api_locatefile_extended);
    lua_setfield(L, -2, "locatefile_extended");

    lua_pushcfunction(L, api_path_toabsolutesystempath);
    lua_setfield(L, -2, "path_toabsolutesystempath");

    lua_pushcfunction(L, api_path_nameconform);
    lua_setfield(L, -2, "path_nameconform");

    lua_pushcfunction(L, api_path_opensysfile);
    lua_setfield(L, -2, "path_opensysfile");

    lua_pushcfunction(L, api_path_createsysfile);
    lua_setfield(L, -2, "path_createsysfile");

    lua_pushcfunction(L, api_path_closesysfile);
    lua_setfield(L, -2, "path_closesysfile");

    lua_pushcfunction(L, api_sysfile_read);
    lua_setfield(L, -2, "sysfile_read");

    lua_pushcfunction(L, api_sysfile_write);
    lua_setfield(L, -2, "sysfile_write");

    lua_pushcfunction(L, api_sysfile_geteof);
    lua_setfield(L, -2, "sysfile_geteof");

    lua_pushcfunction(L, api_sysfile_seteof);
    lua_setfield(L, -2, "sysfile_seteof");

    lua_pushcfunction(L, api_sysfile_getpos);
    lua_setfield(L, -2, "sysfile_getpos");

    lua_pushcfunction(L, api_sysfile_setpos);
    lua_setfield(L, -2, "sysfile_setpos");

    lua_pushcfunction(L, api_sysfile_readtextfile);
    lua_setfield(L, -2, "sysfile_readtextfile");

    lua_pushcfunction(L, api_path_deletefile);
    lua_setfield(L, -2, "path_deletefile");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_PATH_H

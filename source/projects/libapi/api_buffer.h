// api_buffer.h
// Buffer wrapper for luajit-max API
// Provides access to MSP buffer~ objects for audio sample manipulation

#ifndef LUAJIT_API_BUFFER_H
#define LUAJIT_API_BUFFER_H

#include "api_common.h"
#include "ext_buffer.h"

// Metatable name for Buffer userdata
#define BUFFER_MT "Max.Buffer"

// Buffer userdata structure
typedef struct {
    t_buffer_ref* buffer_ref;
    bool owns_ref;  // Whether we should free it
} BufferUD;

// Buffer constructor: Buffer(owner_ptr, name)
static int Buffer_new(lua_State* L) {
    t_object* owner = (t_object*)(intptr_t)luaL_checknumber(L, 1);
    t_symbol* name = NULL;

    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        const char* name_str = luaL_checkstring(L, 2);
        name = gensym(name_str);
    }

    // Create userdata
    BufferUD* ud = (BufferUD*)lua_newuserdata(L, sizeof(BufferUD));
    ud->buffer_ref = buffer_ref_new(owner, name);
    ud->owns_ref = true;

    // Set metatable
    luaL_getmetatable(L, BUFFER_MT);
    lua_setmetatable(L, -2);

    return 1;
}

// Buffer:ref_set(name) - Set buffer reference by name
static int Buffer_ref_set(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);
    const char* name = luaL_checkstring(L, 2);

    t_symbol* buffer_name = gensym(name);
    buffer_ref_set(ud->buffer_ref, buffer_name);

    return 0;
}

// Buffer:exists() - Check if buffer exists
static int Buffer_exists(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);

    if (ud->buffer_ref == NULL) {
        lua_pushboolean(L, false);
        return 1;
    }

    t_buffer_obj* obj = buffer_ref_getobject(ud->buffer_ref);
    lua_pushboolean(L, obj != NULL);
    return 1;
}

// Buffer:getinfo() - Get buffer info as Lua table
static int Buffer_getinfo(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);

    t_buffer_obj* obj = buffer_ref_getobject(ud->buffer_ref);
    if (!obj) {
        return luaL_error(L, "Buffer reference is not valid");
    }

    t_buffer_info info;
    buffer_getinfo(obj, &info);

    // Create Lua table with buffer info
    lua_newtable(L);

    lua_pushnumber(L, info.b_frames);
    lua_setfield(L, -2, "frames");

    lua_pushnumber(L, info.b_nchans);
    lua_setfield(L, -2, "channels");

    lua_pushnumber(L, info.b_sr);
    lua_setfield(L, -2, "samplerate");

    lua_pushnumber(L, info.b_modtime);
    lua_setfield(L, -2, "modtime");

    lua_pushnumber(L, info.b_size);
    lua_setfield(L, -2, "size");

    return 1;
}

// Buffer:frames() - Get number of frames
static int Buffer_frames(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);

    t_buffer_obj* obj = buffer_ref_getobject(ud->buffer_ref);
    if (!obj) {
        lua_pushnumber(L, 0);
        return 1;
    }

    t_buffer_info info;
    buffer_getinfo(obj, &info);
    lua_pushnumber(L, info.b_frames);
    return 1;
}

// Buffer:channels() - Get number of channels
static int Buffer_channels(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);

    t_buffer_obj* obj = buffer_ref_getobject(ud->buffer_ref);
    if (!obj) {
        lua_pushnumber(L, 0);
        return 1;
    }

    t_buffer_info info;
    buffer_getinfo(obj, &info);
    lua_pushnumber(L, info.b_nchans);
    return 1;
}

// Buffer:samplerate() - Get sample rate
static int Buffer_samplerate(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);

    t_buffer_obj* obj = buffer_ref_getobject(ud->buffer_ref);
    if (!obj) {
        lua_pushnumber(L, 0);
        return 1;
    }

    t_buffer_info info;
    buffer_getinfo(obj, &info);
    lua_pushnumber(L, info.b_sr);
    return 1;
}

// Buffer:peek(frame, channel) - Read single sample
static int Buffer_peek(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);
    long frame = (long)luaL_checknumber(L, 2);
    long channel = (long)luaL_checknumber(L, 3);

    t_buffer_obj* obj = buffer_ref_getobject(ud->buffer_ref);
    if (!obj) {
        return luaL_error(L, "Buffer reference is not valid");
    }

    t_buffer_info info;
    buffer_getinfo(obj, &info);

    // Validate indices (0-based)
    if (frame < 0 || frame >= info.b_frames) {
        return luaL_error(L, "Frame index %d out of range [0, %d)", (int)frame, (int)info.b_frames);
    }
    if (channel < 0 || channel >= info.b_nchans) {
        return luaL_error(L, "Channel index %d out of range [0, %d)", (int)channel, (int)info.b_nchans);
    }

    // Lock and read
    float* samples = buffer_locksamples(obj);
    if (!samples) {
        return luaL_error(L, "Failed to lock buffer samples");
    }

    // Calculate index (samples are interleaved: frame * nchans + channel)
    long index = frame * info.b_nchans + channel;
    float value = samples[index];

    buffer_unlocksamples(obj);

    lua_pushnumber(L, value);
    return 1;
}

// Buffer:poke(frame, channel, value) - Write single sample
static int Buffer_poke(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);
    long frame = (long)luaL_checknumber(L, 2);
    long channel = (long)luaL_checknumber(L, 3);
    float value = (float)luaL_checknumber(L, 4);

    t_buffer_obj* obj = buffer_ref_getobject(ud->buffer_ref);
    if (!obj) {
        return luaL_error(L, "Buffer reference is not valid");
    }

    t_buffer_info info;
    buffer_getinfo(obj, &info);

    // Validate indices (0-based)
    if (frame < 0 || frame >= info.b_frames) {
        return luaL_error(L, "Frame index %d out of range [0, %d)", (int)frame, (int)info.b_frames);
    }
    if (channel < 0 || channel >= info.b_nchans) {
        return luaL_error(L, "Channel index %d out of range [0, %d)", (int)channel, (int)info.b_nchans);
    }

    // Lock and write
    float* samples = buffer_locksamples(obj);
    if (!samples) {
        return luaL_error(L, "Failed to lock buffer samples");
    }

    // Calculate index (samples are interleaved)
    long index = frame * info.b_nchans + channel;
    samples[index] = value;

    buffer_unlocksamples(obj);
    buffer_setdirty(obj);

    return 0;
}

// Buffer:to_list(channel, start_frame, num_frames) - Export channel to Lua table
static int Buffer_to_list(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);
    long channel = 0;
    long start_frame = 0;
    long num_frames = -1;

    if (lua_gettop(L) >= 2) {
        channel = (long)luaL_checknumber(L, 2);
    }
    if (lua_gettop(L) >= 3) {
        start_frame = (long)luaL_checknumber(L, 3);
    }
    if (lua_gettop(L) >= 4) {
        num_frames = (long)luaL_checknumber(L, 4);
    }

    t_buffer_obj* obj = buffer_ref_getobject(ud->buffer_ref);
    if (!obj) {
        return luaL_error(L, "Buffer reference is not valid");
    }

    t_buffer_info info;
    buffer_getinfo(obj, &info);

    // Validate channel
    if (channel < 0 || channel >= info.b_nchans) {
        return luaL_error(L, "Channel index out of range");
    }

    // Validate start_frame
    if (start_frame < 0) {
        start_frame = 0;
    }
    if (start_frame >= info.b_frames) {
        start_frame = info.b_frames - 1;
    }

    // Determine num_frames
    if (num_frames < 0 || (start_frame + num_frames) > info.b_frames) {
        num_frames = info.b_frames - start_frame;
    }

    // Lock samples
    float* samples = buffer_locksamples(obj);
    if (!samples) {
        return luaL_error(L, "Failed to lock buffer samples");
    }

    // Create Lua table
    lua_createtable(L, (int)num_frames, 0);

    for (long i = 0; i < num_frames; i++) {
        long frame = start_frame + i;
        long index = frame * info.b_nchans + channel;
        lua_pushnumber(L, samples[index]);
        lua_rawseti(L, -2, i + 1);  // Lua 1-indexed
    }

    buffer_unlocksamples(obj);

    return 1;
}

// Buffer:from_list(channel, lua_table, start_frame) - Import Lua table to channel
static int Buffer_from_list(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);
    long channel = (long)luaL_checknumber(L, 2);
    luaL_checktype(L, 3, LUA_TTABLE);
    long start_frame = 0;

    if (lua_gettop(L) >= 4) {
        start_frame = (long)luaL_checknumber(L, 4);
    }

    t_buffer_obj* obj = buffer_ref_getobject(ud->buffer_ref);
    if (!obj) {
        return luaL_error(L, "Buffer reference is not valid");
    }

    t_buffer_info info;
    buffer_getinfo(obj, &info);

    // Validate channel
    if (channel < 0 || channel >= info.b_nchans) {
        return luaL_error(L, "Channel index out of range");
    }

    // Get table length
    long table_len = (long)lua_objlen(L, 3);

    // Determine how many frames to write
    long num_frames = table_len;
    if (start_frame + num_frames > info.b_frames) {
        num_frames = info.b_frames - start_frame;
    }

    // Lock samples
    float* samples = buffer_locksamples(obj);
    if (!samples) {
        return luaL_error(L, "Failed to lock buffer samples");
    }

    // Write samples from table
    for (long i = 0; i < num_frames; i++) {
        lua_rawgeti(L, 3, i + 1);  // Lua 1-indexed
        float value = (float)lua_tonumber(L, -1);
        lua_pop(L, 1);

        long frame = start_frame + i;
        long index = frame * info.b_nchans + channel;
        samples[index] = value;
    }

    buffer_unlocksamples(obj);
    buffer_setdirty(obj);

    return 0;
}

// Buffer:clear() - Clear buffer (set all samples to 0)
static int Buffer_clear(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);

    t_buffer_obj* obj = buffer_ref_getobject(ud->buffer_ref);
    if (!obj) {
        return luaL_error(L, "Buffer reference is not valid");
    }

    t_buffer_info info;
    buffer_getinfo(obj, &info);

    float* samples = buffer_locksamples(obj);
    if (!samples) {
        return luaL_error(L, "Failed to lock buffer samples");
    }

    // Clear all samples
    long total_samples = info.b_frames * info.b_nchans;
    for (long i = 0; i < total_samples; i++) {
        samples[i] = 0.0f;
    }

    buffer_unlocksamples(obj);
    buffer_setdirty(obj);

    return 0;
}

// Buffer:pointer() - Get raw pointer value
static int Buffer_pointer(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);
    lua_pushnumber(L, (lua_Number)(intptr_t)ud->buffer_ref);
    return 1;
}

// __gc metamethod (destructor)
static int Buffer_gc(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);
    if (ud->owns_ref && ud->buffer_ref) {
        object_free(ud->buffer_ref);
        ud->buffer_ref = NULL;
    }
    return 0;
}

// __tostring metamethod
static int Buffer_tostring(lua_State* L) {
    BufferUD* ud = (BufferUD*)luaL_checkudata(L, 1, BUFFER_MT);

    t_buffer_obj* obj = buffer_ref_getobject(ud->buffer_ref);
    if (!obj) {
        lua_pushstring(L, "Buffer(not bound)");
        return 1;
    }

    t_buffer_info info;
    buffer_getinfo(obj, &info);

    lua_pushfstring(L, "Buffer(frames=%d, channels=%d, sr=%.1f)",
                    (int)info.b_frames, (int)info.b_nchans, info.b_sr);
    return 1;
}

// Register Buffer type
static void register_buffer_type(lua_State* L) {
    // Create metatable
    luaL_newmetatable(L, BUFFER_MT);

    // Register methods
    lua_pushcfunction(L, Buffer_ref_set);
    lua_setfield(L, -2, "ref_set");

    lua_pushcfunction(L, Buffer_exists);
    lua_setfield(L, -2, "exists");

    lua_pushcfunction(L, Buffer_getinfo);
    lua_setfield(L, -2, "getinfo");

    lua_pushcfunction(L, Buffer_frames);
    lua_setfield(L, -2, "frames");

    lua_pushcfunction(L, Buffer_channels);
    lua_setfield(L, -2, "channels");

    lua_pushcfunction(L, Buffer_samplerate);
    lua_setfield(L, -2, "samplerate");

    lua_pushcfunction(L, Buffer_peek);
    lua_setfield(L, -2, "peek");

    lua_pushcfunction(L, Buffer_poke);
    lua_setfield(L, -2, "poke");

    lua_pushcfunction(L, Buffer_to_list);
    lua_setfield(L, -2, "to_list");

    lua_pushcfunction(L, Buffer_from_list);
    lua_setfield(L, -2, "from_list");

    lua_pushcfunction(L, Buffer_clear);
    lua_setfield(L, -2, "clear");

    lua_pushcfunction(L, Buffer_pointer);
    lua_setfield(L, -2, "pointer");

    // Register metamethods
    lua_pushcfunction(L, Buffer_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, Buffer_tostring);
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

    lua_pushcfunction(L, Buffer_new);
    lua_setfield(L, -2, "Buffer");

    lua_pop(L, 1);  // Pop api table
}

#endif // LUAJIT_API_BUFFER_H

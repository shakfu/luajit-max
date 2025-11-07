# New `luajit` External: Architecture and Implementation Plan

## Executive Summary

This document describes the architecture for a new **`luajit`** external that enables writing general-purpose Max externals entirely in Lua, mirroring the functionality of the **pyext** external from the pktpy project.

## Background

### Current State

The luajit-max package currently provides two DSP-focused externals:

1. **luajit~** - Basic LuaJIT engine for real-time audio processing
2. **luajit.stk~** - LuaJIT + STK synthesis library

Both externals:
- Run Lua code in the audio thread with real-time constraints
- Have fixed architecture (1 inlet, 1 outlet)
- Focus on per-sample/vector DSP processing
- Include **libapi** wrapper (21 Max API types)

### Motivation

**pyext** (from pktpy) demonstrates a different use case:
- General-purpose Max scripting (not audio-specific)
- Runs in Max main thread (no real-time constraints)
- Dynamic inlets/outlets (1-16 each)
- Full message routing (bang, int, float, list, anything, custom)
- Python class becomes the external behavior

## Recommendation: Create New `luajit` External

Create a **third external** called `luajit` (non-tilde) that brings pyext's architecture to Lua.

### Why a Separate External?

| Aspect | luajit~ / luajit.stk~ | luajit (new) |
|--------|----------------------|--------------|
| **Thread** | Audio thread | Main thread |
| **Constraints** | Real-time safe | No RT constraints |
| **I/O** | Fixed audio I/O | Dynamic inlets/outlets |
| **Use Case** | DSP algorithms | General Max scripting |
| **Scripting Model** | Function-based | Object/class-based |

Mixing these concerns in one external would create unnecessary complexity.

## Architecture Design

### Project Structure

```
luajit-max/
├── source/projects/
│   ├── luajit~/          # Audio DSP (keep as-is)
│   ├── luajit.stk~/      # Audio DSP + STK (keep as-is)
│   ├── luajit/           # NEW: General-purpose scripting
│   │   ├── luajit.c      # External implementation
│   │   └── CMakeLists.txt
│   └── libapi/           # Shared Max API (already exists)
├── examples/
│   └── luajit_example.lua  # Example script
└── help/
    └── luajit.maxhelp    # Help patch
```

### Data Structure

```c
// luajit.c
#define LUAJIT_MAX_INLETS 16
#define LUAJIT_MAX_OUTLETS 16

typedef struct _luajit {
    t_object ob;                    // Max object header

    // Lua instance
    lua_State* L;                   // Lua state (one per instance)
    char instance_name[128];        // Unique name for registry

    // Script info
    t_symbol* script_name;          // Script filename
    char script_path[MAX_PATH_CHARS];
    short script_path_id;           // Max path ID

    // Dynamic I/O
    long num_inlets;                // Configurable (1-16)
    long num_outlets;               // Configurable (1-16)
    void* inlets[LUAJIT_MAX_INLETS];    // Proxy inlets
    void* outlets[LUAJIT_MAX_OUTLETS];  // Outlet pointers
    long inlet_num;                 // Current inlet (for proxies)

    // Text editor integration (future)
    t_object* editor;
    char** code_buffer;
    long run_on_save;
    long run_on_close;
} t_luajit;
```

### Object Lifecycle

#### 1. Initialization (luajit_new)

```c
void* luajit_new(t_symbol* s, long argc, t_atom* argv) {
    t_luajit* x = object_alloc(luajit_class);

    // 1. Generate unique instance name
    snprintf(x->instance_name, 128, "_luajit_inst_%p", x);

    // 2. Initialize Lua per-instance
    x->L = luaL_newstate();
    luaL_openlibs(x->L);

    // 3. Initialize libapi module
    luajit_api_init(x->L);

    // 4. Default configuration
    x->num_inlets = 1;
    x->num_outlets = 1;

    // 5. Get script filename (first argument)
    x->script_name = atom_getsymarg(0, argc, argv);
    if (x->script_name == gensym("")) {
        error("luajit: requires script filename as first argument");
        object_free(x);
        return NULL;
    }

    // 6. Load Lua script
    if (!luajit_load_script(x)) {
        error("luajit: failed to load script %s", x->script_name->s_name);
        object_free(x);
        return NULL;
    }

    // 7. Read inlet/outlet config from Lua
    luajit_configure_io(x);

    // 8. Create inlets/outlets based on config
    luajit_create_io(x);

    // 9. Inject outlet wrappers into Lua instance
    luajit_inject_outlets(x);

    return x;
}
```

#### 2. Script Loading

```c
bool luajit_load_script(t_luajit* x) {
    char filepath[MAX_PATH_CHARS];
    char filename[MAX_PATH_CHARS];
    t_fourcc filetype = 0;
    t_fourcc outtype = 0;
    short path_id;

    // Locate file in Max search path
    if (locatefile_extended(x->script_name->s_name, &path_id,
                           &outtype, &filetype, 1)) {
        error("luajit: cannot find script %s", x->script_name->s_name);
        return false;
    }

    // Get absolute path
    path_toabsolutesystempath(path_id, x->script_name->s_name, filepath);
    x->script_path_id = path_id;
    strcpy(x->script_path, filepath);

    // Execute script
    if (luaL_dofile(x->L, filepath) != LUA_OK) {
        const char* err = lua_tostring(x->L, -1);
        error("luajit: %s", err);
        lua_pop(x->L, 1);
        return false;
    }

    post("luajit: loaded %s", filepath);
    return true;
}
```

#### 3. I/O Configuration

```c
void luajit_configure_io(t_luajit* x) {
    // Get 'external' table from global scope
    lua_getglobal(x->L, "external");

    if (!lua_istable(x->L, -1)) {
        lua_pop(x->L, 1);
        return;  // Use defaults
    }

    // Read inlets
    lua_getfield(x->L, -1, "inlets");
    if (lua_isnumber(x->L, -1)) {
        long n = lua_tointeger(x->L, -1);
        x->num_inlets = CLAMP(n, 1, LUAJIT_MAX_INLETS);
    }
    lua_pop(x->L, 1);

    // Read outlets
    lua_getfield(x->L, -1, "outlets");
    if (lua_isnumber(x->L, -1)) {
        long n = lua_tointeger(x->L, -1);
        x->num_outlets = CLAMP(n, 1, LUAJIT_MAX_OUTLETS);
    }
    lua_pop(x->L, 1);

    lua_pop(x->L, 1);  // Pop external table
}
```

#### 4. I/O Creation

```c
void luajit_create_io(t_luajit* x) {
    // Create inlets (first inlet is automatic)
    for (long i = 1; i < x->num_inlets; i++) {
        x->inlets[i] = proxy_new(x, i, &x->inlet_num);
    }

    // Create outlets (right to left)
    for (long i = x->num_outlets - 1; i >= 0; i--) {
        x->outlets[i] = outlet_new(x, NULL);
    }
}
```

#### 5. Outlet Injection

```c
void luajit_inject_outlets(t_luajit* x) {
    lua_newtable(x->L);  // Create _outlets table

    for (long i = 0; i < x->num_outlets; i++) {
        // Create lightweight outlet wrapper userdata
        OutletWrapper* wrapper = lua_newuserdata(x->L, sizeof(OutletWrapper));
        wrapper->outlet = x->outlets[i];

        // Set metatable with outlet methods
        luaL_getmetatable(x->L, "api.OutletWrapper");
        lua_setmetatable(x->L, -2);

        lua_rawseti(x->L, -2, i + 1);  // 1-indexed
    }

    lua_setglobal(x->L, "_outlets");
}
```

### Message Dispatch

#### Generic Dispatch Helper

```c
static bool luajit_call_method(t_luajit* x, const char* method_name,
                               long argc, t_atom* argv) {
    // Get external table
    lua_getglobal(x->L, "external");
    if (!lua_istable(x->L, -1)) {
        lua_pop(x->L, 1);
        return false;  // Silent if no external table
    }

    // Get method
    lua_getfield(x->L, -1, method_name);
    if (!lua_isfunction(x->L, -1)) {
        lua_pop(x->L, 2);
        return false;  // Silent if method doesn't exist
    }

    // Push arguments
    for (long i = 0; i < argc; i++) {
        switch (atom_gettype(&argv[i])) {
            case A_LONG:
                lua_pushinteger(x->L, atom_getlong(&argv[i]));
                break;
            case A_FLOAT:
                lua_pushnumber(x->L, atom_getfloat(&argv[i]));
                break;
            case A_SYM:
                lua_pushstring(x->L, atom_getsym(&argv[i])->s_name);
                break;
            default:
                lua_pushnil(x->L);
        }
    }

    // Call method
    if (lua_pcall(x->L, argc, 0, 0) != LUA_OK) {
        error("luajit: %s", lua_tostring(x->L, -1));
        lua_pop(x->L, 1);
        lua_pop(x->L, 1);  // Pop external table
        return false;
    }

    lua_pop(x->L, 1);  // Pop external table
    return true;
}
```

#### Standard Message Handlers

```c
void luajit_bang(t_luajit* x) {
    luajit_call_method(x, "bang", 0, NULL);
}

void luajit_int(t_luajit* x, long n) {
    t_atom atom;
    atom_setlong(&atom, n);
    luajit_call_method(x, "int", 1, &atom);
}

void luajit_float(t_luajit* x, double f) {
    t_atom atom;
    atom_setfloat(&atom, f);
    luajit_call_method(x, "float", 1, &atom);
}

void luajit_list(t_luajit* x, t_symbol* s, long argc, t_atom* argv) {
    luajit_call_method(x, "list", argc, argv);
}

void luajit_anything(t_luajit* x, t_symbol* s, long argc, t_atom* argv) {
    luajit_call_method(x, s->s_name, argc, argv);
}
```

#### Hot Reload

```c
void luajit_reload(t_luajit* x) {
    // Clear current state
    lua_pushnil(x->L);
    lua_setglobal(x->L, "external");

    lua_pushnil(x->L);
    lua_setglobal(x->L, "_outlets");

    // Reload script
    if (!luajit_load_script(x)) {
        error("luajit: reload failed");
        return;
    }

    // Re-configure (may change I/O counts)
    long old_inlets = x->num_inlets;
    long old_outlets = x->num_outlets;

    luajit_configure_io(x);

    // Check if I/O changed
    if (x->num_inlets != old_inlets || x->num_outlets != old_outlets) {
        error("luajit: inlet/outlet count changed - requires recreating object");
        // Restore old config
        x->num_inlets = old_inlets;
        x->num_outlets = old_outlets;
    }

    // Re-inject outlets
    luajit_inject_outlets(x);

    post("luajit: reloaded %s", x->script_path);
}
```

### Lua Script Structure

#### Minimal Example

```lua
-- examples/simple.lua

external = {
    inlets = 1,
    outlets = 1
}

function external.bang()
    api.post("Bang received!")
    _outlets[1]:int(42)
end
```

#### Full Featured Example

```lua
-- examples/counter.lua
local api = api  -- Local reference for performance

external = {
    inlets = 2,
    outlets = 3
}

-- Instance state
local count = 0
local step = 1

function external.bang()
    api.post("Count: " .. count)
    _outlets[1]:int(count)
    count = count + step
end

function external.int(n)
    local inlet = api.proxy_getinlet()  -- Get inlet number
    if inlet == 0 then
        -- First inlet: set count
        count = n
        api.post("Count set to: " .. count)
    elseif inlet == 1 then
        -- Second inlet: set step
        step = n
        api.post("Step set to: " .. step)
    end
end

function external.float(f)
    _outlets[2]:float(f * 2.0)
end

function external.reset()
    count = 0
    step = 1
    api.post("Counter reset")
end

function external.set(n, s)
    count = n or 0
    step = s or 1
    api.post(string.format("Set count=%d, step=%d", count, step))
end
```

### libapi Extensions

The existing libapi already has 21 API wrappers. For the new external, we need to add:

#### 1. Lightweight Outlet Wrapper

```c
// In api_outlet.h - add injection support

typedef struct {
    void* outlet;
} OutletWrapper;

static int outlet_wrapper_bang(lua_State* L) {
    OutletWrapper* w = luaL_checkudata(L, 1, "api.OutletWrapper");
    outlet_bang(w->outlet);
    return 0;
}

static int outlet_wrapper_int(lua_State* L) {
    OutletWrapper* w = luaL_checkudata(L, 1, "api.OutletWrapper");
    long n = luaL_checkinteger(L, 2);
    outlet_int(w->outlet, n);
    return 0;
}

static int outlet_wrapper_float(lua_State* L) {
    OutletWrapper* w = luaL_checkudata(L, 1, "api.OutletWrapper");
    double f = luaL_checknumber(L, 2);
    outlet_float(w->outlet, f);
    return 0;
}

static int outlet_wrapper_list(lua_State* L) {
    OutletWrapper* w = luaL_checkudata(L, 1, "api.OutletWrapper");
    // Convert Lua table to atom array
    // ... implementation
    outlet_list(w->outlet, NULL, argc, argv);
    return 0;
}

void register_outlet_wrapper_type(lua_State* L) {
    luaL_newmetatable(L, "api.OutletWrapper");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, outlet_wrapper_bang);
    lua_setfield(L, -2, "bang");

    lua_pushcfunction(L, outlet_wrapper_int);
    lua_setfield(L, -2, "int");

    lua_pushcfunction(L, outlet_wrapper_float);
    lua_setfield(L, -2, "float");

    lua_pushcfunction(L, outlet_wrapper_list);
    lua_setfield(L, -2, "list");

    lua_pop(L, 1);
}
```

#### 2. Proxy Inlet Helper

```c
// In api_inlet.h - add proxy helper

static int api_proxy_getinlet(lua_State* L) {
    // This requires external context - needs to be set by external
    // Store external pointer in registry
    lua_getfield(L, LUA_REGISTRYINDEX, "_luajit_current_inlet");
    if (lua_isnumber(L, -1)) {
        lua_pushinteger(L, lua_tointeger(L, -1));
        return 1;
    }
    lua_pushinteger(L, 0);
    return 1;
}
```

### CMakeLists.txt

```cmake
# source/projects/luajit/CMakeLists.txt

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-pretarget.cmake)

# Dependencies
set(LUAJIT_LIB "${CMAKE_BINARY_DIR}/deps/luajit-install/lib/libluajit-5.1.a")

# Source files
set(SOURCE_FILES luajit.c)

# Create the external
add_library(${PROJECT_NAME} MODULE ${SOURCE_FILES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../libapi
    ${CMAKE_BINARY_DIR}/deps/luajit-install/include/luajit-2.1
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${LUAJIT_LIB})

# Platform-specific flags
if(APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-all_load")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".mxo")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../../max-sdk-base/script/max-posttarget.cmake)
```

## Implementation Checklist

- [ ] Create `source/projects/luajit/` directory
- [ ] Implement `luajit.c` with core structure
- [ ] Implement script loading and Lua initialization
- [ ] Implement I/O configuration reading
- [ ] Implement dynamic inlet/outlet creation
- [ ] Implement message dispatch (bang, int, float, list, anything)
- [ ] Implement outlet injection system
- [ ] Add outlet wrapper to libapi
- [ ] Add proxy inlet helper to libapi
- [ ] Create `CMakeLists.txt`
- [ ] Create example scripts in `examples/`
- [ ] Create help patch in `help/luajit.maxhelp`
- [ ] Update root `CLAUDE.md` with new external info
- [ ] Build and test

## Future Enhancements

### Phase 2: Text Editor Integration
- Double-click to open code editor
- `run_on_save` attribute
- `run_on_close` attribute
- Live editing support

### Phase 3: Advanced Features
- Inlet-aware message dispatch (pass inlet number to methods)
- Typed inlets (int, float, signal)
- Attribute system (expose Lua variables as Max attributes)
- Clock/timer integration
- Buffer access for non-realtime processing
- Dict/hashtab integration for structured data

### Phase 4: Package Management
- Support for Lua modules (require path setup)
- Bundle external Lua libraries (luafun, etc.)
- Module hot-reload

## Comparison Matrix

| Feature | pyext (Python) | luajit (Lua) |
|---------|----------------|--------------|
| **Language** | Python (pocketpy) | Lua (LuaJIT) |
| **Binary Size** | ~815KB | TBD |
| **Performance** | Interpreted | JIT compiled |
| **Inlets** | 1-16 dynamic | 1-16 dynamic |
| **Outlets** | 1-16 dynamic | 1-16 dynamic |
| **Message Types** | All standard | All standard |
| **Hot Reload** | Yes | Yes |
| **Text Editor** | Yes | Planned |
| **API Coverage** | ~1400 lines | 21 types |
| **Threading** | Main thread | Main thread |
| **Use Case** | General scripting | General scripting |

## Key Design Patterns

### 1. Singleton vs Per-Instance
- **pyext**: Global singleton Python VM (all instances share)
- **luajit**: Per-instance Lua states (isolated)

Rationale: LuaJIT is lightweight enough for per-instance states, providing better isolation.

### 2. Configuration Discovery
- Read `external.inlets` and `external.outlets` from Lua global table
- Configuration must be set before I/O creation
- Cannot change dynamically after creation

### 3. Error Handling
- Missing methods → silent (allows partial implementation)
- Lua errors → error() to Max console
- Type errors → clear error messages

### 4. Memory Management
- Lua state lifecycle tied to Max object lifecycle
- lua_close() on destruction handles cleanup
- Outlets freed automatically by Max

## References

- pyext implementation: `~/projects/pktpy/source/projects/pyext/`
- pktpy API: `~/projects/pktpy/source/projects/pktpy/api/`
- Current libapi: `~/projects/luajit-max/source/projects/libapi/`
- luajit~ external: `~/projects/luajit-max/source/projects/luajit~/`

# libapi - Remaining API Wrapper Tasks

This document tracks the remaining Max C-API wrappers to be implemented for the luajit-max project.

## Status Legend
- ✅ **Complete** - Fully implemented and tested
- 🚧 **In Progress** - Currently being worked on
- 📋 **Planned** - Scheduled for implementation
- 💭 **Proposed** - Potential future addition
- ⏸️ **On Hold** - Deferred for now

---

## Core API Wrappers

### ✅ Completed

- [x] **api_common.h** - Common utilities and type conversions
- [x] **api_symbol.h** - Symbol wrapper (`api.Symbol`, `api.gensym`)
- [x] **api_atom.h** - Atom wrapper (`api.Atom`, `api.parse`, `api.atom_gettext`)
- [x] **api_clock.h** - Clock/scheduling wrapper (`api.Clock`)
- [x] **api_outlet.h** - Outlet wrapper (`api.Outlet`)
- [x] **api_table.h** - Table wrapper (`api.Table`)

### 📋 High Priority (Core Functionality)

- [x] **api_atomarray.h** - AtomArray collection wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_atomarray.h`
  - Implemented features:
    - Create atomarray from Lua table or empty
    - Append/clear operations
    - Array-style indexing (1-based, supports negative indices)
    - Size/length operations
    - Duplicate/copy functionality
    - Type-specific extraction (to_ints, to_floats, to_symbols)
    - to_list/to_text conversion
    - Full integration with api module
  - Status: **COMPLETED** 2025-11-07

- [x] **api_buffer.h** - MSP buffer wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_buffer.h`
  - Implemented features:
    - Buffer reference management (ref_new, ref_set)
    - exists() check for buffer validity
    - getinfo() returns full buffer metadata
    - Lock/unlock sample access (automatic in peek/poke)
    - frames(), channels(), samplerate() queries
    - peek(frame, channel) for single sample reads
    - poke(frame, channel, value) for single sample writes
    - to_list(channel, start, count) for bulk export
    - from_list(channel, table, start) for bulk import
    - clear() to zero all samples
  - Status: **COMPLETED** 2025-11-07

- [x] **api_dictionary.h** - Dictionary wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_dictionary.h`
  - Implemented features:
    - Create empty dictionary
    - Generic get/set with automatic type detection
    - Type-specific operations: getlong, getfloat, getstring, getsym
    - Type-specific setters: setlong, setfloat, setstring, setsym
    - has(key), delete(key), clear()
    - keys() to list all keys
    - size() and __len for entry count
    - read(filename, path) and write(filename, path) for file I/O
    - dump(recurse, console) for debugging
    - dict[key] syntax for get/set operations
    - Nested dictionary support
    - Atomarray conversion to Lua tables
    - Full metatable support
  - Status: **COMPLETED** 2025-11-07

### 📋 Medium Priority (Enhanced Functionality)

- [x] **api_object.h** - Generic object wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_object.h`
  - Implemented features:
    - Object creation via object_new_typed()
    - Object wrapping from pointer (non-owning)
    - Method calling with object_method_typed()
    - Attribute get/set with type detection
    - Class name query
    - Attribute names enumeration
    - Free/destroy with ownership tracking
    - is_null() check
    - Full metatable support
  - Status: **COMPLETED** 2025-11-07

- [x] **api_patcher.h** - Patcher manipulation wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_patcher.h`
  - Implemented features:
    - Wrap existing patcher pointer
    - newobject(text) to create objects
    - locked() get/set for patcher lock state
    - title() get/set for window title
    - rect() get/set for window position/size
    - parentpatcher() and toppatcher() navigation
    - dirty() to mark patcher modified
    - count() to count objects
    - name(), filepath(), filename() for file info
    - Full metatable support
  - Status: **COMPLETED** 2025-11-07

- [x] **api_inlet.h** - Inlet wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_inlet.h`
  - Implemented features:
    - inlet_new() for general-purpose inlets
    - intin() for integer-typed inlets
    - floatin() for float-typed inlets
    - proxy_new() for modern proxy inlets
    - proxy_getinlet() to identify inlet
    - inlet_count() to count inlets
    - inlet_nth() to get specific inlet
    - delete() to remove owned inlets
    - is_proxy(), is_null(), get_num() queries
    - Full metatable support
  - Status: **COMPLETED** 2025-11-07

- [x] **api_box.h** - Box wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_box.h`
  - Implemented features:
    - wrap() to wrap existing box pointer
    - is_null() to check validity
    - classname() to get object class
    - get_object() to get underlying Max object
    - get_rect() to retrieve position/size
    - set_rect() to modify position/size
    - Full metatable support
  - Status: **COMPLETED** 2025-11-07

- [x] **api_patchline.h** - Patchline (connection) wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_patchline.h`
  - Implemented features:
    - wrap() to wrap existing patchline pointer
    - get_box1() and get_box2() for source/destination boxes
    - get_outletnum() and get_inletnum() for connection points
    - get_startpoint() and get_endpoint() for coordinates
    - get_hidden() and set_hidden() for visibility control
    - get_nextline() for patchline iteration
    - Full metatable support
  - Status: **COMPLETED** 2025-11-07

- [ ] **api_box.h** - Box wrapper
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_box.h`
  - Features needed:
    - Get/set box rect
    - Get box object
    - Get box class name
    - Box attributes
  - Priority: **MEDIUM** (complements api_patcher)

### 💭 Low Priority (Advanced Features)

- [ ] **api_hashtab.h** - Hashtable wrapper
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_hashtab.h`
  - Features needed:
    - Create/destroy hashtab
    - Store/lookup/delete
    - Key enumeration
    - Iteration
  - Priority: **LOW** (Dictionary covers most use cases)

- [ ] **api_linklist.h** - Linked list wrapper
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_linklist.h`
  - Features needed:
    - Create/destroy linklist
    - Append/insert operations
    - Index access
    - Iteration
    - Reverse/rotate/shuffle
  - Priority: **LOW** (Lua tables are more idiomatic)

- [ ] **api_qelem.h** - Queue element wrapper
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_qelem.h`
  - Features needed:
    - Create qelem with callback
    - Set/unset/front operations
    - Queue-based deferred execution
  - Priority: **LOW** (Clock covers most scheduling needs)

- [ ] **api_systhread.h** - Threading wrapper
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_systhread.h`
  - Features needed:
    - Thread creation with callback
    - Join/detach
    - Mutex/lock support
    - Sleep operations
  - Priority: **LOW** (Threading from Lua is complex and risky)

- [ ] **api_path.h** - File path utilities
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_path.h`
  - Features needed:
    - Path resolution
    - File location (locatefile_extended)
    - Path conversion (absolute/relative)
    - File I/O helpers
  - Priority: **LOW** (Lua has built-in file I/O)

- [ ] **api_time.h** - ITM (time object) wrapper
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_time.h`
  - Features needed:
    - Get transport time
    - Ticks/ms/samples conversion
    - Time signature
    - Tempo sync
  - Priority: **LOW** (advanced timing features)

- [ ] **api_preset.h** - Preset system integration
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_preset.h`
  - Features needed:
    - Store/recall presets
    - Preset interpolation
  - Priority: **LOW** (specialized use case)

- [ ] **api_database.h** - SQLite database wrapper
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_database.h`
  - Features needed:
    - Open/close database
    - Query execution
    - Result iteration
    - Transaction support
  - Priority: **LOW** (specialized use case)

---

## Implementation Guidelines

### When implementing a new wrapper:

1. **Study the pktpy reference** - Located at `~/projects/pktpy/source/projects/pktpy/api/api_*.h`
2. **Follow established patterns:**
   - Use Lua userdata with metatables
   - Metatable name: `"Max.ClassName"`
   - Constructor: `ClassName_new(lua_State* L)`
   - Methods: `ClassName_method(lua_State* L)`
   - Metamethods: `__gc`, `__tostring`, `__index`, etc.
3. **LuaJIT/Lua 5.1 compatibility:**
   - Use `lua_objlen()` instead of `lua_rawlen()`
   - Check for whole numbers with `modf()` instead of `lua_isinteger()`
   - Avoid Lua 5.2+ specific features
4. **Error handling:**
   - Use `luaL_error()` for runtime errors
   - Use `luaL_checktype()` and `luaL_checkudata()` for type checks
   - Return helpful error messages
5. **Memory management:**
   - Implement `__gc` metamethod for cleanup
   - Use Lua registry for callback references
   - Free Max resources properly
6. **Register the type:**
   - Create `register_*_type(lua_State* L)` function
   - Add to `luajit_api_init()` in `luajit_api.h`
7. **Documentation:**
   - Add to README.md API coverage section
   - Create example usage in comments
   - Update `examples/api_demo.lua` if appropriate
8. **Testing:**
   - Build with `make`
   - Test in Max with both luajit~ and luajit.stk~
   - Verify memory cleanup (check for leaks)

### Template Structure

```c
// api_name.h
// Description wrapper for luajit-max API

#ifndef LUAJIT_API_NAME_H
#define LUAJIT_API_NAME_H

#include "api_common.h"

#define NAME_MT "Max.Name"

typedef struct {
    // Max type pointer
    // Additional state
} NameUD;

static int Name_new(lua_State* L) { /* ... */ }
static int Name_method(lua_State* L) { /* ... */ }
static int Name_gc(lua_State* L) { /* ... */ }
static int Name_tostring(lua_State* L) { /* ... */ }

static void register_name_type(lua_State* L) {
    luaL_newmetatable(L, NAME_MT);
    // Register methods and metamethods
    // Register constructor in api module
}

#endif // LUAJIT_API_NAME_H
```

---

## Progress Tracking

### Statistics
- **Completed:** 13 wrappers (Symbol, Atom, Clock, Outlet, Table, AtomArray, Buffer, Dictionary, Object, Patcher, Inlet, Box)
- **High Priority:** ✅ ALL 3 COMPLETED (AtomArray, Buffer, Dictionary)
- **Medium Priority:** 4 of 5 completed (Object ✅, Patcher ✅, Inlet ✅, Box ✅)
- **Low Priority:** 10 wrappers remaining
- **Total Planned:** 24 wrappers
- **Completion:** 54% (13/24 wrappers completed)

### Current Focus
All HIGH priority wrappers completed! MEDIUM priority nearly done:
1. ✅ **api_object.h** - Generic Max object wrapper (COMPLETED)
2. ✅ **api_patcher.h** - Patcher manipulation (COMPLETED)
3. ✅ **api_inlet.h** - Inlet operations (COMPLETED)
4. ✅ **api_box.h** - Box wrapper (COMPLETED)
5. **api_patchline.h** - Connection wrapper (LAST MEDIUM priority)

---

## Notes

- Some wrappers may not be worth implementing (threading, database) due to complexity or limited use case
- Focus on wrappers that enable common Max/MSP patterns
- Consider user feedback to prioritize which wrappers to implement next
- Each wrapper adds ~200-400 lines of code
- Testing should include memory leak checks and proper cleanup verification

---

Last Updated: 2025-11-07

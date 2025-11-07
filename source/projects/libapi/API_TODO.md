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

- [x] **api_box.h** - Box wrapper
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_box.h`
  - Features needed:
    - Get/set box rect
    - Get box object
    - Get box class name
    - Box attributes
  - Priority: **MEDIUM** (complements api_patcher)

### 💭 Low Priority (Advanced Features)

- [x] **api_hashtab.h** - Hashtable wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_hashtab.h`
  - Implemented features:
    - Create hashtable with optional size
    - Wrap existing hashtable pointer
    - store(key, value) and lookup(key, default) operations
    - delete(key), clear(), keys() operations
    - has_key(key), getsize() queries
    - hashtab[key] syntax for get/set
    - Full metatable support with __len, __index, __newindex
    - Automatic type detection for stored values
  - Status: **COMPLETED** 2025-11-07
  - Note: Dictionary is still preferred for general use, but Hashtab provides direct access to Max's native hashtable API

- [x] **api_linklist.h** - Linked list wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_linklist.h`
  - Implemented features:
    - Create linklist with Linklist() constructor
    - Wrap existing linklist pointer with linklist_wrap()
    - Append/insert operations: append(), insertindex()
    - Index access: getindex(), linklist[index] with negative indices
    - Remove operations: chuckindex(), deleteindex(), clear()
    - List manipulation: reverse(), rotate(), shuffle(), swap()
    - Size queries: getsize(), __len metamethod
    - Full metatable support with __gc, __tostring, __index
  - Status: **COMPLETED** 2025-11-07
  - Note: While Lua tables are more idiomatic, this provides direct access to Max's native linklist API for interoperability with Max objects

- [x] **api_qelem.h** - Queue element wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_qelem.h`
  - Implemented features:
    - Create qelem with Lua callback and optional userdata
    - set(), unset(), front() operations
    - Queue-based deferred execution in main thread
    - Automatic callback reference management via Lua registry
    - is_set(), is_null() state queries
    - Full metatable support with __gc cleanup
  - Status: **COMPLETED** 2025-11-07
  - Note: Provides queue-based deferred execution for UI updates, complementing Clock's timer-based scheduling

- [ ] **api_systhread.h** - Threading wrapper
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_systhread.h`
  - Features needed:
    - Thread creation with callback
    - Join/detach
    - Mutex/lock support
    - Sleep operations
  - Priority: **LOW** (Threading from Lua is complex and risky)

- [x] **api_path.h** - File path utilities ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_path.h`
  - Implemented features:
    - path_getdefault(), path_setdefault(), path_getapppath()
    - locatefile_extended() for finding files in Max search paths
    - path_toabsolutesystempath() for path conversion
    - path_nameconform() for path style conversion
    - File operations: path_opensysfile(), path_createsysfile(), path_closesysfile()
    - Low-level I/O: sysfile_read(), sysfile_write()
    - File positioning: sysfile_getpos(), sysfile_setpos(), sysfile_geteof(), sysfile_seteof()
    - sysfile_readtextfile() for reading entire text files
    - path_deletefile() for file deletion
  - Status: **COMPLETED** 2025-11-07
  - Note: Provides Max-specific file operations beyond Lua's built-in I/O

- [x] **api_time.h** - ITM (time object) wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_time.h`
  - Implemented features:
    - ITM creation: ITM() for global, ITM(name) for named, ITM(ptr) to wrap
    - Time queries: getticks(), gettime(), getstate()
    - Time conversions: tickstoms(), mstoticks(), mstosamps(), sampstoms()
    - BBU (bars/beats/units) support: bbutoticsk(), tickstobbu()
    - Transport control: pause(), resume(), seek()
    - Time signature: settimesignature(), gettimesignature()
    - Utilities: dump(), sync(), pointer(), is_valid()
    - Module functions: itm_getglobal(), itm_setresolution(), itm_getresolution()
    - Full metatable support with ownership tracking
  - Status: **COMPLETED** 2025-11-07
  - Note: Provides tempo-aware timing beyond basic Clock, essential for musical timing

- [x] **api_preset.h** - Preset system integration ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_preset.h`
  - Implemented features:
    - preset_store(format) to store preset data
    - preset_set(object_ptr, value) to set preset value for object
    - preset_int(object_ptr, value) to send preset int to object
    - preset_get_data_symbol() to get preset data symbol name
    - Module-level functions only (no class wrapper needed)
  - Status: **COMPLETED** 2025-11-07
  - Note: Provides integration with Max's preset system for state management

- [x] **api_database.h** - SQLite database wrapper ✅ COMPLETED
  - Reference: `~/projects/pktpy/source/projects/pktpy/api/api_database.h`
  - Implemented features:
    - Database type: open(), close(), is_open()
    - Query execution: query() returns DBResult
    - Transaction support: transaction_start(), transaction_end(), transaction_flush()
    - Table operations: create_table(), add_column()
    - get_last_insert_id() for INSERT operations
    - DBResult type: numrecords(), numfields(), fieldname()
    - Data access: get_string(), get_long(), get_float(), get_record()
    - to_list() for converting results to Lua tables
    - reset(), clear() for result management
    - Full metatable support with __len for DBResult
  - Status: **COMPLETED** 2025-11-07
  - Note: Provides full SQLite integration via Max's database API

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
- **Completed:** 21 wrappers (Symbol, Atom, Clock, Outlet, Table, AtomArray, Buffer, Dictionary, Object, Patcher, Inlet, Box, Patchline, Path, Database, Hashtab, ITM, Preset, Qelem, Linklist)
- **High Priority:** ✅ ALL 3 COMPLETED (AtomArray, Buffer, Dictionary)
- **Medium Priority:** ✅ ALL 5 COMPLETED (Object, Patcher, Inlet, Box, Patchline)
- **Low Priority:** 7 of 10 completed (Path ✅, Database ✅, Hashtab ✅, ITM ✅, Preset ✅, Qelem ✅, Linklist ✅)
- **Total Planned:** 24 wrappers
- **Completion:** 88% (21/24 wrappers completed)

### Current Focus
All HIGH and MEDIUM priority wrappers completed! LOW priority progress:
1. ✅ **api_path.h** - File path utilities (COMPLETED)
2. ✅ **api_database.h** - SQLite database (COMPLETED)
3. ✅ **api_hashtab.h** - Hash table (COMPLETED)
4. ✅ **api_time.h** - ITM transport and timing (COMPLETED)
5. ✅ **api_preset.h** - Preset system integration (COMPLETED)
6. ✅ **api_qelem.h** - Queue-based deferred execution (COMPLETED)
7. ✅ **api_linklist.h** - Linked list data structure (COMPLETED)
8. **Remaining LOW priority:** Threading (3 wrappers remaining)

---

## Notes

- Some wrappers may not be worth implementing (threading, database) due to complexity or limited use case
- Focus on wrappers that enable common Max/MSP patterns
- Consider user feedback to prioritize which wrappers to implement next
- Each wrapper adds ~200-400 lines of code
- Testing should include memory leak checks and proper cleanup verification

---

Last Updated: 2025-11-07

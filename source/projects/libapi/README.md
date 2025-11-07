# libapi - Max API Wrappers for LuaJIT

This directory contains header-only C libraries that wrap the Max C API for use from Lua scripts running in LuaJIT externals.

## Architecture

The API module is header-only and can be included in both C and C++ externals:
- `luajit~` (C external)
- `luajit.stk~` (C++ external)

## Structure

```
libapi/
├── luajit_api.h      # Main header - include this in your external
├── api_common.h      # Common utilities and conversion functions
├── api_symbol.h      # Max symbol wrapper
├── api_atom.h        # Max atom wrapper
└── README.md         # This file
```

## Usage

### In Your External

Include the main header and initialize during external setup:

```c
#include "libapi/luajit_api.h"

// During external initialization
lua_State* L = luaL_newstate();
luaL_openlibs(L);
luajit_api_init(L);  // Initialize the api module
```

### In Lua Scripts

The API is available through the global `api` module:

```lua
-- Console output
api.post("Hello from Lua!")
api.error("Error message")

-- Symbols
local sym = api.gensym("foo")
print(sym:name())           -- "foo"
print(sym == "foo")         -- true

-- Atoms
local atom = api.Atom(42)
print(atom:type())          -- "long"
print(atom:getlong())       -- 42

local atom2 = api.Atom(3.14)
print(atom2:type())         -- "float"
print(atom2:getfloat())     -- 3.14

local atom3 = api.Atom("bar")
print(atom3:type())         -- "symbol"
print(atom3:getsym():name()) -- "bar"

-- Parse strings to atom arrays
local atoms = api.parse("foo 123 4.56")
for i, atom in ipairs(atoms) do
    print(atom)
end

-- Convert atom array to text
local text = api.atom_gettext(atoms)
print(text)  -- "foo 123 4.56"
```

## Design Philosophy

This API wrapper follows the design patterns established by the pktpy project:

1. **Userdata for Max types** - Max types (Symbol, Atom, etc.) are wrapped as Lua userdata with metatables
2. **Method-style API** - Objects have methods accessed via `:` syntax (e.g., `sym:name()`)
3. **Module-level functions** - Utility functions available at module level (e.g., `api.gensym()`)
4. **Automatic memory management** - Lua's GC handles cleanup where appropriate
5. **Type safety** - Functions check argument types and return helpful error messages

## Current API Coverage

**14 API wrappers completed** providing comprehensive access to Max/MSP functionality.

### Console Functions
- `api.post(msg)` - Post message to Max console
- `api.error(msg)` - Post error message to Max console

### Symbol API
- `api.gensym(name)` - Create a Max symbol
- `api.Symbol(name)` - Constructor (alternative to gensym)
- `sym:name()` - Get symbol name as string
- `sym == other` - Compare symbols (works with Symbol or string)

### Atom API
- `api.Atom(value)` - Create atom from number, string, or boolean
- `atom:type()` - Get type as string ("long", "float", "symbol")
- `atom:value()` - Get value as Lua type
- `atom:setvalue(val)` - Set atom value
- `atom:is_long()`, `atom:is_float()`, `atom:is_symbol()` - Type checks
- `atom:getlong()`, `atom:getfloat()`, `atom:getsym()` - Typed getters
- `api.parse(str)` - Parse string to atom array (Lua table)
- `api.atom_gettext(atoms)` - Convert atom array to string

### Clock API (Scheduling)
- `api.Clock(owner_ptr, callback)` - Create a clock with callback function
- `clock:delay(ms)` - Schedule callback in milliseconds
- `clock:fdelay(ms)` - Schedule callback with fractional milliseconds
- `clock:unset()` - Cancel scheduled callback
- `clock:pointer()` - Get raw pointer value

### Outlet API
- `api.Outlet(owner_ptr, type_string)` - Create an outlet
- `outlet:bang()` - Send bang message
- `outlet:int(value)` - Send integer
- `outlet:float(value)` - Send float
- `outlet:symbol(string)` - Send symbol
- `outlet:list({val1, val2, ...})` - Send list from Lua table
- `outlet:anything(symbol, {val1, val2, ...})` - Send typed message with args
- `outlet:pointer()` - Get raw pointer value

### Table API (Max Tables)
- `api.Table(name)` - Create/bind to named Max table
- `table:bind(name)` - Bind to different table
- `table:refresh()` - Refresh table reference
- `table:get(index)` - Get value at index
- `table:set(index, value)` - Set value at index
- `table:size()` - Get table size
- `table:name()` - Get table name
- `table:is_bound()` - Check if successfully bound
- `table:to_list()` - Export to Lua table
- `table:from_list(lua_table)` - Import from Lua table
- `table[index]` - Array-style read access
- `table[index] = value` - Array-style write access
- `#table` - Get table length

### AtomArray API (Atom Collections)
- `api.AtomArray()` - Create empty atomarray
- `api.AtomArray({val1, val2, ...})` - Create from Lua table
- `atomarray:getsize()` or `#atomarray` - Get size
- `atomarray[index]` - Get item at index (1-based, supports negative)
- `atomarray[index] = value` - Set item at index
- `atomarray:append(value)` - Append value to end
- `atomarray:clear()` - Clear all items
- `atomarray:duplicate()` - Create a copy
- `atomarray:to_list()` - Export to Lua table
- `atomarray:to_ints()` - Export as integer table
- `atomarray:to_floats()` - Export as float table
- `atomarray:to_symbols()` - Export as symbol string table
- `atomarray:to_text()` - Export as formatted string
- `atomarray:pointer()` - Get raw pointer value

### Buffer API (MSP Buffer~ Objects)
- `api.Buffer(owner_ptr, name)` - Create buffer reference
- `buffer:ref_set(name)` - Change buffer reference by name
- `buffer:exists()` - Check if buffer exists
- `buffer:getinfo()` - Get buffer metadata table
- `buffer:frames()` - Get number of frames
- `buffer:channels()` - Get number of channels
- `buffer:samplerate()` - Get sample rate
- `buffer:peek(frame, channel)` - Read single sample (0-indexed)
- `buffer:poke(frame, channel, value)` - Write single sample
- `buffer:to_list(channel, start, count)` - Export channel to Lua table
- `buffer:from_list(channel, lua_table, start)` - Import Lua table to channel
- `buffer:clear()` - Zero all samples
- `buffer:pointer()` - Get raw pointer value

### Dictionary API (Structured Data)
- `api.Dictionary()` - Create empty dictionary
- `dict:get(key, default)` - Get value with optional default
- `dict:getlong(key, default)` - Get long integer value
- `dict:getfloat(key, default)` - Get float value
- `dict:getstring(key, default)` - Get string value
- `dict:getsym(key, default)` - Get symbol value
- `dict:set(key, value)` - Set value with type detection
- `dict:setlong(key, value)` - Set long integer value
- `dict:setfloat(key, value)` - Set float value
- `dict:setstring(key, value)` - Set string value
- `dict:setsym(key, value)` - Set symbol value
- `dict:has(key)` - Check if key exists
- `dict:delete(key)` - Delete key
- `dict:clear()` - Clear all entries
- `dict:keys()` - Get array of all keys
- `dict:size()` or `#dict` - Get entry count
- `dict:read(filename, path)` - Read from file
- `dict:write(filename, path)` - Write to file
- `dict:dump(recurse, console)` - Dump for debugging
- `dict[key]` - Get value by key (alternative syntax)
- `dict[key] = value` - Set value by key (alternative syntax)
- `dict:pointer()` - Get raw pointer value

### Object API (Generic Max Objects)
- `api.Object()` - Create empty object wrapper
- `obj:create(classname, ...)` - Create Max object with args
- `obj:wrap(pointer)` - Wrap existing object pointer (non-owning)
- `obj:free()` - Free owned object
- `obj:is_null()` - Check if object is null
- `obj:classname()` - Get object class name
- `obj:method(name, ...)` - Call method with args
- `obj:getattr(name)` - Get attribute value
- `obj:setattr(name, value)` - Set attribute value
- `obj:attrnames()` - Get list of attribute names
- `obj:pointer()` - Get raw pointer value

### Patcher API (Patcher Manipulation)
- `api.Patcher()` - Create empty patcher wrapper
- `patcher:wrap(pointer)` - Wrap existing patcher pointer
- `patcher:is_null()` - Check if patcher is null
- `patcher:newobject(text)` - Create object from text string
- `patcher:locked([state])` - Get or set locked state
- `patcher:title([title])` - Get or set patcher title
- `patcher:rect([x, y, w, h])` - Get or set patcher rectangle
- `patcher:parentpatcher()` - Get parent patcher
- `patcher:toppatcher()` - Get top-level patcher
- `patcher:dirty([state])` - Set dirty flag
- `patcher:count()` - Count objects in patcher
- `patcher:name()` - Get patcher name
- `patcher:filepath()` - Get patcher file path
- `patcher:filename()` - Get patcher filename
- `patcher:pointer()` - Get raw pointer value

### Inlet API (Dynamic Inlet Creation)
- `api.Inlet()` - Create empty inlet wrapper
- `api.inlet_new(owner_ptr, [message])` - Create general inlet
- `api.intin(owner_ptr, inlet_num)` - Create integer inlet (1-9)
- `api.floatin(owner_ptr, inlet_num)` - Create float inlet (1-9)
- `api.proxy_new(owner_ptr, inlet_id, stuffloc_ptr)` - Create proxy inlet
- `api.proxy_getinlet(owner_ptr)` - Get current inlet number
- `api.inlet_count(owner_ptr)` - Count inlets on object
- `api.inlet_nth(owner_ptr, index)` - Get nth inlet (0-indexed)
- `inlet:delete()` - Delete owned inlet
- `inlet:pointer()` - Get raw pointer value
- `inlet:get_num()` - Get inlet number
- `inlet:is_proxy()` - Check if proxy inlet
- `inlet:is_null()` - Check if inlet is null

### Box API (Patcher Box Objects)
- `api.Box()` - Create empty box wrapper
- `box:wrap(pointer)` - Wrap existing box pointer
- `box:is_null()` - Check if box is null
- `box:classname()` - Get object class name
- `box:get_object()` - Get underlying object pointer
- `box:get_rect()` - Get box rectangle {x, y, width, height}
- `box:set_rect(x, y, width, height)` - Set box rectangle
- `box:pointer()` - Get raw pointer value

### Patchline API (Patch Cord Connections)
- `api.Patchline()` - Create empty patchline wrapper
- `patchline:wrap(pointer)` - Wrap existing patchline pointer
- `patchline:is_null()` - Check if patchline is null
- `patchline:get_box1()` - Get source box
- `patchline:get_box2()` - Get destination box
- `patchline:get_outletnum()` - Get outlet number of source
- `patchline:get_inletnum()` - Get inlet number of destination
- `patchline:get_startpoint()` - Get start coordinates {x, y}
- `patchline:get_endpoint()` - Get end coordinates {x, y}
- `patchline:get_hidden()` - Check if patchline is hidden
- `patchline:set_hidden(hidden)` - Set hidden state
- `patchline:get_nextline()` - Get next patchline in list
- `patchline:pointer()` - Get raw pointer value

## Future Extensions

All HIGH priority wrappers completed! See API_TODO.md for details on remaining wrappers:
- **MEDIUM priority**: Object, Patcher, Inlet, Box, Patchline wrappers
- **LOW priority**: Hashtab, Linklist, Qelem, Threading, Path, Time, and more

## CMake Integration

To use this library in a project, add the include path:

```cmake
target_include_directories(your_target PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../libapi
)
```

## Notes

- All API wrappers are header-only for maximum flexibility
- No separate compilation or linking required
- Safe to include from both C and C++ code
- Designed for compatibility with LuaJIT 2.1+
- Follows Max SDK naming conventions where possible
- Error handling uses Lua's error() mechanism

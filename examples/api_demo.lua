-- api_demo.lua
-- Comprehensive demonstration of the luajit-max API module

-- This script demonstrates the Max API wrappers available in Lua
-- It's designed to be loaded by luajit~ or luajit.stk~ externals

api.post("=== Max API Demo ===")

-- ============================================================================
-- Console Functions
-- ============================================================================

api.post("1. Console Functions")
api.post("  - api.post() for normal messages")
api.error("  - api.error() for error messages")

-- ============================================================================
-- Symbol API
-- ============================================================================

api.post("\n2. Symbol API")

local sym1 = api.gensym("test_symbol")
api.post("  Created symbol: " .. tostring(sym1))
api.post("  Symbol name: " .. sym1:name())

local sym2 = api.Symbol("test_symbol")
api.post("  Created another symbol with same name")
api.post("  Symbols equal (pointer comparison): " .. tostring(sym1 == sym2))
api.post("  String comparison: " .. tostring(sym1 == "test_symbol"))

-- ============================================================================
-- Atom API
-- ============================================================================

api.post("\n3. Atom API")

-- Create atoms
local atom_int = api.Atom(42)
local atom_float = api.Atom(3.14159)
local atom_sym = api.Atom("hello")

api.post("  Integer atom: " .. tostring(atom_int))
api.post("    Type: " .. atom_int:type())
api.post("    Value: " .. tostring(atom_int:getlong()))
api.post("    Is long: " .. tostring(atom_int:is_long()))

api.post("  Float atom: " .. tostring(atom_float))
api.post("    Type: " .. atom_float:type())
api.post("    Value: " .. tostring(atom_float:getfloat()))
api.post("    Is float: " .. tostring(atom_float:is_float()))

api.post("  Symbol atom: " .. tostring(atom_sym))
api.post("    Type: " .. atom_sym:type())
api.post("    Symbol: " .. atom_sym:getsym():name())
api.post("    Is symbol: " .. tostring(atom_sym:is_symbol()))

-- Parse and format atoms
api.post("  Parsing string to atoms:")
local atoms = api.parse("foo 123 4.56 bar baz")
api.post("    Parsed " .. #atoms .. " atoms")
for i, atom in ipairs(atoms) do
    api.post("      [" .. i .. "] " .. tostring(atom) .. " (type: " .. atom:type() .. ")")
end

local text = api.atom_gettext(atoms)
api.post("    Reconstructed: " .. text)

-- ============================================================================
-- Clock API (Scheduling)
-- ============================================================================

api.post("\n4. Clock API (Scheduling)")

-- Note: Clock requires owner pointer which externals must provide
-- This is a demonstration of the API structure
api.post("  Clock API available for time-based callbacks")
api.post("  Usage: clock = api.Clock(owner_ptr, callback_function)")
api.post("  Methods: clock:delay(ms), clock:fdelay(ms), clock:unset()")

-- Example callback (won't actually work without proper owner)
--[[
function my_callback()
    api.post("Clock fired!")
end

local clock = api.Clock(owner_ptr, my_callback)
clock:fdelay(1000)  -- Fire in 1 second
]]--

-- ============================================================================
-- Outlet API
-- ============================================================================

api.post("\n5. Outlet API")

-- Note: Outlet requires owner pointer which externals must provide
api.post("  Outlet API available for sending messages")
api.post("  Usage: outlet = api.Outlet(owner_ptr, type_string)")
api.post("  Methods:")
api.post("    outlet:bang()")
api.post("    outlet:int(value)")
api.post("    outlet:float(value)")
api.post("    outlet:symbol(string)")
api.post("    outlet:list({val1, val2, ...})")
api.post("    outlet:anything(symbol, {val1, val2, ...})")

-- Example (won't actually work without proper owner and outlet)
--[[
local outlet = api.Outlet(owner_ptr, nil)
outlet:bang()
outlet:int(42)
outlet:float(3.14)
outlet:list({1, 2, 3, "test"})
outlet:anything("custom", {1, 2, 3})
]]--

-- ============================================================================
-- Table API
-- ============================================================================

api.post("\n6. Table API")

-- Note: Table access requires a named [table] object to exist in Max
api.post("  Table API for accessing Max table objects")
api.post("  Usage: tbl = api.Table(name)")
api.post("  Methods:")
api.post("    tbl:bind(name)")
api.post("    tbl:refresh()")
api.post("    tbl:get(index)")
api.post("    tbl:set(index, value)")
api.post("    tbl:size()")
api.post("    tbl:to_list()  -- Export to Lua table")
api.post("    tbl:from_list(lua_table)  -- Import from Lua table")
api.post("  Array-style access:")
api.post("    value = tbl[index]")
api.post("    tbl[index] = value")

-- Example (requires 'mytable' to exist)
--[[
local tbl = api.Table("mytable")
if tbl:is_bound() then
    api.post("Table size: " .. tbl:size())

    -- Get value
    local val = tbl:get(0)
    api.post("Value at index 0: " .. val)

    -- Set value
    tbl:set(0, 100)

    -- Array-style access
    local val2 = tbl[1]
    tbl[1] = 200

    -- Export to Lua
    local lua_table = tbl:to_list()
    for i, v in ipairs(lua_table) do
        api.post("  [" .. i .. "] = " .. v)
    end
end
]]--

-- ============================================================================
-- AtomArray API
-- ============================================================================

api.post("\n7. AtomArray API (Atom Collections)")

-- Create from table
local arr = api.AtomArray({1, 2.5, "test", 4})
api.post("  Created AtomArray: " .. tostring(arr))
api.post("  Size: " .. #arr)

-- Array-style access
api.post("  arr[1] = " .. tostring(arr[1]))
api.post("  arr[-1] = " .. tostring(arr[-1]) .. " (negative indexing)")

-- Modify
arr[2] = 100
api.post("  Modified arr[2] to 100")

-- Append
arr:append("new")
api.post("  After append: size = " .. arr:getsize())

-- Type-specific extraction
local ints_arr = api.AtomArray({10, 20, 30})
local ints = ints_arr:to_ints()
api.post("  Integer array: {" .. table.concat(ints, ", ") .. "}")

-- to_list conversion
local list = arr:to_list()
api.post("  Converted to Lua table with " .. #list .. " items")

-- to_text
local text = arr:to_text()
api.post("  As text: " .. text)

-- Duplicate
local arr2 = arr:duplicate()
api.post("  Duplicated array: " .. tostring(arr2))

-- ============================================================================
-- Buffer API
-- ============================================================================

api.post("\n8. Buffer API (MSP Buffer~ Objects)")

-- Note: Buffer access requires owner pointer and named buffer~ object
api.post("  Buffer API for accessing MSP buffer~ objects")
api.post("  Usage: buf = api.Buffer(owner_ptr, 'buffername')")
api.post("  Methods:")
api.post("    buf:ref_set(name) - Change buffer reference")
api.post("    buf:exists() - Check if buffer exists")
api.post("    buf:getinfo() - Get metadata table")
api.post("    buf:frames() - Get frame count")
api.post("    buf:channels() - Get channel count")
api.post("    buf:samplerate() - Get sample rate")
api.post("    buf:peek(frame, channel) - Read sample (0-indexed)")
api.post("    buf:poke(frame, channel, value) - Write sample")
api.post("    buf:to_list(channel, start, count) - Export to table")
api.post("    buf:from_list(channel, table, start) - Import from table")
api.post("    buf:clear() - Zero all samples")

-- Example (requires valid owner and buffer~ object)
--[[
local buf = api.Buffer(owner_ptr, "mybuffer")
if buf:exists() then
    local info = buf:getinfo()
    api.post("Buffer info:")
    api.post("  Frames: " .. info.frames)
    api.post("  Channels: " .. info.channels)
    api.post("  Sample rate: " .. info.samplerate)

    -- Read sample
    local sample = buf:peek(0, 0)
    api.post("  Sample at (0,0): " .. sample)

    -- Write sample
    buf:poke(0, 0, 0.5)

    -- Bulk operations
    local samples = buf:to_list(0, 0, 100)  -- Read first 100 samples of channel 0
    buf:from_list(0, samples, 0)  -- Write back
end
]]--

-- ============================================================================
-- Dictionary API
-- ============================================================================

api.post("\n9. Dictionary API (Structured Data)")

-- Create dictionary
local dict = api.Dictionary()
api.post("  Created Dictionary: " .. tostring(dict))

-- Set values with different types
dict:setlong("count", 42)
dict:setfloat("gain", 0.75)
dict:setstring("name", "test")
dict:setsym("type", "audio")

api.post("  Set various values")
api.post("  Dictionary size: " .. dict:size())

-- Get values with type-specific methods
local count = dict:getlong("count")
local gain = dict:getfloat("gain")
local name = dict:getstring("name")
api.post("  count: " .. count .. " (long)")
api.post("  gain: " .. gain .. " (float)")
api.post("  name: " .. name .. " (string)")

-- Generic get/set with automatic type detection
dict:set("items", {1, 2, 3, 4, 5})
local items = dict:get("items")
api.post("  items: " .. type(items) .. " with " .. #items .. " elements")

-- Array-style access
dict["volume"] = 0.5
local volume = dict["volume"]
api.post("  volume (via dict[key]): " .. volume)

-- Check key existence
local has_name = dict:has("name")
local has_missing = dict:has("missing")
api.post("  has 'name': " .. tostring(has_name))
api.post("  has 'missing': " .. tostring(has_missing))

-- List all keys
local keys = dict:keys()
api.post("  Keys: " .. table.concat(keys, ", "))

-- Get with default value
local missing = dict:get("missing", "default_value")
api.post("  Get missing key with default: " .. missing)

-- Nested dictionary
local nested = api.Dictionary()
nested:setstring("foo", "bar")
dict:set("nested", nested)
api.post("  Added nested dictionary")

local retrieved_nested = dict:get("nested")
if type(retrieved_nested) == "userdata" then
    api.post("  Retrieved nested dict: " .. tostring(retrieved_nested))
end

-- Delete key
dict:delete("type")
api.post("  Deleted 'type' key, new size: " .. dict:size())

-- __len metamethod
api.post("  Size via #dict: " .. #dict)

-- Clear dictionary
-- dict:clear()
-- api.post("  Cleared dictionary, size: " .. dict:size())

-- File I/O and dump examples (commented - require valid paths)
--[[
dict:write("mydict.json", 0)
api.post("  Wrote dictionary to file")

dict:read("mydict.json", 0)
api.post("  Read dictionary from file")

dict:dump(1, 1)  -- Recursive dump to console
]]--

-- ============================================================================
-- DSP Function (Required by externals)
-- ============================================================================

-- Simple passthrough DSP function
function process(input, prev, n, param)
    return input
end

api.post("\n=== API Demo Complete ===")
api.post("All API features demonstrated successfully!")

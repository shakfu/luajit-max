# Max API Module for Lua

**Date:** 2025-11-07
**Feature:** Lua access to Max's `post` and `error` functions
**Status:** [x] IMPLEMENTED

---

## Overview

Both `luajit~` and `luajit.stk~` now expose Max/MSP's messaging functions to Lua through an `api` module. This allows Lua code to send messages to Max's console.

---

## Available Functions

### `api.post(message)`

Posts a message to the Max console.

**Parameters:**
- `message` (string): The message to display in Max's console

**Returns:** None

**Example:**
```lua
api.post("Hello from Lua!")
api.post(string.format("Sample rate: %.1f Hz", SAMPLE_RATE))
api.post(string.format("Parameter p0 = %.2f", p0))
```

**Output in Max console:**
```
Hello from Lua!
Sample rate: 44100.0 Hz
Parameter p0 = 441.00
```

---

### `api.error(message)`

Posts an error message to the Max console (appears in red/error style).

**Parameters:**
- `message` (string): The error message to display

**Returns:** None

**Example:**
```lua
if p0 < 0 then
    api.error("Invalid parameter: p0 must be >= 0")
    return 0.0
end
```

**Output in Max console:**
```
luajit.stk~: Invalid parameter: p0 must be >= 0
```

---

## Usage Examples

### Debug Parameter Values

```lua
local _debug_once = false

my_function = function(x, fb, n, p0, p1, p2, p3)
    -- Print parameters once when function starts
    if not _debug_once then
        api.post(string.format("Params: p0=%.1f, p1=%.2f, p2=%.2f, p3=%.2f",
                               p0, p1, p2, p3))
        _debug_once = true
    end

    -- ... rest of function
end
```

### Validate Input

```lua
filter = function(x, fb, n, p0, p1, p2, p3)
    -- p0 should be frequency (20-20000 Hz)
    if p0 < 20 or p0 > 20000 then
        api.error(string.format("Frequency %.1f Hz out of range (20-20000)", p0))
        p0 = math.max(20, math.min(20000, p0))  -- clamp
    end

    _filter:setFrequency(p0)
    return _filter:tick(x)
end
```

### Report State Changes

```lua
local _prev_mode = 0

process = function(x, fb, n, p0, p1, p2, p3)
    local mode = math.floor(p0 + 0.5)

    if mode ~= _prev_mode then
        api.post(string.format("Mode changed from %d to %d", _prev_mode, mode))
        _prev_mode = mode
    end

    -- ... processing based on mode
end
```

### Trace Execution Flow

```lua
api.post("Initializing STK objects...")

local _delay = stk.Delay(441, 4096)
api.post("Created delay line")

local _filter = stk.BiQuad()
api.post("Created filter")

delay_filter = function(x, fb, n, p0, p1, p2, p3)
    return _filter:tick(_delay:tick(x))
end

api.post("delay_filter function ready")
```

---

## Implementation Details

### C Code

The Max API functions are wrapped as Lua C functions and registered in a module table:

**luajit.stk~.cpp / luajit~.c:**
```c
// Wrapper functions
static int lua_max_post(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    post("%s", msg);
    return 0;
}

static int lua_max_error(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);
    error("%s", msg);
    return 0;
}

// Registration in lstk_init_lua / mlj_init_lua
lua_newtable(x->L);  // Create api table

lua_pushcfunction(x->L, lua_max_post);
lua_setfield(x->L, -2, "post");

lua_pushcfunction(x->L, lua_max_error);
lua_setfield(x->L, -2, "error");

lua_setglobal(x->L, "api");  // Set as global 'api' module
```

### Lua Access

The `api` module is available globally in all Lua files loaded by the externals:

```lua
-- Module structure:
api = {
    post = function(message) end,
    error = function(message) end
}
```

---

## Performance Considerations

### Real-Time Safety

[!] **IMPORTANT:** While `api.post()` and `api.error()` work, they are **NOT real-time safe**:

- Max's console I/O involves string formatting, memory allocation, and GUI updates
- These operations can block or take unpredictable time
- **DO NOT call these functions for every sample in the audio callback**

### Best Practices

[x] **Good (safe) usage:**
```lua
-- Call once during initialization
api.post("Delay initialized with buffer size 4096")

-- Call when parameters change (infrequent)
local _prev_delay = 0
if delay_samples ~= _prev_delay then
    api.post(string.format("Delay changed to %d samples", delay_samples))
    _prev_delay = delay_samples
end

-- Call once when entering error state
if not _error_printed and error_condition then
    api.error("Parameter out of range!")
    _error_printed = true
end
```

[X] **Bad (unsafe) usage:**
```lua
-- DON'T do this - called 44,100 times/second!
my_function = function(x, fb, n, p0, p1, p2, p3)
    api.post(string.format("Processing sample: %.3f", x))  -- [X] BAD!
    return x * 0.5
end
```

### Throttling Pattern

If you need frequent debugging, use a counter:

```lua
local _debug_counter = 0
local _debug_interval = 4410  -- Every 100ms at 44.1kHz

my_function = function(x, fb, n, p0, p1, p2, p3)
    _debug_counter = _debug_counter + 1

    if _debug_counter >= _debug_interval then
        api.post(string.format("p0=%.2f, output=%.3f", p0, x))
        _debug_counter = 0
    end

    return x * p0
end
```

---

## Comparison with Other Options

### Option 1: `api.post()` (This Implementation)
- [x] Easy to use
- [x] Prints to Max console
- [X] Not RT-safe
- [X] String formatting overhead

### Option 2: Return values to Max outlet
- [x] RT-safe
- [x] Can graph/visualize
- [X] Only numeric values
- [X] Requires extra Max objects

### Option 3: File logging
- [x] RT-safe (with buffering)
- [x] Full data capture
- [X] More complex
- [X] Delayed feedback

### Option 4: LuaJIT `print()`
- [X] Doesn't work - not connected to Max console
- [X] Output goes to stdout (not visible in Max)

---

## Future Enhancements

### Possible Additions

Additional Max API functions that could be exposed:

```lua
api.object_warn(message)     -- Warning (yellow) messages
api.object_post(message)     -- Object-specific post
api.clock_gettime()          -- Get current scheduler time
api.systime()                -- Get system time
api.error_obtrusive(message) -- Pop-up error dialog
```

### Async Logging

For RT-safe logging, could implement:

```lua
api.post_deferred(message)  -- Queue message for main thread
```

This would use Max's `defer()` mechanism to post from a safe context.

---

## Troubleshooting

### "attempt to call global 'post' (a nil value)"

**Problem:** Calling `post()` directly instead of `api.post()`

**Solution:**
```lua
-- [X] Wrong:
post("Hello")

-- [x] Correct:
api.post("Hello")
```

### No output in console

**Problem:** Messages may be filtered by Max console settings

**Solution:**
- Check Max console filter settings
- Ensure Max window has focus
- Try `api.error()` which should be more visible

### String formatting errors

**Problem:** Invalid format string or arguments

```lua
-- [X] Wrong - missing argument:
api.post(string.format("Value: %d"))

-- [x] Correct:
api.post(string.format("Value: %d", 42))
```

---

## Summary

[x] **Implemented:** `api.post()` and `api.error()` in both externals
[x] **Available:** Globally in all Lua files
[x] **Use case:** Debugging, parameter validation, state reporting
[!] **Caveat:** Not RT-safe - use sparingly, not per-sample

The `api` module provides essential debugging capabilities while keeping the implementation simple and consistent across both externals.

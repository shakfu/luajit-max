# STK Integer Parameter Fix

**Date:** 2025-11-07
**Issue:** LuaBridge3 type conversion error with STK integer parameters
**Status:** [x] FIXED

---

## Problem Description

When setting parameters on certain STK objects from Max/MSP, the following error occurred:

```
luajit.stk~: Lua error: .../examples/dsp_stk.lua:68:
Error decoding argument #1: The native integer can't fit inside a lua integer
```

### Root Cause

Many STK methods expect **integer types** (`unsigned int`, `unsigned long`), but Max sends **float values** for parameters. LuaBridge3 cannot automatically convert floating-point numbers to integer types when the value might not fit exactly.

**Affected STK Methods:**
- `Delay::setDelay(unsigned long delay)` - expects integer sample count
- `BlitSquare::setHarmonics(unsigned int nHarmonics)` - expects integer harmonic count
- Similar methods in other STK classes that take integer parameters

---

## Technical Details

### Why This Happens

1. Max/MSP sends all numeric parameters as **doubles/floats**
2. These get passed to Lua as Lua **numbers** (double precision)
3. LuaBridge3 attempts to pass these to STK methods
4. STK methods have signatures expecting **unsigned int/long**
5. LuaBridge3's type checking rejects the conversion for safety

### The Error Message Breakdown

```
Error decoding argument #1: The native integer can't fit inside a lua integer
```

- "argument #1" = first parameter to the method (after `self`)
- "native integer" = the C++ unsigned int/long type
- "lua integer" = Lua's internal integer representation

LuaBridge3 is being conservative - it won't convert a Lua number (which might be fractional) to an integer without explicit handling.

---

## Solution

**Explicitly convert float parameters to integers in Lua** before passing them to STK methods.

### Conversion Pattern

```lua
-- BEFORE (causes error):
_delay:setDelay(delay_param)  -- delay_param is a float from Max

-- AFTER (works correctly):
local delay_samples = math.floor(delay_param + 0.5)  -- round to nearest int
if delay_samples < 0 then delay_samples = 0 end      -- clamp to valid range
if delay_samples > 4096 then delay_samples = 4096 end -- max delay size
_delay:setDelay(delay_samples)  -- now an integer
```

### Why This Works

- `math.floor(x + 0.5)` rounds to nearest integer
- Clamping ensures value is in valid range
- LuaBridge3 can safely convert Lua integers to C++ integer types

---

## Files Fixed

### `examples/dsp_stk.lua`

#### 1. Fixed `delay` function (line 67-81)

**Before:**
```lua
local _delay = stk.Delay(9, 4096)
delay = function(x, fb, n, delay, fb_ratio, balance, tap_delay)
   _delay:setDelay(delay)  -- [X] ERROR: delay is float
   local x1 = x + (fb * fb_ratio)
   local wet_val = balance * _delay:tick(x1)
   local dry_val = (1 - balance) * x1
   return wet_val + dry_val
end
```

**After:**
```lua
local _delay = stk.Delay(9, 4096)
delay = function(x, fb, n, p0, p1, p2, p3)
   -- p0: delay time in samples (will be converted to integer)
   -- p1: feedback ratio (0.0-1.0)
   -- p2: wet/dry balance (0.0=dry, 1.0=wet)
   local delay_samples = math.floor(p0 + 0.5)  -- [x] round to integer
   if delay_samples < 0 then delay_samples = 0 end
   if delay_samples > 4096 then delay_samples = 4096 end

   _delay:setDelay(delay_samples)  -- [x] now safe
   local x1 = x + (fb * p1)
   local wet_val = p2 * _delay:tick(x1)
   local dry_val = (1 - p2) * x1
   return wet_val + dry_val
end
```

---

#### 2. Fixed `blitsquare` function (line 36-47)

**Before:**
```lua
blitsquare = function(x, fb, n, frequency, phase, nHarmonics, p3)
   _blitsquare:setFrequency(frequency)
   _blitsquare:setPhase(phase)
   _blitsquare:setHarmonics(nHarmonics)  -- [X] ERROR: nHarmonics is float
   return _blitsquare:tick()
end
```

**After:**
```lua
blitsquare = function(x, fb, n, p0, p1, p2, p3)
   -- p0: frequency (Hz)
   -- p1: phase (0.0-1.0)
   -- p2: number of harmonics (integer, 0=all harmonics)
   _blitsquare:setFrequency(p0)
   _blitsquare:setPhase(p1)
   local nHarmonics = math.floor(p2 + 0.5)  -- [x] round to integer
   if nHarmonics < 0 then nHarmonics = 0 end
   _blitsquare:setHarmonics(nHarmonics)  -- [x] now safe
   return _blitsquare:tick()
end
```

---

#### 3. Cleaned up other function signatures

Also updated `pitshift` and `sine` functions to use consistent `p0, p1, p2, p3` parameter names with clear documentation.

---

## Additional Benefits

The fixes also improved code clarity:

1. **Consistent naming** - All functions now use `p0, p1, p2, p3` matching C parameter order
2. **Clear documentation** - Each parameter is documented in comments
3. **Range validation** - Values are clamped to safe ranges
4. **Explicit conversion** - Intent is clear (float → int conversion)

---

## Testing

After these fixes, the delay function should work without errors:

### Test Case 1: Basic Delay
```
1. Create [luajit.stk~ dsp_stk.lua] in Max
2. Send message "delay" to select delay function
3. Send float to inlet 0 (e.g., 441 for 10ms delay at 44.1kHz)
4. Expected: No Lua errors, delay effect works correctly
```

### Test Case 2: BlitSquare Harmonics
```
1. Create [luajit.stk~ dsp_stk.lua] in Max
2. Send message "blitsquare" to select blitsquare function
3. Set inlet 2 to control harmonics (e.g., 10.7 → rounds to 11)
4. Expected: No Lua errors, waveform changes correctly
```

---

## Other STK Methods That May Need Similar Fixes

The following STK methods also expect integer parameters and may need similar treatment if used:

### Classes with Integer Parameters

1. **TapDelay**
   - `setTapDelays(std::vector<unsigned long>)`
   - Need to convert vector elements to integers

2. **Mesh2D**
   - `setNX(unsigned short nX)`
   - `setNY(unsigned short nY)`

3. **Granulate**
   - `setVoices(unsigned int nVoices)`

4. **Shakers**
   - Constructor: `Shakers(int type)`

5. **Various `noteOn` methods**
   - Many take `unsigned int` for note numbers

### Pattern to Use

For any STK method taking integer types:

```lua
function some_stk_function(x, fb, n, p0, p1, p2, p3)
    -- For integer parameter p0:
    local int_param = math.floor(p0 + 0.5)  -- round
    int_param = math.max(min_value, math.min(max_value, int_param))  -- clamp

    stk_object:someMethod(int_param)
    return stk_object:tick()
end
```

---

## Related Documentation

This issue is mentioned in LuaBridge3 documentation:
- Type safety is enforced for integer conversions
- Lua numbers (doubles) don't automatically convert to C++ integers
- Explicit conversion required for safety

---

## Summary

[x] **Fixed:** STK integer parameter conversion errors
[x] **Method:** Explicit float→int conversion with rounding and clamping
[x] **Files:** `examples/dsp_stk.lua`
[x] **Functions Updated:**
   - `delay()` - fixed setDelay integer parameter
   - `blitsquare()` - fixed setHarmonics integer parameter
   - `pitshift()` - cleaned up parameter naming
   - `sine()` - cleaned up parameter naming

The error `"The native integer can't fit inside a lua integer"` should no longer occur for delay and blitsquare functions.

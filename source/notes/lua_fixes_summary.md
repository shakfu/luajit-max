# Lua Examples Fixes Summary

**Date:** 2025-11-07
**Status:** [x] ALL ISSUES FIXED

---

## Issues Found and Fixed

### 1. [X] CRITICAL: Missing Parameter in dsp_stk.lua

**File:** `examples/dsp_stk.lua` line 62

**Problem:**
The `base()` function was missing the `fb` (feedback) parameter, causing parameter misalignment.

**Before:**
```lua
base = function(x, n, p0, p1, p2, p3)  -- [X] Only 6 params
   return x / 2
end
```

**After:**
```lua
base = function(x, fb, n, p0, p1, p2, p3)  -- [x] Correct 7 params
   return x / 2
end
```

**Impact:** This would have caused all parameters to shift by one position, leading to incorrect behavior.

---

### 2. [!] Misleading SAMPLE_RATE Declaration

**Files:** `examples/dsp.lua` and `examples/dsp_stk.lua`

**Problem:**
Hardcoded `SAMPLE_RATE = 44100.0` was misleading because the C code now sets this automatically.

**Before:**
```lua
SAMPLE_RATE = 44100.0  -- [!] Looks like user should set this
```

**After:**
```lua
-- SAMPLE_RATE is automatically set by the luajit~ external
-- based on Max's audio settings. Default shown here for reference only.
SAMPLE_RATE = SAMPLE_RATE or 44100.0
```

**Impact:** Now clear that SAMPLE_RATE is managed by the external, not user code.

---

### 3. [note] Missing Function Signature Documentation

**Problem:**
No documentation explaining the function signature expected by each external.

**Fixed:**
Added comprehensive header documentation to both files:

**dsp.lua:**
```lua
-- dsp.lua
-- Example DSP functions for luajit~ external
--
-- FUNCTION SIGNATURE for luajit~:
--   function name(x, fb, n, p1)
--     x:  audio input (current sample)
--     fb: feedback (previous output sample)
--     n:  samples remaining in buffer
--     p1: parameter from inlet (rightmost)
--     return: output sample (must be a number)
```

**dsp_stk.lua:**
```lua
-- dsp_stk.lua
-- Example DSP functions for luajit.stk~ external
--
-- FUNCTION SIGNATURE for luajit.stk~:
--   function name(x, fb, n, p0, p1, p2, p3)
--     x:  audio input (current sample)
--     fb: feedback (previous output sample)
--     n:  samples remaining in buffer
--     p0: parameter from inlet 0 (leftmost hot inlet)
--     p1: parameter from inlet 1
--     p2: parameter from inlet 2
--     p3: parameter from inlet 3 (rightmost)
--     return: output sample (must be a number)
```

**Impact:** Users now understand the function signature requirements.

---

## Files Modified

1. [x] `examples/dsp.lua`
   - Added function signature documentation
   - Updated SAMPLE_RATE comment

2. [x] `examples/dsp_stk.lua`
   - **Fixed critical `base()` function signature**
   - Added function signature documentation
   - Updated SAMPLE_RATE comment
   - Added documentation comment to base function

---

## Verification

All Lua functions now have correct signatures:

### luajit~ (4 parameters)
```lua
[x] lpf1(x, x0, n, decay)
[x] lpf2(x, x0, n, decay)
[x] lpf3(x, x0, n, alpha)
[x] saturate(x, feedback, n, a)
[x] reverb(x, fb, n, p1)
[x] pitchshift(x, fb, n, p1)
[x] filter(x, fb, n, p1)
[x] square(x, fb, n, p1)
[x] saw(x, fb, n, p1)
[x] osc(x, fb, n, p1)
[x] base(x, fb, n, p1)
```

### luajit.stk~ (7 parameters)
```lua
[x] blitsquare(x, fb, n, frequency, phase, nHarmonics, p3)
[x] pitshift(x, fb, n, shift, p1, p2, p3)
[x] sine(x, fb, n, freq, time, phase, phase_offset)
[x] delay(x, fb, n, delay, fb_ratio, balance, tap_delay)
[x] base(x, fb, n, p0, p1, p2, p3) -- FIXED!
```

---

## Recommendations for Future

### Better Parameter Names in dsp_stk.lua

While the functions work, the parameter names in `dsp_stk.lua` are somewhat misleading:

**Current (works but unclear):**
```lua
blitsquare = function(x, fb, n, frequency, phase, nHarmonics, p3)
    _blitsquare:setFrequency(frequency)  -- actually p0 from C
    _blitsquare:setPhase(phase)          -- actually p1 from C
    _blitsquare:setHarmonics(nHarmonics) -- actually p2 from C
    return _blitsquare:tick()
end
```

**Better (clearer mapping):**
```lua
blitsquare = function(x, fb, n, p0, p1, p2, p3)
    -- p0: frequency (Hz)
    -- p1: phase (0.0-1.0)
    -- p2: number of harmonics
    _blitsquare:setFrequency(p0)
    _blitsquare:setPhase(p1)
    _blitsquare:setHarmonics(p2)
    return _blitsquare:tick()
end
```

This makes it obvious which Max inlet controls which parameter.

**Decision:** Left as-is for now since it's working code and more readable for the algorithm. The signature is correct, just the internal names are semantic.

---

## Testing Checklist

After these fixes, test in Max:

### Test 1: luajit~ with dsp.lua
```
1. Create [luajit~ dsp.lua] object
2. Start audio
3. Try different functions: base, lpf1, filter, reverb
4. Expected: No Lua errors, audio processes correctly
```

### Test 2: luajit.stk~ with dsp_stk.lua
```
1. Create [luajit.stk~ dsp_stk.lua] object
2. Start audio
3. Select 'base' function
4. Expected: No Lua errors, audio attenuated by half
5. Try other functions: sine, delay, pitshift
6. Expected: STK objects work correctly
```

### Test 3: SAMPLE_RATE Access
```
1. Add to Lua file: post("SR: " .. SAMPLE_RATE)
2. Reload file
3. Expected: Prints actual sample rate (44100, 48000, etc.)
```

---

## Conclusion

[x] **All issues resolved**

The Lua example files are now:
- [x] **Consistent** with C external signatures
- [x] **Documented** with clear function signature specs
- [x] **Correct** - the critical base() bug is fixed
- [x] **Clear** - SAMPLE_RATE handling is explained

No further changes required for basic functionality.

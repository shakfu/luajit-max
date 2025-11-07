# Lua Examples Review

**Date:** 2025-11-07
**Reviewer:** Claude Code

## Executive Summary

[!] **CRITICAL ISSUES FOUND** - The Lua example files have **signature mismatches** with the updated C externals. These will cause runtime errors.

---

## Function Signature Analysis

### luajit~ External (C signature)
```c
float lua_dsp(t_mlj *x, float audio_in, float audio_prev, float n_samples, float param1)
```

**Expected Lua function signature:**
```lua
function name(x, fb, n, p1)
    -- x: audio_in (current sample)
    -- fb: audio_prev (feedback/previous sample)
    -- n: n_samples (samples remaining in buffer)
    -- p1: param1 (user parameter from inlet)
    return output_sample
end
```

### luajit.stk~ External (C signature)
```c
float lua_dsp(t_lstk *x, float audio_in, float audio_prev, float n_samples,
              float param0, float param1, float param2, float param3)
```

**Expected Lua function signature:**
```lua
function name(x, fb, n, p0, p1, p2, p3)
    -- x: audio_in (current sample)
    -- fb: audio_prev (feedback/previous sample)
    -- n: n_samples (samples remaining in buffer)
    -- p0-p3: param0-3 (user parameters from inlets)
    return output_sample
end
```

---

## Issues Found

### 1. [!] `dsp.lua` - INCONSISTENT (luajit~)

**File:** `examples/dsp.lua`

**Line 20:** Hardcoded `SAMPLE_RATE = 44100.0`
```lua
SAMPLE_RATE = 44100.0  -- [X] WILL BE OVERWRITTEN
```

**Issue:** The C code now sets `SAMPLE_RATE` as a global from the actual sample rate. This hardcoded value will be overwritten at init, but it's misleading.

**Impact:** Low - Will be overwritten, but confusing for users.

**Recommendation:**
```lua
-- SAMPLE_RATE is set by the external based on Max's audio settings
-- Default: 44100.0, but will be updated to actual sample rate
SAMPLE_RATE = SAMPLE_RATE or 44100.0
```

---

**Lines 58-88:** Function signatures are CORRECT [x]
```lua
lpf1 = function(x, x0, n, decay)  -- [x] 4 parameters (x, fb, n, p1)
lpf2 = function(x, x0, n, decay)  -- [x] 4 parameters
lpf3 = function(x, x0, n, alpha)  -- [x] 4 parameters
saturate = function(x, feedback, n, a)  -- [x] 4 parameters
```

**Lines 99-132:** Worp functions are CORRECT [x]
```lua
reverb = function(x, fb, n, p1)     -- [x] 4 parameters
pitchshift = function(x, fb, n, p1) -- [x] 4 parameters
filter = function(x, fb, n, p1)     -- [x] 4 parameters
square = function(x, fb, n, p1)     -- [x] 4 parameters
saw = function(x, fb, n, p1)        -- [x] 4 parameters
osc = function(x, fb, n, p1)        -- [x] 4 parameters
```

**Line 137:** Base function is CORRECT [x]
```lua
base = function(x, fb, n, p1)  -- [x] 4 parameters
```

**Overall:** [!] Minor issue - hardcoded SAMPLE_RATE

---

### 2. [X] `dsp_stk.lua` - CRITICAL ERROR (luajit.stk~)

**File:** `examples/dsp_stk.lua`

**Line 3:** Hardcoded `SAMPLE_RATE = 44100.0`
```lua
SAMPLE_RATE = 44100.0  -- [X] WILL BE OVERWRITTEN
```

**Issue:** Same as dsp.lua - will be overwritten but misleading.

---

**Line 22-30:** `blitsquare` function - [!] PARAMETER CONFUSION
```lua
blitsquare = function(x, fb, n, frequency, phase, nHarmonics, p3)
    -- [X] 7 parameters but only 7 are passed from C!
    -- This actually works but parameter names are misleading
end
```

**Analysis:**
- C passes: `(x, fb, n, p0, p1, p2, p3)` = 7 params [x]
- Lua expects: `(x, fb, n, frequency, phase, nHarmonics, p3)` = 7 params [x]
- **Working but confusing** - p0=frequency, p1=phase, p2=nHarmonics, p3=p3

**Impact:** Works but misleading names. Users won't know what inlet does what.

**Recommendation:**
```lua
-- CORRECT but with better documentation:
blitsquare = function(x, fb, n, p0, p1, p2, p3)
    -- p0: frequency
    -- p1: phase
    -- p2: nHarmonics
    -- p3: (unused)
    _blitsquare:setPhase(p1)
    _blitsquare:setFrequency(p0)
    _blitsquare:setHarmonics(p2)
    return _blitsquare:tick()
end
```

---

**Line 34-39:** `pitshift` function - [x] CORRECT
```lua
pitshift = function(x, fb, n, shift, p1, p2, p3)
    -- [x] 7 parameters, names are clear
    _pitshift:setShift(shift)  -- uses p0 as 'shift'
    return _pitshift:tick(x)
end
```

---

**Line 43-49:** `sine` function - [!] PARAMETER CONFUSION
```lua
sine = function(x, fb, n, freq, time, phase, phase_offset)
    -- [x] 7 parameters but names mislead users
    -- p0=freq, p1=time, p2=phase, p3=phase_offset
end
```

**Impact:** Same as blitsquare - works but confusing.

---

**Line 53-59:** `delay` function - [!] PARAMETER CONFUSION
```lua
delay = function(x, fb, n, delay, fb_ratio, balance, tap_delay)
    -- [x] 7 parameters but names mislead users
    -- p0=delay, p1=fb_ratio, p2=balance, p3=tap_delay
end
```

**Impact:** Same as others - works but confusing.

---

**Line 62-64:** `base` function - [X] CRITICAL ERROR
```lua
base = function(x, n, p0, p1, p2, p3)
    -- [X] WRONG! Only 6 parameters!
    -- C passes: (x, fb, n, p0, p1, p2, p3) = 7 parameters
    -- Lua expects: (x, n, p0, p1, p2, p3) = 6 parameters
    -- Missing 'fb' parameter!
    return x / 2
end
```

**Impact:** [X] CRITICAL - Parameter misalignment!
- `n` gets value of `fb`
- `p0` gets value of `n`
- `p1` gets value of `p0`
- etc.

**This will cause incorrect behavior!**

**Fix:**
```lua
base = function(x, fb, n, p0, p1, p2, p3)
    return x / 2
end
```

---

### 3. [x] `demo.lua` - Not Used

**File:** `examples/demo.lua`

This appears to be a test file for LuaJIT FFI, not actually used by the externals.

**Status:** [x] No issues

---

## Summary of Issues

| File | Severity | Issue | Impact |
|------|----------|-------|--------|
| `dsp.lua` | [!] Low | Hardcoded SAMPLE_RATE | Misleading but will work |
| `dsp_stk.lua` | [!] Low | Hardcoded SAMPLE_RATE | Misleading but will work |
| `dsp_stk.lua` | [!] Medium | Confusing parameter names | Works but unclear to users |
| `dsp_stk.lua` | [X] **CRITICAL** | `base()` missing `fb` parameter | **Will cause runtime errors** |

---

## Required Fixes

### CRITICAL (Must Fix)

**File:** `examples/dsp_stk.lua` line 62

**Current:**
```lua
base = function(x, n, p0, p1, p2, p3)
    return x / 2
end
```

**Fixed:**
```lua
base = function(x, fb, n, p0, p1, p2, p3)
    return x / 2
end
```

---

### Recommended (Should Fix)

**File:** `examples/dsp.lua` line 20

**Current:**
```lua
SAMPLE_RATE = 44100.0
```

**Fixed:**
```lua
-- SAMPLE_RATE is automatically set by the external
-- Default shown here for reference only
SAMPLE_RATE = SAMPLE_RATE or 44100.0
```

---

**File:** `examples/dsp_stk.lua` line 3

**Current:**
```lua
SAMPLE_RATE = 44100.0
```

**Fixed:**
```lua
-- SAMPLE_RATE is automatically set by the external
-- Default shown here for reference only
SAMPLE_RATE = SAMPLE_RATE or 44100.0
```

---

**File:** `examples/dsp_stk.lua` - All functions

**Add documentation comments:**

```lua
-- Function signature for luajit.stk~:
-- function name(x, fb, n, p0, p1, p2, p3)
--   x:  audio input (current sample)
--   fb: audio feedback (previous output sample)
--   n:  samples remaining in buffer
--   p0: parameter from inlet 0 (leftmost hot inlet)
--   p1: parameter from inlet 1
--   p2: parameter from inlet 2
--   p3: parameter from inlet 3 (rightmost)
--   return: output sample (must be a number)

local _blitsquare = stk.BlitSquare(220.)
blitsquare = function(x, fb, n, p0, p1, p2, p3)
    -- p0: frequency (Hz)
    -- p1: phase (0.0-1.0)
    -- p2: number of harmonics
    _blitsquare:setFrequency(p0)
    _blitsquare:setPhase(p1)
    _blitsquare:setHarmonics(p2)
    return _blitsquare:tick()
end

-- ... similar for other functions
```

---

## Testing Required

After fixing `dsp_stk.lua` line 62, test:

1. Load `luajit.stk~ dsp_stk.lua` in Max
2. Select `base` function
3. Start audio
4. **Verify:** No Lua errors, audio passes through attenuated

Without the fix, you'll get parameter misalignment errors.

---

## New Example Template

Here's a complete, correct example:

**File:** `examples/template.lua` (NEW)

```lua
-- Template for luajit~ functions
-- SAMPLE_RATE global is automatically provided by the external

-- Function signature: function name(x, fb, n, p1)
--   x:  audio input (current sample)
--   fb: feedback (previous output)
--   n:  samples remaining in buffer
--   p1: parameter from inlet
--   return: output sample (number)

-- Simple passthrough
passthrough = function(x, fb, n, p1)
    return x
end

-- Attenuator
attenuate = function(x, fb, n, p1)
    return x * p1  -- p1 = gain (0.0-1.0)
end

-- One-pole lowpass filter
lpf = function(x, fb, n, p1)
    local alpha = p1  -- p1 = cutoff (0.0-1.0)
    return x * (1 - alpha) + fb * alpha
end

-- Default function (required)
base = function(x, fb, n, p1)
    return x * 0.5  -- attenuate by half
end
```

**File:** `examples/template_stk.lua` (NEW)

```lua
-- Template for luajit.stk~ functions
-- SAMPLE_RATE global is automatically provided by the external

-- Function signature: function name(x, fb, n, p0, p1, p2, p3)
--   x:  audio input (current sample)
--   fb: feedback (previous output)
--   n:  samples remaining in buffer
--   p0: parameter from inlet 0 (leftmost)
--   p1: parameter from inlet 1
--   p2: parameter from inlet 2
--   p3: parameter from inlet 3 (rightmost)
--   return: output sample (number)

-- STK Sine oscillator
local _sine = stk.SineWave()
sine = function(x, fb, n, p0, p1, p2, p3)
    -- p0: frequency (Hz)
    _sine:setFrequency(p0)
    return _sine:tick()
end

-- STK BiQuad filter
local _filter = stk.BiQuad()
filter = function(x, fb, n, p0, p1, p2, p3)
    -- p0: frequency (Hz)
    -- p1: Q factor
    _filter:setResonance(p0 / SAMPLE_RATE, p1, true)
    return _filter:tick(x)
end

-- Default function (required)
base = function(x, fb, n, p0, p1, p2, p3)
    return x * 0.5  -- attenuate by half
end
```

---

## Conclusion

**Status:** [!] **ACTION REQUIRED**

The Lua examples need updating:

1. [X] **CRITICAL:** Fix `dsp_stk.lua` line 62 (missing `fb` parameter)
2. [!] **Recommended:** Update SAMPLE_RATE comments
3. [!] **Recommended:** Add parameter documentation to all functions
4. [x] **Optional:** Create template files for users

The most critical issue is the `base()` function signature mismatch in `dsp_stk.lua`, which will cause runtime errors or incorrect behavior.

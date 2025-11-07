# Delay Function Fix

**Date:** 2025-11-07
**Issue:** Delay feedback was incorrectly implemented
**Status:** [x] FIXED

---

## Problem Description

The `delay` function in `examples/dsp_stk.lua` was not working correctly. The feedback path was using the wrong value, leading to incorrect delay behavior.

---

## Root Cause Analysis

### How STK Delay Works

The STK `Delay` class is a simple circular buffer delay line:

```cpp
StkFloat Delay::tick(StkFloat input) {
    inputs_[inPoint_++] = input * gain_;  // Write input to buffer
    // ... wrap inPoint ...

    lastFrame_[0] = inputs_[outPoint_++];  // Read delayed output
    // ... wrap outPoint ...

    return lastFrame_[0];  // Return delayed sample
}
```

**Key points:**
1. `tick(input)` writes the input to the delay line
2. Returns the sample that was written `delay` samples ago
3. The delay line itself has no feedback - it's just a buffer

### The Bug

**Original Code (WRONG):**
```lua
delay = function(x, fb, n, p0, p1, p2, p3)
   local delay_samples = math.floor(p0 + 0.5)
   _delay:setDelay(delay_samples)

   local x1 = x + (fb * p1)  -- [X] WRONG!
   local wet_val = p2 * _delay:tick(x1)
   local dry_val = (1 - p2) * x1
   return wet_val + dry_val
end
```

**Problem:** `fb` is the **previous return value** of the entire function (wet + dry mix), not the delay output!

### What Was Happening

1. `fb` = previous mixed output (dry + wet)
2. Code adds `fb * p1` to input → feeds back the mixed signal
3. This creates strange, incorrect behavior:
   - Feeding back a mix of dry+wet instead of just the delay output
   - Not actually creating a proper delay feedback loop

### Correct Delay with Feedback

For a proper delay with feedback:

```
Input → [+] → Delay → [Mix] → Output
         ↑             ↓
         └─ Feedback ──┘
```

The feedback should come from the **delay output**, not the final mixed output.

---

## The Fix

### New Implementation

```lua
local _delay = stk.Delay(9, 4096)
local _delay_last_out = 0  -- [x] NEW: store last delay output

delay = function(x, fb, n, p0, p1, p2, p3)
   -- p0: delay time in samples
   -- p1: feedback amount (0.0-1.0)
   -- p2: wet/dry mix (0.0=dry, 1.0=wet)

   local delay_samples = math.floor(p0 + 0.5)
   if delay_samples < 0 then delay_samples = 0 end
   if delay_samples > 4096 then delay_samples = 4096 end

   _delay:setDelay(delay_samples)

   -- [x] FIXED: Feedback from delay output, not mixed output
   local delay_input = x + (_delay_last_out * p1)

   -- Get delayed output
   local delay_output = _delay:tick(delay_input)
   _delay_last_out = delay_output  -- [x] Store for next sample

   -- Mix dry and wet
   local dry_val = (1.0 - p2) * x
   local wet_val = p2 * delay_output

   return dry_val + wet_val
end
```

### Key Changes

1. **Added `_delay_last_out` variable** to store the delay output
2. **Feedback from delay output:** `delay_input = x + (_delay_last_out * p1)`
3. **Store delay output:** After `tick()`, save result to `_delay_last_out`
4. **Proper mix:** Mix the original input `x` with `delay_output`

---

## How It Works Now

### Signal Flow

```
Sample n:
1. delay_input = x[n] + (delay_output[n-1] * feedback)
2. delay_output[n] = Delay.tick(delay_input)
3. Store delay_output[n] for next iteration
4. output = (1-mix)*x[n] + mix*delay_output[n]
```

### Parameter Behavior

**p0 (delay time in samples):**
- Value: 0 to 4096 samples
- Example: 441 samples = 10ms at 44.1kHz
- Example: 22050 samples = 500ms at 44.1kHz

**p1 (feedback amount):**
- 0.0 = no feedback (single echo)
- 0.5 = moderate feedback (multiple echoes)
- 0.9 = high feedback (long decay)
- 1.0 = infinite feedback (oscillation)
- >1.0 = grows exponentially (will clip/explode)

**p2 (wet/dry mix):**
- 0.0 = 100% dry (input only, no delay heard)
- 0.5 = 50/50 mix
- 1.0 = 100% wet (delay only, no dry signal)

---

## Example Usage in Max

### Simple Delay (10ms, no feedback)
```
Inlet 0: 441     (10ms at 44.1kHz)
Inlet 1: 0.0     (no feedback)
Inlet 2: 0.5     (50% wet)
Result: Simple 10ms echo
```

### Echo with Repeats
```
Inlet 0: 22050   (500ms at 44.1kHz)
Inlet 1: 0.7     (70% feedback = multiple echoes)
Inlet 2: 0.5     (50% wet)
Result: 500ms echo with repeating taps
```

### Infinite Hold (Freeze Buffer)
```
Inlet 0: 4410    (100ms at 44.1kHz)
Inlet 1: 1.0     (100% feedback = freeze)
Inlet 2: 1.0     (100% wet)
Result: Frozen 100ms loop
```

---

## Why the `fb` Parameter Isn't Used

You might notice the function signature has an `fb` parameter from the C code:

```lua
delay = function(x, fb, n, p0, p1, p2, p3)
```

**Why we don't use `fb`:**

The `fb` parameter is the **previous output sample** returned by this Lua function (for all functions). This is provided by the C code for general DSP use (like filters that need previous output).

However, for a delay with feedback, we need to feed back the **delay line output specifically**, not the final mixed output. That's why we maintain our own `_delay_last_out` variable.

**Conceptual difference:**
- `fb` = previous return value of entire function = `dry_val + wet_val` from previous sample
- `_delay_last_out` = previous delay output only = what we want to feed back

---

## Testing

### Test 1: Basic Delay (No Feedback)
```
1. Load luajit.stk~ dsp_stk.lua
2. Select "delay" function
3. Set inlet 0: 441 (10ms delay)
4. Set inlet 1: 0.0 (no feedback)
5. Set inlet 2: 0.5 (50% mix)
6. Expected: Clean 10ms delay/echo
```

### Test 2: Echo with Repeats
```
1. Set inlet 0: 2205 (50ms delay)
2. Set inlet 1: 0.6 (60% feedback)
3. Set inlet 2: 0.4 (40% wet)
4. Expected: 50ms echo with gradually fading repetitions
```

### Test 3: Long Delay with Feedback
```
1. Set inlet 0: 22050 (500ms delay)
2. Set inlet 1: 0.8 (80% feedback)
3. Set inlet 2: 0.5 (50% mix)
4. Expected: 500ms echo with long decay tail
```

### Test 4: Verify No Runaway Feedback
```
1. Set inlet 1: 0.95 (very high feedback)
2. Play a burst of sound then stop
3. Expected: Long decay but eventually fades to silence
4. Should NOT: Grow louder over time or clip
```

---

## Related: Why State Variables Are Needed

In real-time audio, each sample is processed independently, but effects like delay need **memory** of previous samples.

### State Variable Pattern

```lua
-- [x] CORRECT: Module-level state
local _effect_state = 0

function process(x, fb, n, p0, p1, p2, p3)
    -- Use _effect_state from previous call
    local output = do_something_with(x, _effect_state, p0)
    _effect_state = output  -- Update for next call
    return output
end
```

```lua
-- [X] WRONG: Local state resets every call
function process(x, fb, n, p0, p1, p2, p3)
    local effect_state = 0  -- [X] Always resets to 0!
    local output = do_something_with(x, effect_state, p0)
    effect_state = output  -- [X] Lost on next call
    return output
end
```

**State variables must be declared outside the function** to persist between calls.

---

## Performance Note

The delay line itself is very efficient - it's just reading/writing to a circular buffer. The overhead is minimal:
- One addition (feedback)
- One multiply (feedback amount)
- Two multiplies + one add (wet/dry mix)

This is negligible compared to the LuaJIT→C boundary crossing and function call overhead.

---

## Summary

[x] **Fixed:** Delay feedback now uses correct signal path
[x] **Improvement:** Added `_delay_last_out` state variable
[x] **Result:** Proper delay effect with controllable feedback and mix

### What Changed
- **Before:** Feedback from mixed output (wrong)
- **After:** Feedback from delay output (correct)

The delay function should now behave like a standard audio delay effect with proper feedback control.

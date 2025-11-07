# Using SAMPLE_RATE in Lua

**Date:** 2025-11-07
**Feature:** Time-based DSP calculations
**Available In:** Both luajit~ and luajit.stk~

---

## Overview

The `SAMPLE_RATE` global variable is automatically set by the externals and contains the current audio sample rate in Hz (typically 44100.0 or 48000.0).

This allows you to write sample-rate-independent DSP code that works correctly at any sample rate.

---

## Basic Usage

### Access SAMPLE_RATE

```lua
-- SAMPLE_RATE is available everywhere in your Lua file
api.post(string.format("Sample Rate: %.1f Hz", SAMPLE_RATE))

-- Calculate Nyquist frequency
local nyquist = SAMPLE_RATE / 2.0
```

### Time to Samples Conversion

```lua
-- Convert seconds to samples
local delay_seconds = 0.05  -- 50 milliseconds
local delay_samples = math.floor(delay_seconds * SAMPLE_RATE + 0.5)

-- Convert milliseconds to samples
local delay_ms = 50
local delay_samples = math.floor(delay_ms * SAMPLE_RATE / 1000.0 + 0.5)
```

### Samples to Time Conversion

```lua
-- Convert samples to seconds
local buffer_size = 4096
local buffer_time = buffer_size / SAMPLE_RATE
api.post(string.format("Buffer time: %.3f seconds", buffer_time))

-- Convert samples to milliseconds
local delay_samples = 2205
local delay_ms = delay_samples * 1000.0 / SAMPLE_RATE
api.post(string.format("Delay: %.1f ms", delay_ms))
```

---

## Common Use Cases

### 1. Time-Based Delay

**Before (sample-based, not portable):**
```lua
delay = function(x, fb, n, p0, p1, p2, p3)
   -- p0 is in samples - confusing and sample-rate dependent!
   local delay_samples = math.floor(p0)
   _delay:setDelay(delay_samples)
   -- ...
end
```

**After (time-based, portable):**
```lua
delay = function(x, fb, n, p0, p1, p2, p3)
   -- p0 is in SECONDS - intuitive and sample-rate independent!
   local delay_samples = math.floor(p0 * SAMPLE_RATE + 0.5)
   _delay:setDelay(delay_samples)
   -- ...
end
```

**Usage in Max:**
```
Inlet 0: 0.05   (50ms delay - works at any sample rate!)
```

---

### 2. Frequency-Based Calculations

**Oscillator period:**
```lua
local _phase = 0
local _phase_increment = 0

osc = function(x, fb, n, p0, p1, p2, p3)
   -- p0: frequency in Hz
   _phase_increment = p0 / SAMPLE_RATE
   _phase = _phase + _phase_increment

   if _phase >= 1.0 then
       _phase = _phase - 1.0
   end

   return math.sin(2.0 * math.pi * _phase)
end
```

**Filter cutoff frequency:**
```lua
-- One-pole lowpass coefficient from cutoff frequency
local function cutoff_to_coeff(cutoff_hz)
   return 1.0 - math.exp(-2.0 * math.pi * cutoff_hz / SAMPLE_RATE)
end

lpf = function(x, fb, n, p0, p1, p2, p3)
   -- p0: cutoff frequency in Hz
   local alpha = cutoff_to_coeff(p0)
   return x * (1.0 - alpha) + fb * alpha
end
```

---

### 3. Attack/Decay Times

**Envelope with time-based parameters:**
```lua
local _env = 0

envelope = function(x, fb, n, p0, p1, p2, p3)
   -- p0: attack time in seconds
   -- p1: decay time in seconds

   local attack_samples = p0 * SAMPLE_RATE
   local decay_samples = p1 * SAMPLE_RATE

   -- Calculate increment per sample
   local attack_increment = 1.0 / attack_samples
   local decay_increment = 1.0 / decay_samples

   -- Simple envelope logic
   if x > 0.5 then  -- gate on
       _env = _env + attack_increment
       if _env > 1.0 then _env = 1.0 end
   else  -- gate off
       _env = _env - decay_increment
       if _env < 0.0 then _env = 0.0 end
   end

   return x * _env
end
```

---

### 4. LFO (Low Frequency Oscillator)

**Time-based LFO:**
```lua
local _lfo_phase = 0

lfo_modulation = function(x, fb, n, p0, p1, p2, p3)
   -- p0: LFO rate in Hz (e.g., 0.5 = 2 seconds per cycle)
   -- p1: LFO depth (0.0-1.0)

   -- Update phase
   _lfo_phase = _lfo_phase + (p0 / SAMPLE_RATE)
   if _lfo_phase >= 1.0 then
       _lfo_phase = _lfo_phase - 1.0
   end

   -- Generate sine LFO
   local lfo = math.sin(2.0 * math.pi * _lfo_phase)

   -- Apply modulation
   local modulated = x * (1.0 + lfo * p1)

   return modulated
end
```

---

### 5. BPM-Synced Effects

**Delay synced to tempo:**
```lua
-- Convert BPM and note division to delay time
local function bpm_to_delay(bpm, division)
   -- division: 1.0 = quarter note, 0.5 = eighth, 2.0 = half, etc.
   local seconds_per_beat = 60.0 / bpm
   local delay_time = seconds_per_beat * division
   return math.floor(delay_time * SAMPLE_RATE + 0.5)
end

tempo_delay = function(x, fb, n, p0, p1, p2, p3)
   -- p0: BPM (e.g., 120)
   -- p1: note division (e.g., 0.25 = sixteenth note)

   local delay_samples = bpm_to_delay(p0, p1)
   _delay:setDelay(delay_samples)

   -- ... rest of delay processing
end
```

---

## Helper Functions

### Included Utility File

Use `examples/time_helpers.lua` for common conversions:

```lua
require 'time_helpers'

-- Use helper functions
local delay_samples = ms_to_samples(50)  -- 50ms to samples
local period = freq_to_period(440)       -- A440 period in samples
local alpha = cutoff_to_alpha(1000)      -- 1kHz cutoff coefficient

-- Print timing info
print_timing_info()
```

**Available functions:**
- `ms_to_samples(milliseconds)` → samples
- `sec_to_samples(seconds)` → samples
- `samples_to_ms(samples)` → milliseconds
- `samples_to_sec(samples)` → seconds
- `freq_to_period(frequency_hz)` → samples
- `period_to_freq(period_samples)` → Hz
- `cutoff_to_alpha(cutoff_hz)` → filter coefficient
- `tau_to_cutoff(tau_seconds)` → Hz
- `print_timing_info()` → prints to console

---

## Updated Delay Example

The `delay` function in `examples/dsp_stk.lua` now uses time-based parameters:

```lua
delay = function(x, fb, n, p0, p1, p2, p3)
   -- p0: delay time in SECONDS (0.0 to ~0.093s)
   -- p1: feedback amount (0.0-1.0)
   -- p2: wet/dry mix (0.0-1.0)

   -- Convert seconds to samples
   local delay_samples = math.floor(p0 * SAMPLE_RATE + 0.5)
   -- ... rest of processing
end
```

**Usage:**
```
Inlet 0: 0.01   → 10ms delay
Inlet 0: 0.05   → 50ms delay
Inlet 0: 0.5    → 500ms delay
```

---

## Best Practices

### [x] Do This

**Sample-rate independent:**
```lua
local delay_time_sec = 0.05  -- 50ms
local delay_samples = math.floor(delay_time_sec * SAMPLE_RATE + 0.5)
```

**Pre-calculate constants:**
```lua
-- At module level (calculated once)
local max_delay_sec = 4096 / SAMPLE_RATE

-- In function
delay = function(x, fb, n, p0, p1, p2, p3)
   if p0 > max_delay_sec then
       p0 = max_delay_sec
   end
   -- ...
end
```

**Clear parameter units:**
```lua
-- Good: parameter name indicates units
delay = function(x, fb, n, delay_sec, feedback, mix, unused)
   local samples = math.floor(delay_sec * SAMPLE_RATE + 0.5)
   -- ...
end
```

### [X] Avoid This

**Hardcoded sample rate:**
```lua
-- BAD: Assumes 44.1kHz, breaks at other rates
local delay_samples = delay_ms * 44.1
```

**Sample rate in hot path:**
```lua
-- BAD: Division every sample is slow
my_func = function(x, fb, n, p0, p1, p2, p3)
   local freq = 440.0
   local increment = freq / SAMPLE_RATE  -- [X] Recalculated every sample!
   -- ...
end

-- GOOD: Calculate outside function
local _increment = 440.0 / SAMPLE_RATE
my_func = function(x, fb, n, p0, p1, p2, p3)
   -- Use pre-calculated _increment
   -- ...
end
```

**Ambiguous units:**
```lua
-- BAD: Is p0 in samples, seconds, or milliseconds?
delay = function(x, fb, n, p0, p1, p2, p3)
   _delay:setDelay(p0)  -- ???
end
```

---

## Sample Rate Changes

### When Does SAMPLE_RATE Update?

`SAMPLE_RATE` is set:
1. When the Lua file is first loaded
2. Every time `dsp64()` is called (when Max audio settings change)

### Handling Sample Rate Changes

If Max's sample rate changes (e.g., switching audio interface), your Lua code will use the new sample rate automatically on the next DSP cycle.

**Recalculate dependent values:**
```lua
-- Module-level constant
local _max_delay_samples = 4096

-- Recalculated when sample rate changes (in init or on reload)
local _max_delay_sec = _max_delay_samples / SAMPLE_RATE

-- Use in function
delay = function(x, fb, n, p0, p1, p2, p3)
   if p0 > _max_delay_sec then
       p0 = _max_delay_sec
   end
   -- ...
end
```

---

## Common Sample Rates

| Sample Rate | Nyquist | 1ms samples | 100ms samples |
|-------------|---------|-------------|---------------|
| 44100 Hz    | 22050 Hz| 44          | 4410          |
| 48000 Hz    | 24000 Hz| 48          | 4800          |
| 88200 Hz    | 44100 Hz| 88          | 8820          |
| 96000 Hz    | 48000 Hz| 96          | 9600          |

---

## Troubleshooting

### "SAMPLE_RATE is nil"

**Problem:** Trying to use SAMPLE_RATE before it's set

**Solution:** SAMPLE_RATE is set during initialization, so use it in functions or after init:

```lua
-- [X] BAD: Used at module load time (SAMPLE_RATE not set yet)
local nyquist = SAMPLE_RATE / 2.0

-- [x] GOOD: Calculated in function (SAMPLE_RATE available)
my_func = function(x, fb, n, p0, p1, p2, p3)
   local nyquist = SAMPLE_RATE / 2.0
   -- ...
end

-- [x] ALSO GOOD: Calculated after everything loads (module level)
-- This works because the external sets SAMPLE_RATE before running the file
local _nyquist = SAMPLE_RATE / 2.0
```

### Wrong timing at different sample rates

**Problem:** Hardcoded samples instead of time-based calculations

**Solution:** Always convert time → samples using SAMPLE_RATE:

```lua
-- [X] WRONG: Fixed at 44.1kHz
local delay_samples = 441  -- 10ms at 44.1kHz, but 9.2ms at 48kHz!

-- [x] CORRECT: Works at any sample rate
local delay_samples = math.floor(0.010 * SAMPLE_RATE + 0.5)  -- Always 10ms
```

---

## Summary

[x] **SAMPLE_RATE** is automatically available in all Lua files
[x] **Use it** to convert between time and samples
[x] **Write portable code** that works at any sample rate
[x] **Helper functions** available in `examples/time_helpers.lua`
[x] **Updated delay** now uses seconds instead of samples

Always use SAMPLE_RATE for time-based calculations to ensure your DSP code works correctly regardless of Max's audio settings!

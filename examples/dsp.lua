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
--
----------------------------------------------------------------------------------
-- get path of containing folder and set it as package.path

function script_path()
   local str = debug.getinfo(2, "S").source:sub(2)
   return str:match("(.*/)")
end

pkg_path = ";" .. script_path() .. "?.lua"

package.path = package.path .. pkg_path

----------------------------------------------------------------------------------
-- imports

require 'dsp_worp'
require 'fun'

-- Try to load FFI DSP library (optional, fails gracefully if not available)
local dsp_c
local ffi_available = pcall(function()
   dsp_c = require 'dsp_ffi'
end)

if ffi_available then
   api.post("FFI DSP library loaded successfully")
else
   api.post("FFI DSP library not available (continuing without it)")
end

-- SAMPLE_RATE is automatically set by the luajit~ external
-- based on Max's audio settings. Default shown here for reference only.
SAMPLE_RATE = SAMPLE_RATE or 44100.0

-- PARAMS is a global table for named parameters set via messages
-- e.g., sending "delay 2.0 feedback 0.5" creates PARAMS.delay and PARAMS.feedback
-- Initialize as empty table if not already set by the external
PARAMS = PARAMS or {}


----------------------------------------------------------------------------------
-- utility functions

function clamp(x, min, max)
   return x < min and min or x > max and max or x
end

function scale(x, in_min, in_max, out_min, out_max)
   return ((out_max - out_min)/(in_max - in_min)) * (x - in_min) + out_min
end


function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end

----------------------------------------------------------------------------------
-- working custom functions


-- low-pass single-pole filter
-- y(n) = y(n-1) + b(x(n) - y(n-1))
-- where 
--  b = 1 - d
--  d: Decay between samples (in (0, 1)).
-- see: https://tomroelandts.com/articles/low-pass-single-pole-iir-filter
lpf1 = function(x, x0, n, decay)
    local b = 1 - decay
    x0 = x0 + b * (x - x0)
    -- or just: x0 = x - decay*x - decay*x0
    return x0;
end

-- see: https://dsp.stackexchange.com/questions/60277/is-the-typical-implementation-of-low-pass-filter-in-c-code-actually-not-a-typica
lpf2 = function(x, x0, n, decay)
    x0 = (1 - decay) * x0 + decay * (x + x0)/2
    return x0;
end

-- see: https://www.musicdsp.org/en/latest/Filters/257-1-pole-lpf-for-smooth-parameter-changes.html
lpf3 = function(x, x0, n, alpha)
    local b = 1 - alpha;
    x0 = (x * b) + (x0 * alpha)
    return x0
end

-- a (0-1)
-- see: https://www.musicdsp.org/en/latest/Effects/42-soft-saturation.html
saturate = function(x, feedback, n, a)
   if x < 0 then
      return x
   elseif x > a then
      return a + (x-a)/(1+((x-a)/(1-a))^2)
   elseif x > 1 then
      return (a + 1)/2
   end
end
   
----------------------------------------------------------------------------------
-- problematic worp functions



----------------------------------------------------------------------------------
-- working worp functions

_reverb = Dsp:Reverb { wet = 0.5, dry = 0.5, room = 1.0, damp = 0.1 }
reverb = function(x, fb, n, p1)
   _reverb:set{wet = p1}
   return _reverb(x)
end

_pitchshift = Dsp:Pitchshift{f=1}
pitchshift = function(x, fb, n, p1)
   _pitchshift:set{f = p1}
   return _pitchshift(x)
end

_filter = Dsp:Filter { ft = "lp", f = 1500, Q = 4 }
filter = function(x, fb, n, p1)
   _filter:set{f = p1}
   return _filter(x)
end

_square = Dsp:Square{f=220}
square = function(x, fb, n, p1)
   _square:set{f = p1}
   return _square()
end

_saw = Dsp:Saw{f=220}
saw = function(x, fb, n, p1)
   _saw:set{f = p1}
   return _saw()
end

_osc = Dsp:Osc{f=220}
osc = function(x, fb, n, p1)
   _osc:set{f = p1}
   return _osc()
end

----------------------------------------------------------------------------------
-- base (only attenuate) function

base = function(x, fb, n, p1)
   local c = p1 / 4
   return x * c
end

----------------------------------------------------------------------------------
-- dynamic parameter test functions

-- Waveshaper with variable number of parameters
-- Demonstrates dynamic parameter handling with positional args
-- Usage: send numeric list like "0.5 2.0 0.7"
--
-- Parameters (positional):
--   1 param:  [gain] - simple gain/attenuation
--   2 params: [gain, drive] - gain + waveshaping drive
--   3 params: [gain, drive, mix] - gain + drive + dry/wet mix
--
-- Examples:
--   "0.5" - attenuate by 50%
--   "1.0 2.0" - unity gain with 2x overdrive
--   "0.8 3.0 0.7" - 0.8 gain, 3x drive, 70% wet
waveshape = function(x, fb, n, ...)
   local params = {...}
   local num_params = #params

   if num_params == 0 then
      -- No params: pass through
      return x
   elseif num_params == 1 then
      -- Single param: gain/attenuation
      return x * params[1]
   elseif num_params == 2 then
      -- Two params: gain + waveshaping drive
      local gain = params[1]
      local drive = params[2]
      -- Soft clipping waveshaper: tanh approximation
      local shaped = x * drive
      shaped = shaped / (1.0 + math.abs(shaped))  -- fast tanh approximation
      return shaped * gain
   elseif num_params >= 3 then
      -- Three or more params: gain + drive + dry/wet mix
      local gain = params[1]
      local drive = params[2]
      local mix = params[3]
      -- Waveshape the signal
      local shaped = x * drive
      shaped = shaped / (1.0 + math.abs(shaped))
      shaped = shaped * gain
      -- Mix dry and wet
      return shaped * mix + x * (1.0 - mix)
   end
end

-- One-pole low-pass filter with gain and dry/wet mix
-- Uses named parameters from PARAMS table
-- Usage examples:
--   1. Switch to function then set params:
--      "lpf_gain_mix"  then  "gain 0.8 cutoff 0.3 mix 0.7"
--   2. Combined (switch + set params in one message):
--      "lpf_gain_mix gain 0.8 cutoff 0.3 mix 0.7"
--   3. Update individual params:
--      "cutoff 0.5"  or  "mix 1.0"
--
-- Parameters:
--   gain: output gain (0.0 - 2.0, default 1.0)
--   cutoff: filter cutoff coefficient (0.0 = full filtering, 1.0 = no filtering, default 0.5)
--   mix: dry/wet mix (0.0 = dry only, 1.0 = wet only, default 1.0)
lpf_gain_mix = function(x, fb, n, ...)
   -- Access named parameters with defaults
   local gain = PARAMS.gain or 1.0
   local cutoff = PARAMS.cutoff or 0.5
   local mix = PARAMS.mix or 1.0

   -- One-pole low-pass filter
   -- fb is the previous output (one-sample feedback)
   -- y[n] = y[n-1] + cutoff * (x[n] - y[n-1])
   local filtered = fb + cutoff * (x - fb)

   -- Apply gain
   local processed = filtered * gain

   -- Dry/wet mix
   local output = processed * mix + x * (1.0 - mix)

   return output
end

-- Test function showing both positional and named parameter support
-- Usage examples:
--   Positional: "0.5 0.3"
--   Named: "gain 0.5 tone 0.3"
--   Combined: "hybrid_filter gain 0.8 tone 0.6"
hybrid_filter = function(x, fb, n, ...)
   local gain, tone

   -- Try to get from positional params first
   local pos_params = {...}
   if #pos_params >= 2 then
      gain = pos_params[1]
      tone = pos_params[2]
   else
      -- Fall back to named params with defaults
      gain = PARAMS.gain or 1.0
      tone = PARAMS.tone or 0.5
   end

   -- Simple tone control (low-pass filter coefficient)
   local filtered = fb + tone * (x - fb)
   return filtered * gain
end

-- Simple delay with feedback using a circular buffer
-- Usage: "delay time 0.5 feedback 0.6 mix 0.5"
--
-- Parameters:
--   time: delay time in seconds (0.01 - 2.0, default 0.5)
--   feedback: feedback amount (0.0 - 0.95, default 0.5)
--   mix: dry/wet mix (0.0 = dry, 1.0 = wet, default 0.5)
--
-- Note: This creates a global delay buffer on first use
delay = (function()
   -- Static variables (closure)
   local buffer = {}
   local buffer_size = 0
   local write_pos = 1
   local initialized = false

   return function(x, fb, n, ...)
      -- Get parameters
      local time = PARAMS.time or 0.5
      local feedback_amt = PARAMS.feedback or 0.5
      local mix = PARAMS.mix or 0.5

      -- Clamp feedback to prevent runaway
      feedback_amt = math.min(feedback_amt, 0.95)

      -- Calculate buffer size needed (in samples)
      local needed_size = math.floor(time * SAMPLE_RATE)
      needed_size = math.max(needed_size, 1)
      needed_size = math.min(needed_size, math.floor(2.0 * SAMPLE_RATE)) -- max 2 seconds

      -- Reinitialize buffer if size changed
      if not initialized or needed_size ~= buffer_size then
         buffer = {}
         buffer_size = needed_size
         for i = 1, buffer_size do
            buffer[i] = 0.0
         end
         write_pos = 1
         initialized = true
      end

      -- Read delayed sample
      local delayed = buffer[write_pos] or 0.0

      -- Write new sample (input + feedback)
      buffer[write_pos] = x + delayed * feedback_amt

      -- Advance write position (circular)
      write_pos = write_pos + 1
      if write_pos > buffer_size then
         write_pos = 1
      end

      -- Mix dry and wet
      return x * (1.0 - mix) + delayed * mix
   end
end)()

-- Test function that uses many parameters
-- params: freq1, freq2, freq3, amp1, amp2, amp3, mix
oscillator_bank = function(x, fb, n, ...)
   local params = {...}
   local num_params = #params

   -- Default values
   local freq1 = params[1] or 220
   local freq2 = params[2] or 440
   local freq3 = params[3] or 880
   local amp1 = params[4] or 0.33
   local amp2 = params[5] or 0.33
   local amp3 = params[6] or 0.34
   local mix = params[7] or 1.0

   -- Simple phase-based oscillators (using n as phase proxy)
   local phase1 = (n * freq1 / SAMPLE_RATE) % 1.0
   local phase2 = (n * freq2 / SAMPLE_RATE) % 1.0
   local phase3 = (n * freq3 / SAMPLE_RATE) % 1.0

   local osc1 = math.sin(phase1 * math.pi * 2) * amp1
   local osc2 = math.sin(phase2 * math.pi * 2) * amp2
   local osc3 = math.sin(phase3 * math.pi * 2) * amp3

   local synth = (osc1 + osc2 + osc3)
   return synth * mix + x * (1 - mix)
end

----------------------------------------------------------------------------------
-- FFI-based DSP functions (using optimized C code)
-- These functions demonstrate calling C DSP functions via LuaJIT FFI
----------------------------------------------------------------------------------

if ffi_available then
   -- Soft clipper using C function
   -- Usage: "ffi_softclip drive 3.0 mix 0.7"
   -- Parameters:
   --   drive: saturation amount (1.0 = clean, 5.0 = heavy, default 2.0)
   --   mix: dry/wet mix (0.0 = dry, 1.0 = wet, default 1.0)
   ffi_softclip = function(x, fb, n, ...)
      local drive = PARAMS.drive or 2.0
      local mix = PARAMS.mix or 1.0

      -- Call C function via FFI (much faster than Lua implementation)
      local wet = dsp_c.soft_clip(x, drive)

      -- Mix dry and wet
      return wet * mix + x * (1.0 - mix)
   end

   -- Bit crusher using C function
   -- Usage: "ffi_bitcrush bits 8 mix 0.5"
   -- Parameters:
   --   bits: bit depth (1-16, default 8)
   --   mix: dry/wet mix (default 1.0)
   ffi_bitcrush = function(x, fb, n, ...)
      local bits = PARAMS.bits or 8
      local mix = PARAMS.mix or 1.0

      local wet = dsp_c.bit_crush(x, bits)
      return wet * mix + x * (1.0 - mix)
   end

   -- Wavefolder using C function
   -- Usage: "ffi_wavefold threshold 0.5 mix 0.8"
   -- Parameters:
   --   threshold: folding threshold (0.1 - 1.0, default 0.5)
   --   mix: dry/wet mix (default 1.0)
   ffi_wavefold = function(x, fb, n, ...)
      local threshold = PARAMS.threshold or 0.5
      local mix = PARAMS.mix or 1.0

      local wet = dsp_c.wavefold(x, threshold)
      return wet * mix + x * (1.0 - mix)
   end

   -- Low-pass filter using C function
   -- Usage: "ffi_lpf cutoff 0.3 mix 1.0"
   -- Parameters:
   --   cutoff: filter cutoff (0.0 = max filtering, 1.0 = no filtering, default 0.5)
   --   mix: dry/wet mix (default 1.0)
   ffi_lpf = function(x, fb, n, ...)
      local cutoff = PARAMS.cutoff or 0.5
      local mix = PARAMS.mix or 1.0

      -- Use feedback parameter (fb) as previous output
      local wet = dsp_c.lpf_1pole(x, fb, cutoff)
      return wet * mix + x * (1.0 - mix)
   end

   -- Envelope follower using C function
   -- Usage: "ffi_envelope attack 0.1 release 0.01"
   -- Parameters:
   --   attack: attack coefficient (0.0 - 1.0, higher = faster, default 0.1)
   --   release: release coefficient (0.0 - 1.0, higher = faster, default 0.01)
   ffi_envelope = function(x, fb, n, ...)
      local attack = PARAMS.attack or 0.1
      local release = PARAMS.release or 0.01

      -- Get absolute value for envelope detection
      local rectified = math.abs(x)

      -- Use feedback parameter as previous envelope value
      local envelope = dsp_c.envelope_follow(rectified, fb, attack, release)

      -- Return envelope value (can be used to modulate other parameters)
      return envelope
   end
else
   -- Define stub functions if FFI is not available
   ffi_softclip = function(x, fb, n, ...)
      return x
   end

   ffi_bitcrush = function(x, fb, n, ...)
      return x
   end

   ffi_wavefold = function(x, fb, n, ...)
      return x
   end

   ffi_lpf = function(x, fb, n, ...)
      return x
   end

   ffi_envelope = function(x, fb, n, ...)
      return math.abs(x) * 0.1 -- Simple fallback
   end
end

-- Oscillator functions using FFI (if available)
-- These use a global phase accumulator that persists across calls
if ffi_available then
   -- Global phase state for oscillators (one per oscillator type)
   OSC_PHASE = OSC_PHASE or {}

   -- Sine oscillator using C function
   -- Usage: "ffi_osc_sine freq 440.0 gain 0.5"
   -- Parameters:
   --   freq: frequency in Hz (default 440.0)
   --   gain: output gain (default 0.5)
   ffi_osc_sine = function(x, fb, n, ...)
      local freq = PARAMS.freq or 440.0
      local gain = PARAMS.gain or 0.5

      -- Initialize phase if needed
      OSC_PHASE.sine = OSC_PHASE.sine or 0.0

      -- Get current output
      local output = dsp_c.osc_sine(OSC_PHASE.sine) * gain

      -- Calculate phase increment
      local phase_inc = dsp_c.osc_phase_inc(freq, SAMPLE_RATE)

      -- Update phase and wrap
      OSC_PHASE.sine = dsp_c.osc_phase_wrap(OSC_PHASE.sine + phase_inc)

      return output
   end

   -- Sawtooth oscillator (band-limited) using C function
   -- Usage: "ffi_osc_saw freq 220.0 gain 0.5"
   -- Parameters:
   --   freq: frequency in Hz (default 220.0)
   --   gain: output gain (default 0.5)
   ffi_osc_saw = function(x, fb, n, ...)
      local freq = PARAMS.freq or 220.0
      local gain = PARAMS.gain or 0.5

      -- Initialize phase if needed
      OSC_PHASE.saw = OSC_PHASE.saw or 0.0

      -- Calculate phase increment
      local phase_inc = dsp_c.osc_phase_inc(freq, SAMPLE_RATE)

      -- Get current output (band-limited)
      local output = dsp_c.osc_saw_bl(OSC_PHASE.saw, phase_inc) * gain

      -- Update phase and wrap
      OSC_PHASE.saw = dsp_c.osc_phase_wrap(OSC_PHASE.saw + phase_inc)

      return output
   end

   -- Square/pulse wave oscillator (band-limited) using C function
   -- Usage: "ffi_osc_square freq 110.0 width 0.5 gain 0.5"
   -- Parameters:
   --   freq: frequency in Hz (default 110.0)
   --   width: pulse width (0.0 - 1.0, 0.5 = square, default 0.5)
   --   gain: output gain (default 0.5)
   ffi_osc_square = function(x, fb, n, ...)
      local freq = PARAMS.freq or 110.0
      local width = PARAMS.width or 0.5
      local gain = PARAMS.gain or 0.5

      -- Initialize phase if needed
      OSC_PHASE.square = OSC_PHASE.square or 0.0

      -- Calculate phase increment
      local phase_inc = dsp_c.osc_phase_inc(freq, SAMPLE_RATE)

      -- Get current output (band-limited)
      local output = dsp_c.osc_square_bl(OSC_PHASE.square, width, phase_inc) * gain

      -- Update phase and wrap
      OSC_PHASE.square = dsp_c.osc_phase_wrap(OSC_PHASE.square + phase_inc)

      return output
   end

   -- Triangle oscillator using C function
   -- Usage: "ffi_osc_tri freq 330.0 gain 0.5"
   -- Parameters:
   --   freq: frequency in Hz (default 330.0)
   --   gain: output gain (default 0.5)
   ffi_osc_tri = function(x, fb, n, ...)
      local freq = PARAMS.freq or 330.0
      local gain = PARAMS.gain or 0.5

      -- Initialize phase if needed
      OSC_PHASE.tri = OSC_PHASE.tri or 0.0

      -- Get current output
      local output = dsp_c.osc_triangle(OSC_PHASE.tri) * gain

      -- Calculate phase increment
      local phase_inc = dsp_c.osc_phase_inc(freq, SAMPLE_RATE)

      -- Update phase and wrap
      OSC_PHASE.tri = dsp_c.osc_phase_wrap(OSC_PHASE.tri + phase_inc)

      return output
   end

   -- FM synthesis using sine oscillators
   -- Usage: "ffi_osc_fm carrier 440.0 modulator 220.0 index 2.0 gain 0.5"
   -- Parameters:
   --   carrier: carrier frequency in Hz (default 440.0)
   --   modulator: modulator frequency in Hz (default 220.0)
   --   index: modulation index (0.0 - 10.0, default 2.0)
   --   gain: output gain (default 0.5)
   ffi_osc_fm = function(x, fb, n, ...)
      local carrier_freq = PARAMS.carrier or 440.0
      local mod_freq = PARAMS.modulator or 220.0
      local mod_index = PARAMS.index or 2.0
      local gain = PARAMS.gain or 0.5

      -- Initialize phases if needed
      OSC_PHASE.fm_carrier = OSC_PHASE.fm_carrier or 0.0
      OSC_PHASE.fm_mod = OSC_PHASE.fm_mod or 0.0

      -- Calculate phase increments
      local carrier_inc = dsp_c.osc_phase_inc(carrier_freq, SAMPLE_RATE)
      local mod_inc = dsp_c.osc_phase_inc(mod_freq, SAMPLE_RATE)

      -- Get modulator output
      local modulator = dsp_c.osc_sine(OSC_PHASE.fm_mod) * mod_index

      -- Modulate carrier phase
      local modulated_phase = dsp_c.osc_phase_wrap(OSC_PHASE.fm_carrier + modulator / (2.0 * math.pi))

      -- Get carrier output
      local output = dsp_c.osc_sine(modulated_phase) * gain

      -- Update phases
      OSC_PHASE.fm_carrier = dsp_c.osc_phase_wrap(OSC_PHASE.fm_carrier + carrier_inc)
      OSC_PHASE.fm_mod = dsp_c.osc_phase_wrap(OSC_PHASE.fm_mod + mod_inc)

      return output
   end
else
   -- Fallback oscillators if FFI not available (pure Lua)
   OSC_PHASE = OSC_PHASE or {}

   ffi_osc_sine = function(x, fb, n, ...)
      local freq = PARAMS.freq or 440.0
      local gain = PARAMS.gain or 0.5
      OSC_PHASE.sine = OSC_PHASE.sine or 0.0
      local output = math.sin(OSC_PHASE.sine * 2.0 * math.pi) * gain
      OSC_PHASE.sine = OSC_PHASE.sine + (freq / SAMPLE_RATE)
      while OSC_PHASE.sine >= 1.0 do OSC_PHASE.sine = OSC_PHASE.sine - 1.0 end
      return output
   end

   ffi_osc_saw = function(x, fb, n, ...)
      local freq = PARAMS.freq or 220.0
      local gain = PARAMS.gain or 0.5
      OSC_PHASE.saw = OSC_PHASE.saw or 0.0
      local output = (2.0 * OSC_PHASE.saw - 1.0) * gain
      OSC_PHASE.saw = OSC_PHASE.saw + (freq / SAMPLE_RATE)
      while OSC_PHASE.saw >= 1.0 do OSC_PHASE.saw = OSC_PHASE.saw - 1.0 end
      return output
   end

   ffi_osc_square = function(x, fb, n, ...)
      local freq = PARAMS.freq or 110.0
      local width = PARAMS.width or 0.5
      local gain = PARAMS.gain or 0.5
      OSC_PHASE.square = OSC_PHASE.square or 0.0
      local output = (OSC_PHASE.square < width and 1.0 or -1.0) * gain
      OSC_PHASE.square = OSC_PHASE.square + (freq / SAMPLE_RATE)
      while OSC_PHASE.square >= 1.0 do OSC_PHASE.square = OSC_PHASE.square - 1.0 end
      return output
   end

   ffi_osc_tri = function(x, fb, n, ...)
      local freq = PARAMS.freq or 330.0
      local gain = PARAMS.gain or 0.5
      OSC_PHASE.tri = OSC_PHASE.tri or 0.0
      local phase = OSC_PHASE.tri
      local output = (phase < 0.5 and (4.0 * phase - 1.0) or (-4.0 * phase + 3.0)) * gain
      OSC_PHASE.tri = OSC_PHASE.tri + (freq / SAMPLE_RATE)
      while OSC_PHASE.tri >= 1.0 do OSC_PHASE.tri = OSC_PHASE.tri - 1.0 end
      return output
   end

   ffi_osc_fm = function(x, fb, n, ...)
      return 0.0 -- FM synthesis requires sine, too complex for simple fallback
   end
end



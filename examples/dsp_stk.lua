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
--
-- SAMPLE_RATE is automatically set by the luajit.stk~ external
-- based on Max's audio settings. Default shown here for reference only.
-- SAMPLE_RATE = SAMPLE_RATE or 44100.0


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



local _blitsquare = stk.BlitSquare(220.) -- BlitSquare(double frequency)
blitsquare = function(x, fb, n, p0, p1, p2, p3)
   -- p0: frequency (Hz)
   -- p1: phase (0.0-1.0)
   -- p2: number of harmonics (integer, 0=all harmonics)
   _blitsquare:setFrequency(p0)
   _blitsquare:setPhase(p1)
   -- setHarmonics expects unsigned int - convert float to integer
   local nHarmonics = math.floor(p2 + 0.5)  -- round to nearest integer
   if nHarmonics < 0 then nHarmonics = 0 end
   _blitsquare:setHarmonics(nHarmonics)
   return _blitsquare:tick()
end


local _pitshift = stk.PitShift() -- PitShift()
pitshift = function(x, fb, n, p0, p1, p2, p3)
   -- p0: pitch shift amount (semitones, can be fractional)
   _pitshift:setShift(p0)
   return _pitshift:tick(x)
end


local _sine = stk.SineWave()
sine = function(x, fb, n, p0, p1, p2, p3)
   -- p0: frequency (Hz)
   -- p1: time increment (rate, typically small values)
   -- p2: phase increment (radians)
   -- p3: phase offset (radians)
   _sine:setFrequency(p0)
   _sine:addTime(p1)
   _sine:addPhase(p2)
   _sine:addPhaseOffset(p3)
   return _sine:tick()
end


-- Max delay time in secs (4096 samples / SAMPLE_RATE)

local _delay_max_seconds = 4.0
local _delay_max_samples = _delay_max_seconds * SAMPLE_RATE

local _delay = stk.Delay(SAMPLE_RATE / 100, _delay_max_samples) -- (initial delay = 448 samples ~10ms, maxDelay = _delay_max_seconds * SAMPLE_RATE)
local _delay_last_out = 0  -- store last delay output for feedback
local _delay_current_length = SAMPLE_RATE / 100  -- track current delay length
local _delay_debug_printed = false  -- debug flag

delay = function(x, fb, n, p0, p1, p2, p3)
   -- p0: delay time in SECONDS (0.0 to ~0.093s at 44.1kHz)
   --     Examples: 0.01 = 10ms, 0.05 = 50ms, 0.5 = 500ms
   -- p1: feedback amount (0.0-1.0) - 0.0 = no feedback, 0.9 = long decay
   -- p2: wet/dry mix (0.0=dry/input only, 1.0=wet/delay only)
   -- IMPORTANT: If you hear no delay, make sure p2 is NOT 0 (p2=0 means 100% dry)

   -- Convert seconds to samples using SAMPLE_RATE
   local delay_seconds = p0
   local delay_samples = math.floor(delay_seconds * SAMPLE_RATE + 0.5)  -- round to nearest int

   -- Clamp to valid range
   if delay_samples < 0 then delay_samples = 0 end
   if delay_samples > _delay_max_samples then delay_samples = _delay_max_samples end

   -- Debug output once
   if not _delay_debug_printed then
      api.post(string.format("SAMPLE_RATE: %.1f Hz", SAMPLE_RATE))
      api.post(string.format("Max delay: %.3f seconds (%d samples)",
                             _delay_max_seconds, _delay_max_samples))
      api.post(string.format("Initial params: time=%.3fs, feedback=%.2f, mix=%.2f",
                             p0, p1, p2))
      _delay_debug_printed = true
   end

   -- Only call setDelay when the delay time actually changes
   if delay_samples ~= _delay_current_length then
      _delay:setDelay(delay_samples)
      _delay_current_length = delay_samples
      api.post(string.format("Delay: %.3fs (%d samples)",
                             delay_samples / SAMPLE_RATE, delay_samples))
   end

   -- Create feedback: input + (previous delay output * feedback amount)
   local delay_input = x + (_delay_last_out * p1)

   -- Get delayed output
   local delay_output = _delay:tick(delay_input)
   _delay_last_out = delay_output  -- store for next iteration

   -- Mix dry (input) and wet (delay output)
   local dry_val = (1.0 - p2) * x
   local wet_val = p2 * delay_output

   return dry_val + wet_val
end


-- Default base function
-- Signature: function(x, fb, n, p0, p1, p2, p3)
--   x:  audio input sample
--   fb: feedback (previous output sample)
--   n:  samples remaining in buffer
--   p0-p3: parameters from inlets 0-3
base = function(x, fb, n, p0, p1, p2, p3)
   return x / 2
end


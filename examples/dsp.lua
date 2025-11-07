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

-- SAMPLE_RATE is automatically set by the luajit~ external
-- based on Max's audio settings. Default shown here for reference only.
SAMPLE_RATE = SAMPLE_RATE or 44100.0


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



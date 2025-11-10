-- dsp_ffi.lua
-- FFI wrapper for libdsp C functions
-- Demonstrates using LuaJIT FFI to call optimized C code

local ffi = require 'ffi'

-- Load the libdsp shared library
-- The library is in the support directory (Max package structure)

-- Try loading from different locations
local dsp
local load_success, load_error = pcall(function()
   -- load relative to examples directory (../support/libdsp.dylib)
   local function script_dir()
      local str = debug.getinfo(1, "S").source:sub(2)
      return str:match("(.*/)")
   end
   local examples_dir = script_dir()
   local support_path = examples_dir .. "../support/libdsp.dylib"
   local ok, lib = pcall(function() return ffi.load(support_path) end)
   if ok then
      dsp = lib
      return
   end

   error("Could not load libdsp from any location")
end)

if not load_success then
   error("Failed to load libdsp: " .. tostring(load_error))
end

-- Declare C function signatures
ffi.cdef[[
// Scaling functions
double scale_linear(double x, double in_min, double in_max, double out_min, double out_max);
double scale_sine1(double x, double in_min, double in_max, double out_min, double out_max);
double scale_sine2(double x, double in_min, double in_max, double out_min, double out_max);
double scale_exp1(double x, double s, double in_min, double in_max, double out_min, double out_max);
double scale_exp2(double x, double s, double in_min, double in_max, double out_min, double out_max);
double scale_log1(double x, double p, double i_min, double i_max, double o_min, double o_max);
double scale_log2(double x, double p, double i_min, double i_max, double o_min, double o_max);

// Audio DSP functions
double soft_clip(double x, double drive);
double hard_clip(double x, double threshold);
double bit_crush(double x, double bits);
double lpf_1pole(double x, double prev, double cutoff);
double hpf_1pole(double x, double prev_in, double prev_out, double cutoff);
double lerp(double a, double b, double t);
double envelope_follow(double x, double prev, double attack, double release);
double wavefold(double x, double threshold);
double ring_mod(double x, double modulator);
double clamp(double x, double min, double max);

// Oscillator functions
double osc_sine(double phase);
double osc_saw(double phase);
double osc_saw_bl(double phase, double phase_inc);
double osc_square(double phase, double pulse_width);
double osc_square_bl(double phase, double pulse_width, double phase_inc);
double osc_triangle(double phase);
double osc_phase_inc(double freq, double sample_rate);
double osc_phase_wrap(double phase);
]]

return dsp

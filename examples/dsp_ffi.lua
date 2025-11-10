-- dsp_ffi.lua
-- FFI wrapper for libdsp C functions
-- Demonstrates using LuaJIT FFI to call optimized C code

local ffi = require 'ffi'

-- Load the libdsp shared library
-- The library is in the support directory (Max package structure)
-- Max automatically includes support directory in the library search path
local dsp = ffi.load("libdsp")

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
]]

return dsp

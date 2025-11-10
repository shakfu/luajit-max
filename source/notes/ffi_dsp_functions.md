# FFI DSP Functions

## Overview

The `libdsp` library provides optimized C implementations of common DSP functions that can be called from Lua using LuaJIT's FFI (Foreign Function Interface). This provides a way to extend the externals with high-performance DSP operations while maintaining the flexibility of Lua scripting.

## Architecture

1. **C Library** (`source/projects/libdsp/libdsp.c`)
   - Contains optimized C implementations of DSP functions
   - Compiled as a shared library (`libdsp.dylib`)
   - Output to `support/libdsp.dylib` (standard Max package location for shared libraries)

2. **FFI Wrapper** (`examples/dsp_ffi.lua`)
   - Declares C function signatures using LuaJIT FFI
   - Loads the shared library using `ffi.load("libdsp")`
   - Max automatically includes the `support` directory in the library search path
   - Provides type-safe access to C functions from Lua

3. **Example Functions** (`examples/dsp.lua`)
   - Demonstrates using FFI functions with the parameter system
   - Provides ready-to-use DSP effects that call C code
   - Gracefully falls back if FFI is not available

## Available C Functions

### Scaling Functions
- `scale_linear(x, in_min, in_max, out_min, out_max)` - Linear scaling
- `scale_sine1(x, in_min, in_max, out_min, out_max)` - Sine curve 1
- `scale_sine2(x, in_min, in_max, out_min, out_max)` - Sine curve 2
- `scale_exp1(x, s, in_min, in_max, out_min, out_max)` - Exponential curve 1
- `scale_exp2(x, s, in_min, in_max, out_min, out_max)` - Exponential curve 2
- `scale_log1(x, p, in_min, in_max, out_min, out_max)` - Logarithmic curve 1
- `scale_log2(x, p, in_min, in_max, out_min, out_max)` - Logarithmic curve 2

### Audio DSP Functions
- `soft_clip(x, drive)` - Soft clipping/saturation (tanh approximation)
- `hard_clip(x, threshold)` - Hard clipping
- `bit_crush(x, bits)` - Bit depth reduction
- `lpf_1pole(x, prev, cutoff)` - One-pole low-pass filter
- `hpf_1pole(x, prev_in, prev_out, cutoff)` - One-pole high-pass filter
- `envelope_follow(x, prev, attack, release)` - Envelope follower
- `wavefold(x, threshold)` - Wave folding distortion
- `ring_mod(x, modulator)` - Ring modulation
- `lerp(a, b, t)` - Linear interpolation
- `clamp(x, min, max)` - Clamp value to range

## Example Lua Functions Using FFI

All FFI-based functions in `dsp.lua` follow this pattern:

```lua
ffi_softclip = function(x, fb, n, ...)
   -- Get parameters from PARAMS table
   local drive = PARAMS.drive or 2.0
   local mix = PARAMS.mix or 1.0

   -- Call C function via FFI
   local wet = dsp_c.soft_clip(x, drive)

   -- Mix dry and wet
   return wet * mix + x * (1.0 - mix)
end
```

### Available FFI Functions

1. **ffi_softclip** - Soft clipping distortion
   - Parameters: `drive` (1.0-5.0), `mix` (0.0-1.0)
   - Usage: `ffi_softclip drive 3.0 mix 0.7`

2. **ffi_bitcrush** - Bit crusher effect
   - Parameters: `bits` (1-16), `mix` (0.0-1.0)
   - Usage: `ffi_bitcrush bits 8 mix 0.5`

3. **ffi_wavefold** - Wave folding distortion
   - Parameters: `threshold` (0.1-1.0), `mix` (0.0-1.0)
   - Usage: `ffi_wavefold threshold 0.5 mix 0.8`

4. **ffi_lpf** - One-pole low-pass filter
   - Parameters: `cutoff` (0.0-1.0), `mix` (0.0-1.0)
   - Usage: `ffi_lpf cutoff 0.3 mix 1.0`

5. **ffi_envelope** - Envelope follower
   - Parameters: `attack` (0.0-1.0), `release` (0.0-1.0)
   - Usage: `ffi_envelope attack 0.1 release 0.01`

## Integration with Parameter System

All FFI functions work seamlessly with the dynamic parameter system:

```
# Combined syntax (switch function + set parameters)
ffi_softclip drive 3.0 mix 0.7

# Update individual parameters
drive 4.0
mix 1.0
```

## Performance Benefits

Using FFI provides several advantages:

1. **Speed**: C code runs at native speed (no Lua interpretation overhead)
2. **JIT Optimization**: LuaJIT can often inline FFI calls completely
3. **Extensibility**: Easy to add new C functions without recompiling externals
4. **Flexibility**: Lua wrappers can add parameter handling, mixing, etc.

## Adding New FFI Functions

1. **Add C function to `libdsp.c`**:
   ```c
   double my_effect(double x, double param1, double param2) {
       // Your DSP code here
       return processed_sample;
   }
   ```

2. **Declare in `dsp_ffi.lua`**:
   ```lua
   ffi.cdef[[
   double my_effect(double x, double param1, double param2);
   ]]
   ```

3. **Create Lua wrapper in `dsp.lua`**:
   ```lua
   ffi_myeffect = function(x, fb, n, ...)
      local param1 = PARAMS.param1 or 1.0
      local param2 = PARAMS.param2 or 0.5
      return dsp_c.my_effect(x, param1, param2)
   end
   ```

4. **Rebuild**: Run `make` to recompile `libdsp.dylib`

## Build System

The library is automatically built as part of the main build:

```bash
make        # Builds libdsp along with all externals
```

Output: `support/libdsp.dylib`

The `support` directory is the standard Max package location for shared libraries. Max automatically includes this directory in the dynamic library search path, so FFI can load the library simply with `ffi.load("libdsp")` without needing an absolute path.

## Notes

- FFI loading is optional - functions gracefully fall back to passthrough if library not found
- The library is loaded relative to the examples directory
- All C functions use `double` for audio samples (matches Max/MSP convention)
- Stateless functions (like filters) require external state management in Lua

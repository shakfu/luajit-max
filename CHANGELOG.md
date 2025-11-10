# CHANGELOG

All notable project-wide changes will be documented in this file. Note that each subproject has its own CHANGELOG.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) and [Commons Changelog](https://common-changelog.org). This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Types of Changes

- Added: for new features.
- Changed: for changes in existing functionality.
- Deprecated: for soon-to-be removed features.
- Removed: for now removed features.
- Fixed: for any bug fixes.
- Security: in case of vulnerabilities.

---

## [Unreleased]

### Added
- **Dynamic Parameter System**: Flexible parameter handling for both `luajit~` and `luajit.stk~`
  - **Up to 32 Parameters**: Increased from fixed 1/4 parameters to dynamic array of 32 parameters
  - **Positional Parameters**: Send numeric lists (e.g., `0.5 2.0 0.7`) for quick parameter updates
  - **Named Parameters**: Send arbitrary name/value pairs (e.g., `gain 0.8 cutoff 0.3 mix 0.7`)
  - **Global PARAMS Table**: Named parameters stored in Lua global table accessible as `PARAMS.gain`, `PARAMS.cutoff`, etc.
  - **Combined Syntax**: Switch function and set parameters in one message (e.g., `lpf_gain_mix gain 0.8 cutoff 0.6 mix 1.0`)
  - **Backward Compatible**: Legacy single/4-parameter functions continue to work
  - See `source/notes/dynamic_parameters.md` for complete documentation
- **FFI Integration**: LuaJIT Foreign Function Interface support for calling optimized C code
  - **libdsp Shared Library**: C library with optimized DSP functions (`support/libdsp.dylib`)
  - **FFI Wrapper Module**: `examples/dsp_ffi.lua` declares C function signatures and loads library
  - **FFI-based DSP Functions**: Example functions in `examples/dsp.lua` demonstrating FFI usage
    - `ffi_softclip`: Soft clipping/saturation (drive, mix parameters)
    - `ffi_bitcrush`: Bit depth reduction (bits, mix parameters)
    - `ffi_wavefold`: Wave folding distortion (threshold, mix parameters)
    - `ffi_lpf`: One-pole low-pass filter (cutoff, mix parameters)
    - `ffi_envelope`: Envelope follower (attack, release parameters)
  - **C DSP Functions in libdsp**: Optimized implementations for native-speed execution
    - `soft_clip()`, `hard_clip()` - Clipping and saturation
    - `bit_crush()` - Bit depth reduction
    - `wavefold()` - Wave folding distortion
    - `lpf_1pole()`, `hpf_1pole()` - One-pole filters
    - `envelope_follow()` - Envelope detection
    - `ring_mod()` - Ring modulation
    - `lerp()` - Linear interpolation
    - `clamp()` - Range limiting
    - Plus scaling functions: `scale_linear()`, `scale_sine1/2()`, `scale_exp1/2()`, `scale_log1/2()`
  - **Performance**: C functions run at native speed, LuaJIT can inline FFI calls
  - **Extensibility**: Add DSP functions without recompiling externals
  - **Max Package Integration**: Library built to `support/` directory (Max standard location)
  - **Automatic Loading**: Max includes `support/` in library search path, enabling `ffi.load("libdsp")`
  - **Graceful Fallback**: FFI functions degrade gracefully if library not available
  - See `source/notes/ffi_dsp_functions.md` for complete documentation
- **Example DSP Functions**: New practical examples demonstrating parameter handling
  - `waveshape`: Soft-clipping waveshaper with variable parameters (1-3: gain, drive, mix)
  - `lpf_gain_mix`: One-pole low-pass filter using named parameters (gain, cutoff, mix)
  - `delay`: Proper delay with circular buffer (time, feedback, mix parameters)
  - `hybrid_filter`: Demonstrates both positional and named parameter support
  - `oscillator_bank`: Multi-oscillator example with many parameters
- **luajit External**: New general-purpose Max external for writing complete externals in Lua
  - Dynamic inlet/outlet configuration (1-16 each)
  - Message routing to Lua functions (bang, int, float, list, anything)
  - Text editor integration with hot reload
  - Lua module path configuration for `require()` statements
  - Supports writing non-DSP Max objects entirely in Lua
- **SAMPLE_RATE Global Variable**: Both externals now automatically set `SAMPLE_RATE` global in Lua with current audio sample rate
- **Max API Module**: New `api` module exposes Max console functions to Lua
  - `api.post(message)` - Post message to Max console
  - `api.error(message)` - Post error to Max console
  - Outlet wrappers with methods: `bang()`, `int()`, `float()`, `symbol()`, `list()`, `anything()`
- **Time Helper Utilities**: Created `examples/time_helpers.lua` with sample rate conversion functions
  - Time to samples: `ms_to_samples()`, `sec_to_samples()`
  - Samples to time: `samples_to_ms()`, `samples_to_sec()`
  - Frequency conversions: `freq_to_period()`, `period_to_freq()`
  - Filter helpers: `cutoff_to_alpha()`, `tau_to_cutoff()`
  - Debug helper: `print_timing_info()`
- **Function Reference Caching**: Lua function lookups now cached in `LUA_REGISTRYINDEX` (eliminates 44,100+ lookups per second)
- **Error State Management**: Added `in_error_state` flag for graceful degradation when Lua errors occur
- **Feedback State Variables**: Both externals now track `prev_sample` for proper feedback parameter
- **Documentation**: Comprehensive guides added
  - `SAMPLE_RATE_USAGE.md` - Guide for time-based DSP calculations
  - `API_MODULE.md` - Max API module documentation
  - `CODE_REVIEW.md` - Comprehensive code review findings
  - `CRITICAL_FIXES_SUMMARY.md` - Summary of critical fixes applied

### Changed
- **Parameter Handling Architecture**: Complete overhaul of parameter system
  - Both externals now use `lua_engine_call_dsp_dynamic()` for flexible parameter passing
  - Added `lua_engine_set_named_param()` and `lua_engine_clear_named_params()` functions
  - `mlj_list()` and `lstk_list()` handlers detect and route positional vs named parameters
  - `mlj_anything()` and `lstk_anything()` now support combined function+parameter syntax
- **PARAMS Global Initialization**: Added `PARAMS = PARAMS or {}` to `examples/dsp.lua` for safe initialization
- **Build System - libdsp**: Created CMakeLists.txt for libdsp shared library
  - Outputs directly to `support/libdsp.dylib` (no subdirectories)
  - Configuration-specific output directories explicitly set (Debug, Release)
  - Links with math library (m)
  - Integrated into main build process via root CMakeLists.txt
- **Refactor luajit.stk~**: Moved binding code to a header
- **Time-Based Parameters**: Delay function in `examples/dsp_stk.lua` now uses seconds instead of samples
  - p0 parameter is now in seconds (e.g., 0.05 = 50ms)
  - Automatically converts to samples using SAMPLE_RATE
  - Sample-rate independent and portable
- **Lua Call Protection**: All `lua_call()` replaced with `lua_pcall()` for safe error handling
- **Garbage Collection**: GC now controlled for real-time safety
  - Stopped during initialization
  - Restarted in incremental mode (pause=200, stepmul=100)
- **Function Signatures**: Lua example files now document expected function signatures clearly
- **Delay Initialization**: Default delay increased from 9 samples to 441 samples (~10ms at 44.1kHz)

### Fixed
- **luajit File Loading**: Fixed `locatefile_extended()` call to use correct FOURCC code and separate output/input type parameters
  - Changed from incorrect `FOUR_CHAR_CODE('TEXT')` to correct `FOUR_CHAR_CODE('Jlua')` for Lua files
  - Fixed parameter order: `outtype` (output) vs `filetype` (input typelist) now properly separated
  - Files in `examples/` directory now load correctly from Max package
- **Critical RT Safety**: Eliminated per-sample function name lookups
- **Stack Corruption**: Added proper validation of function references and return values
- **Memory Leaks**:
  - Fixed `dirname()` path corruption by using `strdup()` before modification
  - Added missing `object_free()` calls for `t_string` objects
  - Fixed proxy inlet NULL pointer cleanup
- **NaN/Inf Protection**: Added validation and clamping of Lua return values
- **Delay Feedback Logic**: Fixed incorrect feedback path in delay function
  - Now uses dedicated `_delay_last_out` state variable
  - Properly feeds back delay output instead of mixed signal
- **STK Integer Parameters**: Fixed LuaBridge integer conversion errors
  - `Delay::setDelay()` now receives proper integer with rounding
  - `BlitSquare::setHarmonics()` now receives proper unsigned int
- **Delay Efficiency**: Only calls `setDelay()` when delay time actually changes
- **Missing Parameters**: Fixed `base()` function in `dsp_stk.lua` (was missing `fb` parameter)
- **SAMPLE_RATE Documentation**: Clarified that hardcoded values in examples are fallbacks only

### Security
- **Protected Lua Execution**: All Lua calls now use protected mode to prevent crashes
- **Input Validation**: Added bounds checking and clamping for audio values
- **STK Exception Handling**: Added try-catch blocks around STK initialization in `luajit.stk~`

## [0.2.x]

- Project renamed to `luajit-max`

## [0.1.0]

- Initial release of project
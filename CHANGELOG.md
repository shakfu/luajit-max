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
- **SAMPLE_RATE Global Variable**: Both externals now automatically set `SAMPLE_RATE` global in Lua with current audio sample rate
- **Max API Module**: New `api` module exposes Max console functions to Lua
  - `api.post(message)` - Post message to Max console
  - `api.error(message)` - Post error to Max console
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
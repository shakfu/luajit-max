# Dynamic Parameter Implementation

## Overview

Both `luajit~` and `luajit.stk~` now support flexible parameter handling with up to 32 dynamic parameters, replacing the previous fixed 1-parameter and 4-parameter systems.

## Changes

### Core Implementation

1. **Dynamic Parameter Array**: Both externals now use a `params[MAX_PARAMS]` array (MAX_PARAMS = 32) to store positional parameter values
2. **Named Parameter Table**: Named parameters are stored in a global Lua table `PARAMS` accessible to all DSP functions
3. **Parameter Count Tracking**: `num_params` tracks the number of active positional parameters
4. **Backward Compatibility**: Legacy parameter fields (`param1` for luajit~, `param0-3` for luajit.stk~) are maintained and synchronized with the dynamic array

### New Message Handlers

Both externals now support multiple parameter input methods:

#### 1. Positional Numeric List
Send a list of numeric values to set parameters by position:
```
10 0.1 4
```
Sets positional params accessible via variadic args: `...`

#### 2. Named Parameters
Send arbitrary parameter name/value pairs:
```
gain 0.5 tone 0.3 mix 0.7
```
Sets named parameters in the global `PARAMS` table accessible as:
- `PARAMS.gain = 0.5`
- `PARAMS.tone = 0.3`
- `PARAMS.mix = 0.7`

#### 3. Function Switching
Send just a function name to switch to that function:
```
named_params_example
```
Switches to the `named_params_example` function without changing parameters.

#### 4. Combined Function + Parameters (NEW)
Send a function name followed by parameter name/value pairs to switch function AND set parameters in one message:
```
named_params_example gain 0.8 tone 0.6 mix 1.0
```
This switches to `named_params_example` AND sets the three named parameters.

**How it works:** If the first symbol is a valid function name and is followed by arguments, both operations happen atomically.

Parameter names can be any valid symbol (no numbers, spaces, or special characters at the start).

### Modified Files

#### luajit~
- `source/projects/luajit~/luajit~.c`
  - Added dynamic parameter array and num_params to struct
  - Added `mlj_list()` handler for list messages
  - Modified `mlj_anything()` to handle named parameters
  - Updated `mlj_float()` to sync with params array
  - Modified `mlj_perform64()` to use `lua_engine_call_dsp_dynamic()`

#### luajit.stk~
- `source/projects/luajit.stk~/luajit.stk~.cpp`
  - Added dynamic parameter array and num_params to struct
  - Added `lstk_list()` handler for list messages
  - Modified `lstk_anything()` to handle named parameters
  - Updated `lstk_float()` to sync with params array and maintain backward compatibility
  - Modified `lstk_perform64()` to use `lua_engine_call_dsp_dynamic()`

#### lua_engine
- `source/projects/common/lua_engine.h` - Added `lua_engine_set_named_param()` and `lua_engine_clear_named_params()`
- `source/projects/common/lua_engine.c` - Implemented named parameter functions using global `PARAMS` table

### Lua Function Signatures

Lua DSP functions can now use three different approaches:

#### 1. Legacy Single Parameter (Backward Compatible)
```lua
function my_dsp(x, fb, n, p1)
  return x * p1
end
```

#### 2. Positional Parameters (Variadic)
```lua
function my_dsp(x, fb, n, ...)
  local params = {...}
  local amp = params[1] or 1.0
  local freq = params[2] or 440
  local mix = params[3] or 1.0
  return x * amp
end
```

#### 3. Named Parameters (via PARAMS table)
```lua
function my_dsp(x, fb, n, ...)
  -- Access named parameters with defaults
  local delay = PARAMS.delay or 1.0
  local feedback = PARAMS.feedback or 0.5
  local dry_wet = PARAMS.dry_wet or 1.0
  return x * dry_wet + fb * feedback * (1.0 - dry_wet)
end
```

#### 4. Hybrid Approach (Both Positional and Named)
```lua
function my_dsp(x, fb, n, ...)
  local params = {...}
  local gain, tone

  -- Try positional first
  if #params >= 2 then
    gain = params[1]
    tone = params[2]
  else
    -- Fall back to named with defaults
    gain = PARAMS.gain or 1.0
    tone = PARAMS.tone or 0.5
  end

  return x * gain
end
```

Where:
- `x`: audio input (current sample)
- `fb`: feedback (previous output sample)
- `n`: samples remaining in buffer
- `...`: variable number of positional parameters (up to 32)
- `PARAMS`: global table containing named parameters

### Example Usage

See `examples/dsp.lua` for example functions:
- `waveshape`: Demonstrates variable positional parameter handling (1-3 params: gain, drive, mix)
- `lpf_gain_mix`: One-pole low-pass filter using named parameters (gain, cutoff, mix)
- `hybrid_filter`: Supports both positional and named parameters
- `oscillator_bank`: Uses 7 positional parameters (frequencies, amplitudes, mix)

### Benefits

1. **Flexibility**: Up to 32 parameters instead of 1 or 4
2. **Backward Compatibility**: Existing patches and Lua functions continue to work
3. **Named Parameters**: Easier to set specific parameters without affecting others
4. **Consistent API**: Both externals now share the same parameter handling approach

### Testing

Build the project:
```bash
make
```

Test in Max/MSP:

#### Test 1: Positional Parameters
1. Create a `luajit~ dsp.lua` object
2. Send message: `waveshape` (switch to waveshape function)
3. Send positional lists:
   - `0.5` (single param - gain/attenuation)
   - `1.0 2.0` (two params - gain + waveshaping drive)
   - `0.8 3.0 0.7` (three params - gain, drive, dry/wet mix)

#### Test 2: Named Parameters
1. Send message: `lpf_gain_mix` (switch function)
2. Send named parameter lists:
   - `gain 0.8 cutoff 0.3 mix 0.7`
   - `cutoff 0.5` (just update cutoff, others keep previous values)
   - `mix 1.0` (just update mix to 100% wet)

#### Test 3: Combined Function + Parameters
1. Send combined message: `lpf_gain_mix gain 0.8 cutoff 0.6 mix 1.0`
   - This switches to the function AND sets all three parameters in one message
2. Send another combined message: `hybrid_filter gain 0.5 tone 0.3`
   - Switches to different function with different parameters

#### Test 4: Hybrid Approach
1. Send message: `hybrid_filter` (switch to hybrid_filter function)
2. Test both methods:
   - `0.5 0.3` (positional: gain, tone)
   - `gain 0.8 tone 0.6` (named parameters)

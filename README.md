# luajit-max

A collection of Max/MSP externals that embed LuaJIT, enabling dynamic scripting for both audio processing and general Max object development.

## Externals

### 1. **luajit**

A general-purpose Max external for writing complete Max objects in Lua. Supports non-DSP message-based objects with dynamic inlet/outlet configuration.

**Features:**
- Dynamic inlets and outlets (1-16 each, configured in Lua)
- Message routing (bang, int, float, list, anything) to Lua functions
- Text editor integration with hot reload
- Lua module path configuration for `require()` statements
- Access to Max API through `api` module (post, error, outlets, etc.)

**Example:** See `examples/simple_luajit.lua`, `examples/counter.lua`, `examples/math_ops.lua`

### 2. **luajit~**

Embeds the LuaJIT engine in a Max/MSP external's audio thread for real-time DSP processing.

**Features:**
- Real-time Lua DSP functions with per-sample or per-vector processing
- On-the-fly function modification and hot reload
- Automatic `SAMPLE_RATE` global variable
- Function reference caching for RT performance
- Protected execution with graceful error handling

A number of basic DSP functions are implemented in `examples/dsp.lua` including examples from [worp](https://github.com/zevv/worp).

The help patch demonstrates live coding: functions can be changed and selected by name, and the `dsp.lua` script can be reloaded by sending a bang to the object even while the audio stream is active.

### 3. **luajit.stk~**

Combines LuaJIT DSP with the [Synthesis ToolKit (STK)](https://github.com/thestk/stk) library. STK C++ objects are automatically wrapped using [LuaBridge3](https://github.com/kunitoki/LuaBridge3) and a custom Python parser.

**Features:**
- All features of `luajit~`
- Access to STK synthesis and effects classes from Lua
- Automatic C++ binding generation via header parsing
- Generated API documentation in `examples/dsp_stk_api.lua`

The relevant Lua file is `examples/dsp_stk.lua` which demonstrates STK integration.


## Installation

Just type the following:

```bash
git clone https://github.com/shakfu/luajit-max.git
make setup
make
```

Note: `make setup` does the following:

```bash
git submodule init
git submodule update
ln -s $(shell pwd) "$(HOME)/Documents/Max $(MAX_VERSION)/Packages/$(shell basename `pwd`)"
```

## Usage

Open the help files for demonstrations of the externals.


## References

- [LuaJIT](https://luajit.org)
- [STK Introduction](http://www.music.mcgill.ca/~gary/307/week8/stk.html)
- [luajit_audio](https://github.com/grrrwaaa/luajit_audio.git)
- [LuaBridge3](https://github.com/kunitoki/LuaBridge3)
- [cxxheaderparser](https://github.com/robotpy/cxxheaderparser) -- python c++ header parser used in the parser/generator script.


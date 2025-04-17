# luajit-max

Includes two externals which both embed luajit in a Max/Msp external:

1. **luajit~**

Embeds the luajit engine in a max/msp external's audio thread.

A number of basic dsp functions are implemented in `examples/dsp.lua` including some examples from [worp](https://github.com/zevv/worp).

The dynamic nature of lua makes it possible to add, modify or change dsp functions on-the-fly. 

The help patch provides an example of this: functions can be changed and selected by name from the dropdown and the`dsp.lua` script can be changed and then reloaded by sending a bang to the luajit~ object even while the audio stream is active.


2. **luajit.stk~**

Same as (1) but with the addition of c++ objects from the [Synthesis ToolKit (stk)](https://github.com/thestk/stk) library which are wrapped automatically using [LuaBridge3](https://github.com/kunitoki/LuaBridge3) and a custom python script.

In this case, the relevant lua file loaded at start is called `dsp_stk.lua`. There's also another generated file, `dsp_stk_api.lua`, which includes reference and documentation for all wrapped stk lua functions.


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


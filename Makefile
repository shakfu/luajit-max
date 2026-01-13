MAX_VERSION := 9
SCRIPTS := source/scripts
BUILD := build
LUAJIT := $(BUILD)/deps/luajit-install/lib/libluajit-5.1.a
STK := $(BUILD)/deps/stk-install/lib/libstk.a

# Check if system LuaJIT is available via Homebrew
# Use FORCE_BUILD_LUAJIT=1 to skip system LuaJIT and build from source
ifdef FORCE_BUILD_LUAJIT
    LUAJIT_DEP := $(LUAJIT)
    CMAKE_EXTRA_ARGS := -DFORCE_BUILD_LUAJIT=ON
else
    SYSTEM_LUAJIT := $(shell brew --prefix luajit 2>/dev/null)
    ifneq ($(SYSTEM_LUAJIT),)
        LUAJIT_DEP :=
        CMAKE_EXTRA_ARGS :=
    else
        LUAJIT_DEP := $(LUAJIT)
        CMAKE_EXTRA_ARGS :=
    endif
endif

.PHONY: cmake fixup clean setup

all: cmake


cmake: $(LUAJIT_DEP) $(STK)
	@mkdir -p build && \
		cd build && \
		cmake .. -GXcode \
			-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
			$(CMAKE_EXTRA_ARGS) \
			&& \
		cmake --build . --config Release

$(LUAJIT):
	@bash $(SCRIPTS)/build_dependencies.sh

$(STK):
	@bash $(SCRIPTS)/build_stk.sh

fixup: cmake
	@bash $(SCRIPTS)/fix_bundle.sh


clean:
	@rm -rf build


setup:
	git submodule init
	git submodule update
	ln -s $(shell pwd) "$(HOME)/Documents/Max $(MAX_VERSION)/Packages/$(shell basename `pwd`)"

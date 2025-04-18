MAX_VERSION := 9
SCRIPTS := source/scripts
BUILD := build
LUAJIT := $(BUILD)/deps/luajit-install/lib/libluajit-5.1.a
STK := $(BUILD)/deps/stk-install/lib/libstk.a

.PHONY: cmake fixup clean setup 

all: cmake


cmake: $(LUAJIT) $(STK)
	@mkdir -p build && \
		cd build && \
		cmake .. -GXcode \
			-DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
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

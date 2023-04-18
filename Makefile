SCRIPTS := source/scripts


.PHONY: cmake deps fixup clean setup 

all: cmake


cmake:
	@mkdir -p build && cd build && cmake .. && make

deps:
	@bash $(SCRIPTS)/build_dependencies.sh


fixup: cmake
	@bash $(SCRIPTS)/fix_bundle.sh


clean:
	@rm -rf build


setup:
	git submodule init
	git submodule update
	ln -s $(shell pwd) "$(HOME)/Documents/Max 8/Packages/$(shell basename `pwd`)"

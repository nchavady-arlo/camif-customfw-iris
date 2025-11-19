# GEN_DIR is defined at file: VENDOR/PEGA/make/tools.mk
# GEN_DIR = build
include vendor.mk
#include evn, PEGA_ROOT_DIR

BUILD_ROOT_DIR=$(shell pwd)
export BUILD_ROOT_DIR

include $(BUILD_ROOT_DIR)/make/tools.mk

BUILD_INSTALL_DIR=$(BUILD_ROOT_DIR)/$(GEN_DIR)
export BUILD_INSTALL_DIR


ALL_BUILD = $(patsubst %/, %, $(dir $(wildcard */Makefile)))


.PHONY:

all:
	@$(MAKE) build_general
	@$(MAKE) build_all
	
build_general:
	@$(MKDIR) -p $(GEN_DIR)
	@$(MKDIR) -p $(GEN_DIR)/bin
	@$(MKDIR) -p $(GEN_DIR)/lib
	
build_all:
	@$(foreach i, $(ALL_BUILD), \
		$(MAKE) -C $i || exit 1; \
	)
clean:
	@$(foreach i, $(ALL_BUILD), $(MAKE) -C $i clean || exit 1;)

cleanall:
	@$(MAKE) clean
	@$(RM) -rf $(BUILD_INSTALL_DIR)

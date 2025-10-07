include config
include $(BUILD_ROOT_DIR)/make/tools.mk

BUILD_PROG_HOME=$(shell pwd)
export BUILD_PROG_HOME

ifeq (y,$(BUILD_IN))
all: package
else
all:
endif

package:
	@$(MKDIR) -p $(GEN_DIR)
	@$(MAKE) -f pega_diag.mak

clean:
	@make -f pega_diag.mak clean
	@rm -rf build/

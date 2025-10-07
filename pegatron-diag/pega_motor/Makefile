include config
include $(BUILD_ROOT_DIR)/make/tools.mk

BUILD_PROG_HOME=$(shell pwd)
export BUILD_PROG_HOME

ifeq (y,$(BUILD_IN))
all: package
else
all:
endif

full:
ifeq (y,$(BUILD_IN))
	@$(MAKE) all
endif

package:
	@cp -f src/gpio.sh ../build/bin
	@$(MKDIR) -p $(GEN_DIR)
	@$(MAKE) -f pega_motor.mak

clean:
	@$(MAKE) -f pega_motor.mak clean
	@$(RM) -rf $(GEN_DIR)


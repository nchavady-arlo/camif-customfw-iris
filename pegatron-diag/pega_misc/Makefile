include config
include $(BUILD_ROOT_DIR)/make/tools.mk

BUILD_PROG_HOME=$(shell pwd)
export BUILD_PROG_HOME

ifeq (y,$(BUILD_IN))
all: nfc_lib package
else
all:
endif

full:
ifeq (y,$(BUILD_IN))
	@$(MAKE) all
endif

package:
	@$(MKDIR) -p $(GEN_DIR)
	@$(MAKE) -f pega_misc.mak

nfc_lib :
	@$(MKDIR) -p $(GEN_DIR)
	@$(MAKE) -f nfc_lib.mak

install:
	@cp -f ./$(TARGET) ./../../ROOTFS/lib
clean:
	@$(MAKE) -f pega_misc.mak clean
	@$(RM) -rf $(GEN_DIR)


############################
#   Define TARGET_BOARD    #
############################

ifeq (C551, $(SDK_HARDWARE))
DEFINES		+= -DTARGET_BOARD=1
#else ifeq (MACAW, $(SDK_HARDWARE))
#DEFINES		+= -DTARGET_BOARD=2
else
$(error SDK_HARDWARE is null string)
endif

OUTPUT_BIN_PATH=$(BUILD_PROG_HOME)/$(GEN_DIR)/bin
OUTPUT_LIB_PATH=$(BUILD_PROG_HOME)/$(GEN_DIR)/lib
OUTPUT_INC_PATH=$(BUILD_PROG_HOME)/$(GEN_DIR)/include

BUILD_DATE=$(shell date -u +%F-%T)

TARGET_NAME	= diag

LDDEPS		= -lpthread -lrt -lm -Llib

SRC	= $(wildcard src/*.c)

INCLUDE		+= -Iinclude

include $(BUILD_ROOT_DIR)/make/makefile.mk

TARGET_NAME	= pega_misc_dbg

LDDEPS		= -lpthread -lrt -lm

SRC			= \
	src/main.c \
	src/pega_debug_msgq_cmd.c

INCLUDE		+= -Iinclude

include $(BUILD_ROOT_DIR)/make/makefile.mk

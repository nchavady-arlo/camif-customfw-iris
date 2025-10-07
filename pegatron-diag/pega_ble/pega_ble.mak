TARGET_NAME	= pega_ble

LDDEPS		= -lpthread -lrt -lm

SRC			= \
	src/main.c \
	src/pega_ble.c

INCLUDE		+= -Iinclude

include $(BUILD_ROOT_DIR)/make/makefile.mk

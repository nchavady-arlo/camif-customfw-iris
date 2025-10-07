TARGET_NAME	= pega_ip

LDDEPS		= -lpthread -lrt -lm

SRC			= \
	src/main.c

INCLUDE		+= -Iinclude

include $(BUILD_ROOT_DIR)/make/makefile.mk

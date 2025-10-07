TARGET_NAME	= diag

LDDEPS		= -lpthread -lrt -lm -Llib

SRC			= \
	src/pega_diag.c \
	src/pega_network_ip.c \
	src/pega_diag_msgq_cmd.c

INCLUDE		+= -Iinclude

include $(BUILD_ROOT_DIR)/make/makefile.mk

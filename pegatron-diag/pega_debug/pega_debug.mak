TARGET_NAME	= pega_debug

CFLAGS = -Wall -O2
LDFLAGS = -lrt 

SRC			= \
	src/main.c \
	src/pega_mq.c

include $(BUILD_ROOT_DIR)/make/makefile.mk

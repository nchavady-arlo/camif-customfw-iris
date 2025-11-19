TARGET_NAME	= pega_dtbo

CFLAGS = -Wall -O2
LDFLAGS = -lrt

SRC	= $(wildcard src/*.c)

include $(BUILD_ROOT_DIR)/make/makefile.mk

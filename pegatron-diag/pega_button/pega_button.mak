TARGET_NAME	= pega_button
LIBDIR := lib

CFLAGS  += -Isrc -I$(LIBDIR)/Inc
LDDEPS  = -lpthread -lrt -lm -L$(LIBDIR) -lgpiod

SRC	= $(wildcard src/*.c)

include $(BUILD_ROOT_DIR)/make/makefile.mk

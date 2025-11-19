TARGET_NAME	= pega_gpiod
LIBDIR := lib

CFLAGS  += -Isrc -I$(LIBDIR)/Inc
LDDEPS  = -lpthread -lrt -lm -L$(LIBDIR) -lgpiod

SRC			= \
	src/main.c \
	src/pega_gpiod.c \
	src/pega_gpiod_int.c

include $(BUILD_ROOT_DIR)/make/makefile.mk

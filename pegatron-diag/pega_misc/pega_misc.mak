TARGET_NAME	= pega_misc
LIBDIR := build/lib
LIBDIR2 := lib
CHIP := ST25R3916
INTERFACE := RFAL_USE_I2C


CFLAGS  += -D$(CHIP) -D$(INTERFACE) -INFC/src -INFC/rfal/include -Ilib/Inc -Isrc
LDDEPS  = -lpthread -lrt -lm -Llib -L$(LIBDIR) -lrfal -L$(LIBDIR2) -lgpiod

SRC	= $(wildcard src/*.c)
SRC += $(wildcard NFC/src/*.c)
include $(BUILD_ROOT_DIR)/make/makefile.mk

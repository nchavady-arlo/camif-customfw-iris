TARGET_NAME	= TCA9538

SRC			= \
	src/pega_TCA9538.c \
	src/pega_i2c_control.c

INCLUDE		+= -Iinclude

include $(BUILD_ROOT_DIR)/make/makefile.mk

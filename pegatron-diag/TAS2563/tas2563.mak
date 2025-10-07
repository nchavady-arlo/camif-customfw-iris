TARGET_NAME	= tas2563

SRC			= \
	src/Pega_Amp2563.c \
	src/Pega_gpio.c \
	src/Pega_i2c_control.c

INCLUDE		+= -Iinclude

include $(BUILD_ROOT_DIR)/make/makefile.mk

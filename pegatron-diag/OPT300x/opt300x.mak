TARGET_NAME	= opt300x

SRC			= \
	src/Pega_ALS_Opt300x.c \
	src/Pega_i2c_control.c

INCLUDE		+= -Iinclude

include $(BUILD_ROOT_DIR)/make/makefile.mk

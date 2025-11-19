TARGET_NAME	= pega_demo

LDDEPS		= -lpthread -lrt -lm

SRC			= \
	src/main.c \
	src/pega_voice_cmd.c \
	src/pega_led_flash.c \
	src/pega_gpio.c

include $(BUILD_ROOT_DIR)/make/makefile.mk

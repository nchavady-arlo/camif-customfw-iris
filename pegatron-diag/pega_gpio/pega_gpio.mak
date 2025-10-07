TARGET_NAME	= pega_gpio

SRC			= \
	src/main.c \
	src/pega_adc.c \
	src/pega_pwm.c \
	src/pega_gpio.c

include $(BUILD_ROOT_DIR)/make/makefile.mk

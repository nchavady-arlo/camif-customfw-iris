TARGET_NAME	= pega_pwm

SRC			= \
	src/main.c \
	src/pega_motor_awd8833.c \
	src/pega_gpio.c

include $(BUILD_ROOT_DIR)/make/makefile.mk

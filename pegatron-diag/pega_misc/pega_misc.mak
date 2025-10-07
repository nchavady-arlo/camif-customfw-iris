TARGET_NAME	= pega_misc

LDDEPS		= -lpthread -lrt -lm -Llib

SRC			= \
	src/main.c \
	src/pega_schedule.c \
	src/pega_nv_mode.c \
	src/pega_led_flash.c \
	src/pega_network_ip.c \
	src/pega_wifi.c \
	src/pega_motor_awd8833.c \
	src/pega_ble_uart.c \
	src/pega_motor_interrupt.c \
	src/pega_gpio_interrupt.c \
	src/pega_misc_diag.c \
	src/pega_als_opt300x.c \
	src/pega_amp_tas256x.c \
	src/pega_TCA9538.c \
	src/pega_gpio_key.c \
	src/pega_debug.c \
	src/pega_i2c_control.c \
	src/pega_adc.c \
	src/pega_pwm.c \
	src/pega_gpio.c

include $(BUILD_ROOT_DIR)/make/makefile.mk

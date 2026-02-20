#ifndef _PEGA_GPIO_H_
#define _PEGA_GPIO_H_

#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

#define PAD_PM_UART2_TX     63
#define PAD_PM_UART2_RX     64
#define PAD_GPIO9           17
#define PAD_PM_GPIO11       44
#define PAD_PM_GPIO8        54
#define PAD_PM_GPIO7        55
#define PAD_PM_PWM1         58
#define PAD_PM_PWM0         59

// Interrupt pin
#define MOTOR_nFAULT_PAN_PIN 				PAD_PM_UART2_TX
#define MOTOR_nFAULT_RISING_PIN  			PAD_PM_UART2_RX
//==============================================================================
// Sel pin
#define RISING_MOTOR_nSLEEP_PIN 			PAD_GPIO9
#define PAN_MOTOR_nSLEEP_PIN 				PAD_PM_GPIO11
//==============================================================================
// Contorl pin
#define MOTO_PWM0_PIN						PAD_PM_PWM0
#define MOTO_PWM1_PIN						PAD_PM_PWM1
#define MOTO_PWM2_PIN						PAD_PM_GPIO7
#define MOTO_PWM3_PIN						PAD_PM_GPIO8

#define IO_AWD8833C_PAN_MOTOR_ON()			gpiod_set_value(IO_O_PAN_MOTOR_nSLEEP,1)
#define IO_AWD8833C_PAN_MOTOR_OFF()			gpiod_set_value(IO_O_PAN_MOTOR_nSLEEP,0)
//==============================================================================
#define IO_AWD8833C_RISING_MOTOR_ON()		gpiod_set_value(IO_O_RISING_MOTOR_nSLEEP,	 1)
#define IO_AWD8833C_RISING_MOTOR_OFF()		gpiod_set_value(IO_O_RISING_MOTOR_nSLEEP,	 0)
//==============================================================================
#define IO_AWD8833C_AIN1_PIN_ON()			gpiod_set_value(IO_O_MOTO_PWM0,	 1)
#define IO_AWD8833C_AIN1_PIN_OFF()			gpiod_set_value(IO_O_MOTO_PWM0,	 0)
#define IO_AWD8833C_AIN2_PIN_ON()			gpiod_set_value(IO_O_MOTO_PWM1,	 1)
#define IO_AWD8833C_AIN2_PIN_OFF()			gpiod_set_value(IO_O_MOTO_PWM1,	 0)
//==============================================================================
#define IO_AWD8833C_BIN1_PIN_ON()			gpiod_set_value(IO_O_MOTO_PWM2,	 1)
#define IO_AWD8833C_BIN1_PIN_OFF()			gpiod_set_value(IO_O_MOTO_PWM2,	 0)
#define IO_AWD8833C_BIN2_PIN_ON()			gpiod_set_value(IO_O_MOTO_PWM3,	 1)
#define IO_AWD8833C_BIN2_PIN_OFF()			gpiod_set_value(IO_O_MOTO_PWM3,	 0)


extern struct gpio_desc *IO_O_nFAULT_PAN;
extern struct gpio_desc *IO_O_nFAULT_RISING;
extern struct gpio_desc *IO_O_RISING_MOTOR_nSLEEP;
extern struct gpio_desc *IO_O_PAN_MOTOR_nSLEEP;
extern struct gpio_desc *IO_O_MOTO_PWM0;
extern struct gpio_desc *IO_O_MOTO_PWM1;
extern struct gpio_desc *IO_O_MOTO_PWM2;
extern struct gpio_desc *IO_O_MOTO_PWM3;

int drv_motor_config_io(struct platform_device *pdev);
void drv_motor_io_init(void);


#endif /* _PEGA_GPIO_H_ */
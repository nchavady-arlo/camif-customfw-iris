#include "drv_motor_gpio.h"


struct gpio_desc *IO_O_nFAULT_PAN;
struct gpio_desc *IO_O_nFAULT_RISING;
struct gpio_desc *IO_O_RISING_MOTOR_nSLEEP;
struct gpio_desc *IO_O_PAN_MOTOR_nSLEEP;
struct gpio_desc *IO_O_MOTO_PWM0;
struct gpio_desc *IO_O_MOTO_PWM1;
struct gpio_desc *IO_O_MOTO_PWM2;
struct gpio_desc *IO_O_MOTO_PWM3;

/**
 *  @brief  unexport gpio and register gpio from dts
 *  @param  pin_num, gpio pointer, gpio string, dir
 *  @retval gpio status
 */
#define CHECK_GPIO(pin_num,gpio, name, dir)                     \
    do {                                                        \
        gpio_free(pin_num);                                      \
        gpio = devm_gpiod_get(&pdev->dev, name, dir);           \
        if (IS_ERR(gpio)) {                                    	\
            dev_err(&pdev->dev, "Failed to get %s: %ld\n",      \
                    name, PTR_ERR(gpio));                		\
            return PTR_ERR(gpio);                         		\
        }          												\
    } while (0)


//pega_gpio gpio init
int drv_motor_config_io(struct platform_device *pdev)
{
	// 2 select pin
	CHECK_GPIO(RISING_MOTOR_nSLEEP_PIN, IO_O_RISING_MOTOR_nSLEEP, "IO_O_RISING_MOTOR_nSLEEP",GPIOD_OUT_LOW);
	CHECK_GPIO(PAN_MOTOR_nSLEEP_PIN, IO_O_PAN_MOTOR_nSLEEP, "IO_O_PAN_MOTOR_nSLEEP",GPIOD_OUT_LOW);
	// 4 control pin
	CHECK_GPIO(MOTO_PWM0_PIN, IO_O_MOTO_PWM0, "IO_O_MOTO_PWM0",GPIOD_OUT_LOW);
	CHECK_GPIO(MOTO_PWM1_PIN, IO_O_MOTO_PWM1, "IO_O_MOTO_PWM1",GPIOD_OUT_LOW);
	CHECK_GPIO(MOTO_PWM2_PIN, IO_O_MOTO_PWM2, "IO_O_MOTO_PWM2",GPIOD_OUT_LOW);
	CHECK_GPIO(MOTO_PWM3_PIN, IO_O_MOTO_PWM3, "IO_O_MOTO_PWM3",GPIOD_OUT_LOW);
	
    dev_info(&pdev->dev, "All GPIOs initialized.\n");
    return 0;
}

void drv_motor_io_init(void)
{	 
	IO_AWD8833C_AIN1_PIN_OFF();
	IO_AWD8833C_AIN2_PIN_OFF();
	IO_AWD8833C_BIN1_PIN_OFF();
	IO_AWD8833C_BIN2_PIN_OFF();
	 	 
	IO_AWD8833C_PAN_MOTOR_OFF();
	IO_AWD8833C_RISING_MOTOR_OFF();
}
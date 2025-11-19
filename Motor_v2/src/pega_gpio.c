/*******************************************************************************
* File Name: pega_gpio.c
*
*******************************************************************************/
#include "pega_gpio.h"

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
int Motor_gpio_init(struct platform_device *pdev)
{
	CHECK_GPIO(MOTOR_nFAULT_PAN_PIN,IO_O_nFAULT_PAN, "IO_I_MOTOR_nFAULT_PAN",GPIOD_IN);
	CHECK_GPIO(MOTOR_nFAULT_RISING_PIN, IO_O_nFAULT_RISING, "IO_I_MOTOR_nFAULT_RISING",GPIOD_IN);
	CHECK_GPIO(RISING_MOTOR_nSLEEP_PIN, IO_O_RISING_MOTOR_nSLEEP, "IO_O_RISING_MOTOR_nSLEEP",GPIOD_OUT_LOW);
	CHECK_GPIO(PAN_MOTOR_nSLEEP_PIN, IO_O_PAN_MOTOR_nSLEEP, "IO_O_PAN_MOTOR_nSLEEP",GPIOD_OUT_LOW);
	CHECK_GPIO(MOTO_PWM0_PIN, IO_O_MOTO_PWM0, "IO_O_MOTO_PWM0",GPIOD_OUT_LOW);
	CHECK_GPIO(MOTO_PWM1_PIN, IO_O_MOTO_PWM1, "IO_O_MOTO_PWM1",GPIOD_OUT_LOW);
	CHECK_GPIO(MOTO_PWM2_PIN, IO_O_MOTO_PWM2, "IO_O_MOTO_PWM2",GPIOD_OUT_LOW);
	CHECK_GPIO(MOTO_PWM3_PIN, IO_O_MOTO_PWM3, "IO_O_MOTO_PWM3",GPIOD_OUT_LOW);
	
    dev_info(&pdev->dev, "All GPIOs initialized.\n");
    return 0;
}

/*
	Use GPIOD Function
| --------------------------------------------- | ------------------------------------------------ | ------------------------------------------------ |
| `devm_gpiod_get()` / `devm_gpiod_get_index()` | 從 device tree（或 ACPI）根據 label 取得 GPIO descriptor | 可設定方向                                            |
| `gpiod_direction_input()`                     | 將 GPIO 設為輸入                                      | 一般不常用，因為 `devm_gpiod_get(..., GPIOD_IN)` 就可設定方向  |
| `gpiod_direction_output(desc, value)`         | 設為輸出並設初始值                                        | 同上，通常用 `devm_gpiod_get(..., GPIOD_OUT_LOW/HIGH)` |
| `gpiod_set_value(desc, value)`                | 設定 GPIO 輸出值（0 or 1）                              | 只有在 GPIO 已為輸出狀態時才有效                              |
| `gpiod_get_value(desc)`                       | 讀取 GPIO 輸入值（回傳 0 or 1）                           | 用於 input GPIO                                    |
| `gpiod_set_consumer_name(desc, "name")`       | 設定這個 GPIO 的 consumer 名稱                          | 非必要，但有助於 debug                                   |
| `gpiod_put(desc)`                             | 釋放 GPIO descriptor（非 devm 版本用）                   | 如果不用 `devm_` 管理資源時才要手動釋放                         |


*/
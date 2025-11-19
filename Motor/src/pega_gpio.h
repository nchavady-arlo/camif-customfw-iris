#ifndef _PEGA_GPIO_H_
#define _PEGA_GPIO_H_

#include <linux/of_gpio.h>         // for of_get_named_gpio(), of_gpio_*()
#include <linux/platform_device.h>

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

int Motor_gpio_init(struct platform_device *pdev);


#endif /* _PEGA_GPIO_H_ */
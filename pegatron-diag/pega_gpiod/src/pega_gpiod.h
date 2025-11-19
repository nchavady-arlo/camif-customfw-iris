#ifndef _PEGA_GPIOD_H_
#define _PEGA_GPIOD_H_
//==============================================================================
#include "pega_gpiopad.h"
#include "pega_defines.h"
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#define GPIO_DIR_INPUT			0
#define GPIO_DIR_OUTPUT			1
#define GPIO_DIR_INT			2
//==============================================================================
//for SPI interrupt
#define IO_INT_IW610_SPI_IRQ				PAD_PM_GPIO0
//for ACC interrupt
#define IO_INT_ACC_IRQ						PAD_PM_GPIO6
//for ALS interrupt
#define IO_INT_ALS_IRQ						PAD_PM_GPIO1
//for NFC interrupt
#define IO_INT_NFC_IRQ						PAD_PM_GPIO3
//==============================================================================
//for IO Sync button
#define IO_I_SYNC_BUTTON					PAD_PM_GPIO4
//for MOTOR
#define IO_I_MOTOR_nFAULT 					PAD_PM_UART2_TX
#define IO_I_PAN_MOTOR_ERR 					PAD_PM_UART2_RX
#define IO_I_RISING_MOTOR_ERR 				PAD_OUTN_RX0_CH4
//==============================================================================
//for NXP wifi moduler
#define IO_O_IW610F_PDn						PAD_GPIO6 //(0:disable , 1:enable )
#define IO_O_IW610F_RST_WL					PAD_SD0_CDZ
#define IO_O_IW610F_RST_BLE					PAD_PM_PSPI0_INT
//==============================================================================
#define IO_O_IW610G_PDn						PAD_GPIO7 //(0:disable , 1:enable )
#define IO_O_IW610G_RST_WL					PAD_GPIO10
#define IO_O_IW610G_RST_Thread				PAD_GPIO11
//==============================================================================
//for TI amp
#define IO_O_AUDIO_SHUTDOWN					PAD_GPIO8 //(0:shutdown , 1:active) //AUDIO_AMP_SDZ
//for motor
#define IO_O_RISING_MOTOR_nSLEEP			PAD_GPIO9
#define IO_O_PAN_MOTOR_nSLEEP				PAD_PM_GPIO11
#define IO_O_MOTOR_CTR0						PAD_PM_PWM0
#define IO_O_MOTOR_CTR1						PAD_PM_PWM1
#define IO_O_MOTOR_CTR2						PAD_PM_GPIO7
#define IO_O_MOTOR_CTR3						PAD_PM_GPIO8
//==============================================================================
#define IO_O_IR_CUT_IN1						PAD_OUTP_RX0_CH0
#define IO_O_IR_CUT_IN2						PAD_OUTN_RX0_CH0
//==============================================================================
#define IO_O_CAM_1V8_EN						PAD_PM_GPIO5
//==============================================================================
#define IO_O_LED_R							PAD_PM_FUART_RX
#define IO_O_LED_G							PAD_PM_FUART_TX
#define IO_O_LED_B							PAD_OUTP_RX0_CH4
//==============================================================================
#define IO_O_IR_LED_EN						PAD_PM_GPIO2
#define IO_O_Spotlight_LED_EN				PAD_OUTP_RX0_CH5 
//==============================================================================
#define IO_O_PWM_IR_Spotlight_LED			PAD_OUTN_RX0_CH5 //PWM7
//============================================================================== 
#define IO_IRLED_CTRL_ON()					pga_gpio_control(IO_O_IR_LED_EN,	 GPIO_HIGH) 
#define IO_IRLED_CTRL_OFF()					pga_gpio_control(IO_O_IR_LED_EN,	 GPIO_LOW)
//============================================================================== 
#define IO_Spotlight_CTRL_ON()				pga_gpio_control(IO_O_Spotlight_LED_EN,	 GPIO_HIGH) 
#define IO_Spotlight_CTRL_OFF()				pga_gpio_control(IO_O_Spotlight_LED_EN,	 GPIO_LOW)
//==============================================================================
#define IO_LED_R_CTRL_ON()					pga_gpio_control(IO_O_LED_R,	 GPIO_HIGH)
#define IO_LED_R_CTRL_OFF()					pga_gpio_control(IO_O_LED_R, 	 GPIO_LOW)
#define IO_LED_G_CTRL_ON()					pga_gpio_control(IO_O_LED_G,	 GPIO_HIGH)
#define IO_LED_G_CTRL_OFF()					pga_gpio_control(IO_O_LED_G, 	 GPIO_LOW)
#define IO_LED_B_CTRL_ON()					pga_gpio_control(IO_O_LED_B,	 GPIO_HIGH)
#define IO_LED_B_CTRL_OFF()					pga_gpio_control(IO_O_LED_B, 	 GPIO_LOW)
//==============================================================================
#define IO_IR_CUT1_CTRL_ON()				pga_gpio_control(IO_O_IR_CUT_IN1,	 GPIO_LOW) //11/29 Chris modify for correcting icr behavior 
#define IO_IR_CUT1_CTRL_OFF()				pga_gpio_control(IO_O_IR_CUT_IN1, 	 GPIO_HIGH)
#define IO_IR_CUT2_CTRL_ON()				pga_gpio_control(IO_O_IR_CUT_IN2, 	 GPIO_LOW)
#define IO_IR_CUT2_CTRL_OFF()				pga_gpio_control(IO_O_IR_CUT_IN2, 	 GPIO_HIGH)
//==============================================================================
#define IO_AMP_SHUTDOWN_ON()				pga_gpio_control(IO_O_AUDIO_SHUTDOWN,	 	GPIO_LOW)
#define IO_AMP_SHUTDOWN_OFF()				pga_gpio_control(IO_O_AUDIO_SHUTDOWN, 	 	GPIO_HIGH)
//==============================================================================
#define IO_AWD8833C_PAN_MOTOR_ON()			pga_gpio_control(IO_O_PAN_MOTOR_nSLEEP,	 	GPIO_HIGH)
#define IO_AWD8833C_PAN_MOTOR_OFF()			pga_gpio_control(IO_O_PAN_MOTOR_nSLEEP,	 	GPIO_LOW)
//==============================================================================
#define IO_AWD8833C_RISING_MOTOR_ON()		pga_gpio_control(IO_O_RISING_MOTOR_nSLEEP,	GPIO_HIGH)
#define IO_AWD8833C_RISING_MOTOR_OFF()		pga_gpio_control(IO_O_RISING_MOTOR_nSLEEP,	GPIO_LOW)
//==============================================================================
typedef struct 
{
    const uint8_t pad;
    const char *pad_desc; //consumer
    //struct gpiod_line *line;
    const uint8_t direction; //in or out
    uint8_t value;	 //out value
} stGpio_cfg_t;
//==============================================================================
int  pga_gpio_chip_init(void);
int  pga_gpio_init(void);
//==============================================================================
void pga_gpio_control(const uint8_t pad, const uint8_t value);
void pga_gpio_ircut_control(uint8_t bFlag);
//==============================================================================
struct gpiod_chip* pga_gpio_chip_get(void);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_GPIO_H_

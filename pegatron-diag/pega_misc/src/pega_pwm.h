#ifndef _PEGA_PWM_H_
#define _PEGA_PWM_H_

#include "pega_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//20kHz to 200kHz input PWM frequency can be supported to avoid audible noise
//TPS92201 20kHz ~ 200kHz
#define PWM_IR_LED_FREQ			20000
#define PWM_IR_LED_DUTY			50
//==============================================================================
#define PWM_SPOT_LED_FREQ		30000
#define PWM_SPOT_LED_DUTY		50
//==============================================================================
#define PWM_LED_R_FREQ			20000
#define PWM_LED_R_DUTY			50
#define PWM_LED_G_FREQ			20000
#define PWM_LED_G_DUTY			50
#define PWM_LED_B_FREQ			20000
#define PWM_LED_B_DUTY			50
//==============================================================================
typedef enum 
{
	PWM_CH0 = 0,
	PWM_CH1,
	PWM_CH2,
	PWM_CH3, //PAD_PM_GPIO3
	PWM_CH4, //PAD_PM_GPIO4 
	PWM_CH5, //PAD_PM_GPIO5
	PWM_CH6, //PAD_PM_GPIO6
	PWM_CH7, //PAD_PM_GPIO7
	PWM_CH8,
	PWM_CH9,
	PWM_CH10,
	PWM_CH11,
	PWM_CH_MAX,
}PwmEnumType;
//==============================================================================
void Pega_pwm_init(unsigned int bIsOn);
int  Pega_pwm_control(unsigned int pwm, int bEnable);
int  Pega_pwm_config(unsigned int pwm, unsigned int period, unsigned int duty_cycle);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_PWM_H_
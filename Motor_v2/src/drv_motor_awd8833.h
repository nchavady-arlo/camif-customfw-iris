#ifndef _AWD8833C_H_
#define _AWD8833C_H_

//==============================================================================  
#include <linux/types.h>
#include <linux/delay.h>

#include "drv_motor_gpio.h"

/** @defgroup GPIO_pins_define  GPIO pins define
  * @{
  */
//==============================================================================  
/** @defgroup full step and half step mode delay, uint us.
  * @{
  */
#define AW8646_DRIVER_VERSION			"v0.0.0.5"
//==============================================================================
/* Num of Timer : 12*/
/* Timer0 ~ 3	(unit) 61 us */
/* Timer4 ~ 11	(unit) 1  ms */
#define MOTOR_TIMER				3
/* Timer unit (61 us) */
#define SPEED_OPTION_NUM		4
#define PAN_SPEED_1				34	//2.074 ms		//warning 
#define PAN_SPEED_2				30	//1.830 ms
#define PAN_SPEED_3				20	//1.220 ms
#define PAN_SPEED_4				17	//1.037 ms		//max
#define RISING_SPEED_1			60	//3.660 ms
#define RISING_SPEED_2			40	//2.440 ms
#define RISING_SPEED_3			30	//1.830 ms
#define RISING_SPEED_4			19	//1.159 ms		//max

typedef enum aw_motor
{
	AW_PAN_MOTOR = 0,
	AW_RISING_MOTOR = 1,
} aw_motor;

typedef enum aw_direction {
	AW_REVERSE = 0,
	AW_FORWARD = 1,
} aw_direction;

typedef enum aw_step_select {
	AW_FULL_STEP = 0,
	AW_HALF_STEP = 1,
} aw_step_select;

typedef enum aw_full_step_state {
	AW_FULL_STEP_1 = 0,
	AW_FULL_STEP_2 = 1,
	AW_FULL_STEP_3 = 2,
	AW_FULL_STEP_4 = 3,
} aw_full_step_state;

typedef enum aw_half_step_state {
	AW_HALF_STEP_1 = 0,
	AW_HALF_STEP_2 = 1,
	AW_HALF_STEP_3 = 2,
	AW_HALF_STEP_4 = 3,
	AW_HALF_STEP_5 = 4,
	AW_HALF_STEP_6 = 5,
	AW_HALF_STEP_7 = 6,
	AW_HALF_STEP_8 = 7,
} aw_half_step_state;

typedef enum aw_gpio_state {
	AW_GPIO_LOW = 0,
	AW_GPIO_HIGH = 1,
} aw_gpio_state;

typedef enum aw_status {
	AW_IDLE = 0,
	AW_BUSY = 1,
} aw_bool;

//==============================================================================
void drv_motor_sel(u8 en_select, u8 bIsRisingMotor);
void drv_motor_stop(void);
void drv_motor_restore_IOstatus(int motor);
void drv_motor_fullstep_forward(void);
void drv_motor_fullstep_reverse(void);
void drv_motor_halfstep_forward(void);
void drv_motor_halfstep_reverse(void);

//==============================================================================
#endif

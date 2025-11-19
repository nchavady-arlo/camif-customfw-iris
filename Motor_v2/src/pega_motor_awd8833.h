#ifndef _AWD8833C_H_
#define _AWD8833C_H_

//==============================================================================  
#include <linux/types.h>
#include <linux/delay.h>

#include "pega_gpio.h"

/** @defgroup GPIO_pins_define  GPIO pins define
  * @{
  */
//==============================================================================  
/** @defgroup full step and half step mode delay, uint us.
  * @{
  */
#define AW8646_DRIVER_VERSION			"v0.0.0.3"


typedef enum aw_motor
{
	AW_PAN_MOTOR = 0,
	AW_RISING_MOTOR = 1,
} aw_motor;

typedef enum aw_direction {
	AW_REVERSE = 0,
	AW_FORWARD = 1,
} aw_direction;

typedef enum aw_mode {
	AW_STEP_MODE = 0,
	AW_DEGREE_MODE = 1,
	AW_TIMER_MODE = 2,
} aw_mode;

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

/*
typedef enum aw_motor_speed {
	AW_SPEED_LOW = 0,
	AW_SPEED_MID = 1,
	AW_SPEED_HIGH = 2,
	AW_SPEED_VREY_HIGH 	= 3,
} aw_motor_speed;
*/

typedef enum aw_gpio_state {
	AW_GPIO_LOW = 0,
	AW_GPIO_HIGH = 1,
} aw_gpio_state;

typedef enum aw_bool {
	AW_FALSE = 0,
	AW_TRUE = 1,
} aw_bool;

//==============================================================================
void awd8833c_gpio_init(void);
void awd8833c_set_enable(u8 en_select, u8 bIsRisingMotor);
void awd8833c_step_function(u8 direction, u8 mode_select, u32 u32step);
void awd8833c_timer_function(u8 direction, u8 mode_select);
void awd8833c_stop(void);
void awd8833c_restore_state(int motor);
u32 awd8833c_set_motor_speed(u8 speed_level);
u32 awd8833c_degree2step(u8 Motor, u32 degree);
void awd8833c_set_rotate_speed(u32 angle_s, u8 step_angle);
void awd8833c_full_step_forward_function(void);
void awd8833c_full_step_reverse_function(void);
void awd8833c_half_step_forward_function(void);
void awd8833c_half_step_reverse_function(void);

//==============================================================================
#endif

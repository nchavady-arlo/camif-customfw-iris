#ifndef _AWD8833C_H_
#define _AWD8833C_H_

#include <stdint.h>
//==============================================================================  
#include "pega_gpio.h"

/** @defgroup GPIO_pins_define  GPIO pins define
  * @{
  */
//==============================================================================  
/** @defgroup full step and half step mode delay, uint us.
  * @{
  */
#define AW_FULL_STEP_DELAY				5000 // 20000
#define AW_HALF_STEP_DELAY				AW_FULL_STEP_DELAY / 2
#define AW_TEST_CYCLE_COUNT				5

typedef enum aw_motor
{
	AW_PAN_MOTOR = 0,
	AW_RISING_MOTOR = 1,
} aw_motor;

typedef enum aw_direction 
{
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

typedef enum aw_motor_speed 
{
	AW_SPEED_LOW 		= 1,
	AW_SPEED_MID,
	AW_SPEED_HIGH,
	AW_SPEED_VREY_HIGH,
} aw_motor_speed;

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
void awd8833c_set_enable(uint8_t en_select, uint8_t bIsRisingMotor);
void awd8833c_step_function_action(uint8_t direction, uint8_t mode_select, uint32_t cycle_count);
void awd8833c_second_function_action(uint8_t direction, uint8_t mode_select);
void awd8833c_stop(void);
void awd8833c_set_speed_level(uint8_t speed_level);
void awd8833c_set_rotate_speed(uint32_t angle_s, uint8_t step_angle);
//==============================================================================
void awd8833c_pan_rotate_degrees(uint8_t direction, float degree); 
void awd8833c_rising_rotate_degrees(uint8_t direction, float degree) ;
//==============================================================================
void awd8833c_test(uint8_t bIsRisingMotor);
//==============================================================================
void awd8833c_motor_launch_by_time(uint8_t bIsRisingMotor, uint8_t direction, uint8_t mode_select, uint8_t speed, uint32_t u32msecs);
//==============================================================================
int  awd8833c_timer_init(void);
int  awd8833c_timer_start(uint32_t u32msecs);
void awd8833c_timer_handler(void);
//==============================================================================
void awd8833c_Data_Print(void);
//==============================================================================
void awd8833c_debug(int eCmdSetId, int value1, int value2);
//==============================================================================
#endif

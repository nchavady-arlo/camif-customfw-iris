/*
 * File: pega_motor_awd8833.c
 *
 * Author: Leo<wangzhi@awinic.com>
 *
 * Copyright (c) 2023 AWINIC Technology CO., LTD
 *
 */
//============================================================================== 
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <pthread.h> 
//============================================================================== 
#include "pega_gpio.h"
//============================================================================== 
#include "pega_motor_awd8833.h"
//==============================================================================
/*
   // I_AOUT / I_BOUT:  AIN1/2, BIN1/2
   // I_AOUT forward:   AIN1=1, AIN2=0
   // I_AOUT reverse:   AIN1=0, AIN2=1
   // I_AOUT brake:     AIN1=1, AIN2=1
   // I_AOUT coast:     AIN1=0, AIN2=0
   // I_BOUT forward:   BIN1=1, BIN2=0
   // I_BOUT reverse:   BIN1=0, BIN2=1
   // I_BOUT brake:     BIN1=1, BIN2=1
   // I_BOUT coast:     BIN1=0, BIN2=0
*/
//==============================================================================
static uint8_t  g_full_step_state = AW_FULL_STEP_1;
static uint8_t  g_half_step_state = AW_HALF_STEP_1;
static uint32_t g_full_step_delay = AW_FULL_STEP_DELAY;
static uint32_t g_half_step_delay = AW_HALF_STEP_DELAY;
//==============================================================================
#define AW8646_DRIVER_VERSION			"v0.0.0.2"
/**
  * @brief Delay functions.
  * @note Please implement it with your own platform.
  * @param us specifies the delay time length, in us.
  * @retval None
  */
static void awd8833c_delay_us(uint32_t us)
{
	//HAL_Delay_us(us);
	usleep(us);
}

/**
  * @brief Delay functions.
  * @note Please implement it with your own platform.
  * @param us specifies the delay time length, in ms.
  * @retval None
  */
static void awd8833c_delay_ms(uint32_t ms)
{
	//HAL_Delay(ms);
	usleep(ms*1000);
}

/**
  * @brief GPIO setup functions.
  * @note Please implement it with your own platform, flexible selection of parameters.
  * @param GPIOx  specifies the GPIO port in stm32, Please choose whether to use it or not according to your own platform.
  * @param GPIO_Pin  specifies the pin.
  * @param state  specifies the pin state.
  * @retval None
  */


void awd8833c_gpio_init(void)
{
	 Pega_Gpio_init();
	 
	//awd8833c_set_gpio(AWD8833C_AIN1_GPIO_PORT, AWD8833C_AIN1_PIN, AW_GPIO_LOW);
	//awd8833c_set_gpio(AWD8833C_AIN2_GPIO_PORT, AWD8833C_AIN2_PIN, AW_GPIO_LOW);
	//awd8833c_set_gpio(AWD8833C_BIN1_GPIO_PORT, AWD8833C_BIN1_PIN, AW_GPIO_LOW);
	//awd8833c_set_gpio(AWD8833C_BIN2_GPIO_PORT, AWD8833C_BIN2_PIN, AW_GPIO_LOW);
	//awd8833c_set_gpio(AWD8833C_NSLEEP_GPIO_PORT, AWD8833C_NSLEEP_PIN, AW_GPIO_LOW);
}

void awd8833c_set_enable(uint8_t en_select, uint8_t bIsRisingMotor)
{
	if (en_select) 
	{
		(bIsRisingMotor > 0) ? IO_AWD8833C_RISING_MOTOR_ON() : IO_AWD8833C_PAN_MOTOR_ON();	
	} 
	else 
	{
		(bIsRisingMotor > 0) ? IO_AWD8833C_RISING_MOTOR_OFF() : IO_AWD8833C_PAN_MOTOR_OFF();
	}
}
/**
  * @brief Motor full step forward drive.
  * @param None
  * @retval None
  */
static void awd8833c_full_step_forward_function(void)
{
	switch (g_full_step_state) 
	{
	case AW_FULL_STEP_1:
		 //awd8833c_set_gpio(AWD8833C_AIN1_GPIO_PORT, AWD8833C_AIN1_PIN, AW_GPIO_HIGH);
		 //awd8833c_set_gpio(AWD8833C_AIN2_GPIO_PORT, AWD8833C_AIN2_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_AIN1_PIN_ON();
		 IO_AWD8833C_AIN2_PIN_OFF();
		 g_full_step_state = AW_FULL_STEP_2;
		 break;
	case AW_FULL_STEP_2:
		 //awd8833c_set_gpio(AWD8833C_BIN2_GPIO_PORT, AWD8833C_BIN2_PIN, AW_GPIO_HIGH);
		 //awd8833c_set_gpio(AWD8833C_BIN1_GPIO_PORT, AWD8833C_BIN1_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_BIN2_PIN_ON();
		 IO_AWD8833C_BIN1_PIN_OFF();
		 g_full_step_state = AW_FULL_STEP_3;
		 break;
		 
	case AW_FULL_STEP_3:
		 //awd8833c_set_gpio(AWD8833C_AIN2_GPIO_PORT, AWD8833C_AIN2_PIN, AW_GPIO_HIGH);
		 //awd8833c_set_gpio(AWD8833C_AIN1_GPIO_PORT, AWD8833C_AIN1_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_AIN2_PIN_ON();
		 IO_AWD8833C_AIN1_PIN_OFF();
		 g_full_step_state = AW_FULL_STEP_4;
		 break;
		 
	case AW_FULL_STEP_4:
		 //awd8833c_set_gpio(AWD8833C_BIN1_GPIO_PORT, AWD8833C_BIN1_PIN, AW_GPIO_HIGH);
		 //awd8833c_set_gpio(AWD8833C_BIN2_GPIO_PORT, AWD8833C_BIN2_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_BIN1_PIN_ON();
		 IO_AWD8833C_BIN2_PIN_OFF();
		 g_full_step_state = AW_FULL_STEP_1;
		 break;
		 
	default:
		 break;
	}
	awd8833c_delay_us(g_full_step_delay);
}

/**
  * @brief Motor full step reverse drive.
  * @param None
  * @retval None
  */
static void awd8833c_full_step_reverse_function(void)
{
	switch (g_full_step_state)
	{
	case AW_FULL_STEP_1:
		 //awd8833c_set_gpio(AWD8833C_BIN1_GPIO_PORT, AWD8833C_BIN1_PIN, AW_GPIO_LOW);
		 //awd8833c_set_gpio(AWD8833C_BIN2_GPIO_PORT, AWD8833C_BIN2_PIN, AW_GPIO_HIGH);//b
		 IO_AWD8833C_BIN1_PIN_OFF();
		 IO_AWD8833C_BIN2_PIN_ON();
		 g_full_step_state = AW_FULL_STEP_2;
		 break;
		 
	case AW_FULL_STEP_2:
		 //awd8833c_set_gpio(AWD8833C_AIN2_GPIO_PORT, AWD8833C_AIN2_PIN, AW_GPIO_LOW);
		 //awd8833c_set_gpio(AWD8833C_AIN1_GPIO_PORT, AWD8833C_AIN1_PIN, AW_GPIO_HIGH);//a
		 IO_AWD8833C_AIN2_PIN_OFF();
		 IO_AWD8833C_AIN1_PIN_ON();
		 g_full_step_state = AW_FULL_STEP_3;
		 break;
		 
	case AW_FULL_STEP_3:
		 //awd8833c_set_gpio(AWD8833C_BIN2_GPIO_PORT, AWD8833C_BIN2_PIN, AW_GPIO_LOW);
		 //awd8833c_set_gpio(AWD8833C_BIN1_GPIO_PORT, AWD8833C_BIN1_PIN, AW_GPIO_HIGH);//d
		 IO_AWD8833C_BIN2_PIN_OFF();
		 IO_AWD8833C_BIN1_PIN_ON();
		 g_full_step_state = AW_FULL_STEP_4;
		 break;
		 
	case AW_FULL_STEP_4:
		 //awd8833c_set_gpio(AWD8833C_AIN1_GPIO_PORT, AWD8833C_AIN1_PIN, AW_GPIO_LOW);
		 //awd8833c_set_gpio(AWD8833C_AIN2_GPIO_PORT, AWD8833C_AIN2_PIN, AW_GPIO_HIGH);//c
		 IO_AWD8833C_AIN1_PIN_OFF();
		 IO_AWD8833C_AIN2_PIN_ON();
		 g_full_step_state = AW_FULL_STEP_1;
		 break;
		 
	default:
		break;
	}
	
	awd8833c_delay_us(g_full_step_delay);
}

/**
  * @brief Motor half step forward drive.
  * @param None
  * @retval None
  */
static void awd8833c_half_step_forward_function(void)
{
	//awd8833c_delay_us(g_full_step_delay);
	switch (g_half_step_state) 
	{
	case AW_HALF_STEP_1:
		 //awd8833c_set_gpio(AWD8833C_AIN1_GPIO_PORT, AWD8833C_AIN1_PIN, AW_GPIO_HIGH);
		 IO_AWD8833C_AIN1_PIN_ON();
		 g_half_step_state = AW_HALF_STEP_2;
		 break;
		 
	case AW_HALF_STEP_2:
		 //awd8833c_set_gpio(AWD8833C_AIN2_GPIO_PORT, AWD8833C_AIN2_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_AIN2_PIN_OFF();
		 g_half_step_state = AW_HALF_STEP_3;
		 break;
		 
	case AW_HALF_STEP_3:
		 //awd8833c_set_gpio(AWD8833C_BIN2_GPIO_PORT, AWD8833C_BIN2_PIN, AW_GPIO_HIGH);
		 IO_AWD8833C_BIN2_PIN_ON();
		 g_half_step_state = AW_HALF_STEP_4;
		 break;
		 
	case AW_HALF_STEP_4:
		 //awd8833c_set_gpio(AWD8833C_BIN1_GPIO_PORT, AWD8833C_BIN1_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_BIN1_PIN_OFF();
		 g_half_step_state = AW_HALF_STEP_5;
		 break;
		 
	case AW_HALF_STEP_5:
		 //awd8833c_set_gpio(AWD8833C_AIN2_GPIO_PORT, AWD8833C_AIN2_PIN, AW_GPIO_HIGH);
		 IO_AWD8833C_AIN2_PIN_ON();
		 g_half_step_state = AW_HALF_STEP_6;
		 break;
		
	case AW_HALF_STEP_6:
		 //awd8833c_set_gpio(AWD8833C_AIN1_GPIO_PORT, AWD8833C_AIN1_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_AIN1_PIN_OFF();
		 g_half_step_state = AW_HALF_STEP_7;
		 break;
		 
	case AW_HALF_STEP_7:
		 //awd8833c_set_gpio(AWD8833C_BIN1_GPIO_PORT, AWD8833C_BIN1_PIN, AW_GPIO_HIGH);
		 IO_AWD8833C_BIN1_PIN_ON();
		 g_half_step_state = AW_HALF_STEP_8;
		 break;
		 
	case AW_HALF_STEP_8:
		 //awd8833c_set_gpio(AWD8833C_BIN2_GPIO_PORT, AWD8833C_BIN2_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_BIN2_PIN_OFF();
		 g_half_step_state = AW_HALF_STEP_1;
		 break;
		 
	default:
		 break;
	}
	
	awd8833c_delay_us(g_half_step_delay);
}

/**
  * @brief Motor half step reverse drive.
  * @param None
  * @retval None
  */
static void awd8833c_half_step_reverse_function(void)
{
	switch (g_half_step_state) 
	{
	case AW_HALF_STEP_1:
		 //awd8833c_set_gpio(AWD8833C_BIN2_GPIO_PORT, AWD8833C_BIN2_PIN, AW_GPIO_HIGH);
		 IO_AWD8833C_BIN2_PIN_ON();
		 g_half_step_state = AW_HALF_STEP_2;
		 break;
		 
	case AW_HALF_STEP_2:
		 //awd8833c_set_gpio(AWD8833C_BIN1_GPIO_PORT, AWD8833C_BIN1_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_BIN1_PIN_OFF();
		 g_half_step_state = AW_HALF_STEP_3;
		 break;
		 
	case AW_HALF_STEP_3:
		 //awd8833c_set_gpio(AWD8833C_AIN1_GPIO_PORT, AWD8833C_AIN1_PIN, AW_GPIO_HIGH);
		 IO_AWD8833C_AIN1_PIN_ON();
		 g_half_step_state = AW_HALF_STEP_4;
		 break;
		 
	case AW_HALF_STEP_4:
		 //awd8833c_set_gpio(AWD8833C_AIN2_GPIO_PORT, AWD8833C_AIN2_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_AIN2_PIN_OFF();
		 g_half_step_state = AW_HALF_STEP_5;
		 break;
		 
	case AW_HALF_STEP_5:
		 //awd8833c_set_gpio(AWD8833C_BIN1_GPIO_PORT, AWD8833C_BIN1_PIN, AW_GPIO_HIGH);
		 IO_AWD8833C_BIN1_PIN_ON();
		 g_half_step_state = AW_HALF_STEP_6;
		 break;
		 
	case AW_HALF_STEP_6:
		 //awd8833c_set_gpio(AWD8833C_BIN2_GPIO_PORT, AWD8833C_BIN2_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_BIN2_PIN_OFF();
		 g_half_step_state = AW_HALF_STEP_7;
		 break;
		 
	case AW_HALF_STEP_7:
		 //awd8833c_set_gpio(AWD8833C_AIN2_GPIO_PORT, AWD8833C_AIN2_PIN, AW_GPIO_HIGH);
		 IO_AWD8833C_AIN2_PIN_ON();
		 g_half_step_state = AW_HALF_STEP_8;
		 break;
		 
	case AW_HALF_STEP_8:
		 //awd8833c_set_gpio(AWD8833C_AIN1_GPIO_PORT, AWD8833C_AIN1_PIN, AW_GPIO_LOW);
		 IO_AWD8833C_AIN1_PIN_OFF();
		 g_half_step_state = AW_HALF_STEP_1;
		 break;
		 
	default:
		 break;
	}
	awd8833c_delay_us(g_half_step_delay);
}

/**
  * @brief Test functions.
  * @note This function is for testing purposes only.
  * @param direction  Motor direction set
  *          This parameter can be one of the following values:
  *            @arg AW_FORWARD: 1.
  *            @arg AW_REVERSE: 0.
  *        mode_select  Motor drive mode select
  *          This parameter can be one of the following values:
  *            @arg AW_HALF_STEP: 1.
  *            @arg AW_FULL_STEP: 0.
  *        cycle_count  Motor drive pluse count
  *            @example : AW_TEST_CYCLE_COUNT refer to 13 cycle of drive pluse.
  * @retval None
  */
void awd8833c_step_function_test(uint8_t direction, uint8_t mode_select, uint32_t cycle_count)
{
	uint32_t aw_count = 0;

	// motor drive mode select
	if (direction == AW_FORWARD) 
	{
		if (mode_select == AW_FULL_STEP) 
		{
			for (aw_count = 0; aw_count < cycle_count; aw_count++) 
			{
				awd8833c_full_step_forward_function();
			}
		} 
		else if (mode_select == AW_HALF_STEP) 
		{
			for (aw_count = 0; aw_count < cycle_count; aw_count++) 
			{
				awd8833c_half_step_forward_function();
			}
		}
	} 
	else if (direction == AW_REVERSE) 
	{
		if (mode_select == AW_FULL_STEP) 
		{
			for (aw_count = 0; aw_count < cycle_count; aw_count++) 
			{
				awd8833c_full_step_reverse_function();
			}
		} 
		else if (mode_select == AW_HALF_STEP) 
		{
			for (aw_count = 0; aw_count < cycle_count; aw_count++) 
			{
				awd8833c_half_step_reverse_function();
			}
		}
	}
}

/**
  * @brief Set speed level functions.
  * @param speed_level specifies the speed level
  *            This parameter can be one of the following values:
  *            @arg AW_SPEED_LOW: 0.
  *            @arg AW_SPEED_MID: 1.
  *            @arg AW_SPEED_HIGH: 2.
  * @retval None
  */
void awd8833c_set_speed_level(uint8_t speed_level)
{
 switch (speed_level) 
	{
	case AW_SPEED_LOW://45°/s
		 g_full_step_delay = 20000;
		 g_half_step_delay = 10000;
		 break;
		 
	case AW_SPEED_MID://90°/s
		 g_full_step_delay = 10000;
		 g_half_step_delay = 5000;
		 break;
		 
	case AW_SPEED_HIGH://180°/s
		 g_full_step_delay = 5000;
		 g_half_step_delay = 2500;
		 break;
		 
	default:
		 break;
	}
}

/**
  * @brief Set rotate speed by angles/s functions.
  * @param angle_s specifies the angles/s
  * @param step_angle specifies the motor angle per step(tenfold)
  * @retval None
  */
void awd8833c_set_rotate_speed(uint32_t angle_s, uint8_t step_angle)
{
	uint32_t n_step = angle_s * 10 / step_angle;
	uint32_t n_us = 1000000 / n_step;

	g_full_step_delay = n_us;
	g_half_step_delay = n_us / 2;
}

/**
  * @brief Test functions.
  * @note This function is for stop motor but not go to sleep state.
  * @retval None
  */
void awd8833c_stop(void)
{
	// gpio are configured to reset
	//awd8833c_set_gpio(AWD8833C_AIN1_GPIO_PORT, AWD8833C_AIN1_PIN, AW_GPIO_LOW);
	//awd8833c_set_gpio(AWD8833C_BIN1_GPIO_PORT, AWD8833C_BIN1_PIN, AW_GPIO_LOW);
	//awd8833c_set_gpio(AWD8833C_AIN2_GPIO_PORT, AWD8833C_AIN2_PIN, AW_GPIO_LOW);
	//awd8833c_set_gpio(AWD8833C_BIN2_GPIO_PORT, AWD8833C_BIN2_PIN, AW_GPIO_LOW);
	IO_AWD8833C_AIN1_PIN_OFF();
	IO_AWD8833C_AIN2_PIN_OFF();
	IO_AWD8833C_BIN1_PIN_OFF();
	IO_AWD8833C_BIN2_PIN_OFF();
}

static void awd8833c_rising_rotate_degrees(uint8_t direction, float degree) 
{
    if (degree < 0.0f) 
	{
        return;
    }
	//24steps for 360 degrees
	uint32_t steps = (uint32_t)(degree / 15.0f + 0.5f); 
    awd8833c_step_function_test(direction, AW_FULL_STEP, steps);
}

static void awd8833c_rising_rotate_steps(uint8_t direction, uint32_t steps) 
{
	if (steps < 0) 
	{
		return;
	}
    awd8833c_step_function_test(direction, AW_FULL_STEP, steps);
}

static void awd8833c_pan_rotate_degrees(uint8_t direction, float degree) 
{
    if (degree < 0.0f) 
	{
        return;
    }
	//96steps for 360 degrees
	uint32_t half_steps = (uint32_t)(degree / 3.75f + 0.5f); 
    awd8833c_step_function_test(direction, AW_HALF_STEP, half_steps);
}

static void awd8833c_pan_rotate_steps(uint8_t direction, uint32_t half_steps) 
{
	if (half_steps < 0) 
	{
		return;
	}
	
    awd8833c_step_function_test(direction, AW_HALF_STEP, half_steps);
}
/**
  * @brief Test functions.
  * @note This function is for testing purposes only.
  * @retval None
  */
void awd8833c_test(uint8_t bIsRisingMotor)
{
	printf("\n[%s]%d\n", __func__, bIsRisingMotor);  
	
	/* init gpio state */
	awd8833c_gpio_init();
	/* set no sleep */
	awd8833c_set_enable(AW_TRUE, bIsRisingMotor);
	/* set motor speed 180°/s default AW_SPEED_HIGH*/
	awd8833c_set_speed_level(AW_SPEED_HIGH);
	/* go forward by full_step mode 400 pulses */
	awd8833c_step_function_test(AW_FORWARD, AW_FULL_STEP, 400);
	/* set motor speed 360°/s step angle is 0.9° */
	awd8833c_set_rotate_speed(360, 9);
	/* go reverse by full_step mode 800 / 2 pulses*/
	awd8833c_step_function_test(AW_REVERSE, AW_HALF_STEP, 800);
	/* set go sleep */
	awd8833c_set_enable(AW_FALSE, bIsRisingMotor);
}

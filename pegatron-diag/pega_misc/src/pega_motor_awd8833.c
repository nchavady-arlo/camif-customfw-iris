/*
 * File: pega_motor_awd8833.c
 *
 * Author: Leo<wangzhi@awinic.com>
 *
 * Copyright (c) 2023 AWINIC Technology CO., LTD
 *
 */
//============================================================================== 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
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
static timer_t 	m_timer_id;
//==============================================================================
static uint8_t  m_isTimeout = 0;
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

static void awd8833c_set_motor_speed(uint8_t speed)
{		
	//if ((speed == 0) || (speed == 0)
	if (speed == 0)
	{
		speed = 1;
	}
    else if (speed > 10)
	{     
        speed = 10;
	}
    
	speed--;
    //g_full_step_delay = 1800 * (uint32_t)speed; 	
	g_full_step_delay = (uint32_t)18000 -  (uint32_t)1800 * (uint32_t)speed; 	
	g_half_step_delay = g_full_step_delay / 2;	
	
	#if 1
	printf("-----------------------\n");
	printf("speed=%d\n", speed);
	printf("-----------------------\n");
	printf("g_full_step_delay  = %d\n", g_full_step_delay);
	printf("g_half_step_delay  = %d\n", g_half_step_delay);
	printf("-----------------------\n");
	#endif
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
	 IO_AWD8833C_AIN1_PIN_OFF();
	 IO_AWD8833C_AIN2_PIN_OFF();
	 IO_AWD8833C_BIN1_PIN_OFF();
	 IO_AWD8833C_BIN2_PIN_OFF();
	 	 
	 IO_AWD8833C_PAN_MOTOR_OFF();
	 IO_AWD8833C_RISING_MOTOR_OFF();
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
void awd8833c_step_function_action(uint8_t direction, uint8_t mode_select, uint32_t cycle_count)
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

void awd8833c_second_function_action(uint8_t direction, uint8_t mode_select)
{	
	// motor drive mode select
	if (direction == AW_FORWARD) 
	{
		if (mode_select == AW_FULL_STEP) 
		{
			while(m_isTimeout == 0)
			{
				awd8833c_full_step_forward_function();
			}
		} 
		else if (mode_select == AW_HALF_STEP) 
		{
			while(m_isTimeout == 0)
			{
				awd8833c_half_step_forward_function();
			}
		}
	} 
	else if (direction == AW_REVERSE) 
	{
		if (mode_select == AW_FULL_STEP) 
		{
			while(m_isTimeout == 0)
			{
				awd8833c_full_step_reverse_function();
			}
		} 
		else if (mode_select == AW_HALF_STEP) 
		{
			while(m_isTimeout == 0)
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
//pega_misc_dbg motor 71 xx  
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
		
		case AW_SPEED_VREY_HIGH://225°/s
			 g_full_step_delay = 2500;
			 g_half_step_delay = 1250;
			 break;
			 
		default:
			 g_full_step_delay = 5000;
			 g_half_step_delay = 2500;
			 break;
	}
	
	#if 1
	printf("-----------------------\n");
	printf("speed_level  = %d\n", speed_level);
	printf("-----------------------\n");
	printf("g_full_step_delay  = %d\n", g_full_step_delay);
	printf("g_half_step_delay  = %d\n", g_half_step_delay);
	printf("-----------------------\n");
	#endif
}

/**
  * @brief Set rotate speed by angles/s functions.
  * @param angle_s specifies the angles/s
  * @param step_angle specifies the motor angle per step(tenfold)
  * @retval None
  */
//pega_misc_dbg motor 70 xx xx  
void awd8833c_set_rotate_speed(uint32_t angle_s, uint8_t step_angle)
{
	uint32_t n_step = angle_s * 10 / step_angle;
	uint32_t n_us = 1000000 / n_step;

	g_full_step_delay = n_us;
	g_half_step_delay = n_us / 2;
	
	#if 1
	printf("-----------------------\n");
	printf("angle_s=%d,step_angle=%d\n", angle_s, step_angle);
	printf("-----------------------\n");
	printf("g_full_step_delay  = %d\n", g_full_step_delay);
	printf("g_half_step_delay  = %d\n", g_half_step_delay);
	printf("-----------------------\n");
	#endif
}

/**
  * @brief Test functions.
  * @note This function is for stop motor but not go to sleep state.
  * @retval None
  */
void awd8833c_stop(void)
{
	// gpio are configured to reset	
	IO_AWD8833C_AIN1_PIN_OFF();
	IO_AWD8833C_AIN2_PIN_OFF();
	IO_AWD8833C_BIN1_PIN_OFF();
	IO_AWD8833C_BIN2_PIN_OFF();
}
//pega_misc_dbg motor 3 0/1 xx
void awd8833c_rising_rotate_degrees(uint8_t direction, float degree) 
{
	 uint32_t steps = 0;
	 
     if (degree < 0.0f) 
	 {
         return;
     }
	 
	 if (degree  > 360.0f) 
	 {
         degree = 360.0;
     }
	
	direction = (direction < AW_FORWARD) ? AW_REVERSE : AW_FORWARD;
	
	//24steps for 360 degrees
	steps = (uint32_t)(degree / 15.0f + 0.5f) * 42;
	
	printf("[%s] degree=%f, full_steps=%d \n", __func__, degree, steps); 
	
	/* init gpio state */
	awd8833c_gpio_init();
    /* set no sleep */
	awd8833c_set_enable(AW_TRUE, 1);
    /* set motor speed 180°/s default AW_SPEED_HIGH*/
	awd8833c_set_speed_level(AW_SPEED_HIGH);
    /* set motor speed 360°/s step angle is 0.9° */
	//awd8833c_set_rotate_speed(360, 9);
	
    awd8833c_step_function_action(direction, AW_FULL_STEP, steps);
	
	/* set go sleep */
	awd8833c_set_enable(AW_FALSE, 1);

}
//pega_misc_dbg motor 2 0/1 xx
void awd8833c_pan_rotate_degrees(uint8_t direction, float degree) 
{
	 uint32_t half_steps = 0;
     
	 if (degree < 0.0f) 
	 {
        return;
     }
	 
	 if (degree  > 360.0f) 
	 {
         degree = 360.0;
     }
	 
	direction = (direction < AW_FORWARD) ? AW_REVERSE : AW_FORWARD;
	//96steps for 360 degrees
	half_steps = (uint32_t)(degree / 3.75f + 0.5f) * 42;
	
	printf("[%s] degree=%f, half_steps=%d \n", __func__, degree, half_steps); 
	
	/* init gpio state */
	awd8833c_gpio_init();
    /* set no sleep */
	awd8833c_set_enable(AW_TRUE, 0);
    /* set motor speed 180°/s default AW_SPEED_HIGH*/
	awd8833c_set_speed_level(AW_SPEED_HIGH);
	/* set motor speed 360°/s step angle is 0.9° */
	//awd8833c_set_rotate_speed(360, 9);	
	
    awd8833c_step_function_action(direction, AW_HALF_STEP, half_steps);
	
	/* set go sleep */
	awd8833c_set_enable(AW_FALSE, 0);
}
/**
  * @brief Test functions.
  * @note This function is for testing purposes only.
  * @retval None
  */
//pega_misc_dbg motor 1 0/1
void awd8833c_test(uint8_t bIsRisingMotor)
{
	//printf("\n[%s]%d\n", __func__, bIsRisingMotor);  
	
	/* init gpio state */
	awd8833c_gpio_init();
	/* set no sleep */
	awd8833c_set_enable(AW_TRUE, bIsRisingMotor);
	/* set motor speed 180°/s default AW_SPEED_HIGH*/
	awd8833c_set_speed_level(AW_SPEED_HIGH);
	/* go forward by full_step mode 400 pulses */
	awd8833c_step_function_action(AW_FORWARD, AW_FULL_STEP, 400);
	/* set motor speed 360°/s step angle is 0.9° */
	awd8833c_set_rotate_speed(360, 9);
	/* go reverse by full_step mode 800 / 2 pulses*/
	awd8833c_step_function_action(AW_REVERSE, AW_HALF_STEP, 800);
	/* set go sleep */
	awd8833c_set_enable(AW_FALSE, bIsRisingMotor);
}

//AW_PAN_MOTOR:
//pega_misc_dbg motor 11 xx xx
//pega_misc_dbg motor 12 xx xx
//AW_RISING_MOTOR:
//pega_misc_dbg motor 13 xx xx
//pega_misc_dbg motor 14 xx xx
void awd8833c_motor_launch_by_time(uint8_t bIsRisingMotor, uint8_t direction, uint8_t mode_select, uint8_t speed, uint32_t u32msecs)
{	
	printf("[%s]%d,%d\n", __func__, u32msecs);  
	
	/* init gpio state */
	awd8833c_gpio_init();
	/* set no sleep */
	awd8833c_set_enable(AW_TRUE, bIsRisingMotor);
	/* set motor speed 360°/s step angle is 0.9° */	
	//awd8833c_set_motor_speed(speed);
	awd8833c_set_speed_level(speed);
	
	if (awd8833c_timer_start(u32msecs) > -1)
	{	        
	    awd8833c_second_function_action(direction, mode_select);
	}
	
	/* set go sleep */
	awd8833c_set_enable(AW_FALSE, bIsRisingMotor);	
}
//==============================================================================
//pega_misc_dbg motor 81
int awd8833c_timer_init(void)
{
	 int rtn = 0;
	 
	 struct sigevent sev;     
     struct sigaction sa;

     // Set up signal handler
     sa.sa_flags = SA_SIGINFO;
     sa.sa_sigaction = awd8833c_timer_handler;
     sigemptyset(&sa.sa_mask);
     
	 if (sigaction(SIGRTMIN, &sa, NULL) == -1) 
	 {
		 rtn = -1; 
         perror("sigaction");
         exit(EXIT_FAILURE);
     }

     // Create the timer
     sev.sigev_notify = SIGEV_SIGNAL;
     sev.sigev_signo = SIGRTMIN;
     sev.sigev_value.sival_ptr = &m_timer_id;
     
	 if (timer_create(CLOCK_REALTIME, &sev, &m_timer_id) == -1) 
	 {
		 rtn = -1; 
         perror("timer_create");
         exit(EXIT_FAILURE);
     }

     return rtn;	 
}
//pega_misc_dbg motor 82 xx
int awd8833c_timer_start(uint32_t u32msecs)
{	 
     int rtn = 0;
	 
	 struct itimerspec its;
	 long long freq_nanosecs = 0;
	 // Configure timer expiration (initial and interval)
	 
	 printf("[%s]u32msecs=%d\n", __func__, u32msecs); 
	 	 
	 if ((u32msecs == 0) || (awd8833c_timer_init() < 0))
	 {
		 return -1;
	 }
	
     //start the timer
     freq_nanosecs = u32msecs * 1000000;
     its.it_interval.tv_sec = u32msecs / 1000;
     its.it_interval.tv_nsec = freq_nanosecs % 1000000000;
     its.it_value.tv_sec = its.it_interval.tv_sec;
     its.it_value.tv_nsec = its.it_interval.tv_nsec;
	 
	 // Start the timer
	 m_isTimeout = 0;
		
	 if (timer_settime(m_timer_id, 0, &its, NULL) == -1) 
	 {
		 rtn = -1;
		 perror("timer_settime");
		 exit(EXIT_FAILURE);
	 }
	 
	 return rtn;
}

void awd8833c_timer_handler(void)
{	 
	 timer_delete(m_timer_id);	 
	 m_isTimeout = 1;
	 printf("Timer expired(%d)!\n" , m_isTimeout);	 
}
//==============================================================================
//pega_misc_dbg motor 99
void awd8833c_Data_Print(void)
{	 
     printf("-----------------------\n");	 
     printf("m_timer_id   = %d\n", m_timer_id);	 
	 printf("m_isTimeout  = %d\n", m_isTimeout);	 
	 printf("-----------------------\n");
	 printf("g_full_step_state  = %d\n", g_full_step_state);
	 printf("g_half_step_state  = %d\n", g_half_step_state);
	 printf("-----------------------\n");
	 printf("g_full_step_delay  = %d\n", g_full_step_delay);
	 printf("g_half_step_delay  = %d\n", g_half_step_delay);
	 printf("-----------------------\n");	 
}
//pega_misc_dbg motor 98
void awd8833c_GPIO_Value_Print(void)
{	 
     int fd = -1, i=0;
     char path[64]={0};
	 char buf[3]={0};
	 int  gpio[4];
	 
	 for (i=0;i<4;i++)
	 {
		memset(path, 0, sizeof(path));
		memset(buf, 0, sizeof(buf));
		
		switch(i)
		{
			case 0:
			default:
				 sprintf(path,"/sys/class/gpio/gpio%d/value", IO_O_MOTOR_CTRL_PIN0);
				 break;
			case 1:
				 sprintf(path,"/sys/class/gpio/gpio%d/value", IO_O_MOTOR_CTRL_PIN1);
				 break;
			case 2:
				 sprintf(path,"/sys/class/gpio/gpio%d/value", IO_O_MOTOR_CTRL_PIN2);
				 break;
			case 3:
				 sprintf(path,"/sys/class/gpio/gpio%d/value", IO_O_MOTOR_CTRL_PIN3);
				 break;	 
		}
	 
		if ((fd = open(path, O_RDONLY)) < 0) 
		{
			perror("Failed to open gpio value");     
		}
	 
		if (fd > 0 )
		{
			lseek(fd, 0, SEEK_SET);  
			read(fd, buf, sizeof(buf));
			//printf("Motor interrupt occurred(Rising), value: %c\n", buf[0]);
			gpio[i] = atoi(buf);
			close(fd);
		}
	 }
	 
	 printf("-----------------------\n");
	 printf("IO_O_MOTOR_CTRL_PIN0  = %d\n", gpio[0]);
	 printf("IO_O_MOTOR_CTRL_PIN1  = %d\n", gpio[1]);	 
	 printf("IO_O_MOTOR_CTRL_PIN2  = %d\n", gpio[2]);
	 printf("IO_O_MOTOR_CTRL_PIN3  = %d\n", gpio[3]);
	 printf("-----------------------\n");	 
}
//==============================================================================
//pega_misc_dbg motor xx xx xx
void awd8833c_debug(int eCmdSetId, int value1, int value2)
{
	 //printf("[%s]%d,%d,%d\n", __func__, eCmdSetId, value1, value2);  
	
	 switch(eCmdSetId)
	 {
		 case 1: //pega_misc_dbg motor 1 0/1
		        awd8833c_test(value1>0);
		        break;
		 //==============================================================================
		 case 2://pega_misc_dbg motor 2 0/1 xx
		        awd8833c_pan_rotate_degrees((uint8_t)value1, (float)value2);
		        break;
				
		 case 3://pega_misc_dbg motor 3 0/1 xx
		        awd8833c_rising_rotate_degrees((uint8_t)value1, (float)value2);
		        break;			
		 
		 //==============================================================================
		 case 11://pega_misc_dbg motor 11 xx xx
		        awd8833c_motor_launch_by_time(AW_PAN_MOTOR, AW_REVERSE, AW_HALF_STEP, (uint8_t)value1, (uint32_t)value2);
		        break;
		 case 12://pega_misc_dbg motor 12 xx xx
		        awd8833c_motor_launch_by_time(AW_PAN_MOTOR, AW_FORWARD, AW_HALF_STEP, (uint8_t)value1, (uint32_t)value2);
		        break;
		 case 13://pega_misc_dbg motor 13 xx xx
		        awd8833c_motor_launch_by_time(AW_RISING_MOTOR, AW_REVERSE, AW_FULL_STEP, (uint8_t)value1, (uint32_t)value2);
		        break;
		 case 14://pega_misc_dbg motor 14 xx xx
		        awd8833c_motor_launch_by_time(AW_RISING_MOTOR, AW_FORWARD, AW_FULL_STEP, (uint8_t)value1, (uint32_t)value2);
		        break;
		 //==============================================================================
		 case 15://pega_misc_dbg motor 15 xx xx
		        awd8833c_motor_launch_by_time(AW_PAN_MOTOR, AW_REVERSE, AW_FULL_STEP, (uint8_t)value1, (uint32_t)value2);
		        break;
		 case 16://pega_misc_dbg motor 16 xx xx
		        awd8833c_motor_launch_by_time(AW_PAN_MOTOR, AW_FORWARD, AW_FULL_STEP, (uint8_t)value1, (uint32_t)value2);
		        break;
		 case 17://pega_misc_dbg motor 17 xx xx
		        awd8833c_motor_launch_by_time(AW_RISING_MOTOR, AW_REVERSE, AW_HALF_STEP, (uint8_t)value1, (uint32_t)value2);
		        break;
		 case 18://pega_misc_dbg motor 18 xx xx
		        awd8833c_motor_launch_by_time(AW_RISING_MOTOR, AW_FORWARD, AW_HALF_STEP, (uint8_t)value1, (uint32_t)value2);
		        break;
		 //==============================================================================		
		 case 70://pega_misc_dbg motor 70 xx //(uint32_t angle_s, uint8_t step_angle)
		        awd8833c_set_rotate_speed((uint32_t)value1, (uint8_t)value2);
		        break;		
		 //==============================================================================		
		 case 82://pega_misc_dbg motor 82 xx
		        awd8833c_timer_start(value1);
		        break;		
		 //==============================================================================
		 case 98://pega_misc_dbg motor 98
		        awd8833c_GPIO_Value_Print();
		        break;
		 case 99://pega_misc_dbg motor 99
		        awd8833c_Data_Print();
				awd8833c_GPIO_Value_Print();
		        break;		
		 //============================================================================== 		
		 default:
                break;		 
	 }
}
//============================================================================== 

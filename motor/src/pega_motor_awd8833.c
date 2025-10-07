/*
 * File: pega_motor_awd8833.c
 *
 * Author: Leo<wangzhi@awinic.com>
 *
 * Copyright (c) 2023 AWINIC Technology CO., LTD
 *
 */
//============================================================================== 
#include <linux/ioctl.h>
//============================================================================== 
#include "pega_gpio.h"
//============================================================================== 
#include "pega_motor_awd8833.h"
// Marco 
#define USLEEP_MARGIN 1000  // 1ms margin (可視需要調整)
#define usleep(us) usleep_range((us) , (us) + USLEEP_MARGIN)



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
static u8  g_full_step_state = AW_FULL_STEP_1;
static u8  g_half_step_state = AW_HALF_STEP_1;
static u32 g_full_step_delay = AW_FULL_STEP_DELAY;
static u32 g_half_step_delay = AW_HALF_STEP_DELAY;
//==============================================================================
void awd8833c_gpio_init(void)
{	 
	IO_AWD8833C_AIN1_PIN_OFF();
	IO_AWD8833C_AIN2_PIN_OFF();
	IO_AWD8833C_BIN1_PIN_OFF();
	IO_AWD8833C_BIN2_PIN_OFF();
	 	 
	IO_AWD8833C_PAN_MOTOR_OFF();
	IO_AWD8833C_RISING_MOTOR_OFF();
}

/**
  * @brief Delay functions.
  * @note Please implement it with your own platform.
  * @param us specifies the delay time length, in us.
  * @retval None
  */
static void awd8833c_delay_us(u32 us)
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
/*

static void awd8833c_delay_ms(u32 ms)
{
	//HAL_Delay(ms);
	usleep(ms*1000);
}
*/

void awd8833c_set_enable(u8 en_select, u8 bIsRisingMotor)
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
  * @brief Set speed level functions.
  * @param speed_level specifies the speed level
  *            This parameter can be one of the following values:
  *            @arg AW_SPEED_LOW: 0.
  *            @arg AW_SPEED_MID: 1.
  *            @arg AW_SPEED_HIGH: 2.
  * @retval None
  */

//pega_misc_dbg motor 71 xx  
u32 awd8833c_set_motor_speed(uint8_t speed)
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
	
	return g_full_step_delay;
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
void awd8833c_step_function(u8 direction, u8 mode_select, u32 cycle_count)
{
	u32 aw_count = 0;

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
  * @brief Set rotate speed by angles/s functions.
  * @param angle_s specifies the angles/s
  * @param step_angle specifies the motor angle per step(tenfold)
  * @retval None
  */
void awd8833c_set_rotate_speed(u32 angle_s, u8 step_angle)
{
	u32 n_step = angle_s * 10 / step_angle;
	u32 n_us = 1000000 / n_step;

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
/*
static void awd8833c_rising_rotate_steps(u8 direction, u32 steps) 
{
	if (steps < 0) 
	{
		return;
	}
    awd8833c_step_function_test(direction, AW_FULL_STEP, steps);
}

static void awd8833c_pan_rotate_steps(u8 direction, u32 half_steps) 
{
	if (half_steps < 0) 
	{
		return;
	}
	
    awd8833c_step_function_test(direction, AW_HALF_STEP, half_steps);
}
*/

/**
  * @brief Test functions.
  * @note This function is for testing purposes only.
  * @retval None
  */
void awd8833c_test(u8 bIsRisingMotor)
{
	printk("\n[%s]%d\n", __func__, bIsRisingMotor);  
	
	/* init gpio state */
	awd8833c_gpio_init();
	/* set no sleep */
	printk("awd8833c_set_enable\n");  
	awd8833c_set_enable(AW_TRUE, bIsRisingMotor);
	/* set motor speed 180°/s default AW_SPEED_HIGH*/
	printk("awd8833c_set_motor_speed\n");  
	awd8833c_set_motor_speed(AW_SPEED_HIGH);
	/* go forward by full_step mode 400 pulses */
	printk("awd8833c_step_function_test\n");  
	awd8833c_step_function(AW_FORWARD, AW_FULL_STEP, 400);
	/* set motor speed 360°/s step angle is 0.9° */
	printk("awd8833c_set_rotate_speed\n");  
	awd8833c_set_rotate_speed(360, 9);
	/* go reverse by full_step mode 800 / 2 pulses*/
	printk("awd8833c_step_function_test\n");  
	awd8833c_step_function(AW_REVERSE, AW_HALF_STEP, 800);
	/* set go sleep */
	printk("awd8833c_set_enable\n");  
	awd8833c_set_enable(AW_FALSE, bIsRisingMotor);
}


void awd8833c_second_function_action(uint8_t direction, uint8_t mode_select)
{	
	// motor drive mode select
	if (direction == AW_FORWARD) 
	{
		if (mode_select == AW_FULL_STEP) 
		{
			while(1)//while(m_isTimeout == 0)
			{
				awd8833c_full_step_forward_function();
			}
		} 
		else if (mode_select == AW_HALF_STEP) 
		{
			while(1)//while(m_isTimeout == 0)
			{
				awd8833c_half_step_forward_function();
			}
		}
	} 
	else if (direction == AW_REVERSE) 
	{
		if (mode_select == AW_FULL_STEP) 
		{
			while(1)//while(m_isTimeout == 0)
			{
				awd8833c_full_step_reverse_function();
			}
		} 
		else if (mode_select == AW_HALF_STEP) 
		{
			while(1)//while(m_isTimeout == 0)
			{
				awd8833c_half_step_reverse_function();
			}
		}
	}
}
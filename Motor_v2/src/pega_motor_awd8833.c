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
#include <linux/timer.h>
#include <linux/jiffies.h>
//============================================================================== 
#include "pega_gpio.h"
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
static u8  g_full_step_state = AW_FULL_STEP_1;
static u8  g_half_step_state = AW_HALF_STEP_1;

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


void awd8833c_step2angle(int motor)
{

}
/**
  * @brief Motor full step forward drive.
  * @param None
  * @retval None
  */
void awd8833c_full_step_forward_function(void)
{
	switch (g_full_step_state) 					//now state
	{
	case AW_FULL_STEP_1:						//step 1 -> step 2
		 IO_AWD8833C_BIN1_PIN_OFF();
		 IO_AWD8833C_BIN2_PIN_ON();
		 g_full_step_state = AW_FULL_STEP_2;	//now : step 2
		 break;
	case AW_FULL_STEP_2:
		 IO_AWD8833C_AIN1_PIN_OFF();
		 IO_AWD8833C_AIN2_PIN_ON();
		 g_full_step_state = AW_FULL_STEP_3;
		 break;
		 
	case AW_FULL_STEP_3:
		 IO_AWD8833C_BIN1_PIN_ON();
		 IO_AWD8833C_BIN2_PIN_OFF();
		 g_full_step_state = AW_FULL_STEP_4;
		 break;
		 
	case AW_FULL_STEP_4:
		 IO_AWD8833C_AIN1_PIN_ON();
		 IO_AWD8833C_AIN2_PIN_OFF();
		 g_full_step_state = AW_FULL_STEP_1;
		 break;
		 
	default:
		 break;
	}
}

/**
  * @brief Motor full step reverse drive.
  * @param None
  * @retval None
  */
void awd8833c_full_step_reverse_function(void)
{
	switch (g_full_step_state)
	{
	case AW_FULL_STEP_1:
		 IO_AWD8833C_AIN1_PIN_OFF();
		 IO_AWD8833C_AIN2_PIN_ON();
		 g_full_step_state = AW_FULL_STEP_4;
		 break;
		 
	case AW_FULL_STEP_2:
		 IO_AWD8833C_BIN1_PIN_ON();
		 IO_AWD8833C_BIN2_PIN_OFF();
		 g_full_step_state = AW_FULL_STEP_1;
		 break;
		 
	case AW_FULL_STEP_3:
		 IO_AWD8833C_AIN1_PIN_ON();
		 IO_AWD8833C_AIN2_PIN_OFF();
		 g_full_step_state = AW_FULL_STEP_2;
		 break;
		 
	case AW_FULL_STEP_4:
		 IO_AWD8833C_BIN1_PIN_OFF();
		 IO_AWD8833C_BIN2_PIN_ON();
		 g_full_step_state = AW_FULL_STEP_3;
		 break;
		 
	default:
		break;
	}
}

/**
  * @brief Motor half step forward drive.
  * @param None
  * @retval None
  */
void awd8833c_half_step_forward_function(void)
{
	switch (g_half_step_state) 
	{
	case AW_HALF_STEP_1:
		IO_AWD8833C_AIN2_PIN_OFF();
		g_half_step_state = AW_HALF_STEP_2;
		break;
		 
	case AW_HALF_STEP_2:
		IO_AWD8833C_BIN2_PIN_ON();
		g_half_step_state = AW_HALF_STEP_3;
		break;
		 
	case AW_HALF_STEP_3:
		IO_AWD8833C_BIN1_PIN_OFF();
		g_half_step_state = AW_HALF_STEP_4;
		break;
		 
	case AW_HALF_STEP_4:
		IO_AWD8833C_AIN2_PIN_ON();
		g_half_step_state = AW_HALF_STEP_5;
		break;
		 
	case AW_HALF_STEP_5:
		IO_AWD8833C_AIN1_PIN_OFF();
		g_half_step_state = AW_HALF_STEP_6;
		break;
		
	case AW_HALF_STEP_6:
		IO_AWD8833C_BIN1_PIN_ON();
		g_half_step_state = AW_HALF_STEP_7;
		break;
		 
	case AW_HALF_STEP_7:
		IO_AWD8833C_BIN2_PIN_OFF();
		g_half_step_state = AW_HALF_STEP_8;
		break;
		 
	case AW_HALF_STEP_8:
		IO_AWD8833C_AIN1_PIN_ON();
		g_half_step_state = AW_HALF_STEP_1;
		break;
		 
	default:
		 break;
	}	
}


/**
  * @brief Motor half step reverse drive.
  * @param None
  * @retval None
  */
void awd8833c_half_step_reverse_function(void)
{
	switch (g_half_step_state) 
	{
	case AW_HALF_STEP_1:
		IO_AWD8833C_AIN1_PIN_OFF();
  		g_half_step_state = AW_HALF_STEP_8;
		break;
		 
	case AW_HALF_STEP_2:
		IO_AWD8833C_AIN2_PIN_ON();
  		g_half_step_state = AW_HALF_STEP_1;
		break;
		 
	case AW_HALF_STEP_3:
		IO_AWD8833C_BIN2_PIN_OFF();
 		g_half_step_state = AW_HALF_STEP_2;
		break;
		 
	case AW_HALF_STEP_4:
		IO_AWD8833C_BIN1_PIN_ON();
		g_half_step_state = AW_HALF_STEP_3;
		break;
		 
	case AW_HALF_STEP_5:
		IO_AWD8833C_AIN2_PIN_OFF();
		g_half_step_state = AW_HALF_STEP_4;
		break;
		 
	case AW_HALF_STEP_6:
		IO_AWD8833C_AIN1_PIN_ON();
		g_half_step_state = AW_HALF_STEP_5;
		break;
		 
	case AW_HALF_STEP_7:
		IO_AWD8833C_BIN1_PIN_OFF();
  		g_half_step_state = AW_HALF_STEP_6;
		break;
		 
	case AW_HALF_STEP_8:
		IO_AWD8833C_BIN2_PIN_ON();
  		g_half_step_state = AW_HALF_STEP_7;
		break;
		 
	default:
		 break;
	}
}

/**
  * @brief Test functions.
  * @note This function is for restore the last state 
  * @retval None
  */
void awd8833c_restore_state(int motor)
{
	//Disable the all motor
	IO_AWD8833C_PAN_MOTOR_OFF();	
	IO_AWD8833C_RISING_MOTOR_OFF();
	// gpio are configured to reset
	if(motor==AW_PAN_MOTOR)
	{
		//restore IO status of laste state
		switch (g_half_step_state) 
		{
		case AW_HALF_STEP_1:
			IO_AWD8833C_AIN1_PIN_ON();
			IO_AWD8833C_AIN2_PIN_ON();
			IO_AWD8833C_BIN1_PIN_ON();
			IO_AWD8833C_BIN2_PIN_OFF();

			break;
			
		case AW_HALF_STEP_2:
			IO_AWD8833C_AIN1_PIN_ON();
			IO_AWD8833C_AIN2_PIN_OFF();
			IO_AWD8833C_BIN1_PIN_ON();
			IO_AWD8833C_BIN2_PIN_OFF();
			break;
			
		case AW_HALF_STEP_3:
			IO_AWD8833C_AIN1_PIN_ON();
			IO_AWD8833C_AIN2_PIN_OFF();
			IO_AWD8833C_BIN1_PIN_ON();
			IO_AWD8833C_BIN2_PIN_ON();
			break;
			
		case AW_HALF_STEP_4:
			IO_AWD8833C_AIN1_PIN_ON();
			IO_AWD8833C_AIN2_PIN_OFF();
			IO_AWD8833C_BIN1_PIN_OFF();
			IO_AWD8833C_BIN2_PIN_ON();
			break;
			
		case AW_HALF_STEP_5:
			IO_AWD8833C_AIN1_PIN_ON();
			IO_AWD8833C_AIN2_PIN_ON();
			IO_AWD8833C_BIN1_PIN_OFF();
			IO_AWD8833C_BIN2_PIN_ON();
			break;
			
		case AW_HALF_STEP_6:
			IO_AWD8833C_AIN1_PIN_OFF();
			IO_AWD8833C_AIN2_PIN_ON();
			IO_AWD8833C_BIN1_PIN_OFF();
			IO_AWD8833C_BIN2_PIN_ON();
			break;
			
		case AW_HALF_STEP_7:
			IO_AWD8833C_AIN1_PIN_OFF();
			IO_AWD8833C_AIN2_PIN_ON();
			IO_AWD8833C_BIN1_PIN_ON();
			IO_AWD8833C_BIN2_PIN_ON();
			break;
			
		case AW_HALF_STEP_8:
			IO_AWD8833C_AIN1_PIN_OFF();
			IO_AWD8833C_AIN2_PIN_ON();
			IO_AWD8833C_BIN1_PIN_ON();
			IO_AWD8833C_BIN2_PIN_OFF();
			break;
			
		default:
			break;
		}
	}else{
		//restore IO status of laste state
		switch (g_full_step_state)
		{
			case AW_FULL_STEP_1:
				IO_AWD8833C_AIN1_PIN_ON();
				IO_AWD8833C_AIN2_PIN_OFF();
				IO_AWD8833C_BIN1_PIN_ON();
				IO_AWD8833C_BIN2_PIN_OFF();
				break;
				
			case AW_FULL_STEP_2:
				IO_AWD8833C_AIN1_PIN_ON();
				IO_AWD8833C_AIN2_PIN_OFF();
				IO_AWD8833C_BIN1_PIN_OFF();
				IO_AWD8833C_BIN2_PIN_ON();
				break;
				
			case AW_FULL_STEP_3:
				IO_AWD8833C_AIN1_PIN_OFF();
				IO_AWD8833C_AIN2_PIN_ON();
				IO_AWD8833C_BIN1_PIN_OFF();
				IO_AWD8833C_BIN2_PIN_ON();
				break;
				
			case AW_FULL_STEP_4:
				IO_AWD8833C_AIN1_PIN_OFF();
				IO_AWD8833C_AIN2_PIN_ON();
				IO_AWD8833C_BIN1_PIN_ON();
				IO_AWD8833C_BIN2_PIN_OFF();
				break;
				
			default:
				break;
		}
	}
}

/**
  * @brief Test functions.
  * @note This function is for stop motor but not go to sleep state.
  * @retval None
  */
void awd8833c_stop(void)
{
	IO_AWD8833C_PAN_MOTOR_OFF();	
	IO_AWD8833C_RISING_MOTOR_OFF();
	// gpio are configured to reset
	IO_AWD8833C_AIN1_PIN_OFF();
	IO_AWD8833C_AIN2_PIN_OFF();
	IO_AWD8833C_BIN1_PIN_OFF();
	IO_AWD8833C_BIN2_PIN_OFF();
}

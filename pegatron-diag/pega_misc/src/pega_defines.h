/*
 * Copyright (c) 2009-2014 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef PEGA_DEFINES_H
#define PEGA_DEFINES_H

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#include <sys/time.h>
//==============================================================================
#define MISC_VERSTION	    "0.01"
//==============================================================================
#define THREAD_PROC_1          "t1_DebugProc"
#define THREAD_PROC_2          "t2_Schedule"
#define THREAD_PROC_3          "t3_DiagProc"
#define THREAD_PROC_4          "t4_Key_Detection"
#define THREAD_PROC_5          "t5_LedFlash"
#define THREAD_PROC_6          "t6_GpioInt"
#define THREAD_PROC_7          "t7_MotorInt1"
#define THREAD_PROC_8          "t8_MotorInt2"
//==============================================================================
#define ERR_LOG          printf
#define INFO_LOG         printf
#define WARN_LOG         printf
#define INFO_SYSLOG      printf
//==============================================================================
#define PT_PRINTF(fmt, ...) \
		{ \
			fprintf(stdout, fmt "\n", ##__VA_ARGS__); \
		}

#define PT_DIAG_GET(intResult,data,...) \
						PT_PRINTF("\n#INFO#%s:" data "#END#", (intResult)?"-1":"0", ##__VA_ARGS__ )

#define PT_DIAG_SET(intResult) \
			PT_PRINTF("\n#INFO#%s#END#", (intResult)?"-1":"0")						
//==============================================================================
#define DBG_T(fmt, args...) do { \
    fprintf(stdout, "[%s] " fmt "\n", get_time(), ##args); \
	fflush(stdout);\
} while(0)
//==============================================================================	
#define DEVICE_ALS_ENABLE		1
#define DEVICE_AMP_ENABLE		1
#define DEVICE_MOTOR_ENABLE		1
#define DEVICE_IO_EXT_ENABLE	0
//==============================================================================
#define FAILED   	   -1
#define NOT_THING  	  	0
#define SUCCEED   		1
#define GPIO_LOW    	0
#define GPIO_HIGH   	1
#define GPIO_OUTPUT   	0
#define GPIO_INPUT    	1
#define GPIO_NULL	   -1
#define VALUE_NULL	   -1
//==============================================================================
#define ENABLE	        1
#define DISABLE	   		0
#define TRUE	        1
#define FALSE		    0
//==============================================================================
#define BIT_00		0x0001
#define BIT_01		0x0002
#define BIT_02		0x0004
#define BIT_03		0x0008
#define BIT_04		0x0010
#define BIT_05		0x0020
#define BIT_06		0x0040
#define BIT_07		0x0080
#define BIT_08		0x0100
#define BIT_09		0x0200
#define BIT_10		0x0400
#define BIT_11		0x0800
#define BIT_12		0x1000
#define BIT_13		0x2000
#define BIT_14		0x4000
#define BIT_15		0x8000
//==============================================================================
typedef enum 
{
	E_GPIO_INT_TRIGGER_NONE = 0,
	E_GPIO_INT_TRIGGER_RISING,
	E_GPIO_INT_TRIGGER_FALLING,
	E_GPIO_INT_TRIGGER_BOTH,
	E_GPIO_INT_TRIGGER_MAX,
}eGpio_Interrupt_EnumType;
//==============================================================================
typedef enum 
{
	E_WIFI_IF_TYPE_NONE = 0,
	E_WIFI_IF_TYPE_MLAN0,
	E_WIFI_IF_TYPE_MLAN1,	
	E_WIFI_IF_TYPE_MAX,
}eWiFi_IF_EnumType;


typedef enum 
{
  NV_MODE_UNKNOW = 0,
  NV_MODE_AUTO,  
  NV_MODE_NIGHT,  
  NV_MODE_DAY,  
  NV_MODE_MAX,
}NVModeEnumType;

typedef enum 
{
  NV_MODE_STATE_UNKNOW = 0,
  NV_MODE_STATE_DAY,  
  NV_MODE_STATE_NIGHT, 
  NV_MODE_STATE_MAX,
}NVModeStateEnumType;

typedef enum 
{
/*00*/LED_State_None = 0,
/*01*/LED_State_Off,
/*02*/LED_State_System_Start_Up,
/*03*/LED_State_Wifi_Ready,
/*04*/LED_State_Wifi_off,
/*05*/LED_State_Wifi_fw_error,
/*06*/LED_State_Power_off,
/*00*/LED_State_MAX,
}LedStateEnumType;
//==============================================================================
#ifdef __cplusplus
}
#endif

#endif //PEGA_DEFINES_H

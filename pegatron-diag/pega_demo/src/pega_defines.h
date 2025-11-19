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

//==============================================================================
#define MISC_VERSTION	    "0.01"
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
/*00*/LED_State_None = 0,
/*01*/LED_State_Voice_Cmd_training, 	//Fast blinking green
/*02*/LED_State_Voice_Cmd_standby,		//Solid green
/*03*/LED_State_Voice_Cmd_recognized,	//Led off
/*04*/LED_State_Voice_Cmd_wait,			//Slow blinking green
/*00*/LED_State_MAX,
}LedStateEnumType;
//==============================================================================
#ifdef __cplusplus
}
#endif

#endif //PEGA_DEFINES_H

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
#define 	FAILED   		-1
#define 	NOT_THING  	  	0
#define 	SUCCEED   		1
#define 	MMP_FALSE   	0
#define 	MMP_TRUE    	1
#define 	MMP_DISABLE   	0
#define 	MMP_ENABLE    	1
#define 	GPIO_LOW    	0
#define 	GPIO_HIGH   	1
#define 	GPIO_OUTPUT   	0
#define 	GPIO_INPUT    	1
#define 	GPIO_NULL		-1
//==============================================================================
#define 	BIT_00	0x01
#define 	BIT_01	0x02
#define 	BIT_02	0x04
#define 	BIT_03	0x08
#define 	BIT_04	0x10
#define 	BIT_05	0x20
#define 	BIT_06	0x40
#define 	BIT_07	0x80
#define 	BIT_08	0x0100
#define 	BIT_09	0x0200
#define 	BIT_10	0x0400
#define 	BIT_11	0x0800
#define 	BIT_12	0x1000
#define 	BIT_13	0x2000
#define 	BIT_14	0x4000
#define 	BIT_15	0x8000
//==============================================================================
#define   	DELAY_1S    (1000*1000) //ms
//==============================================================================
#define 	NA_BYTE  0xFF
#define 	NA_WORD  0xFFFF
//==============================================================================

//==============================================================================
#define I2C_BUS "0" //AMP
//==============================================================================
typedef	char  			MMP_BOOL;
typedef	unsigned char  	MMP_UBYTE;
typedef	unsigned short	MMP_USHORT;
typedef	unsigned int	MMP_ULONG;
//==============================================================================
#ifdef __cplusplus
}
#endif

#endif //PEGA_DEFINES_H

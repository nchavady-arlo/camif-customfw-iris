/*
 * Copyright (c) 2009-2014 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef PEGA_I2C_CTRL_H
#define PEGA_I2C_CTRL_H

#include "pega_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================

/* types */

typedef enum 
{
    I2C_FMT_A8D8, /**< 8 bits Address, 8 bits Data */
    I2C_FMT_A16D8,/**< 16 bits Address 8 bits Data */
    I2C_FMT_A8D16,/**< 8 bits Address 16 bits Data */
    I2C_FMT_A16D16,/**< 16 bits Address 16 bits Data */
    I2C_FMT_END/**< Reserved */
} ISP_I2C_FMT;
//==============================================================================
int  pega_i2cOpen(void);
void pega_i2cClose(void);
int  pega_i2c_Write(int slaveAddr, unsigned int reg, unsigned short value, ISP_I2C_FMT fmt);
int  pega_i2c_Read(int slaveAddr, unsigned int reg, unsigned short *val, ISP_I2C_FMT fmt);
//==============================================================================
#ifdef __cplusplus
}
#endif

#endif //PEGA_I2C_CTRL_H

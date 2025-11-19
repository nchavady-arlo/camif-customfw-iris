/*
 * Copyright (c) 2009-2014 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef PEGA_I2C_CTRL_H
#define PEGA_I2C_CTRL_H

#include <linux/i2c-dev.h>
#include "pega_defines.h"
#include "pega_i2c_control.h"

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
void pega_i2c_Init(void);
void pega_i2c_Deinit(void);
//==============================================================================
int  pega_i2c_Write(int i2cFile, int slaveAddr, unsigned int reg, uint16_t value, ISP_I2C_FMT fmt);
int  pega_i2c_Read(int i2cFile, int slaveAddr, unsigned int reg, uint16_t *val, ISP_I2C_FMT fmt);
int  pega_i2c_RW_Bytes(int i2cFile, struct i2c_rdwr_ioctl_data *packets);
 
//==============================================================================
#ifdef __cplusplus
}
#endif

#endif //PEGA_I2C_CTRL_H

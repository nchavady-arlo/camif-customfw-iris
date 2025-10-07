#ifndef _PEGA_TCA9538_H_
#define _PEGA_TCA9538_H_

#include "pega_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
// Register address and bit definitions for  Ambient Light Sensor
#define TCA9538_ADDRESS							0x70 * 2//7 bit i2c address 
//==============================================================================
//
//                              REGISTER ADDRESS
//
//==============================================================================
// TCA9538 registers
#define	TCA9538_REG_INPUT_PORT				0x00 //
#define	TCA9538_REG_OUTPUT_PORT				0x01 //default : 0xff
#define	TCA9538_REG_POLARITY_INV			0x02 //default : 0x00
#define	TCA9538_REG_CONFIGURATION			0x03 //default : 0xff
//==============================================================================

//==============================================================================
//
//                              STRUCTURE / ENUM
//
//==============================================================================

//==============================================================================
void TCA9538_Device_Init(void);
void TCA9538_Data_Print(void);
void TCA9538_Register_Info_Print(void);
//==============================================================================
#ifdef __cplusplus
}
#endif

#endif /* (_PEGA_OPT300X_H_) */

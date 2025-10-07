#ifndef _PEGA_OPT300X_H_
#define _PEGA_OPT300X_H_

#include "pega_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
// Register address and bit definitions for  Ambient Light Sensor
//==============================================================================
#define OPT300x_ADDRESS				   0x44 * 2// 0x44 7 bit i2c address 
//==============================================================================
//
//                              REGISTER ADDRESS
//
//==============================================================================
// OPT300x registers
#define	OPT300x_REG_SYS_RESULT			0x00
#define	OPT300x_REG_SYS_CONF			0x01
#define	OPT300x_REG_LOW_LIMIT			0x02
#define	OPT300x_REG_HIGH_LIMIT			0x03
#define	OPT300x_REG_MANUFACTURER_ID	    0x7E
#define	OPT300x_REG_DEVICE_ID			0x7F
//==============================================================================
#define BIT(nr)			(1UL << (nr))

#define OPT300x_CONFIGURATION_RN_MASK				(0xf << 12)
#define OPT300x_CONFIGURATION_RN_AUTO				(0xc << 12)

#define OPT300x_CONFIGURATION_CT						(1 << 11)//BIT(11)

#define OPT300x_CONFIGURATION_M_MASK			(3 << 9)
#define OPT300x_CONFIGURATION_M_SHUTDOWN 		(0 << 9)
#define OPT300x_CONFIGURATION_M_SINGLE			(1 << 9)
#define OPT300x_CONFIGURATION_M_CONTINUOUS 		(2 << 9) /* also 3 << 9 */

#define OPT300x_CONFIGURATION_OVF						BIT(8)
#define OPT300x_CONFIGURATION_CRF						BIT(7)
#define OPT300x_CONFIGURATION_FH						BIT(6)
#define OPT300x_CONFIGURATION_FL						BIT(5)
#define OPT300x_CONFIGURATION_ME						BIT(2)

#define OPT300x_CONFIGURATION_FC_MASK					(3 << 0)

#define OPT300x_CONFIGURATION_DEF						(OPT300x_CONFIGURATION_RN_AUTO + OPT300x_CONFIGURATION_CT)
/* The end-of-conversion enable is located in the low-limit register */
#define OPT300x_LOW_LIMIT_EOC_ENABLE					0xc000

#define OPT300x_REG_EXPONENT(n)							((n) >> 12)
#define OPT300x_REG_MANTISSA(n)							((n) & 0xfff)

#define OPT300x_INT_TIME_LONG							800000
#define OPT300x_INT_TIME_SHORT							100000
/*
 * Time to wait for conversion result to be ready. The device datasheet
 * sect. 6.5 states results are ready after total integration time plus 3ms.
 * This results in worst-case max values of 113ms or 883ms, respectively.
 * Add some slack to be on the safe side.
 */
#define OPT300x_RESULT_READY_SHORT			120 //150
#define OPT300x_RESULT_READY_LONG			1000
//==============================================================================
//
//                              STRUCTURE / ENUM
//
//==============================================================================
typedef struct 
{	
	uint16_t wDeviceID;
    uint16_t wConfReg;       
    float      fLux;
    float      fLux_old;            
}stALSDefType;
//==============================================================================
void 	OPT300x_Device_I2C_open(void);
void 	OPT300x_Device_I2C_close(void);
//==============================================================================
int      OPT300x_Is_Init_Already(void);
void 	 OPT300x_Device_Init(void);;
float 	 OPT300x_Read_Lux_Handler(void);
float 	 OPT300x_Lux_Value_Get(void);
uint16_t OPT300x_Device_ID_Get(void);
float 	 OPT300x_Diag_Lux_Value_Get(void);
void 	 OPT300x_Data_Print(void);
void 	 OPT300x_Register_Info_Print(void);
//==============================================================================
#ifdef __cplusplus
}
#endif

#endif /* (_PEGA_OPT300X_H_) */

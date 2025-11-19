
#ifndef _PEGA_AMP_TAS2563_H_
#define _PEGA_AMP_TAS2563_H_

#include "pega_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
// Register address
#define TAS2563_ADDRESS						    0x98 //0x4C 7bit
#define TAS2563_REG_MAX                 		0x7F

/* speaker load */
#define LOAD_8_OHM                               (0)
#define LOAD_6_OHM                               (1)
#define LOAD_4_OHM                               (2)

/* sample rate */
#define SAMPLE_RATE_48000                        (0)
#define SAMPLE_RATE_44100                        (1)
#define SAMPLE_RATE_16000                        (2)
#define SAMPLE_RATE_8000                         (3)

/* ASI(Audio Serail Interface) mode*/
#define I2S_MODE		0
#define DSP_MODE		1
#define RJF_MODE		2 /* Right Justified mode */
#define LJF_MODE		3 /* Left Justified mode */
#define MONO_PCM_MODE	4
#define TDM_MODE		5

#define TAS2563_ASI_MODE_SHIFT  2
#define TAS2563_ASI_FORMAT_MASK                    (0x7 << TAS2563_ASI_MODE_SHIFT)

/* ASI channel mode */
#define TAS2563_LEFT				0
#define TAS2563_RIGHT				1
#define TAS2563_LEFT_RIGHT_DIV_2	2
#define TAS2563_MONO_PCM			3

/* PCM/PDM in/out mode */
#define PCM_INPUT_PLAYBACK_ONLY				1
#define PCM_INPUT_PLAYBACK__PCM_IV_OUT		2
#define PCM_INPUT_PLAYBACK__PDM_IV_OUT		3
#define PDM_INPUT_PLAYBACK_ONLY				4
#define PDM_INPUT_PLAYBACk__PDM_IV_OUT		5
// TAS2563 registers
/* Registers Address */
/* Book0, Page0 registers */
#define TAS2563_PAGE						0x00
#define TAS2563_SW_RESET					0x01
#define TAS2563_PWR_CTL						0x02
#define TAS2563_PB_CFG1						0x03
#define TAS2563_MISC_CFG1					0x04
#define TAS2563_MISC_CFG2					0x05
#define TAS2563_TDM_CFG0					0x06
#define TAS2563_TDM_CFG1					0x07
#define TAS2563_TDM_CFG2					0x08
#define TAS2563_TDM_CFG3					0x09
#define TAS2563_TDM_CFG4					0x0A
#define TAS2563_TDM_CFG5					0x0B
#define TAS2563_TDM_CFG6					0x0C
#define TAS2563_TDM_CFG7					0x0D
#define TAS2563_TDM_CFG8					0x0E
#define TAS2563_TDM_CFG9					0x0F
#define TAS2563_TDM_CFG10					0x10
#define TAS2563_DSP_MODE					0x11
#define TAS2563_LIM_CFG0					0x12
#define TAS2563_LIM_CFG1					0x13
#define TAS2563_DSP_FREQ					0x14
#define TAS2563_BOP_CFG0					0x15
#define TAS2563_BIL_ICLA_CFG0				0x16
#define TAS2563_BIL_ICLA_CFG1				0x17
#define TAS2563_GAIN_ICLA_CFG0				0x18
#define TAS2563_ICLA_CFG1					0x19
#define TAS2563_INT_MASK0					0x1A
#define TAS2563_INT_MASK1					0x1B
#define TAS2563_INT_MASK2					0x1C
#define TAS2563_INT_MASK3					0x1D
#define TAS2563_INT_LIVE0					0x1F
#define TAS2563_INT_LIVE1					0x20
#define TAS2563_INT_LIVE3					0x21
#define TAS2563_INT_LIVE4					0x22
#define TAS2563_INT_LTCH0					0x24
#define TAS2563_INT_LTCH1					0x25
#define TAS2563_INT_LTCH3					0x26
#define TAS2563_INT_LTCH4					0x27
#define TAS2563_VBAT_MSB					0x2A
#define TAS2563_VBAT_LSB					0x2B
#define TAS2563_TEMP						0x2C
#define TAS2563_INT_CLK_CFG					0x30
#define TAS2563_DIN_PD						0x31
#define TAS2563_MISC1						0x32
#define TAS2563_BOOST_CFG1					0x33
#define TAS2563_BOOST_CFG2					0x34
#define TAS2563_BOOST_CFG3					0x35
#define TAS2563_MISC2						0x3B
#define TAS2563_TG_CFG0						0x3F
#define TAS2563_BST_ILIM_CFG0				0x40
#define TAS2563_PDM_CONFIG0					0x41
#define TAS2563_DIN_PD_PDM_CONFIG3			0x42
#define TAS2563_ASI2_CONFIG0				0x43
#define TAS2563_ASI2_CONFIG1				0x44
#define TAS2563_ASI2_CONFIG2				0x45
#define TAS2563_ASI2_CONFIG3				0x46
#define TAS2563_PVDD_MSB_DSP				0x49
#define TAS2563_PVDD_LSB_DSP				0x4A
#define TAS2563_REV_ID						0x7D
#define TAS2563_I2C_CKSUM					0x7E
#define TAS2563_BOOK						0x7F
//==============================================================================
#define TAS2563_SP_8khz		8000
#define TAS2563_SP_16khz	16000
#define TAS2563_SP_MAX		2
//==============================================================================
void Pega_AMP_Device_I2C_open(void);
void Pega_AMP_Device_I2C_close(void);
//==============================================================================
void  Pega_AMP_Initialize(int bIsNewCfg, float fdB);
void  Pega_AMP_Software_Reset(void);
void  Pega_AMP_Software_Mute(uint8_t bMute);
void  Pega_AMP_Volume_Control(float fdB);
float Pega_AMP_Volume_Get(void);
//==============================================================================
int  Pega_AMP_Clock_Error_check(void);
//==============================================================================
void Pega_AMP_Reg_Write(uint8_t ucRegID, uint8_t val);
void Pega_AMP_Reg_Read(uint8_t ucRegID, uint8_t *val);
void Pega_AMP_Reg_Data_Print(void);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_AMP_TAS2563_H_

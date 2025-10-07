#ifndef _PEGA_NV_MODE_H_
#define _PEGA_NV_MODE_H_

#include "pega_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
//
//                              STRUCTURE / ENUM
//
//==============================================================================
//__attribute__ ((packed)) 的作用就是告訴編譯器取消結構在編譯過程中的优化對齊
typedef struct __attribute__((aligned (4)))
{	     
	float fLux;
	float fOffset;
	float fLuxCal;   
	float fNightModeThd;      //Default=2.0 (in hundredths of lux)
	float fDayModeThd;        //Default=5.0 (in hundredths of lux)
	uint8_t bNVModeSet;
	uint8_t bIsNVModeAuto;
    uint8_t bIsNightMode;         //Chris modify 20210802	
	uint8_t bIRLedEn;
	uint8_t bIsIRLedOn;
	uint8_t u8DayModeCounts;
	uint8_t u8NightModeCounts;
	int     sTriggerCounts;
} stNVDefType;

enum 
{
	enNV_Manual=0,
	enNV_OPT300x,
	enNV_Lux_Update,
	enNV_Night_Mode_Set,
	enNV_Day_Mode_Set,
	enNV_Num,
};
//==============================================================================
int  	Pega_NV_Mode_Processing(void);
//==============================================================================
void 	Pega_NV_Mode_Init(void);
void 	Pega_NV_Mode_nvram_update(void);
void 	Pega_NV_DayNighMode_Manual_Set(uint8_t bNightMode);
int  	Pega_NV_EnterNVThreshold_Update(int night_thd);
int  	Pega_NV_ExitNVThreshold_Update(int day_thd);
void 	Pega_NV_Mode_IR_Led_Control(int bEnable);
void 	Pega_NV_Mode_IR_Led_Control_recover(void);
void 	Pega_NV_Mode_Night_Mode_Set(void);
void 	Pega_NV_Mode_Day_Mode_Set(void);
void 	Pega_NV_Mode_Threshold_Set(float fNightModeThd, float fDayModeThd);
void 	Pega_NV_Mode_Control_Set(uint8_t bFlag);
void 	Pega_NV_Mode_parameters_Set(stNVDefType para);
void 	Pega_NV_Mode_parameters_Get(stNVDefType *para);
void 	Pega_NV_Mode_NVModeSet_Flag_Clear(void);
void 	Pega_NV_Mode_NVModeSet_LuxCal_Set(float fLuxCal);
float   Pega_NV_Mode_NVModeSet_LuxCal_Get();
void 	Pega_NV_Mode_NVModeSet_LuxOffset_Set(float fLuxOffset);
void 	Pega_NV_Mode_Night_IR_Led_Control(uint8_t bFlag);
void 	Pega_NV_Mode_Data_Print(void);
uint8_t Pega_VN_Mode_NightMode_Get(void);
void  	Pega_NV_Mode_NightMode_Set(uint8_t day_night);
void  	Pega_NV_Mode_IRLedEn_Set(uint8_t enable);
float 	Pega_NV_Mode_Diag_Lux_Get(void);
int  	pga_get_InitialLSValue(void);
int  	pga_get_LastLSValue(void);
void 	pga_set_NV_TriggerCounts(int value);
//==============================================================================
void Pega_Diag_Force_Day_Night_Mode(uint8_t IsNightMode);
//==============================================================================
#ifdef __cplusplus
}
#endif

#endif /* (_PEGA_NV_MODE_H_) */

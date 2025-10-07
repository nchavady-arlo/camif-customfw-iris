#ifndef _MAIN_H_
#define _MAIN_H_
//==============================================================================
#include "pega_defines.h"
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#define MISC_DELAY_CNT_MS		    (1000*1000) //ms //1s
//==============================================================================
typedef struct
{	  	
      uint16_t	bIsBurnInOn			:1;	  
	  uint16_t	bIsALSDisable		:1;	  
	  uint16_t	bIsWiFiDisable		:1;
	  uint16_t	bIsACCDisable		:1;
	  uint16_t	bIsNFCDisable		:1;
	  uint16_t	bIsMotorDisable		:1;
	  uint16_t	bIsWiFiFwReady		:1;
	  uint16_t	bIsWiFiFwError		:1;
	  uint16_t	bReserved			:8;
	  
	  uint8_t	eWIFI1_if; //->ifs_sdmmc0
	  uint8_t	eWIFI2_if; //->ifs_sdmmc1
}stMiscType;
//==============================================================================
void Pega_Misc_Burn_Enable_Control(uint8_t bIsEnable);
//==============================================================================
void Pega_Misc_ALS_Disable_Control(uint8_t bIsDisable);
void Pega_Misc_WiFi_Disable_Control(uint8_t bIsDisable);
//==============================================================================
void Pega_Misc_Update_WiFi_interface(void);
//==============================================================================
void Pega_Misc_Diag_wifi_interface_Print(void);
//==============================================================================
void Pega_Misc_Data_Print(void);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_MAIN_H_

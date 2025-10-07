#ifndef _PEGA_MISC_DIAG_H_
#define _PEGA_MISC_DIAG_H_

#include "pega_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#define MSGQ_MISC_DIAG_QUEUE_KEY  85695540  //receive msg from pega_diag
//==============================================================================
typedef enum 
{
/*000*/ Diag_CmdId_None = 0,
/*001*/ Diag_CmdId_HW_Version,
/*002*/ Diag_CmdId_ICR_Control,
/*003*/ Diag_CmdId_IRLed_Control,
/*004*/ Diag_CmdId_IRLed_PWM_Control,
/*005*/ Diag_CmdId_SpotLightLed_Control,
/*006*/ Diag_CmdId_SpotLightLed_PWM_Control,
/*007*/ Diag_CmdId_Led_Control,
/*008*/ Diag_CmdId_ALS_Control,
/*009*/ Diag_CmdId_MIC_Control,
/*010*/ Diag_CmdId_Audio_Control,
/*011*/ Diag_CmdId_Button_Control,
/*012*/ Diag_CmdId_OTA_Control,
/*013*/ Diag_CmdId_Therma_Control,
/*014*/ Diag_CmdId_wifi_Control,
/*015*/ Diag_CmdId_Power_Off_Control,
/*016*/ Diag_CmdId_Out2TelnetPegaMisc,
/*017*/ Diag_CmdId_BurnIn,
/*018*/ Diag_CmdId_Max,
} stDiagMsgqCmdIdType;
//==============================================================================
typedef struct
{	    
	  int   eCmdId;   
	  int   value1;      
      int   value2;	   
	  int   value3;
	  float fval1;	
	  float fval2;
	  char  bDebugEn; 	  
	  char 	wCmdCnt;	
}stDiagCmdStrType;
//==============================================================================
typedef struct
{
	  int 	msgid;	  
	  int   eCmdId;   
	  int   value1;      
      int   value2;	   
	  int   value3;	  
	  float fval1;	
	  float fval2;	
}stDiagMsgqCmdType;
//==============================================================================
void Pega_diag_msgq_handler_start(void);
void Pega_diag_msgq_Cmd_Execution(void);
void Pega_diag_msgq_debug_trigger(void);
void Pega_diag_msgq_Data_Info_Print(void);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_MISC_DIAG_H_
#ifndef _PEGA_DIAG_H_
#define _PEGA_DIAG_H_
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#define ENABLE_PRINTF_PARAMETER_INFO   	0
//==============================================================================
#define ENABLE_NEW_BT_MAC   			1
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
// command function
typedef struct {
	char *cmd;
	char *doc;
	int (*func)(int argc, char **argv);
	void *eNext;
} command_line_t ;
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
#ifdef __cplusplus
}
#endif
#endif //_PEGA_DIAG_H_

#ifndef _PEGA_QUECTEL_MSGQ_CMD_H_
#define _PEGA_QUECTEL_MSGQ_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#define MSGQ_MISC_DEBUG_QUEUE_KEY    89695540  //send msg to pega_misc
//==============================================================================
#define MSGQ_ONVIF_RX_FROM_MISC_KEY  89796520  //pega_misc send msg to onvif_msgq_rx
//==============================================================================
#define MSGQ_ONVIF_TX_TO_MISC_KEY    81762510  //onvif send msg to pega_misc
//==============================================================================
#define MSGQ_DEBUG_TX_TO_MEDIA_KEY   84269130  //receive msg from pega_misc_dbg
//==============================================================================
typedef enum 
{
/*000*/ Debug_CmdId_None = 0,
/*001*/ Debug_CmdId_Debug_Msg_Control,
/*002*/ Debug_CmdId_Debug_Print,
/*003*/ Debug_CmdId_Debug_Set,
/*004*/ Debug_CmdId_Debug_Get,
/*005*/ Debug_CmdId_Debug_Motor,
/*006*/ Debug_CmdId_Schedule_Event,
/*007*/ Debug_CmdId_Max,
} DebugCmdMsgTypeEnum;
//==============================================================================
typedef struct
{
	  int 	msgid;	  
	  int   eCmdId;   
	  int   value1;      
      int   value2;	   
	  int   value3;
	  int   value4;	
}stDebugCmdType;
//==============================================================================
int Pega_debug_msgq_send_to_misc(stDebugCmdType stDebugMsg);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_QUECTEL_MSGQ_CMD_H_

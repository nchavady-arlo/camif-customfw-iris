#ifndef _PEGA_DEBUG_H_
#define _PEGA_DEBUG_H_

#include "pega_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#define MSGQ_MISC_DEBUG_QUEUE_KEY  89695540  //receive msg from pega_misc_dbg
//==============================================================================
typedef enum 
{
/*000*/ SysDebug_CmdId_None = 0,
/*001*/ SysDebug_CmdId_Info,
/*002*/ SysDebug_CmdId_Set,
/*003*/ SysDebug_CmdId_Get,
/*004*/ SysDebug_CmdId_Motor,
/*005*/ SysDebug_CmdId_Max,
} SysDebugCmdTypeEnum;

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
}stDebugCmdMsgType;
//==============================================================================
typedef struct
{	 
	  int   eCmdId;   
	  int   value1;      
      int   value2;	   
	  int   value3;	  
}stDebugCmdType;
//==============================================================================
void Pega_debug_msgq_handler_start(void);
//==============================================================================
void Pega_debug_schedule_command(void);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_DEBUG_H_
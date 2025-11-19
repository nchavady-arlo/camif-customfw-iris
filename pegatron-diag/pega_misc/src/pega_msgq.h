#ifndef _PEGA_MSGQ_H_
#define _PEGA_MSGQ_H_

#include "pega_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#define MSGQ_QUEUE_NAME 		"/MsgQueue"  //receive msg from pega_misc_dbg
#define MSGQ_BUFF_MAX_SIZE 		512//1024
#define MSGQ_CMD_NAME_LEN 		32//1024
#define MSGQ_CMD_FROM_LEN 		16
#define MSGQ_MAX_MESSAGES       6
//==============================================================================
#define MSGQ_ARGC_MAX      	 	5
#define MSGQ_ARGV_LEN  			16
//==============================================================================
typedef struct 
{    
    int   argc;
	char  argv[MSGQ_ARGC_MAX][MSGQ_ARGV_LEN];
}stMsgQSubCmd_t;

typedef struct 
{
    int  CmdCount;
	int  bRequestAck;
    char CmdName[MSGQ_CMD_NAME_LEN];
	char CmdFrom[MSGQ_CMD_FROM_LEN];
	
	stMsgQSubCmd_t stData;
}stMsgQdata_t __attribute__((aligned(8)));
//==============================================================================
// command function
typedef struct 
{
	char *strCmd;
	char *doc;
	int (*func)(stMsgQSubCmd_t stData);	
}stMsgQCmd_t;
//==============================================================================
void Pega_msgq_listen_handler_start(void);
//==============================================================================
void Pega_msgq_command_handler_exec(void);
//==============================================================================
void Pega_msgq_debug_message_enable(char bDebugOn);
//==============================================================================
void Pega_msgq_attr_info(void);
void Pega_msgq_data_info(void);
void Pega_msgq_data_content_info(void);
//==============================================================================
#endif
#ifdef __cplusplus
}
#endif

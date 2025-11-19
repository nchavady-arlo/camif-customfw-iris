#ifndef _PEGA_MQ_H
#define _PEGA_MQ_H
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
#endif

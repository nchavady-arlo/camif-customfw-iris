/*******************************************************************************
* File Name: pega_diag_mq.c
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>    // For O_* constants
#include <sys/stat.h> // For mode constants
#include <unistd.h>
//==============================================================================
#include "pega_diag_mq.h"
//==============================================================================
int pga_diag_msgq_command_send(stMsgQdata_t stDiagCmd)
{
	int rtn = 0;
	
	strncpy(stDiagCmd.CmdName, "diag", MSGQ_CMD_NAME_LEN);
	strncpy(stDiagCmd.CmdFrom, "pega_diag", MSGQ_CMD_FROM_LEN);
	
	if (stDiagCmd.stData.argc > MSGQ_ARGC_MAX) 
	{
        fprintf(stderr, "argc is over(%d,%d).\n", stDiagCmd.stData.argc, MSGQ_ARGC_MAX);
        return -1;
    }
	
	// 打開 message queue
    mqd_t mq = mq_open(MSGQ_QUEUE_NAME, O_WRONLY);
	
    if (mq == (mqd_t)-1) 
	{
        perror("mq_open");
        return -1;
    }

    // 傳送資料
    rtn = mq_send(mq, (const char *)&stDiagCmd, sizeof(stDiagCmd), 1);
	
    if (rtn == -1) 
	{
        perror("mq_send");
    }

    mq_close(mq);
	
	return rtn;
}
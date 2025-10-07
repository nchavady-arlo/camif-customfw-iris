/*******************************************************************************
* File Name: pega_diag_msgq_cmd.c
*
*******************************************************************************/
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <malloc.h>
//==============================================================================
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/msg.h>
//==============================================================================
#include "pega_diag_msgq_cmd.h"
//==============================================================================
int Pega_diag_msgq_send_request(stDiagMsgqCmdType stDiagMsg)
{
	int rcv;
	int msgid = -1;
    
	printf("\n%s (eCmdId=%d)\n", __func__, stDiagMsg.eCmdId);
	 
	//stDiagMsgqCmdType stDiagMsg;
     
	if((msgid = msgget(MSGQ_MISC_DIAG_QUEUE_KEY, IPC_CREAT | 0666)) == -1)
	{
		printf("Pega_debug_report_status() get MSGQ_MISC_DIAG_QUEUE_KEY id error!\n");
		return -1;
	}
    	
	stDiagMsg.msgid = 1;	
	
	rcv = msgsnd(msgid, &stDiagMsg, sizeof(stDiagMsgqCmdType) - sizeof(int) , IPC_NOWAIT);
	
	if ( rcv < 0 )
	  {
		  printf("Pega_debug_msgq_send_request sendMsg error %d\n", rcv);
		  return -1;
	  }
	  
    return 0;
}
//==============================================================================
int Pega_diag_msgq_send_mediaserver(T_MiCommonMsg stDiagMsg)
{
	int rcv;
	int msgid = -1;
     
	printf("\n%s (eCmdId=%d)\n", __func__, stDiagMsg.snapshot_info.cmdid);
	 
	//stDiagMsgqCmdType stDiagMsg;
     
	if((msgid = msgget(MSG_MI_QUEUE_KEY, IPC_CREAT | 0666)) == -1)
	{
		printf("Pega_debug_report_status() get MSG_MI_QUEUE_KEY id error!\n");
		return -1;
	}
    	
	//stDiagMsg.msgid = 1;	
	
	rcv = msgsnd(msgid, &stDiagMsg, sizeof(T_MiCommonMsg) - sizeof(int) , IPC_NOWAIT);
	
	if ( rcv < 0 )
	  {
		  printf("Pega_debug_msgq_send_request sendMsg error %d\n", rcv);
		  return -1;
	  }
	  
    return 0;
	
	
	
}
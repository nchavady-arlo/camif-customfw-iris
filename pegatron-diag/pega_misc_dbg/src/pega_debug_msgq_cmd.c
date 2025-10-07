/*******************************************************************************
* File Name: pega_debug_msgq_cmd.c
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
//==============================================================================
#include "pega_debug_msgq_cmd.h"
//==============================================================================
int Pega_debug_msgq_send_to_misc(stDebugCmdType stDebugMsg)
{
	int rcv;
	int msgid = -1;
    
	printf("%s (eCmdId=%d)\n", __func__, stDebugMsg.eCmdId);
	 
	//stDebugCmdType stDebugMsg;
     
	if((msgid = msgget(MSGQ_MISC_DEBUG_QUEUE_KEY, IPC_CREAT | 0666)) == -1)
	{
		printf("Pega_debug_report_status() get MSGQ_MISC_DEBUG_QUEUE_KEY id error!\n");
		return -1;
	}
    	
	stDebugMsg.msgid = 1;	
	
	rcv = msgsnd(msgid, &stDebugMsg, sizeof(stDebugCmdType) - sizeof(int) , IPC_NOWAIT);
	
	if ( rcv < 0 )
	  {
		  printf("Pega_debug_msgq_send_to_misc sendMsg error %d\n", rcv);
		  return -1;
	  }
	  
    return 0;
}
//==============================================================================

//==============================================================================
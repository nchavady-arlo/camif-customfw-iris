/*******************************************************************************
* File Name: pega_msgq_cmd.c
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
#include <sys/prctl.h>
#include <pthread.h>
#include <mqueue.h>
#include <signal.h>
#include <sys/stat.h> // For mode constants
//==============================================================================
#include "pega_msgq_cmd.h"
#include "pega_msgq_diag.h"
#include "pega_msgq_debug.h"
//==============================================================================
#include "main.h"
#include "pega_schedule.h"
#include "pega_nv_mode.h"
#include "pega_gpio.h"
#include "pega_wifi.h"
//==============================================================================	
static int Msqq_command_exec_diag(stMsgQSubCmd_t stData);
static int Msqq_command_exec_debug(stMsgQSubCmd_t stData);
//==============================================================================
static const stMsgQCmd_t m_eCmdTable[] = 
{	
	{ "diag",		"diag command",		&Msqq_command_exec_diag},
	{ "debug",		"debug command",	&Msqq_command_exec_debug},	
	{ NULL, NULL, NULL },
};
//==============================================================================
static int Msqq_command_exec_diag(stMsgQSubCmd_t stData)
{	 
	 return pga_diag_command_exec(stData);
}
//==============================================================================
static int Msqq_command_exec_debug(stMsgQSubCmd_t stData)
{	 
	 return pga_debug_command_exec(stData);
}
//==============================================================================
int Pega_msgq_message_processing(stMsgQdata_t stMsgQdata)
{
	   int intIndex  = 0;
	   int intResult = -1;
	
	   stMsgQCmd_t *eCommand = m_eCmdTable;
	   
	   while (eCommand[intIndex].strCmd != NULL)
	   {		   
		   if (!strcmp(stMsgQdata.CmdName, eCommand[intIndex].strCmd))
		   {
		     if(eCommand[intIndex].func != NULL)
			 {
				intResult = eCommand[intIndex].func(stMsgQdata.stData);
				break;
			 }
		   }
		   
		   intIndex++;
	   }
	  
	   return intResult;
}
//==============================================================================

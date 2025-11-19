/*******************************************************************************
* File Name: pega_msgq_debug.c
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
#include "pega_msgq.h"
#include "pega_msgq_debug.h"
//==============================================================================
#include "main.h"
#include "pega_schedule.h"
#include "pega_nv_mode.h"
#include "pega_wifi.h"
#include "pega_dtbo.h"
//==============================================================================
#include "demo.h"
#include "pega_gpio.h"
#include "pega_gpio_key.h"
#include "pega_als_opt300x.h"
#include "pega_amp_tas256x.h"
//==============================================================================
static int pga_debug_command_help(stMsgQSubCmd_t stData);
static int pga_debug_command_msgq(stMsgQSubCmd_t stData);
static int pga_debug_command_info(stMsgQSubCmd_t stData);
static int pga_debug_command_set(stMsgQSubCmd_t stData);
static int pga_debug_command_schedule(stMsgQSubCmd_t stData);
//==============================================================================
static const stMsgQCmd_t m_eDebugCmdTable[] = 
{	
	{ "help",		"list all debug commands",	&pga_debug_command_help},
	{ "msgq",		"msgq print on/off",		&pga_debug_command_msgq},
	{ "info",		"debug info command",		&pga_debug_command_info},
	{ "set",		"debug set command",		&pga_debug_command_set},
	{ "sch",		"debug schedule command",	&pga_debug_command_schedule},
	{ NULL, NULL, NULL },
};
//==============================================================================
//pega_debug debug help
static int pga_debug_command_help(stMsgQSubCmd_t stData)
{
	int intIndex = 0;
	
	stMsgQCmd_t *eCommand = m_eDebugCmdTable;
	
	printf("\n");
	
	while (eCommand[intIndex].strCmd != NULL)
	{
		printf("%16s \t %s\n", eCommand[intIndex].strCmd, eCommand[intIndex].doc);
		
		intIndex++;
	}
			
	return 0;
}
//pega_debug debug msgq on / off
static int pga_debug_command_msgq(stMsgQSubCmd_t stData)
{
	int rtn = -1;
	
	//printf("[%s] argc=%d\n", __func__, stData.argc);
	
	if (stData.argc < 2)
	{
		return rtn;
	}
				
	if (!strcmp(stData.argv[1],"on"))
	{
		rtn = 0;
		Pega_msgq_debug_message_enable(1);
	}
	else if (!strcmp(stData.argv[1],"off"))
	{
		rtn = 0;
		Pega_msgq_debug_message_enable(0);
	}	
	
	return rtn;
}
//pega_debug debug info xx
//pega_debug debug info misc
//pega_debug debug info sch
//pega_debug debug info gpio
//pega_debug debug info als
//pega_debug debug info als_reg
//pega_debug debug info amp_reg
//pega_debug debug info nvmode
//pega_debug debug info dtbo
//pega_debug debug info key
static int pga_debug_command_info(stMsgQSubCmd_t stData)
{
	if (stData.argc < 2)
	{
		fprintf(stderr, "Command error.[%d]\n", stData.argc);		
		return 0;
	}
	
	if (!strcmp(stData.argv[1],"misc"))
	{		
		Pega_Misc_Data_Print();
	}
	else if (!strcmp(stData.argv[1],"sch"))
	{		
		pega_schedule_Data_Info_Print();
	}
	else if (!strcmp(stData.argv[1],"gpio"))
	{		
		Pega_Gpio_Data_Info_Print();
	}
	else if (!strcmp(stData.argv[1],"als"))
	{		
		OPT300x_Data_Print();
	}
	else if (!strcmp(stData.argv[1],"als_reg"))
	{		
		OPT300x_Register_Info_Print();
	}
	else if (!strcmp(stData.argv[1],"amp_reg"))
	{		
		Pega_AMP_Reg_Data_Print();
	}
	else if (!strcmp(stData.argv[1],"nvmode"))
	{		
		Pega_NV_Mode_Data_Print();
	}
	else if (!strcmp(stData.argv[1],"dtbo"))
	{		
		Pega_dtbo_data_info();
	}
	else if (!strcmp(stData.argv[1],"key"))
	{		
		Pega_Gpio_Key_Data_Info_Print();
	}
			
	return 0;
}
//pega_debug debug set xx
//pega_debug debug set burnin 0/1
//pega_debug debug set dis_als 0/1
//pega_debug debug set dis_wifi 0/1
static int pga_debug_command_set(stMsgQSubCmd_t stData)
{
	if (stData.argc < 2)
	{
		fprintf(stderr, "Command error.[%d]\n", stData.argc);		
		return 0;
	}

    if (!strcmp(stData.argv[1],"burnin"))
	{		
        if (stData.argc == 3)//(stData.argv[2] != NULL)
			Pega_Misc_BurnIn_Enable_Control(atoi(stData.argv[2])>0);
	}
	else if (!strcmp(stData.argv[1],"dis_als"))
	{		
		if (stData.argc == 3)//(stData.argv[2] != NULL)
			Pega_Misc_ALS_Disable_Control(atoi(stData.argv[2])>0);
	}
	else if (!strcmp(stData.argv[1],"dis_wifi"))
	{		
		if (stData.argc == 3)//(stData.argv[2] != NULL)
			Pega_Misc_WiFi_Disable_Control(atoi(stData.argv[2])>0);
	}
	
	
	return 0;
}
//pega_debug debug sch xx xx
//pega_debug debug sch 1 1
static int pga_debug_command_schedule(stMsgQSubCmd_t stData)
{
	uint8_t  eEvent = 0;
	uint16_t wDelayTime = 1; //100ms
		
	if (stData.argc < 2)
	{
		fprintf(stderr, "Command error.[%d]\n", stData.argc);		
		return 0;
	}

    eEvent = atoi(stData.argv[1]);
	
	if (stData.argc == 3)
	{
		wDelayTime = atoi(stData.argv[2]);
	}
		
	printf("Sch:(eEvent:%d, wDelayTime:%d)\n", eEvent, wDelayTime);		
	//_LOG_INFO("eEvent:%d, wDelayTime:%d", eEvent, wDelayTime);	
	
    if (wDelayTime > 0)
	{
		pega_schedule_Event_push(eEvent, wDelayTime);
	}
	
	return 0;
}
//==============================================================================
int pga_debug_command_exec(stMsgQSubCmd_t stData)
{
	int intIndex  = 0;
	int intResult = -1;
	
	stMsgQCmd_t *eCommand = m_eDebugCmdTable;
	   
	while (eCommand[intIndex].strCmd != NULL)
	   {		   
		   if (!strcmp(stData.argv[0], eCommand[intIndex].strCmd))
		   {
		     if(eCommand[intIndex].func != NULL)
			 {
				intResult = eCommand[intIndex].func(stData);
				break;
			 }
		   }
		   
		   intIndex++;
	   }

    if (intResult < 0)
	{
		pga_debug_command_help(stData);
	}		
	 
	return intResult;
}
//==============================================================================
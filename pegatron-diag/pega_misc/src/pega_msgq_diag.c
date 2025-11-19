/*******************************************************************************
* File Name: pega_msgq_diag.c
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
#include "pega_msgq_diag.h"
//==============================================================================
#include "main.h"
#include "pega_schedule.h"
#include "pega_nv_mode.h"
#include "pega_wifi.h"
//==============================================================================
#include "demo.h"
#include "pega_gpio.h"
#include "pega_als_opt300x.h"
#include "pega_amp_tas256x.h"
//==============================================================================
static int pga_diag_command_help(stMsgQSubCmd_t stData);
static int pga_diag_command_ircut(stMsgQSubCmd_t stData);
static int pga_diag_command_irled(stMsgQSubCmd_t stData);
static int pga_diag_command_spotled(stMsgQSubCmd_t stData);
static int pga_diag_command_led(stMsgQSubCmd_t stData);
static int pga_diag_command_led_r(stMsgQSubCmd_t stData);
static int pga_diag_command_led_g(stMsgQSubCmd_t stData);
static int pga_diag_command_led_b(stMsgQSubCmd_t stData);
static int pga_diag_command_als(stMsgQSubCmd_t stData);
static int pga_diag_command_audio(stMsgQSubCmd_t stData);
static int pga_diag_command_nfc(stMsgQSubCmd_t stData);
static int pga_diag_command_button(stMsgQSubCmd_t stData);
//==============================================================================
static const stMsgQCmd_t m_eDiagCmdTable[] = 
{	
    { "help",		"list diag commands",		&pga_diag_command_help},
	{ "ircut",		"ircut control",			&pga_diag_command_ircut},
	{ "irled",		"irled control",			&pga_diag_command_irled},
	{ "spotled",	"spotled light control",	&pga_diag_command_spotled},
	{ "led",		"led control",				&pga_diag_command_led},
	{ "led_r",		"led_r control",			&pga_diag_command_led_r},
	{ "led_g",		"led_g control",			&pga_diag_command_led_g},
	{ "led_b",		"led_b control",			&pga_diag_command_led_b},
	{ "als",		"als control",				&pga_diag_command_als},
	{ "audio",		"audio control",			&pga_diag_command_audio},	
	{ "nfc",		"nfc control",				&pga_diag_command_nfc},
	{ "button",		"button control",			&pga_diag_command_button},
	{ NULL, NULL, NULL },
};
//==============================================================================
//pega_debug diag help
static int pga_diag_command_help(stMsgQSubCmd_t stData)
{
	int intIndex = 0;
	
	stMsgQCmd_t *eCommand = m_eDiagCmdTable;
	
	printf("\n");
	
	while (eCommand[intIndex].strCmd != NULL)
	{
		printf("%16s \t %s\n", eCommand[intIndex].strCmd, eCommand[intIndex].doc);
		
		intIndex++;
	}
			
	return 0;
}
//==============================================================================
//pega_debug diag ircut day/night
static int pga_diag_command_ircut(stMsgQSubCmd_t stData)
{	
    if (stData.argc < 2)
	 {
        goto cmd_err;
	 }	
	 
	if (!strcmp(stData.argv[1],"day"))
	{
		Pega_Gpio_IRCut_Control(1);
	}
	else if (!strcmp(stData.argv[1],"night"))
	{
        Pega_Gpio_IRCut_Control(0);
	}	
    else	
	{
        goto cmd_err;
	}	
	
	PT_DIAG_SET(0);		
	return 0;

cmd_err:
	
	fprintf(stderr, "Command argc error.\n");
	PT_DIAG_SET(-1);
	
	return 0;
}
//pega_debug diag irled on/off
//pega_debug diag irled duty 1~100
static int pga_diag_command_irled(stMsgQSubCmd_t stData)
{
	
	if (stData.argc < 2)
	 {
        goto cmd_err;
	 }	
	 
	if (!strcmp(stData.argv[1],"on"))
	{
		IO_IRLED_CTRL_ON();
	}
	else if (!strcmp(stData.argv[1],"off"))
	{
        IO_IRLED_CTRL_OFF();
	}
	else if (!strcmp(stData.argv[1],"duty"))
	{
		if (stData.argc == 3)
			Pega_pwm_config(PWM_CH6, PWM_IR_LED_FREQ, atoi(stData.argv[2]));
	}
    else	
	{
        goto cmd_err;
	}	
	
	PT_DIAG_SET(0);		
	return 0;

cmd_err:
	
	fprintf(stderr, "Command argc error.\n");
	PT_DIAG_SET(-1);
	
	return 0;
}
//pega_debug diag spotled on/off
static int pga_diag_command_spotled(stMsgQSubCmd_t stData)
{
	if (stData.argc < 2)
	 {
        goto cmd_err;
	 }	
	 
	if (!strcmp(stData.argv[1],"on"))
	{
		IO_SPOTLIGHT_CTRL_ON();
	}
	else if (!strcmp(stData.argv[1],"off"))
	{
        IO_SPOTLIGHT_CTRL_OFF();
	}	
    else	
	{
        goto cmd_err;
	}	
	
	PT_DIAG_SET(0);		
	return 0;

cmd_err:
	
	fprintf(stderr, "Command argc error.\n");
	PT_DIAG_SET(-1);
			
	return 0;
}
//pega_debug diag led on/off
static int pga_diag_command_led(stMsgQSubCmd_t stData)
{
	if (stData.argc < 2)
	 {
        goto cmd_err;
	 }	
	 
	if (!strcmp(stData.argv[1],"on"))
	{
		IO_LED_R_CTRL_ON();
		IO_LED_G_CTRL_ON();
		IO_LED_B_CTRL_ON();
	}
	else if (!strcmp(stData.argv[1],"off"))
	{
        IO_LED_R_CTRL_OFF();
		IO_LED_G_CTRL_OFF();
		IO_LED_B_CTRL_OFF();
	}	
    else	
	{
        goto cmd_err;
	}	
	
	PT_DIAG_SET(0);		
	return 0;

cmd_err:
	
	fprintf(stderr, "Command argc error.\n");
	PT_DIAG_SET(-1);
			
	return 0;
}
//pega_debug diag led_r on/off
static int pga_diag_command_led_r(stMsgQSubCmd_t stData)
{
	if (stData.argc < 2)
	 {
        goto cmd_err;
	 }	
	 
	if (!strcmp(stData.argv[1],"on"))
	{
		IO_LED_R_CTRL_ON();		
	}
	else if (!strcmp(stData.argv[1],"off"))
	{
        IO_LED_R_CTRL_OFF();		
	}	
    else	
	{
        goto cmd_err;
	}	
	
	PT_DIAG_SET(0);		
	return 0;

cmd_err:
	
	fprintf(stderr, "Command argc error.\n");
	PT_DIAG_SET(-1);
			
	return 0;
}
//pega_debug diag led_g on/off
static int pga_diag_command_led_g(stMsgQSubCmd_t stData)
{
	if (stData.argc < 2)
	 {
        goto cmd_err;
	 }	
	 
	if (!strcmp(stData.argv[1],"on"))
	{
		IO_LED_G_CTRL_ON();		
	}
	else if (!strcmp(stData.argv[1],"off"))
	{
        IO_LED_G_CTRL_OFF();		
	}	
    else	
	{
        goto cmd_err;
	}	
	
	PT_DIAG_SET(0);		
	return 0;

cmd_err:
	
	fprintf(stderr, "Command argc error.\n");
	PT_DIAG_SET(-1);
			
	return 0;
}
//pega_debug diag led_b on/off
static int pga_diag_command_led_b(stMsgQSubCmd_t stData)
{
	if (stData.argc < 2)
	 {
        goto cmd_err;
	 }	
	 
	if (!strcmp(stData.argv[1],"on"))
	{
		IO_LED_B_CTRL_ON();		
	}
	else if (!strcmp(stData.argv[1],"off"))
	{
        IO_LED_B_CTRL_OFF();		
	}	
    else	
	{
        goto cmd_err;
	}	
	
	PT_DIAG_SET(0);		
	return 0;

cmd_err:
	
	fprintf(stderr, "Command argc error.\n");
	PT_DIAG_SET(-1);
			
	return 0;
}
//pega_debug diag als
static int pga_diag_command_als(stMsgQSubCmd_t stData)
{
	if (stData.argc < 2)
	 {
        goto cmd_err;
	 }	
	 
	if (!strcmp(stData.argv[1],"id")) //als id
	{
		PT_DIAG_GET(0, "%04x", OPT300x_Device_ID_Get());
		goto fun_end;
	}
	else if (!strcmp(stData.argv[1],"day")) //als day
	{			
			
	}
	else if (!strcmp(stData.argv[1],"night")) //als night
	{			
			
	}
	else if (!strcmp(stData.argv[1],"value")) //als value
	{			
		PT_DIAG_GET(0, "%.3lf", Pega_NV_Mode_Diag_Lux_Get());
		goto fun_end;
	}
	else if (!strcmp(stData.argv[1],"calidata")) //als calidata
	{			
			
	}
	else if (!strcmp(stData.argv[1],"gain")) //als gain
	{			
			
	}
	else if (!strcmp(stData.argv[1],"on")) //als on
	{			
		Pega_Misc_ALS_Disable_Control(0);
	}
	else if (!strcmp(stData.argv[1],"off")) //als off
	{			
		Pega_Misc_ALS_Disable_Control(1);
	}
    else	
	{
        goto cmd_err;
	}	
	
	PT_DIAG_SET(0);		
	return 0;

fun_end:
    return 0;
	
cmd_err:
	
	fprintf(stderr, "Command argc error.\n");
	PT_DIAG_SET(-1);
			
	return 0;
}
//diag audio init
//diag audio volume
static int pga_diag_command_audio(stMsgQSubCmd_t stData)
{
	if (stData.argc < 2)
	 {
        goto cmd_err;
	 }	
	 
	if (!strcmp(stData.argv[1],"init")) //audio init xx
	{		
        if (stData.argc == 3)	
			Pega_AMP_Initialize(1, atof(stData.argv[2]));
		else
			Pega_AMP_Initialize(1, 12.0);
	}
	else if (!strcmp(stData.argv[1],"volume")) //audio volume xx (xx:8.5 ~ 22.0)	
	{		     
		 if (stData.argc == 3)//stData.argv[2] != NULL	 
          {
			 Pega_AMP_Volume_Control(atof(stData.argv[2]));
		   }
         else		   
		  {
		  	 PT_DIAG_GET(0, "%2.1f(dB)", Pega_AMP_Volume_Get());	
			 return 0;
		  }		   
	}		
    else	
	{
        goto cmd_err;
	}	
	
	PT_DIAG_SET(0);		
	return 0;

cmd_err:
	
	fprintf(stderr, "Command argc error.\n");
	PT_DIAG_SET(-1);
			
	return 0;
}
//pega_debug diag nfc
static int pga_diag_command_nfc(stMsgQSubCmd_t stData)
{
	if (stData.argc < 2)
	 {
        goto cmd_err;
	 }	
	 
	if (!strcmp(stData.argv[1],"type")) //nfc type
	{
		PT_DIAG_GET(0, "%s",  (char*)Page_GetNFCInfo(Diag_NFC_Type));		
	}
	else if (!strcmp(stData.argv[1],"uid")) //nfc uid
	{
		PT_DIAG_GET(0, "%s",  (char*)Page_GetNFCInfo(Diag_NFC_UID));
	}
	else if (!strcmp(stData.argv[1],"data")) //nfc data
	{
		PT_DIAG_GET(0, "%s",  (char*)Page_GetNFCInfo(Diag_NFC_Data));
	}
	else if (!strcmp(stData.argv[1],"clear")) //nfc clear
	{
		Page_ClearNFCBuf();
		PT_DIAG_SET(0);		
		return 0;
	}
    else	
	{
        goto cmd_err;
	}	
		
	return 0;

cmd_err:
	
	fprintf(stderr, "Command argc error.\n");
	PT_DIAG_SET(-1);
						
	return 0;
}
//pega_debug diag button get
static int pga_diag_command_button(stMsgQSubCmd_t stData)
{
	if (stData.argc < 2)
	 {
        goto cmd_err;
	 }	
	 
	if (!strcmp(stData.argv[1],"get")) //button get
	{
		PT_DIAG_GET(0, "%s", (Pega_Gpio_Read(IO_I_SYNC_BUTTON) == 0) ? "Pressed" : "Not Pressed");				
	}
    else	
	{
        goto cmd_err;
	}	
		
	return 0;

cmd_err:
	
	fprintf(stderr, "Command argc error.\n");
	PT_DIAG_SET(-1);
			
	return 0;
}
//==============================================================================
//diag ircut day/night
//diag irled on/off/duty
//diag spotled on/off
//diag led on/off
//diag led_r on/off
//diag led_g on/off
//diag led_b on/off
//diag als id
//diag als day
//diag als night
//diag als value
//diag als calidata
//diag als gain
//diag als on/off
//diag audio init
//diag audio init xx
//diag audio volume
//diag button get
//diag nfc type
//diag nfc uid
//diag nfc data
//diag nfc clear
int pga_diag_command_exec(stMsgQSubCmd_t stData)
{
	int intIndex  = 0;
	int intResult = -1;
	
	stMsgQCmd_t *eCommand = m_eDiagCmdTable;
	   
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

    return intResult;	
}
//==============================================================================
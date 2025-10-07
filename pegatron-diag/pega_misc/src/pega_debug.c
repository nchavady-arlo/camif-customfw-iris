/*******************************************************************************
* File Name: pega_debug.c
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
//==============================================================================
#include "pega_debug.h"
//==============================================================================
#include "main.h"
#include "pega_schedule.h"
#include "pega_nv_mode.h"
#include "pega_gpio.h"
#include "pega_wifi.h"
#include "pega_misc_diag.h"
//==============================================================================
#include "pega_als_opt300x.h"
#include "pega_amp_tas256x.h"
//==============================================================================
#include "pega_motor_awd8833.h"
//==============================================================================
static uint8_t m_bDebugMsgEn = 0; //pega_misc_dbg set 1 0/1
//==============================================================================
static stDebugCmdType m_eDebugCmd;
//==============================================================================
//pega_misc_dbg info xx
static void Pega_debug_print_info_processer(int value)
{
	 switch(value)
	 {
		 case 1: //pega_misc_dbg info 1
		        Pega_Misc_Data_Print();	
		        break;
		 case 2: //pega_misc_dbg info 2
		        pega_schedule_Data_Info_Print();	
		        break;
         case 3: //pega_misc_dbg info 3
		        Pega_diag_msgq_Data_Info_Print();	
		        break;
		 case 4: //pega_misc_dbg info 4
		        Pega_Gpio_Data_Info_Print();	
		        break;
		 case 5: //pega_misc_dbg info 5
		        OPT300x_Data_Print();	
		        break;
		 case 6: //pega_misc_dbg info 6
		        OPT300x_Register_Info_Print();	
		        break;
		 case 7: //pega_misc_dbg info 7
		        Pega_AMP_Reg_Data_Print();	
		        break;
		 case 8: //pega_misc_dbg info 8
		        Pega_NV_Mode_Data_Print();	
		        break;			
		 //==============================================================================
		 default:
	            printf("(%s) value error! (%d)\n", __func__, value); 
	            break;
	 }
	
}
//pega_misc_dbg set xx xx xx
static void Pega_debug_command_set_processer(int eCmdSetId, int value1, int value2)
{
	 switch(eCmdSetId)
	 {
		 case 1: //pega_misc_dbg set 1
		        Pega_diag_msgq_debug_trigger();	
		        break;
				
		 case 2: //pega_misc_dbg set 2
		        Pega_Misc_Update_WiFi_interface();	
		        break;
		 
         case 3: //pega_misc_dbg set 3
		        Pega_Gpio_HW_Amp_Reset();	
		        break;
		 
         case 4: //pega_misc_dbg set 4 0/1
		        Pega_Misc_Burn_Enable_Control(value1>0);
		        break;
				
		 case 5: //pega_misc_dbg set 5 0/1
		        Pega_Misc_ALS_Disable_Control(value1>0);
		        break;
				
		 case 6: //pega_misc_dbg set 6 0/1
		        Pega_Misc_WiFi_Disable_Control(value1>0);
		        break;		
		 //==============================================================================
		 default:
	            printf("(%s) eCmdSetId error! (%d)\n", __func__, eCmdSetId); 
	            break;
	 }
	
}
//pega_misc_dbg get xx xx xx
static void Pega_debug_command_get_processer(int eCmdSetId, int value1, int value2)
{
	 switch(eCmdSetId)
	 {
		 case 1: //pega_misc_dbg set 1
		        //Pega_Misc_Data_Print();	
		        break;
		 
		 //==============================================================================
		 default:
	            printf("(%s) eCmdSetId error! (%d)\n", __func__, eCmdSetId); 
	            break;
	 }
	
}
//pega_misc_dbg set id val1 val2 val3 val4
//pega_misc_dbg set 1 1 2 3 4 
static int Pega_debug_commands_processer(stDebugCmdMsgType *pData)
{    
   unsigned char eEvent = schEVT_None; 
   
   if (m_bDebugMsgEn > 0)
   {
	    printf("\n"); 
		printf("(%s)pData->eCmdId = %d\n", __func__, pData->eCmdId); 
		printf("(%s)pData->value1 = %d\n", __func__, pData->value1);    
		printf("(%s)pData->value2 = %d\n", __func__, pData->value2);
		printf("(%s)pData->value3 = %d\n", __func__, pData->value3);
		printf("(%s)pData->value4 = %d\n", __func__, pData->value4);
   }
   
   switch(pData->eCmdId)
   {	 
      //pega_misc_dbg debug 1 0/1
      case Debug_CmdId_Debug_Msg_Control:
	       m_bDebugMsgEn = pData->value1;
		   printf("(%s) (m_bDebugMsgEn = %d)\n", __func__, m_bDebugMsgEn); 
	       return 0;
	  //==============================================================================
	  //pega_misc_dbg info xx
	  case Debug_CmdId_Debug_Print:
	       m_eDebugCmd.eCmdId = SysDebug_CmdId_Info;  
		   m_eDebugCmd.value1 = pData->value1;
		   eEvent = schEVT_System_debug_info_Execution;
	       break;
	  //pega_misc_dbg set xx xx xx	   
	  case Debug_CmdId_Debug_Set:
	       m_eDebugCmd.eCmdId = SysDebug_CmdId_Set;  
		   m_eDebugCmd.value1 = pData->value1;
		   m_eDebugCmd.value2 = pData->value2;
		   m_eDebugCmd.value3 = pData->value3;
		   eEvent = schEVT_System_debug_info_Execution;
	       break;
	  //pega_misc_dbg get
	  case Debug_CmdId_Debug_Get:
	       m_eDebugCmd.eCmdId = SysDebug_CmdId_Get;
		   m_eDebugCmd.value1 = pData->value1;
		   eEvent = schEVT_System_debug_info_Execution;
	       break;
	   //pega_misc_dbg motor
	  case Debug_CmdId_Debug_Motor:
	       m_eDebugCmd.eCmdId = SysDebug_CmdId_Motor;
		   m_eDebugCmd.value1 = pData->value1;
		   m_eDebugCmd.value2 = pData->value2;
		   m_eDebugCmd.value3 = pData->value3;
		   eEvent = schEVT_System_debug_info_Execution;
	       break;	   
	  //==============================================================================	   
      case Debug_CmdId_Schedule_Event:
	       eEvent = pData->value1;
		   printf("(%s) (eEvent = %d)\n", __func__, eEvent);		  
	       break;
	 
	  //==============================================================================   
      default:
	       printf("(%s) cmd error! (eCmdId = %d)\n", __func__, pData->eCmdId); 
	       return -1;
   }	   
   
   pega_schedule_Event_push(eEvent, SCH_100ms);
   
   return 1;
}
//==============================================================================
static void* Pega_debug_msgq_Listen_Handle(void* argv)
{	
	int msgid,loopStop;
	
	stDebugCmdMsgType msgST;
  
	(void)argv;
	pthread_detach(pthread_self());    
	prctl(PR_SET_NAME, THREAD_PROC_1); //set the thread name
	
    loopStop = 0;
    
    printf("%s\n", __func__);
  			
	if (0 > (msgid = msgget(MSGQ_MISC_DEBUG_QUEUE_KEY, IPC_CREAT | 0666)))
	{
		printf("msgget(): error id!\n");		
		loopStop = 1;			
		goto func_end;
	}
	
	do
	{		
		if ((msgrcv(msgid, (void *)&msgST, sizeof(stDebugCmdMsgType) - sizeof(int), 1, 0)) == -1)
        {
            //INFO_LOG("[ERR] ST_Mi_ISP_Ctrl_thread Receive message error -1");

            printf("Receive isp message error!\n");
        }
        else
        {
            //INFO_LOG("Msg ISP MsgmiType %d, Msgmicommand %d, ctrl_val %d, ctrl_data %d", MIMessageData.iType, MIMessageData.command, MIMessageData.ctrl_val, MIMessageData.ctrl_data);
            
			Pega_debug_commands_processer(&msgST);
						
			//fprintf(stdout, "%s(), (11) msgST.eSIM_Status = %d, msgST.bIsIP_Ready=%d", __func__, msgST.eSIM_Status, msgST.bIsIP_Ready);
        }
    
		//printf("evt_type=%d\n", msgST.evt_type);
        		
	}while(loopStop == 0);

func_end:	
	printf("End of Pega_debug_msgq_Listen_Handle()\n");
	
	return 0;
}
//==============================================================================
void Pega_debug_msgq_handler_start(void)
{
	pthread_t thread_id;
    
    printf("%s(m_bDebugMsgEn=%d)\n", __FUNCTION__, m_bDebugMsgEn);
    
    pthread_create(&thread_id, NULL, &Pega_debug_msgq_Listen_Handle, NULL);
	
}
//==============================================================================
//pega_misc_dbg info xx
void Pega_debug_schedule_command(void)
{
	 switch(m_eDebugCmd.eCmdId)
	 { 
	     case SysDebug_CmdId_Info:
		   	  Pega_debug_print_info_processer(m_eDebugCmd.value1); 
			  break;
		 
         case SysDebug_CmdId_Set:
              Pega_debug_command_set_processer(m_eDebugCmd.value1, m_eDebugCmd.value2, m_eDebugCmd.value3);		 
			  break;
				
		 case SysDebug_CmdId_Get:
		      Pega_debug_command_get_processer(m_eDebugCmd.value1, m_eDebugCmd.value2, m_eDebugCmd.value3);
			  break;
			  
		 case SysDebug_CmdId_Motor:
		      awd8833c_debug(m_eDebugCmd.value1, m_eDebugCmd.value2, m_eDebugCmd.value3);
			  break;
			  
		 default:		        
			{
				printf("\n"); 
				printf("(%s)m_eDebugCmd.eCmdId = %d\n", __func__, m_eDebugCmd.eCmdId); 
				printf("(%s)m_eDebugCmd.value1 = %d\n", __func__, m_eDebugCmd.value1);    
				printf("(%s)m_eDebugCmd.value2 = %d\n", __func__, m_eDebugCmd.value2);
				printf("(%s)m_eDebugCmd.value3 = %d\n", __func__, m_eDebugCmd.value3);				
			}
                break;		 
	 }     
}
//==============================================================================
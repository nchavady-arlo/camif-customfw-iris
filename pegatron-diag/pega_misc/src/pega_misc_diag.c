/*******************************************************************************
* File Name: pega_misc_diag.c
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
#include <sys/msg.h>
//==============================================================================
#include "main.h"
//==============================================================================
#include "pega_misc_diag.h"
#include "pega_schedule.h"
#include "pega_nv_mode.h"
//==============================================================================
#include "pega_gpio.h"
#include "pega_als_opt300x.h"
#include "pega_amp_tas256x.h"
//==============================================================================
static stDiagCmdStrType m_stDiagCmd;
//==============================================================================
static int Pega_diag_commands_processer(stDiagMsgqCmdType *pData)
{      
   if (m_stDiagCmd.bDebugEn > 0)
   {
		printf("\n(%s)pData->eCmdId = %d", __func__, pData->eCmdId); 
		printf("\n(%s)pData->value1 = %d", __func__, pData->value1);    
		printf("\n(%s)pData->value2 = %d", __func__, pData->value2);
		printf("\n(%s)pData->value3 = %d", __func__, pData->value3);
		printf("\n(%s)pData->fval1  = %.3lf", __func__, pData->fval1);
		printf("\n(%s)pData->fval2  = %.3lf", __func__, pData->fval2);
		printf("\n"); 
   }
   
   if (m_stDiagCmd.wCmdCnt > 0)
   {
	   printf("\n-----------------------");
	   printf("\nDiag cmd(%d) is not processing finished![%d,%d,%d,cnt:%d]", m_stDiagCmd.eCmdId, m_stDiagCmd.value1, m_stDiagCmd.value2, m_stDiagCmd.value3, m_stDiagCmd.wCmdCnt);
	   printf("\n-----------------------\n");	   
   }
   
   m_stDiagCmd.eCmdId = pData->eCmdId;
   m_stDiagCmd.value1 = pData->value1;
   m_stDiagCmd.value2 = pData->value2;
   m_stDiagCmd.value3 = pData->value3;
   m_stDiagCmd.fval1  = pData->fval1;
   m_stDiagCmd.fval2  = pData->fval2;
   
   m_stDiagCmd.wCmdCnt++;
   
   pega_schedule_Event_push(schEVT_System_diag_Cmd_Execution, SCH_100ms);
   
   return 1;
}
//==============================================================================
static void* Pega_diag_msgq_Listen_Handle(void* argv)
{	
	int msgid,loopStop;
	
	stDiagMsgqCmdType msgST;
  
	(void)argv;
	pthread_detach(pthread_self());    
	prctl(PR_SET_NAME, THREAD_PROC_3); //set the thread name
	
    loopStop = 0;
    
    printf("\n%s\n", __func__);
  			
	if (0 > (msgid = msgget(MSGQ_MISC_DIAG_QUEUE_KEY, IPC_CREAT | 0666)))
	{
		printf("\nmsgget(): error id!\n");		
		loopStop = 1;			
		goto func_end;
	}
	
	do
	{		
		if ((msgrcv(msgid, (void *)&msgST, sizeof(stDiagMsgqCmdType) - sizeof(int), 1, 0)) == -1)
        {
            //INFO_LOG("[ERR] ST_Mi_ISP_Ctrl_thread Receive message error -1");

            printf("\nReceive isp message error!\n");
        }
        else
        {
            //INFO_LOG("Msg ISP MsgmiType %d, Msgmicommand %d, ctrl_val %d, ctrl_data %d", MIMessageData.iType, MIMessageData.command, MIMessageData.ctrl_val, MIMessageData.ctrl_data);
            
			Pega_diag_commands_processer(&msgST);
						
			//fprintf(stdout, "%s(), (11) msgST.eSIM_Status = %d, msgST.bIsIP_Ready=%d", __func__, msgST.eSIM_Status, msgST.bIsIP_Ready);
        }
    
		//printf("evt_type=%d\n", msgST.evt_type);
        		
	}while(loopStop == 0);

func_end:

	printf("End of Pega_debug_msgq_Listen_Handle()\n");
	
	return 0;
}
//==============================================================================
void Pega_diag_msgq_handler_start(void)
{
	 pthread_t thread_id;
     	 
	 printf("\n%s\n", __func__);
	 
	 memset(&m_stDiagCmd, 0, sizeof(m_stDiagCmd));
	 	 
     pthread_create(&thread_id, NULL, &Pega_diag_msgq_Listen_Handle, NULL);
	 
}
//==============================================================================
int Pega_Diag_cmd_out2Telnet(int index)
{
	int fd;
	char tempPath[64];

	memset(tempPath,0,sizeof(tempPath));	
    
	if (0xFF == index )
	  {
		  sprintf(tempPath,"/dev/console");
		}  
	else
	  {
		  sprintf(tempPath,"/dev/pts/%d", index);
		}  

	if((fd = open(tempPath, O_RDONLY | O_WRONLY)) < 0)
	{
		printf("open file failed\r\n");
		return -1;
	}

	dup2(fd, 1);
	close(fd);
	
	return 0;
}
//==============================================================================
//pega_misc_dbg info 2 17
void Pega_diag_msgq_Cmd_Execution(void)
{
	 if (m_stDiagCmd.bDebugEn > 0)
	 {
		 printf("\n%s(eCmdId:%d)\n", __func__, m_stDiagCmd.eCmdId);
	 }
			 
	 switch(m_stDiagCmd.eCmdId)
	 {
		 case Diag_CmdId_HW_Version: //diag ver hw
		      //PT_DIAG_GET(0, "%d", Pega_Gpio_HW_Version_Get());
		      break;		  
		 case Diag_CmdId_ICR_Control: //diag icr day/night
		      Pega_Gpio_IRCut_Control(m_stDiagCmd.value1);
		      PT_DIAG_SET(0);
		      break;
		 case Diag_CmdId_IRLed_Control: //diag irled on/off
		      if (m_stDiagCmd.value1 > 0)
			  {
				  IO_IRLED_CTRL_ON();
			  }
			  else
			  {
				  IO_IRLED_CTRL_OFF();
			  }  
		      PT_DIAG_SET(0);
		      break;
		 case Diag_CmdId_IRLed_PWM_Control:	//diag irled set xx	      
			  Pega_pwm_config(PWM_CH6, PWM_LED_FREQ, m_stDiagCmd.value1); 
		      PT_DIAG_SET(0);
		      break;
		 case Diag_CmdId_SpotLightLed_Control: //diag spotled on/off
		      if (m_stDiagCmd.value1 > 0)
			  {
				  IO_SPOTLIGHT_CTRL_ON();
			  }
			  else
			  {
				  IO_SPOTLIGHT_CTRL_OFF();
			  }  
		      PT_DIAG_SET(0);
		      break;
		 case Diag_CmdId_SpotLightLed_PWM_Control:	//diag spotled set xx	      
			  Pega_pwm_config(PWM_CH7, PWM_LED_FREQ, m_stDiagCmd.value1); 
		      PT_DIAG_SET(0);
		      break;	  
		 case Diag_CmdId_Led_Control:		      
		      switch(m_stDiagCmd.value1)
			  {
				   case 1: //diag led_r on/off
				          if (m_stDiagCmd.value2 > 0)
						  {
							  IO_LED_R_CTRL_ON();
						  }
						  else
						  {
							  IO_LED_R_CTRL_OFF();
						  }  
                          break;
                   case 2: //diag led_g on/off
				          if (m_stDiagCmd.value2 > 0)
						  {
							  IO_LED_G_CTRL_ON();
						  }
						  else
						  {
							  IO_LED_G_CTRL_OFF();
						  }  
                          break;
                   case 3: //diag led_b on/off
				          if (m_stDiagCmd.value2 > 0)
						  {
							  IO_LED_B_CTRL_ON();
						  }
						  else
						  {
							  IO_LED_B_CTRL_OFF();
						  }  
                          break; 
				   case 4: //diag led on
				          IO_LED_R_CTRL_ON();
						  IO_LED_G_CTRL_ON();
						  IO_LED_B_CTRL_ON();
                          break;  
				   case 5: //diag led off
				          IO_LED_R_CTRL_OFF();
						  IO_LED_G_CTRL_OFF();
						  IO_LED_B_CTRL_OFF();
                          break;  						  
                   default:
				          printf("\nDiag LED control index(%d) error!", m_stDiagCmd.value1);
                          break;				   
			  }			  
		      PT_DIAG_SET(0);
		      break;
		 //============================================================================== 
         case Diag_CmdId_ALS_Control:
		      switch(m_stDiagCmd.value1)
			  {
				  case 1: //diag als id
				         PT_DIAG_GET(0, "%04x", OPT300x_Device_ID_Get());
                         break;
				  case 2: //diag als day
				         //Pega_Diag_Force_Day_Night_Mode(FALSE);
						 PT_DIAG_SET(0);
                         break;
                  case 3: //diag als night
				         //Pega_Diag_Force_Day_Night_Mode(TRUE);
						 PT_DIAG_SET(0);
                         break; 		 
				  case 4: //diag als value
				         //PT_DIAG_GET(0, "%.3lf", OPT300x_Diag_Lux_Value_Get());
						 PT_DIAG_GET(0, "%.3lf", Pega_NV_Mode_Diag_Lux_Get());
                         break;
				  case 5: //diag als calidata xx xx
				          if (m_stDiagCmd.fval1 != 0.0)
						  {
                              //Pega_NV_Mode_NVModeSet_LuxCal_Set(m_stDiagCmd.fval1);
						  }	
						  
						  if (m_stDiagCmd.fval2 != -1000.0)
						  {
                              //Pega_NV_Mode_NVModeSet_LuxOffset_Set(m_stDiagCmd.fval2);
						  }							  
				          PT_DIAG_SET(0);	
                          break;
						  
				  case 6: //diag als on
				         Pega_Misc_ALS_Disable_Control(0);
				         PT_DIAG_SET(0);			 
                         break;
				  case 7: //diag als off
				         Pega_Misc_ALS_Disable_Control(1);
				         PT_DIAG_SET(0);			 
                         break;
                  case 8: //diag als calidata_Get
				          //PT_DIAG_GET(0, "%f", Pega_NV_Mode_NVModeSet_LuxCal_Get());
						  break;	
						 
				  default:
                         break;				  
			  }
		      break;
			  
		 case Diag_CmdId_Audio_Control:
		      switch(m_stDiagCmd.value1)
			  {
				  case 1: //diag audio init				         
				         Pega_AMP_Initialize(1, 12.0);
						 //system("tas2563 cfg");
				         PT_DIAG_SET(0);			 
                         break;	
				  case 2: //diag audio volume xx (xx:8.5 ~ 22.0)			         
				         Pega_AMP_Volume_Control(m_stDiagCmd.fval1);
				         PT_DIAG_SET(0);			 
                         break;	
						 
				  default:
                         break;				  
			  }
		      break;	  
		 //============================================================================== 
         case Diag_CmdId_Button_Control: 
		      switch(m_stDiagCmd.value1)
			  {				   
				   case 1: //diag button get
				           PT_DIAG_GET(0, "%s", (Pega_Gpio_Read(IO_I_SYNC_BUTTON) == 0) ? "Pressed" : "Not Pressed");						   
                           //PT_DIAG_GET(0, "%s", (Pega_Gpio_Key_Button_Status_Get(KEY_DET_BT_RESET) == 1) ? "On" : "Off");						   
						   break;
						   
				   default:
				           printf("\nDiag button control index(%d) error!", m_stDiagCmd.value1);
				           PT_DIAG_SET(-1);	
				           break;
			  }
             				  
			  break;
		 //============================================================================== 	  
		 case Diag_CmdId_wifi_Control:
		 {
              switch(m_stDiagCmd.value1)
			  {
				  case 1: //diag wifi interface
				       Pega_Misc_Diag_wifi_interface_Print();
				       break;
			  }			  
		 }	
              break;		 
		 //============================================================================== 			  
		 default:
              break;		 
	 }
	 
	 if (m_stDiagCmd.wCmdCnt > 0)
	 {
		 m_stDiagCmd.wCmdCnt--;
	 }
}
//==============================================================================
//pega_misc_dbg set 1
void Pega_diag_msgq_debug_trigger(void)
{
	 m_stDiagCmd.bDebugEn = !m_stDiagCmd.bDebugEn;
	 printf("-----------------------\n");
	 printf("m_stDiagCmd.bDebugEn    => %d\n", m_stDiagCmd.bDebugEn);	 
	 printf("-----------------------\n");	   
}
//pega_misc_dbg info 3
void Pega_diag_msgq_Data_Info_Print(void)
{     	     
	 printf("\n-----------------------\n");
	 printf("m_stDiagCmd.eCmdId      = %d\n", m_stDiagCmd.eCmdId);
	 printf("m_stDiagCmd.value1      = %d\n", m_stDiagCmd.value1);
	 printf("m_stDiagCmd.value2      = %d\n", m_stDiagCmd.value2);
	 printf("m_stDiagCmd.value3      = %d\n", m_stDiagCmd.value3);
	 printf("m_stDiagCmd.fval1       = %.3lf\n",m_stDiagCmd.fval1);
	 printf("m_stDiagCmd.fval2       = %.3lf\n",m_stDiagCmd.fval2);
	 printf("m_stDiagCmd.wCmdCnt     = %d\n", m_stDiagCmd.wCmdCnt);	
	 printf("-----------------------\n");
	 printf("m_stDiagCmd.bDebugEn    = %d\n", m_stDiagCmd.bDebugEn);	 
	 printf("-----------------------\n");	   
}
//==============================================================================
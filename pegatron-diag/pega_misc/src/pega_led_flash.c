/*******************************************************************************
* File Name: pega_led_flash.c
*
*******************************************************************************/
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/un.h>
#include <signal.h>
#include <stddef.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/msg.h>
//==============================================================================
#include "main.h"
//==============================================================================
#include "pega_gpio.h"
#include "pega_led_flash.h"
//==============================================================================
static uint8_t m_eLED_State      = LED_State_None;
static uint8_t m_eLED_State_prev = LED_State_None;
//==============================================================================
static stLedFlashingType m_stLED;
//==============================================================================
static pthread_t m_thread_id = 0;
//==============================================================================
static void* Pega_led_flash_TaskHandler(void* argv)
{			 
	 (void)argv;
	 
	 pthread_detach(pthread_self());
	 prctl(PR_SET_NAME, THREAD_PROC_5); //set the thread name
	 
     m_stLED.bIsLedFlashing = 1;
	 
	 //printf("\n%s\n", __func__);
	 
	 if (m_stLED.u16Period < 50)
	 {
		 m_stLED.u16Period = 100;
	 }	 	 	 	 
	 
	 while(1)
	 {
          if (m_stLED.eLedFlashing_Color & LED_FLASH_COLOR_R)
		  {
		      IO_LED_R_CTRL_ON();
		  }
		  
		  if (m_stLED.eLedFlashing_Color & LED_FLASH_COLOR_G) 		 
		  {
		      IO_LED_G_CTRL_ON();
		  }
		  
		  if (m_stLED.eLedFlashing_Color & LED_FLASH_COLOR_B) 		 
		  {
		      IO_LED_B_CTRL_ON();
		  }
		  
		  usleep(1000*500); //0.5s
		  
		  if (m_stLED.eLedFlashing_Color & LED_FLASH_COLOR_R) 		 
		  {
		      IO_LED_R_CTRL_OFF();
		  }
		  
		  if (m_stLED.eLedFlashing_Color & LED_FLASH_COLOR_G) 		 
		  {
		      IO_LED_G_CTRL_OFF();
		  }
		  
		  if (m_stLED.eLedFlashing_Color & LED_FLASH_COLOR_B) 		 
		  {
		      IO_LED_B_CTRL_OFF();
		  }
		  
		  usleep(1000 * m_stLED.u16Period); //0.5s
	 }	 
	 	 	 
     m_stLED.bIsLedFlashing	= 0;
	 
	 return 0;
}
//==============================================================================
//pega_camcli misc set led_flash 0 1 500
void Pega_led_flash_handler_Start(void)
{
	 //pthread_t thread_id;
	                
     //printf("\n%s", __FUNCTION__);	
     pthread_create(&m_thread_id, NULL, &Pega_led_flash_TaskHandler, NULL);     
}

int Pega_led_flash_Led_Flashing_Check(void)
{
	return (m_stLED.bIsLedFlashing > 0);
}
//pega_camcli misc set led_pattern 0 
int Pega_led_flash_Pattern_Set(uint16_t ePattern)
{	
	//INFO_LOG("[%s]ePattern=%d,m_thread_id=%d", __func__,ePattern, (int)m_thread_id);
	
	if (m_thread_id != 0)
	{
		pthread_cancel(m_thread_id);
		m_thread_id = 0;
	}
	
	usleep(1000 * 200); 
	
	IO_LED_R_CTRL_OFF();
	IO_LED_G_CTRL_OFF();
	IO_LED_B_CTRL_OFF();
	
    m_stLED.eLedFlashing_Pattern = ePattern;
	m_stLED.u16Period            = 500;
	
	switch(ePattern)
	{
		case LED_PATTERN_COLOR_RED:
		     IO_LED_R_CTRL_ON();			
		     return 0;
			 
		case LED_PATTERN_COLOR_GREEN:
		     IO_LED_G_CTRL_ON();			
		     return 0;
			 
		case LED_PATTERN_COLOR_BLUE:
		     IO_LED_B_CTRL_ON();			
		     return 0;	 
		
		case LED_PATTERN_COLOR_CYAN:
		     IO_LED_G_CTRL_ON();
			 IO_LED_B_CTRL_ON();
		     return 0;
		
        case LED_PATTERN_COLOR_MAGENTA:
		     IO_LED_R_CTRL_ON();
			 IO_LED_B_CTRL_ON();
		     return 0;
			 
		case LED_PATTERN_COLOR_YELLOW:
		     IO_LED_R_CTRL_ON();
			 IO_LED_G_CTRL_ON();
		     return 0;
			 
        case LED_PATTERN_COLOR_WHITE:
		     IO_LED_R_CTRL_ON();
			 IO_LED_G_CTRL_ON();
			 IO_LED_B_CTRL_ON();
		     return 0;
			 
		case LED_PATTERN_FLASHING_RED:		     
			 m_stLED.eLedFlashing_Color   = LED_FLASH_COLOR_R;			 
		     break;
		
		case LED_PATTERN_FLASHING_GREEN:		     
			 m_stLED.eLedFlashing_Color   = LED_FLASH_COLOR_G;			 
		     break;
		
		case LED_PATTERN_FLASHING_BLUE:		     
			 m_stLED.eLedFlashing_Color   = LED_FLASH_COLOR_B;			 
		     break;
		
		case LED_PATTERN_FLASHING_CYAN:		     
			 m_stLED.eLedFlashing_Color   = LED_FLASH_COLOR_G + LED_FLASH_COLOR_B;			 
		     break;		
        
		case LED_PATTERN_FLASHING_MAGENTA:		     
			 m_stLED.eLedFlashing_Color   = LED_FLASH_COLOR_R + LED_FLASH_COLOR_B;			 
		     break;
		
		case LED_PATTERN_FLASHING_YELLOW:		     
			 m_stLED.eLedFlashing_Color   = LED_FLASH_COLOR_R + LED_FLASH_COLOR_G;			 
		     break;
		
        case LED_PATTERN_FLASHING_WHITE: 		     
			 m_stLED.eLedFlashing_Color   = LED_FLASH_COLOR_R + LED_FLASH_COLOR_G + LED_FLASH_COLOR_B;			 
		     break;
			 
        default:
             return -1;	
	}
			
	Pega_led_flash_handler_Start();
	
	return 0;
}
//==============================================================================
static const stLedPatternPriorityType m_stLedPatternPriorityTbl[] =
{ //eLedPattery, sPriority	    
/*00*/{  LED_State_None,				 	0},
/*01*/{  LED_State_Off,				 		1},
/*02*/{  LED_State_System_Start_Up,		    5},
/*03*/{  LED_State_Wifi_Ready,		       10},
/*04*/{  LED_State_Wifi_off,		       10},
/*05*/{  LED_State_Wifi_fw_error,		   10},
/*16*/{  LED_State_Power_off,		 	  100},
};

static int Pega_led_flash_state_priority_get(const int ePattern)
{ 		   	   
	  int i, value = -1;
	  
	  stLedPatternPriorityType *pTemp_tbl = (stLedPatternPriorityType *)&m_stLedPatternPriorityTbl;
	  int sizeOfSt = sizeof(m_stLedPatternPriorityTbl)/sizeof(stLedPatternPriorityType);
	  	  	 	  
	  //printf("%s(sizeOfSt=%d,sADCVal=%d)\n", __FUNCTION__, sizeOfSt, sADCVal);
	   
	  for (i=0;i<sizeOfSt;i++)
	      {
	      	if (ePattern == pTemp_tbl->eLedPattery)
	      	  {
	      	  	value = pTemp_tbl->sPriority;
	      	  	break;
	      	  } 	
	      	
	      	pTemp_tbl++;        	    
	      }	
	   
	   //printf("%s(sTemperature=%d)\n", __FUNCTION__, m_stADC.wSysTemperature); 
      return value;	   
}
//==============================================================================
int Pega_led_flash_state_Set(uint16_t estate)
{
	uint16_t ePattern = LED_PATTERN_NONE;
	int sPriority_now, sPriority_Prev;
			
	if (m_eLED_State == estate)
	{
		return 1;
	}
	
	sPriority_now  = Pega_led_flash_state_priority_get(estate);
	sPriority_Prev = Pega_led_flash_state_priority_get(m_eLED_State);
	
    if (sPriority_now < 0)
	{
		printf("set LED type[%d] is invalid, priority < 0", estate);
		return -3;
	}
	
	//High priority, need to reboot system if no sim card.
	if (sPriority_Prev > sPriority_now)//	if (m_eLED_State == LED_PATTERN_SIM_CARD_ABSENT)
	{
		printf("set LED type[%d] not allowed, current type[%d] has high priority !!", estate, m_eLED_State);
		return -2;
	}
	
	if (m_eLED_State_prev != m_eLED_State)
	{
		m_eLED_State_prev = m_eLED_State;
	}
	
	m_eLED_State = estate;
		
	switch(m_eLED_State)
	{
		case LED_State_Off:
		     ePattern = LED_PATTERN_NONE;
		     break;
		case LED_State_System_Start_Up:
		     ePattern = LED_PATTERN_FLASHING_YELLOW;
		     break;
		case LED_State_Wifi_Ready:
		     ePattern = LED_PATTERN_COLOR_BLUE;
		     break;
		case LED_State_Wifi_off:
		     ePattern = LED_PATTERN_COLOR_YELLOW;
		     break;	 
		case LED_State_Wifi_fw_error:
		     ePattern = LED_PATTERN_COLOR_RED;
		     break;	 	 
		case LED_State_Power_off:
		     ePattern = LED_PATTERN_NONE;
		     break;	 
        //==============================================================================		
        default:
             break;
	}
	
	return Pega_led_flash_Pattern_Set(ePattern);
}

void Pega_led_flash_Chanel(void) 
{
	if (m_thread_id != 0)
	{
		pthread_cancel(m_thread_id);
		m_thread_id = 0;
	}
}

void Pega_led_flash_State_Clear(void) 
{
	 m_eLED_State      = LED_State_None;  
	 m_eLED_State_prev = LED_State_None;
}
//==============================================================================
//pega_misc_dbg info 6 8
void Pega_led_flash_Data_Print(void) 
{			
     int sPri_1, sPri_2;
	 
     sPri_1 = Pega_led_flash_state_priority_get(m_eLED_State);
	 sPri_2 = Pega_led_flash_state_priority_get(m_eLED_State_prev);
	 
     printf("\n-----------------------");
	 printf("\n m_thread_id                  = %d", (int)m_thread_id);	
	 printf("\n-----------------------");
	 printf("\n m_eLED_State(%03d)           = %d", sPri_1, m_eLED_State);	
	 printf("\n m_eLED_State_prev(%03d)      = %d", sPri_2, m_eLED_State_prev);	
	 printf("\n-----------------------");	 
	 printf("\n m_stLED.bIsLedFlashing       = %d", m_stLED.bIsLedFlashing);
	 printf("\n m_stLED.eLedFlashing_Pattern = %d", m_stLED.eLedFlashing_Pattern);
	 printf("\n m_stLED.eLedFlashing_Color   = %d", m_stLED.eLedFlashing_Color);
	 printf("\n m_stLED.u16Period            = %d", m_stLED.u16Period);
	 printf("\n-----------------------\n");	 
}
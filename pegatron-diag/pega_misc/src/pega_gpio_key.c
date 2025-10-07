/*******************************************************************************
* File Name: Pega_gpio_key.c
*
*******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <linux/input.h>
//#include <linux/gpio_keys.h>
//==============================================================================
#include "pega_gpio_key.h"
#include "pega_gpio.h"
#include "pega_schedule.h"
//==============================================================================
#define KEY_DBG(x)          x
//==============================================================================
static stKeyDetType  m_stKey[KEY_DET_BT_MAX];
static long Press_firstT;
static long Press_finalT;
//==============================================================================
static inline MMP_ULONG Pega_Gpio_Key_timespec_diff(MMP_UBYTE eKeyBT)
{
	  long long dwPressT, dwReleaseT;
	  
	  dwPressT = m_stKey[eKeyBT].Pressed_Time * 1000LL * 1000LL + m_stKey[eKeyBT].Pressed_Time_us;
	  
	  dwReleaseT = m_stKey[eKeyBT].Release_Time * 1000LL * 1000LL + m_stKey[eKeyBT].Release_Time_us;
	  
	  dwReleaseT = (dwReleaseT - dwPressT) / 1000;//ms (1000 * 1000); 	 
	  
	  //printf("\n %s [%d]",__func__, dwReleaseT);
	  
    return dwReleaseT;
}
/*
SYNC Button to Enter Debug Mode
Debug mode is a special mode on the device, where the SSH on the device must be enabled. On QA and Dev environments, SSH must be
enabled on the device by default. On Production and field trial environments, SSH must be enabled on the device only when the debug mode is
enabled on the device. To enable the debug mode, the user must have 5 clicks within 10 seconds period.
Time Period: 10 seconds
Number of clicks required to enable debug mode: 5 clicks.
Click: Each Click is defined as button press for the minimum of 30 ms (to tackle the switch debounce effect) and the maximum of 2999 ms. If the
button is not released before 3000 ms, this must not be considered as a click.
*/
static void Pega_Gpio_Key_Reset_processing(struct input_event key_ev)
{
	     if (key_ev.value == 1) // pressed
	       {
	         m_stKey[KEY_DET_BT_RESET].bKeyPressed     = TRUE;
	         m_stKey[KEY_DET_BT_RESET].bKeyPressedCnt++;
	         m_stKey[KEY_DET_BT_RESET].Pressed_Time    = key_ev.time.tv_sec;
	         m_stKey[KEY_DET_BT_RESET].Pressed_Time_us = key_ev.time.tv_usec;	
			 
			 if  (m_stKey[KEY_DET_BT_RESET].bKeyPressedCnt == 1 )
			 {
				Press_firstT = key_ev.time.tv_sec * 1000LL * 1000LL + key_ev.time.tv_usec;
			 }
            			 
	       }
	     else // released
	       {	 
	       	 MMP_ULONG dwtime;	       	            
	         MMP_ULONG dPresstime;	         	 
	       	 m_stKey[KEY_DET_BT_RESET].bKeyPressed     = FALSE;
	         m_stKey[KEY_DET_BT_RESET].Release_Time    = key_ev.time.tv_sec;
	       	 m_stKey[KEY_DET_BT_RESET].Release_Time_us = key_ev.time.tv_usec;
	       	 Press_finalT = key_ev.time.tv_sec * 1000LL * 1000LL + key_ev.time.tv_usec;
			 dPresstime = (Press_finalT - Press_firstT) / 1000;
			 Press_firstT = 0;
			 Press_finalT = 0;
	       	 	       	 
	       	 dwtime = Pega_Gpio_Key_timespec_diff(KEY_DET_BT_RESET);
	       	 
	       	 if (dwtime < 3000) //3s
	       	   {	     
                 if (m_stKey[KEY_DET_BT_RESET].bKeyPressedCnt > 5)
				 {					 
	       	   	     //pega_schedule_Event_push(schEVT_System_qrcode_scan_enable,  SCH_100ms); //2~9s 
				 }
	       	   }
             else if (dPresstime >= 5000) //5s
			 {									 
	       	   	    //pega_schedule_Event_push(schEVT_Factory_Reset,  SCH_100ms); 				 
			 }             			   
	       }      
}

static void* Pega_Gpio_Key_Detection_Handler(void* argv)
{	 
	 int fd;
	 struct input_event key_ev;
	 char *filename="/dev/input/event0";
	 
	 (void)argv;
	 pthread_detach(pthread_self());
	 prctl(PR_SET_NAME, THREAD_PROC_4); //set the thread name
	 
	 fd = open(filename, O_RDONLY);
	 
	 while(1)
	 {	
	 	if (read(fd, &key_ev, sizeof(struct input_event)) < 0) 
	 	{          
          fprintf(stderr, "failed to read input event from input device %s: %s\n", filename, strerror(errno));
          if (errno == EINTR)
            {
              continue;
            }  
          break;            
        }
	 		 
	 	if (key_ev.type == EV_KEY) 
	 	{    
	 	    Pega_Gpio_Key_Reset_processing(key_ev);	 		   	
           //printf("\n code=%d,value=%d,time=%d",key_ev.code, key_ev.value, key_ev.time.tv_sec);
        }		  
		//printf("\n %s [%d]",__func__,i);
	 }	 
	 
	 return 0;
}

void Pega_Gpio_key_Detection_Start(void)
{	   
	 pthread_t thread_id;    
               
     pthread_create(&thread_id, NULL, &Pega_Gpio_Key_Detection_Handler, NULL); 
          
}

int Pega_Gpio_Key_Button_Status_Get(MMP_UBYTE button)
{	
//	printf("key %d, %d\n", button, m_stKey[button].bKeyPressed);
	return m_stKey[button].bKeyPressed;
}

MMP_ULONG Pega_Gpio_Key_Button_Count_Get(MMP_UBYTE button)
{	
//	printf("key %d, %d\n", button, m_stKey[button].bKeyPressed);
	return m_stKey[button].bKeyPressedCnt;
}
//==============================================================================
//pega_misc_dbg info 2 29	 
void Pega_Gpio_key_Detection_Clear(void)
{	   
	 //printf("\n[%s]\n", __func__);  
     //Pega_Gpio_Key_Data_Info_Print();	 
     memset(&m_stKey, 0, sizeof(m_stKey));     
}
//==============================================================================
//pega_misc_dbg info 6 5
void Pega_Gpio_Key_Data_Info_Print(void)
{     
#if 1	
	 	 
	 printf("\n-----------------------");
	 printf("\n sizeof(m_stKey) => %d", sizeof(m_stKey));
	 printf("\n-----------------------");
	 printf("\n m_stKey[KEY_DET_BT_RESET].bKeyPressed     => %d", m_stKey[KEY_DET_BT_RESET].bKeyPressed);
	 printf("\n m_stKey[KEY_DET_BT_RESET].bKeyPressedCnt  => %d", m_stKey[KEY_DET_BT_RESET].bKeyPressedCnt);
	 printf("\n m_stKey[KEY_DET_BT_RESET].Pressed_Time    => %lld", m_stKey[KEY_DET_BT_RESET].Pressed_Time);
	 printf("\n m_stKey[KEY_DET_BT_RESET].Pressed_Time_us => %d", m_stKey[KEY_DET_BT_RESET].Pressed_Time_us);
	 printf("\n m_stKey[KEY_DET_BT_RESET].Release_Time    => %lld", m_stKey[KEY_DET_BT_RESET].Release_Time);
	 printf("\n m_stKey[KEY_DET_BT_RESET].Release_Time_us => %d", m_stKey[KEY_DET_BT_RESET].Release_Time_us);	 
	 printf("\n-----------------------\n");	 

#endif	   
}
//==============================================================================
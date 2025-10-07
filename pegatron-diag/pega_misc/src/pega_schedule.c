/*******************************************************************************
* File Name: pega_schedule.c
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
#include "pega_defines.h"
#include "pega_schedule.h"
#include "pega_wifi.h"
#include "pega_misc_diag.h"
#include "pega_debug.h"
//==============================================================================
static pthread_mutex_t m_Mutexlock;
//==============================================================================
#define SCH_DBG(x)          x
//==============================================================================
static stSchDefType m_SchBuff[SCH_BUFF_MAX];
static int 			m_SchCnt = 0;
//==============================================================================
static void pega_schedule_Buff_Reset(void)
{	     	 
	   for (m_SchCnt=0;m_SchCnt<SCH_BUFF_MAX;m_SchCnt++)
	      {
	      	m_SchBuff[m_SchCnt].enEVT     = schEVT_None;	      	
	      	m_SchBuff[m_SchCnt].wTMCount  = 0;
	      }  
	    
	   m_SchCnt  = 0; 	  
}

static int pega_schedule_Event_Timeout_Check(int ucIndex)
{	    
	   if (m_SchBuff[ucIndex].enEVT == schEVT_None)
	   	 {
	   	   return (FALSE);	   
	   	 }  
	   
	   m_SchBuff[ucIndex].wTMTime++;
	   
	   if (m_SchBuff[ucIndex].wTMCount > 0)
	   	 { 
	   	   m_SchBuff[ucIndex].wTMCount--;
	   	   
	       if (m_SchBuff[ucIndex].wTMCount <= 0)  
	       	 {	 	 
	   	       return (TRUE);	   	   
	   	     }  
	   	 }      
	   
	   return (FALSE);	    
}

static void pega_schedule_Buff_Reschedlue(int ucIndex)
{	   
	   int i;   	   
	   stSchDefType sTempData;
	   
	   pthread_mutex_lock(&m_Mutexlock);
	   	   	  	   	 
	   for (i=ucIndex;i<(SCH_BUFF_MAX-1);i++)
	      {
	      	sTempData = m_SchBuff[i+1];
	      	m_SchBuff[i] = sTempData;
	      }   
	   
	   if (m_SchCnt > 0)
	   	 {
	       m_SchCnt--;
	     } 
	   
	   pthread_mutex_unlock(&m_Mutexlock);  
}

static unsigned char pega_schedule_Event_Get(void)
{  
	 int i=0; 
	 unsigned char eMsg;
	 
	 eMsg = schEVT_None;
	 
	 if (m_SchCnt == 0)
	   {	
	 	 return (eMsg);
	   }	 
	 
	 for (i=0; i<=m_SchCnt; i++)
	   {
	   	  if (pega_schedule_Event_Timeout_Check(i))
	   	  	{
	   	  	  eMsg = m_SchBuff[i].enEVT; 
	   	  	  pega_schedule_Buff_Reschedlue(i);	   	  		
	   	  	  //SCH_DBG(printf("\r\n Event was timeout!"));
	   	  	  break;	// if one timeout found, keep others in buffer waiting next round, Luke 090102
	   	  	}
	   }
	 	 	 	 	 
	 return (eMsg);	
}
//pega_misc_dbg sch xx
static void pega_schedule_Event_Processing(void)
{	   
	unsigned char eEvent = schEVT_None;
	
    eEvent = pega_schedule_Event_Get();
	
	switch(eEvent)
	{
		case schEVT_None:
			 return;
		//==============================================================================
		case schEVT_DeleteAll:
			 return;
		//==============================================================================
		case schEVT_Load_wifi_fw:
		     Pega_wifi_load_firmware_enable();
			 return;	 
		//==============================================================================
        case schEVT_System_diag_Cmd_Execution:
		     Pega_diag_msgq_Cmd_Execution();  
			 return;	
        
        case schEVT_System_debug_info_Execution:
		     Pega_debug_schedule_command();  
			 return; 		
        
        case schEVT_Factory_Reset:		      
			 return;			 
		//==============================================================================				 
		default:
			 break;
	}   

	SCH_DBG(printf("schedule_Event_Processing(Evt:%d)\n",eEvent));    
}

static void* pega_schedule_TaskHandler(void* argv)
{	 
	 (void)argv;
	 pthread_detach(pthread_self());
	 prctl(PR_SET_NAME, THREAD_PROC_2); //set the thread name
	 //printf("\n%s\n", __func__);
     pthread_mutex_init(&m_Mutexlock, NULL);
          	 
	 while(1)
	 {	
	 	pega_schedule_Event_Processing();
	 		 	
	 	usleep(SCH_HANDLER_DELAY_CNT); //every lool by 100ms
	 		 		
		//printf("pega_schedule_TaskHandler[%d] \n",i++);		
	 }	 
	 
	 pthread_mutex_destroy(&m_Mutexlock);
	 
	 return 0;
}

void pega_schedule_handler_Start(void)
{
	pthread_t thread_id;
	pthread_attr_t attr;
                
    pega_schedule_Buff_Reset();
        	
    //printf("\n%s", __FUNCTION__);
    pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 2048*1024);	// increase the size of statck		// 1024*1024);
    pthread_create(&thread_id, &attr, &pega_schedule_TaskHandler, NULL);
    
}
//wDelayTime : unit 100ms
int pega_schedule_Event_push(unsigned char eEvent, unsigned int wDelayTime)
{	   
	int i;   	   
	
	pthread_mutex_lock(&m_Mutexlock);
	   
	   //SCH_DBG(printf("\n%s(%d,%d)", __FUNCTION__,eEvent,wDelayTime));
	   
	   if (eEvent >= schEVT_Max)
	   	 {
	   	   printf("Error! the event msg was wrong!\n");	   	   
	   	   goto FAIL_RTN;
	   	   //return (FALSE);
	   	 }    
	  
	   //printf("\n%s(eEvent=%d,Delay=%d)", __FUNCTION__,eEvent,wDelayTime);
	   	   	 
	   for (i=0;i<=m_SchCnt;i++)
	      {
	      	if (m_SchBuff[i].enEVT == eEvent)	
	      	  {	      		
	      		goto TIME_COUNT_SET;	      		  
	      	  }	
	      }
	   
	   if (m_SchCnt >= (SCH_BUFF_MAX-1))
	   	 {
	   	 	 //SCH_DBG(printf("\n Error! the event buff was full! "));
	   	 	 printf("Error! the event buff was full!\n");	   	 	 
	   	 	 goto FAIL_RTN;
	   	 	 //return (FALSE);
	   	 }   
	   
	   i = m_SchCnt++;
	   	 
	   m_SchBuff[i].enEVT    = eEvent;

TIME_COUNT_SET:
	
	   m_SchBuff[i].wTMCount = wDelayTime; 
	   m_SchBuff[i].wTMTime  = 0;
	   
	   pthread_mutex_unlock(&m_Mutexlock);
	   
	   return (TRUE);

FAIL_RTN:
	
	   pthread_mutex_unlock(&m_Mutexlock);
	   
	   return (FALSE);   
}

void pega_schedule_Event_pop(void)
{	   	   
	 SCH_DBG(printf("%s\n", __func__));
	   
	 pega_schedule_Buff_Reschedlue(0);
}

int pega_schedule_Event_Cancel(unsigned char ucEvent)
{	   
	int i=0;   	   

	if (ucEvent >= schEVT_Max)
	{
		// SCH_DBG(printf("\n Error! the event msg was wrong! "));
		printf("Error! the event msg(%d) was wrong!", ucEvent);		
		return (FALSE);
	}    

	if (ucEvent == schEVT_DeleteAll)
	{
		pega_schedule_Buff_Reset();
		//SCH_DBG(printf("\n Cancel all schedule events! "));
		return (TRUE);
	} 

	for (i=0; i<=m_SchCnt; i++)
	{
		if (m_SchBuff[i].enEVT == ucEvent)
		{	      		
			pega_schedule_Buff_Reschedlue(i);	      		  
			//	SCH_DBG(printf("\n ucEvent was canceled! (%d)", ucEvent));
			return (TRUE);
		}	
	}

	//SCH_DBG(printf("\n ucEvent not found! (%d)", ucEvent));

	//	 printf("ucEvent(%d) not found!", ucEvent);

	return (FALSE);
}

int pega_schedule_Event_Check(unsigned char ucEvent)
{	   
	 int i;   	   
	   
	 if (ucEvent >= schEVT_Max)
	   {
	   	 // SCH_DBG(printf("\n Error! the event msg was wrong! "));
	   	 return (FALSE);
	   }    	   
	  
	 for (i=0;i<=m_SchCnt;i++)
	      {
	      	if (m_SchBuff[i].enEVT == ucEvent)
	      		{	 
	      	      //SCH_DBG(printf("\n ucEvent was found! (%d)", ucEvent));      			      		
	   	 	      return (TRUE);
	      		}	
	      }
	   
	   
	   //SCH_DBG(printf("\n ucEvent not found! (%d)", ucEvent));
	   return (FALSE);
}

int pega_schedule_Event_DelayTime_Get(unsigned char ucEvent, unsigned int *u16Delayms)
{	   
	  int i=0;   	   
	   
	 *u16Delayms = 0;
	   
	 if (ucEvent >= schEVT_Max)
	   {
	   	// SCH_DBG(printf("\n Error! the event msg was wrong! "));
	   	 return (FALSE);
	   }    	   
	  
	 for (i=0;i<=m_SchCnt;i++)
	      {
	      	if (m_SchBuff[i].enEVT == ucEvent)
	      	  {	 
	      			*u16Delayms = m_SchBuff[i].wTMCount * 100;//SCH_100ms
	      		 //SCH_DBG(printf("\n ucEvent was found! (%d)", ucEvent));      			      		
	   	 	    	return (TRUE);
	          }	
	      }
	   
	   
	   //SCH_DBG(printf("\n ucEvent not found! (%d)", ucEvent));
	   return (FALSE);
}

int pega_schedule_Event_Time_Get(unsigned char ucEvent, unsigned int *u16Timems)
{	   
	  int i=0;   	   
	   
	 *u16Timems = 0;
	   
	 if (ucEvent >= schEVT_Max)
	   {
	   	// SCH_DBG(printf("\n Error! the event msg was wrong! "));
	   	 return (FALSE);
	   }    	   
	  
	 for (i=0;i<=m_SchCnt;i++)
	      {
	      	if (m_SchBuff[i].enEVT == ucEvent)
	      	  {	 
	      		*u16Timems = m_SchBuff[i].wTMTime * 100;//SCH_100ms
	      		//SCH_DBG(printf("\n ucEvent was found! (%d)", ucEvent));      			      		
	   	 	     return (TRUE);
	          }	
	      }
	   
	   
	   //SCH_DBG(printf("\n ucEvent not found! (%d)", ucEvent));
	   return (FALSE);
}
//==============================================================================
int pega_schedule_Event_Time_Count_Set(unsigned char ucEvent)
{
//	printf("\n %s (ucEvent=%d)\n", __func__, ucEvent);
	
	return pega_schedule_Event_push(ucEvent, SCH_1Hour * 4);
}

int pega_schedule_Event_Time_Count_Get(unsigned char ucEvent, unsigned int *u16Delayms)
{	
	int rtn = pega_schedule_Event_Time_Get(ucEvent, u16Delayms);
	
	//fprintf(stderr, "\n %s (ucEvent=%d, u16Delayms=%d)\n", __func__, ucEvent, *u16Delayms);
	
	pega_schedule_Event_Cancel(ucEvent);
	
	return rtn;
}
//==============================================================================
//pega_misc_dbg info 2
void pega_schedule_Data_Info_Print(void)
{     
#if 1	
	 unsigned int i;
		    
	 SCH_DBG(printf("-----------------------\n"));		 
	 SCH_DBG(printf("m_SchCnt = %d\n",m_SchCnt));	
	 
	 for (i=0;i<SCH_BUFF_MAX;i++)
	    {
	      SCH_DBG(printf("m_SchBuff[%d].enEVT     => %d\n",i,m_SchBuff[i].enEVT));	      
	      SCH_DBG(printf("m_SchBuff[%d].wTMCount  => %d\n",i,m_SchBuff[i].wTMCount));
	      SCH_DBG(printf("m_SchBuff[%d].wTMTime   => %d\n",i,m_SchBuff[i].wTMTime));
	    }	    
	 SCH_DBG(printf("-----------------------\n"));	  
	 SCH_DBG(printf("SCH_100ms     => %d\n",SCH_100ms));
	 SCH_DBG(printf("SCH_1Sec      => %d\n",SCH_1Sec));
	 SCH_DBG(printf("SCH_1Min      => %d\n",SCH_1Min));
	 SCH_DBG(printf("SCH_1Hour     => %d\n",SCH_1Hour));
	 SCH_DBG(printf("-----------------------\n"));	 

#endif	   
}

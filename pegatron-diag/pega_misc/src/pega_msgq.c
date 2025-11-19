/*******************************************************************************
* File Name: pega_msgq.c
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
#include "pega_msgq_cmd.h"
//==============================================================================
#include "main.h"
#include "pega_schedule.h"
#include "pega_nv_mode.h"
#include "pega_gpio.h"
#include "pega_wifi.h"
//==============================================================================
static mqd_t m_msqQ;  // 全域 queue descriptor
//==============================================================================
static stMsgQdata_t   m_stMsgQdata;
//==============================================================================
static char m_bDebugOn = 0;
//==============================================================================
static void* Pega_msgq_message_handler(void* argv)
{       
    struct mq_attr attr;   
    ssize_t bytes_read;

    (void)argv;
	pthread_detach(pthread_self());    
	prctl(PR_SET_NAME, THREAD_PROC_3); //set the thread name
	
    // 設定訊息佇列屬性
    attr.mq_flags   = 0;                   // 0 = BLOCK 模式
    attr.mq_maxmsg  = MSGQ_MAX_MESSAGES;   // 佇列最多訊息數
    attr.mq_msgsize = MSGQ_BUFF_MAX_SIZE;  // 每則訊息最大長度
    attr.mq_curmsgs = 0;

    // 建立或開啟訊息佇列
    m_msqQ = mq_open(MSGQ_QUEUE_NAME, O_RDONLY | O_CREAT, 0666, &attr);
	
    if (m_msqQ == (mqd_t)-1) 
	{
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    //printf("Message queue opened for BLOCK read: %s\n", MSGQ_QUEUE_NAME);
    //printf("Waiting for messages (type 'exit' to stop)...\n");

    // BLOCK 模式：無限迴圈等待訊息
    while (1) 
	{             
		bytes_read = mq_receive(m_msqQ, (char *)&m_stMsgQdata, MSGQ_BUFF_MAX_SIZE, NULL);
		
        if (bytes_read >= 0) 
		{
			if (m_stMsgQdata.CmdCount > 0)
			{				
				_LOG_ALERT("MsgQ Buffer overflow.(%d)", m_stMsgQdata.CmdCount);
			}
			
			m_stMsgQdata.CmdCount++;
			//printf("bytes_read = %d\n", bytes_read);
			//Pega_msgq_attr_info();
            pega_schedule_Event_push(schEVT_System_msgq_Cmd_Execution, SCH_100ms);//Pega_msgq_command_handler_exec();			
        } 
		else 
		{
            perror("mq_receive");
            sleep(1); // 錯誤時稍等
        }
    }

    // 清理資源
    mq_close(m_msqQ);
    mq_unlink(MSGQ_QUEUE_NAME);
}

//==============================================================================
void Pega_msgq_listen_handler_start(void)
{
	 pthread_t thread_id;
	 pthread_create(&thread_id, NULL, &Pega_msgq_message_handler, NULL);	
}
//==============================================================================
void Pega_msgq_command_handler_exec(void)
{
	if (m_bDebugOn > 0)
	{
		Pega_msgq_data_info();	
	}
	
	if (m_stMsgQdata.CmdCount > 0)
	{
		m_stMsgQdata.CmdCount--;								
	 
		if (Pega_msgq_message_processing(m_stMsgQdata) < 0)
		{
			fprintf(stderr, "Command[%s] error.[From:%s]\n", m_stMsgQdata.CmdName, m_stMsgQdata.CmdFrom);
		}
	}	
}
//==============================================================================
void Pega_msgq_debug_message_enable(char bDebugOn)
{
	 m_bDebugOn = (bDebugOn > 0) ? 1 : 0;
}
//==============================================================================
void Pega_msgq_attr_info(void)
{
	 struct mq_attr attr;
	 	 
	 if (mq_getattr(m_msqQ, &attr) == -1) 
	 {
        perror("mq_getattr");
        exit(EXIT_FAILURE);
     }
	
	 printf("-----------------------\n");
	 printf("attr.mq_flags        = %x\n", attr.mq_flags);
	 printf("attr.mq_maxmsg       = %ld\n", attr.mq_maxmsg);
	 printf("attr.mq_msgsize      = %ld\n", attr.mq_msgsize);
	 printf("attr.mq_curmsgs      = %ld\n", attr.mq_curmsgs);
	 printf("-----------------------\n");
}

void Pega_msgq_data_info(void)
{		
	 printf("-----------------------\n");
	 printf("m_stMsgQdata.CmdCount    = %d\n", m_stMsgQdata.CmdCount);
	 printf("m_stMsgQdata.bRequestAck = %d\n", m_stMsgQdata.bRequestAck);
	 printf("-----------------------\n");
	 printf("m_stMsgQdata.CmdName  = %s\n", m_stMsgQdata.CmdName);
	 printf("m_stMsgQdata.CmdFrom  = %s\n", m_stMsgQdata.CmdFrom);	 
	 printf("-----------------------\n");
	 printf("sizeof(m_stMsgQdata)  = %d\n", sizeof(m_stMsgQdata));
	 printf("-----------------------\n");
	 
	 Pega_msgq_data_content_info();
}

void Pega_msgq_data_content_info(void)
{		
     int i;
	 printf("-----------------------\n");
	 printf("m_stMsgQdata.stData.argc    = %d\n", m_stMsgQdata.stData.argc);
	 for (i = 0; i < m_stMsgQdata.stData.argc; i++) 
	 {
        printf("m_stMsgQdata.stData.argv[%d] = %s\n", i, m_stMsgQdata.stData.argv[i]);
     }
     printf("-----------------------\n");
}
//==============================================================================
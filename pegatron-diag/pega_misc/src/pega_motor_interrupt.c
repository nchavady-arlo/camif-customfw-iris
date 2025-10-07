/*******************************************************************************
* File Name: pega_motor_interrupt.c
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
#include <poll.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/un.h>
//==============================================================================
#include "pega_gpio.h"
#include "pega_motor_interrupt.h"
//==============================================================================
/*
#define POLLIN 		0x0001  
#define POLLPRI 	0x0002  
#define POLLOUT 	0x0004  
#define POLLERR 	0x0008  
#define POLLHUP 	0x0010  
#define POLLNVAL 	0x0020  
  
#define POLLRDNORM 	0x0040  
#define POLLRDBAND 	0x0080  
#define POLLWRNORM 	0x0100  
#define POLLWRBAND 	0x0200  
#define POLLMSG 	0x0400  
#define POLLREMOVE 	0x1000  
#define POLLRDHUP 	0x2000

struct pollfd {  
 int fd;        
 short events;  
 short revents; 
};
*/
//==============================================================================
static pthread_t m_pthread_id1 = 0;
static pthread_t m_pthread_id2 = 0;
//==============================================================================
static unsigned char m_bIsMotorIntEn = 1;
//==============================================================================
static void* Pega_Motor_Pan_interrupt_TaskHandler(void* argv)
{			 
#if (DEVICE_MOTOR_ENABLE == 1)
	
	 (void)argv;
	 int fd;
     struct pollfd pfd;
     char buf[3];
	 char path[64];
	 
	 pthread_detach(pthread_self());
	 prctl(PR_SET_NAME, THREAD_PROC_7); //set the thread name
	 
     Pega_Motor_interrupt_trigger_set(IO_I_MOTOR_nFAULT_PAN, E_GPIO_INT_TRIGGER_FALLING);
	 	 
	 sprintf(path,"/sys/class/gpio/gpio%d/value", IO_I_MOTOR_nFAULT_PAN);  
	 
	 if ((fd = open(path, O_RDONLY)) < 0) 
	 {
        perror("Failed to open gpio value");
        return;
     }
	
	 pfd.fd = fd;
     pfd.events = POLLPRI;
	 
	 read(fd, buf, sizeof(buf));
	 printf("Waiting for Motor(Pan) interrupt...\n");
	 
	 while (m_bIsMotorIntEn > 0) 
	 {        
        int ret = poll(&pfd, 1, -1);
		
		printf("Waiting for GPIO interrupt(%d)...\n" ,m_bIsMotorIntEn);
	    // -1: poll error.
		//  0: poll time out.        
        if (ret > 0) 
		{      
            if (pfd.revents & POLLPRI) 
			{
                lseek(fd, 0, SEEK_SET);  
                read(fd, buf, sizeof(buf));
                printf("Motor interrupt occurred(Pan), value: %c\n", buf[0]);
            }
        } 
		else 
		{
            perror("poll");
        }
    }
 
    close(fd);

#endif
}

static void* Pega_Motor_Rising_interrupt_TaskHandler(void* argv)
{		
#if (DEVICE_MOTOR_ENABLE == 1)
	
	 (void)argv;
	 int fd;
     struct pollfd pfd;
     char buf[3];
	 char path[64];
	 
	 pthread_detach(pthread_self());
	 prctl(PR_SET_NAME, THREAD_PROC_8); //set the thread name
	 
     Pega_Motor_interrupt_trigger_set(IO_I_MOTOR_nFAULT_RISING, E_GPIO_INT_TRIGGER_FALLING);
	 	 
	 sprintf(path,"/sys/class/gpio/gpio%d/value", IO_I_MOTOR_nFAULT_RISING);  
	 
	 if ((fd = open(path, O_RDONLY)) < 0) 
	 {
        perror("Failed to open gpio value");
        return;
     }
	
	 pfd.fd = fd;
     pfd.events = POLLPRI;
	 
	 read(fd, buf, sizeof(buf));
	 printf("Waiting for Motor(Rising) interrupt...\n");
	 
	 while (m_bIsMotorIntEn > 0) 
	 {        
        int ret = poll(&pfd, 1, -1);
		
		printf("Waiting for GPIO interrupt(%d)...\n" ,m_bIsMotorIntEn);
	    // -1: poll error.
		//  0: poll time out.        
        if (ret > 0) 
		{      
            if (pfd.revents & POLLPRI) 
			{
                lseek(fd, 0, SEEK_SET);  
                read(fd, buf, sizeof(buf));
                printf("Motor interrupt occurred(Rising), value: %c\n", buf[0]);
            }
        } 
		else 
		{
            perror("poll");
        }
    }
 
    close(fd);
	    	 
#endif
}
//==============================================================================
void Pega_Motor_interrupt_handler_Start(void)
{
	 //pthread_t thread_id;
	                
     //printf("\n%s", __FUNCTION__);	
     pthread_create(&m_pthread_id1, NULL, &Pega_Motor_Pan_interrupt_TaskHandler, NULL);
	 pthread_create(&m_pthread_id2, NULL, &Pega_Motor_Rising_interrupt_TaskHandler, NULL);
}

void Pega_Motor_interrupt_handler_Stop(void)
{
	 if (m_pthread_id1 != 0)
	 {
		 pthread_cancel(m_pthread_id1);
		 m_pthread_id1 = 0;
	 }
	 
	 if (m_pthread_id2 != 0)
	 {
		 pthread_cancel(m_pthread_id2);
		 m_pthread_id2 = 0;
	 }
}

void Pega_Motor_interrupt_handler_Exit(void)
{
	 m_bIsMotorIntEn = 0;
}
//==============================================================================
int Pega_Motor_interrupt_trigger_set(int sGpioNum, unsigned int eInterrupt) 
{  
    FILE *fp;
    char s1[60]={0};  
    	
    //printf("\n%s[%d,%d]\n", __func__, sGpioNum, bHigh);  
    if (eInterrupt > E_GPIO_INT_TRIGGER_BOTH)  
	{
		return -1;  
	}
	
    sprintf(s1,"/sys/class/gpio/gpio%d/edge", sGpioNum);  
  
    //printf("[%s]%s\n", __func__, s1);  
	
    if ((fp = fopen(s1, "rb+")) == NULL)   
    {  
        printf("Cannot open %s\n",s1);		
        return -1;  
    }
	
	switch(eInterrupt)
	{		
		case E_GPIO_INT_TRIGGER_RISING:
		     fwrite("rising", sizeof(char), strlen("rising"), fp); 
		     break;
			 
		case E_GPIO_INT_TRIGGER_FALLING:
		     fwrite("falling", sizeof(char), strlen("falling"), fp); 
		     break;
			 
        case E_GPIO_INT_TRIGGER_BOTH:
		     fwrite("both", sizeof(char), strlen("both"), fp); 
		     break;
			 
		default:
		     fwrite("none", sizeof(char), strlen("none"), fp); 
		     break;
	}
	    
    fclose(fp);  
    
    return 0;  
}  

//==============================================================================
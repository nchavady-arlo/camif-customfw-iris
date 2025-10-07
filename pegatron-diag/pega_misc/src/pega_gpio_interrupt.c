/*******************************************************************************
* File Name: pega_gpio_interrupt.c
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
#include "pega_gpio_interrupt.h"
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
#define GPIO_NUM "6"
#define GPIO_PATH "/sys/class/gpio/gpio" GPIO_NUM "/value"
//==============================================================================
static pthread_t m_pthread_id = 0;
//==============================================================================
static unsigned char m_bIsHandlerEn = 1;
//==============================================================================
static void* Pega_Gpio_interrupt_TaskHandler(void* argv)
{			 
	 (void)argv;
	 int fd;
     struct pollfd pfd;
     char buf[3];
	
	 pthread_detach(pthread_self());
	 prctl(PR_SET_NAME, THREAD_PROC_6); //set the thread name
	 
     Pega_Gpio_interrupt_trigger_set(6, E_GPIO_INT_TRIGGER_FALLING);
	 
	 if ((fd = open(GPIO_PATH, O_RDONLY)) < 0) 
	 {
        perror("Failed to open gpio value");
        return;
     }
	
	 pfd.fd = fd;
     pfd.events = POLLPRI;
	 
	 read(fd, buf, sizeof(buf));
	 printf("Waiting for GPIO interrupt...\n");
	 
	 while (m_bIsHandlerEn > 0) 
	 {        
        int ret = poll(&pfd, 1, -1);      
	    // -1: poll error.
		//  0: poll time out.        
        if (ret > 0) 
		{      
            if (pfd.revents & POLLPRI) 
			{
                lseek(fd, 0, SEEK_SET);  
                read(fd, buf, sizeof(buf));
                printf("GPIO interrupt occurred, value: %c\n", buf[0]);
            }
        } 
		else 
		{
            perror("poll");
        }
    }
 
    close(fd);
	    	 
	 return;
}
//==============================================================================
void Pega_Gpio_interrupt_handler_Start(void)
{
	 //pthread_t thread_id;
	                
     //printf("\n%s", __FUNCTION__);	
     pthread_create(&m_pthread_id, NULL, &Pega_Gpio_interrupt_TaskHandler, NULL);     
}

void Pega_Gpio_interrupt_handler_Stop(void)
{
	 if (m_pthread_id != 0)
	 {
		 pthread_cancel(m_pthread_id);
		 m_pthread_id = 0;
	 }
}

void Pega_Gpio_interrupt_handler_Exit(void)
{
	 m_bIsHandlerEn = 0;
}
//==============================================================================
int Pega_Gpio_interrupt_trigger_set(int sGpioNum, unsigned int eInterrupt) 
{  
    FILE *fp;
    char s1[50]={0};  
   
    //printf("\n%s[%d,%d]\n", __func__, sGpioNum, bHigh);  
    if (eInterrupt > E_GPIO_INT_TRIGGER_BOTH)  
	{
		return -1;  
	}
	
    sprintf(s1,"/sys/class/gpio/gpio%d/edge", sGpioNum);  
  
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
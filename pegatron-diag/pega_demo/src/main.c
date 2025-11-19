#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
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
#include <time.h>
#include <stdbool.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/watchdog.h>
//==============================================================================
#include "pega_led_flash.h"
//==============================================================================
int Pega_percent_check(char* pPercent) 
{
	int i,value=0;
	 
	if (pPercent == NULL)
	{
		 return -1;			
	}
	
	 for (i=0;i<strlen(pPercent);i++)
	 {
	    if (pPercent[i] == '%')
		{
			pPercent[i] = 0;
			value = atoi(pPercent);
			printf("\n[%s] value=%d \n", __func__, value);  
			return value;
		}
	 }	
	 
	 return -1;
}
//==============================================================================
int main(int argc, char* argv[])
{
    printf("\n[%s] argc=%d \n", __func__, argc);  
	
	if (argc > 1)
    {
    	printf("\n[%s]%s,%s \n", __func__, argv[0],argv[1]);    			 	    	 	
    } 
    
	if (!strcmp(argv[1],"led1"))
  	  {
  		Pega_led_flash_state_Set(LED_State_Voice_Cmd_training);
  	  }
    else if (!strcmp(argv[1],"led2"))
  	  {
  		Pega_led_flash_state_Set(LED_State_Voice_Cmd_standby);
  	  }
	else if (!strcmp(argv[1],"led3"))
  	  {
  		Pega_led_flash_state_Set(LED_State_Voice_Cmd_recognized);
  	  }
    else if (!strcmp(argv[1],"led4"))
  	  {
  		Pega_led_flash_state_Set(LED_State_Voice_Cmd_wait);
  	  }
	 
    sleep(5);
	
    return 0;
}

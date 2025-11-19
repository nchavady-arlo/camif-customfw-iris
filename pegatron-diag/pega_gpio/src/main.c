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
#include "pega_defines.h"
#include "pega_gpio.h"
#include "pega_pwm.h"
#include "pega_adc.h"
//==============================================================================
static void Pega_gpio_help_print(void)
{	   	   
       //fprintf(stdout, "Pega_gpio --version:%s\n", MISC_VERSTION);
	   fprintf(stdout, "Pega_gpio --date:%s\n", __DATE__);
	   fprintf(stdout, "Pega_gpio --time:%s\n", __TIME__);
				
	   printf("\n Usage :");	   
	   printf("\n Pega_gpio help info [options]");
	   printf("\n options:");
	   printf("\n -m : disable 5G module");
	   
	   printf("\n -h : help info");
	   printf("\n");		 
}

int main(int argc, char* argv[])
{
    printf("\n[%s] argc=%d \n", __func__, argc);  
	
	if (argc > 1)
    {
    	printf("\n[%s]%s,%s \n", __func__, argv[0],argv[1]);    			 	    	 	
    } 
    else
    {
      	Pega_gpio_help_print();
     	goto Cmd_End;
    }  
    
	if (!strcmp(argv[1],"gpio"))
  	  {
  		if (!strcmp(argv[2],"init"))
		{
			Pega_Gpio_init();
		}
		else if (!strcmp(argv[2],"wifi1"))
		{
			Pega_Gpio_wifi_power_down(1);
		}
		else if (!strcmp(argv[2],"wifi2"))
		{
			Pega_Gpio_wifi_power_down(0);
		}
        else if (!strcmp(argv[2],"info"))
		{
			Pega_Gpio_Data_Info_Print();
		}
  	  }
	else if (!strcmp(argv[1],"wifi"))
  	  {
  		if (!strcmp(argv[2],"reset"))
		{			
			if (argv[3] != NULL)
			{
				Pega_Gpio_wifi_software_reset(atoi(argv[3]));
			}
		}		
  	  }	  
	else if (!strcmp(argv[1],"adc"))
  	  {
  		if (!strcmp(argv[2],"init"))
		{
			Pega_SarADC_init();
		}		
        else if (!strcmp(argv[2],"get"))
		{
			if (argv[3] != NULL)
			{				
				Pega_SarADC_Value_Print(strtol(argv[3],NULL,10));
			}			
		}
  	  }  
	else if (!strcmp(argv[1],"pwm"))
  	  {
  		if (!strcmp(argv[2],"init"))
		{
			Pega_pwm_init(1);
		}		
        else if (!strcmp(argv[2],"set"))
		{			
			unsigned int wPWM = PWM_CH6;
			int bEnable = 0;
			
			if (argv[3] != NULL)
			{
				wPWM = strtol(argv[3],NULL,10); //16 
			}
			
			if (argv[4] != NULL)
			{
				bEnable = strtol(argv[4],NULL,10); //16 
			}
			
			Pega_pwm_control(wPWM, bEnable);
		}		
		else if (!strcmp(argv[2],"config"))
		{			
			unsigned int wPWM = PWM_CH6;
			unsigned int period = PWM_LED_FREQ, duty_cycle = PWM_LED_DUTY;
			
			if (argv[3] != NULL)
			{
				wPWM = strtol(argv[3],NULL,10); //16 
			}
			
			if (argv[4] != NULL)
			{
				period = strtol(argv[4],NULL,10); //16 
			}
			
			if (argv[5] != NULL)
			{
				duty_cycle = strtol(argv[5],NULL,10); //16 
			}
			
			Pega_pwm_config(wPWM, period, duty_cycle);
		}
  	  }  

Cmd_End:
	  
    return 0;
}

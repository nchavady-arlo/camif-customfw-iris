/*******************************************************************************
* File Name: pega_gpio.c
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
//==============================================================================
#include "pega_gpio.h"
//==============================================================================
#define VALUE_MAX 30
//==============================================================================
static int Pega_Gpio_pin_state_init(int sGpioNum, int bInput, int bHigh) 
{  
    FILE *fp;           
    char s[50]="";  
    
    if ((fp = fopen("/sys/class/gpio/export", "w")) == NULL)   
    {  
        ERR_LOG("Cannot open export file.");  
        return FAILED;  
    }  
    	
    fprintf(fp, "%d", sGpioNum);  
    fclose(fp);  
  
    sprintf(s,"/sys/class/gpio/gpio%d/direction", sGpioNum);  
    
    if ((fp = fopen(s, "rb+")) == NULL)   
    {  
        ERR_LOG("Cannot open %s.",s);  
        return FAILED;  
    }  
    
    if (bInput)	
	{
        fprintf(fp, "in");
	}
    else 
	{		
        fprintf(fp, "out");  
	}
               
    fclose(fp);            
    
    if (bInput)	
	{
        return SUCCEED;
	}
            
    return Pega_Gpio_pin_output_set(sGpioNum, bHigh);  
}  

int Pega_Gpio_direction_set(int bOutput, unsigned char uGPIONum)
{
	char buff[255];
	 
	snprintf(buff, sizeof(buff), "echo %d > /sys/class/gpio/export", uGPIONum);
    system(buff);  
	 
	if (bOutput)	 
	    snprintf(buff, sizeof(buff), "echo out > /sys/class/gpio/gpio%d/direction", uGPIONum);
	else
	    snprintf(buff, sizeof(buff), "echo in > /sys/class/gpio/gpio%d/direction", uGPIONum);
	         
    system(buff);  
	 
	 return SUCCEED;
}

int Pega_Gpio_Read(int pin)
{
	char path[VALUE_MAX];
	char value_str[3];
	int fd,value;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	
	if (-1 == fd) 
	{
		ERR_LOG("Failed to open gpio(%d) value for reading!", pin);
		return (FAILED);
	}

	if (-1 == read(fd, value_str, 3)) 
	{
		ERR_LOG("Failed to read gpio value!");
		return(FAILED);
	}

	close(fd);
  
    value = (atoi(value_str)==0) ? GPIO_LOW : GPIO_HIGH;
  
  //printf("\n%s(%d)\n", __func__, value);
  
	return (value);
}

int Pega_Gpio_pin_output_set(int sGpioNum, int bHigh) 
{  
    FILE *fp;  
    char buffer[10];         
    char s1[50]="";  
    
    //printf("\n%s[%d,%d]\n", __func__, sGpioNum, bHigh);  
                  
    sprintf(s1,"/sys/class/gpio/gpio%d/value",sGpioNum);  
  
    if ((fp = fopen(s1, "rb+")) == NULL)   
    {  
        ERR_LOG("Cannot open %s.",s1);  
        return FAILED;  
    }  
    	
    if (bHigh)	
        strcpy(buffer,"1");  
    else
        strcpy(buffer,"0"); 
         
    fwrite(buffer, sizeof(char), sizeof(buffer) - 1, fp);         
    fclose(fp);  
    
    return SUCCEED;  
}  

//==============================================================================

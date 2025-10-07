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
#include "Pega_gpio.h"
//==============================================================================
#define GPIO_DBG(x)         // x
//==============================================================================
static int Pega_Gpio_pin_state_init(int sGpioNum, int bInput, int bHigh) 
{  
    FILE *fp;           
    char s[50]="";  
    
    if ((fp = fopen("/sys/class/gpio/export", "w")) == NULL)   
    	{  
        printf("Cannot open export file.\n");  
        return -1;  
    	}  
    	
    fprintf(fp, "%d", sGpioNum);  
    fclose(fp);  
  
    sprintf(s,"/sys/class/gpio/gpio%d/direction", sGpioNum);  
    
    if ((fp = fopen(s, "rb+")) == NULL)   
    	{  
        printf("Cannot open %s.\n",s);  
        return -1;  
    	}  
    
    if (bInput)	
        fprintf(fp, "in");
    else    
        fprintf(fp, "out");  
               
    fclose(fp);            
    
    if (bInput)	
        return 1;
            
    return Pega_Gpio_pin_output_set(sGpioNum, bHigh);  
}  

static int Pega_Gpio_direction_set(int bOutput, int uGPIONum)
{
	 char buff[255];
	 
	 snprintf(buff, sizeof(buff), "echo %d > /sys/class/gpio/export", uGPIONum);
   system(buff);  
	 
	 if (bOutput)	 
	     snprintf(buff, sizeof(buff), "echo out > /sys/class/gpio/gpio%d/direction", uGPIONum);
	 else
	     snprintf(buff, sizeof(buff), "echo in > /sys/class/gpio/gpio%d/direction", uGPIONum);
	         
   system(buff);  
	 
	 return 0;
}

#define VALUE_MAX 30

int Pega_Gpio_Read(int pin)
{
	char path[VALUE_MAX];
	char value_str[3];
	int fd,value;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		//fprintf(stderr, "Failed to open gpio value for reading!\n");
		return (FAILED);
	}

	if (-1 == read(fd, value_str, 3)) {
		//fprintf(stderr, "Failed to read value!\n");
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
                  
    sprintf(s1,"/sys/class/gpio/gpio%d/value",sGpioNum);  
  
    if ((fp = fopen(s1, "rb+")) == NULL)   
    	{  
        printf("Cannot open %s.\n",s1);  
        return -1;  
    	}  
    	
    if (bHigh)	
        strcpy(buffer,"1");  
    else
        strcpy(buffer,"0"); 
         
    fwrite(buffer, sizeof(char), sizeof(buffer) - 1, fp);         
    fclose(fp);  
    
    return 1;  
}
//==============================================================================
void Pega_Gpio_HW_Amp_Reset(void)
{
	 printf("\n%s\n", __func__);
	 //I2C communication is disabled in shutdown mode. 
	 IO_AMP_SHUTDOWN_ON();
	 usleep(100*1000);//30ms
	 IO_AMP_SHUTDOWN_OFF();
}

//==============================================================================
void Pega_Gpio_init(void)
{	   	   
	 GPIO_DBG(printf("\n%s\n", __func__));
	   
	 //for AMP
	 Pega_Gpio_pin_state_init(IO_O_AUDIO_SHUTDOWN, 		GPIO_OUTPUT, GPIO_LOW);
	 
	 Pega_Gpio_HW_Amp_Reset();
}

void Pega_Gpio_Data_Info_Print(void)
{     
#if 1	  
	 printf("\n-----------------------");	 	 
	 printf("\n IO_I_IDAC                => %d", IO_I_IDAC);
	 printf("\n IO_I_SYNC_BUTTON         => %d", IO_I_SYNC_BUTTON);	 
	 printf("\n IO_I_IW610_SPI_INT       => %d", IO_I_IW610_SPI_INT);
	 printf("\n IO_I_ACC_INT             => %d", IO_I_ACC_INT);
	 printf("\n IO_I_MOTOR_nFAULT_PAN    => %d", IO_I_MOTOR_nFAULT_PAN);
	 printf("\n IO_I_MOTOR_nFAULT_RISING => %d", IO_I_MOTOR_nFAULT_RISING);
	 printf("\n IO_I_NFC_IRQ             => %d", IO_I_NFC_IRQ);
	 printf("\n-----------------------");	 
	 printf("\n IO_O_IW610F_PDn          => %d", IO_O_IW610F_PDn);
	 printf("\n IO_O_IW610F_RST_WL       => %d", IO_O_IW610F_RST_WL);
	 printf("\n IO_O_IW610F_RST_BLE      => %d", IO_O_IW610F_RST_BLE);
	 printf("\n IO_O_IW610G_PDn          => %d", IO_O_IW610G_PDn);
	 printf("\n IO_O_IW610G_RST_WL       => %d", IO_O_IW610G_RST_WL);
	 printf("\n IO_O_IW610G_RST_Thread   => %d", IO_O_IW610G_RST_Thread);
	 printf("\n-----------------------");	
	 printf("\n IO_O_RISING_MOTOR_nSLEEP => %d", IO_O_RISING_MOTOR_nSLEEP);
	 printf("\n IO_O_PAN_MOTOR_nSLEEP    => %d", IO_O_PAN_MOTOR_nSLEEP);
	 printf("\n IO_O_MOTO_PWM0           => %d", IO_O_MOTO_PWM0);
	 printf("\n IO_O_MOTO_PWM1           => %d", IO_O_MOTO_PWM1);
	 printf("\n IO_O_MOTO_PWM2           => %d", IO_O_MOTO_PWM2);
	 printf("\n IO_O_MOTO_PWM3           => %d", IO_O_MOTO_PWM3);
	 printf("\n-----------------------");	 
	 printf("\n IO_O_IR_CUT_IN1          => %d", IO_O_IR_CUT_IN1);
	 printf("\n IO_O_IR_CUT_IN2          => %d", IO_O_IR_CUT_IN2);	 
	 printf("\n IO_O_CAM_1V8_EN          => %d", IO_O_CAM_1V8_EN);
	 printf("\n IO_O_LED_R               => %d", IO_O_LED_R);
	 printf("\n IO_O_LED_G               => %d", IO_O_LED_G);
	 printf("\n IO_O_LED_B               => %d", IO_O_LED_B);
	 printf("\n-----------------------\n");	 

#endif
}
//==============================================================================
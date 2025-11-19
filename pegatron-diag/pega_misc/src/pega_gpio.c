/*******************************************************************************
* File Name: pega_gpio.c
*
*******************************************************************************/
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
#include "pega_utilites.h"
//==============================================================================
#define VALUE_MAX 30
//==============================================================================
static int bIrcut_enable = -1;
//==============================================================================
static int Pega_Gpio_pin_state_init(int sGpioNum, int bInput, int bHigh) 
{  
    FILE *fp;           
    char s[50]="";  
    
    if ((fp = fopen("/sys/class/gpio/export", "w")) == NULL)   
    {  
        _LOG_ERROR("Cannot open export file.");  
        return FAILED;  
    }  
    	
    fprintf(fp, "%d", sGpioNum);  
    fclose(fp);  
  
    sprintf(s,"/sys/class/gpio/gpio%d/direction", sGpioNum);  
    
    if ((fp = fopen(s, "rb+")) == NULL)   
    {  
        _LOG_ERROR("Cannot open %s.",s);  
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
	char cmd[255]={0};
	char buff[1024]={0};
	 
	snprintf(cmd, sizeof(cmd), "echo %d > /sys/class/gpio/export", uGPIONum);
      
	if (pega_util_shell(cmd, buff, sizeof(buff)) != 0)
	{
        printf("Error: %s failed\n", cmd);        
    }
	
	if (bOutput)	 
	    snprintf(cmd, sizeof(cmd), "echo out > /sys/class/gpio/gpio%d/direction", uGPIONum);
	else
	    snprintf(cmd, sizeof(cmd), "echo in > /sys/class/gpio/gpio%d/direction", uGPIONum);
	         
    if (pega_util_shell(cmd, buff, sizeof(buff)) != 0)
	{
        printf("Error: %s failed\n", cmd);       
    } 
	 
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
		_LOG_ERROR("Failed to open gpio(%d) value for reading!", pin);
		return (FAILED);
	}

	if (-1 == read(fd, value_str, 3)) 
	{
		_LOG_ERROR("Failed to read gpio value!");
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
        _LOG_ERROR("Cannot open %s.",s1);  
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

//pega_gpio gpio init
void Pega_Gpio_init(void)
{	   	   
	 printf("\n%s\n", __func__);
	 //Set gpio to input
	 Pega_Gpio_pin_state_init(IO_I_IDAC, 				GPIO_INPUT, GPIO_NULL);
	 #if (BUTTON_DET_EN == 0)
	 Pega_Gpio_pin_state_init(IO_I_SYNC_BUTTON, 		GPIO_INPUT, GPIO_NULL);	 
	 #endif
	 Pega_Gpio_pin_state_init(IO_I_ACC_INT, 			GPIO_INPUT, GPIO_NULL);
	 // Pega_Gpio_pin_state_init(IO_I_NFC_IRQ, 			GPIO_INPUT, GPIO_NULL);     //controlled by gpiochip0 
	 //For Motor
	 #if (DEVICE_MOTOR_ENABLE == 1)
	 Pega_Gpio_pin_state_init(IO_I_MOTOR_nFAULT_PAN, 	GPIO_INPUT, GPIO_NULL);      
	 Pega_Gpio_pin_state_init(IO_I_MOTOR_nFAULT_RISING, GPIO_INPUT, GPIO_NULL);      
	 #endif
	 
	 //Set gpio to output
	 //can not control PDn to prevent loading fw failed.	 
	 Pega_Gpio_pin_state_init(IO_O_IW610F_RST_WL, 		GPIO_OUTPUT, GPIO_HIGH);
	 Pega_Gpio_pin_state_init(IO_O_IW610F_RST_BLE, 		GPIO_OUTPUT, GPIO_HIGH);
	 //can not control PDn to prevent loading fw failed.
	 Pega_Gpio_pin_state_init(IO_O_IW610G_RST_WL, 		GPIO_OUTPUT, GPIO_HIGH); 
	 //Pega_Gpio_pin_state_init(IO_O_IW610G_RST_Thread, 	GPIO_OUTPUT, GPIO_HIGH);  	//controlled by OTBR
	 
	 //For Audio amp.
	 Pega_Gpio_pin_state_init(IO_O_AUDIO_SHUTDOWN, 		GPIO_OUTPUT, GPIO_LOW);	 
	 //For Motor
	 #if (DEVICE_MOTOR_ENABLE == 1)
	 Pega_Gpio_pin_state_init(IO_O_RISING_MOTOR_nSLEEP, GPIO_OUTPUT, GPIO_LOW);
	 Pega_Gpio_pin_state_init(IO_O_PAN_MOTOR_nSLEEP,    GPIO_OUTPUT, GPIO_LOW);	 	 
	 Pega_Gpio_pin_state_init(IO_O_MOTOR_CTRL_PIN0, 	GPIO_OUTPUT, GPIO_LOW);
	 Pega_Gpio_pin_state_init(IO_O_MOTOR_CTRL_PIN1, 	GPIO_OUTPUT, GPIO_LOW);	 
	 Pega_Gpio_pin_state_init(IO_O_MOTOR_CTRL_PIN2, 	GPIO_OUTPUT, GPIO_LOW);
	 Pega_Gpio_pin_state_init(IO_O_MOTOR_CTRL_PIN3, 	GPIO_OUTPUT, GPIO_LOW);	 
	 #endif
	 //For IRcut
	 Pega_Gpio_pin_state_init(IO_O_IR_CUT_IN1, 		GPIO_OUTPUT, GPIO_LOW);
	 Pega_Gpio_pin_state_init(IO_O_IR_CUT_IN2, 		GPIO_OUTPUT, GPIO_LOW);	
	 //For image sensor 
	 Pega_Gpio_pin_state_init(IO_O_CAM_1V8_EN, 		GPIO_OUTPUT, GPIO_HIGH);
	 //For LED
	 Pega_Gpio_pin_state_init(IO_O_LED_R, 			GPIO_OUTPUT, GPIO_LOW);
	 Pega_Gpio_pin_state_init(IO_O_LED_G, 			GPIO_OUTPUT, GPIO_LOW);
	 Pega_Gpio_pin_state_init(IO_O_LED_B, 			GPIO_OUTPUT, GPIO_LOW);
}

void Pega_Gpio_Null(void)
{
	
}
//==============================================================================
//pega_misc_dbg set 3
void Pega_Gpio_HW_Amp_Reset(void)
{
	 printf("\n%s\n", __func__);
	 //I2C communication is disabled in shutdown mode. 
	 IO_AMP_SHUTDOWN_ON();
	 usleep(100*1000);//30ms
	 IO_AMP_SHUTDOWN_OFF();
}

int Pega_Gpio_IRCut_status_get(void)
{
	return bIrcut_enable;
}

void Pega_Gpio_IRCut_status_set(int enable)
{
	 bIrcut_enable = enable;
}

void Pega_Gpio_IRCut_Control(uint8_t bFlag)
{
	printf("%s[%d]\n", __FUNCTION__, bFlag);	
	
	#if 0
	if (Pega_Gpio_IRCut_status_get() == bFlag)
	{
		return;
	}
	#endif

	Pega_Gpio_IRCut_status_set(bFlag);	
		
	if (bFlag == ENABLE)
	{
		IO_IR_CUT1_CTRL_OFF(); 	
		IO_IR_CUT2_CTRL_ON(); 	
	} 
	else
	{
		 
        IO_IR_CUT1_CTRL_ON(); 	
		IO_IR_CUT2_CTRL_OFF();		
	}  	 	

	usleep(1000*200); //100ms, Spec is 100 ~ 250ms 2024/11/11 	 

	IO_IR_CUT1_CTRL_OFF(); 	
	IO_IR_CUT2_CTRL_OFF(); 	 
}
//pega_debug debug info gpio
void Pega_Gpio_Data_Info_Print(void)
{     
#if 1	  
	 printf("\n-----------------------");	 
	 printf("\n bIrcut_enable            => %d", bIrcut_enable);	 
	 printf("\n-----------------------");
	 printf("\n IO_I_IDAC                => %d", IO_I_IDAC);
	 printf("\n IO_I_SYNC_BUTTON         => %d", IO_I_SYNC_BUTTON);	 
	 printf("\n IO_I_IW610_SPI_INT       => %d", IO_I_IW610_SPI_INT);
	 printf("\n IO_I_ACC_INT             => %d", IO_I_ACC_INT);
	 printf("\n IO_I_MOTOR_nFAULT_PAN    => %d", IO_I_MOTOR_nFAULT_PAN);
	 printf("\n IO_I_MOTOR_nFAULT_RISING => %d", IO_I_MOTOR_nFAULT_RISING);
	 printf("\n IO_I_NFC_IRQ             => %d", IO_I_NFC_IRQ);
	 printf("\n-----------------------");
	 printf("\n IO_O_PWM_IR_LED          => %d", IO_O_PWM_IR_LED);
	 printf("\n IO_O_PWM_Spotlight_LED   => %d", IO_O_PWM_Spotlight_LED);
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
	 printf("\n IO_O_MOTOR_CTRL_PIN0       => %d", IO_O_MOTOR_CTRL_PIN0);
	 printf("\n IO_O_MOTOR_CTRL_PIN1       => %d", IO_O_MOTOR_CTRL_PIN1);
	 printf("\n IO_O_MOTOR_CTRL_PIN2       => %d", IO_O_MOTOR_CTRL_PIN2);
	 printf("\n IO_O_MOTOR_CTRL_PIN3       => %d", IO_O_MOTOR_CTRL_PIN3);
	 printf("\n-----------------------");
	 printf("\n IO_O_AUDIO_SHUTDOWN      => %d", IO_O_AUDIO_SHUTDOWN);
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

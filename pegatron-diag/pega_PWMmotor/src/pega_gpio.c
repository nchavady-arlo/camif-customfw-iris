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
#define IFS_PWM_DRIVER	/sys/class/ifs/pwm/

#define DEBUG 0
#if DEBUG
    #define debug_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define debug_log(fmt, ...) ((void)0)
#endif
//==============================================================================
static int Pega_Gpio_pin_state_init(int sGpioNum, int bInput, int bHigh) 
{  
    FILE *fp;           
    char s[50]="";  
    
    if ((fp = fopen("/sys/class/gpio/export", "w")) == NULL)   
    {  
        printf("Cannot open export file.\n");  
        return FAILED;  
    }  
    	
    fprintf(fp, "%d", sGpioNum);  
    fclose(fp);  
  
    sprintf(s,"/sys/class/gpio/gpio%d/direction", sGpioNum);  
    
    if ((fp = fopen(s, "rb+")) == NULL)   
    {  
        printf("Cannot open %s.\n",s);  
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

static int Pega_Gpio_pin_state_deinit(int sGpioNum) 
{  
    FILE *fp;           
    char s[50]="";  
    
    if ((fp = fopen("/sys/class/gpio/unexport", "w")) == NULL)   
    {  
        printf("Cannot open unexport file.\n");  
        return FAILED;  
    }  
    	
    fprintf(fp, "%d", sGpioNum);  
    fclose(fp);   
            
    return SUCCEED;  
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
		printf("Failed to open gpio(%d) value for reading!\n", pin);
		return (FAILED);
	}

	if (-1 == read(fd, value_str, 3)) 
	{
		printf("Failed to read gpio value!\n");
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
        printf("Cannot open %s.\n",s1);  
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
	 Pega_Gpio_pin_state_init(IO_I_MOTOR_nFAULT_PAN, 	GPIO_INPUT, GPIO_NULL);      
	 Pega_Gpio_pin_state_init(IO_I_MOTOR_nFAULT_RISING, GPIO_INPUT, GPIO_NULL);      
	 //Set gpio to output
	 Pega_Gpio_pin_state_init(IO_O_RISING_MOTOR_nSLEEP, GPIO_OUTPUT, GPIO_LOW);
	 Pega_Gpio_pin_state_init(IO_O_PAN_MOTOR_nSLEEP,    GPIO_OUTPUT, GPIO_LOW);
	 	 
	 Pega_Gpio_pin_state_init(IO_O_MOTO_PWM0, 		GPIO_OUTPUT, GPIO_LOW);
	 Pega_Gpio_pin_state_init(IO_O_MOTO_PWM1, 		GPIO_OUTPUT, GPIO_LOW);	 
	 Pega_Gpio_pin_state_init(IO_O_MOTO_PWM2, 		GPIO_OUTPUT, GPIO_LOW);
	 Pega_Gpio_pin_state_init(IO_O_MOTO_PWM3, 		GPIO_OUTPUT, GPIO_LOW);	 	 
	 
}


void Pega_PWM_config(char* PWMgroup,char* attr,char* value)
{
    FILE *fp;           
    char path[256]="";  
    snprintf(path, sizeof(path), "/sys/class/ifs/pwm/%s/%s", PWMgroup, attr);
    debug_log("\t[debug] %s > %s\n",value,path);
    if ((fp = fopen(path, "w")) == NULL)   
    {  
        printf("Cannot open %s file.\n", path);  
        return FAILED;  
    }
    if (fprintf(fp, "%s", value) < 0) {
        perror("Error writing to file");
    }
    fclose(fp);
}

void Pega_IFS_PWM_init(void)
{
    // Pega_PWM_config(PWMgroup,attr,value)
    // Init GROUP0 
    /*	Add the pwm0 	*/
    Pega_PWM_config(IFS_PWMGROUP0,IFS_JOIN,ISF_GRP_ADD_0);
    /*	Add the pwm1	*/
    Pega_PWM_config(IFS_PWMGROUP0,IFS_JOIN,ISF_GRP_ADD_1);
    /*	Remove the unused pwm2	*/
    Pega_PWM_config(IFS_PWMGROUP0,IFS_JOIN,ISF_GRP_DEL_2);
    /*	Remove the unused pwm3	*/
    Pega_PWM_config(IFS_PWMGROUP0,IFS_JOIN,ISF_GRP_DEL_3);
    /*  Group Period 0 ns       */
    Pega_PWM_config(IFS_PWMGROUP0,IFS_G_PERIOD,      "0");
    /*  Group Duty   0 ns       */
    Pega_PWM_config(IFS_PWMGROUP0,IFS_G_DUTY,        "0");
    /*  Group Shift  0 ns       */
    Pega_PWM_config(IFS_PWMGROUP0,IFS_G_SHIFT,       "0");
    /*  Group Poloarity normal  */
    Pega_PWM_config(IFS_PWMGROUP0,IFS_G_POLARITY,    "0");
    /*  Group Enable : disable  */
    Pega_PWM_config(IFS_PWMGROUP0,IFS_G_ENABLE,      "0");
    // Init GROUP1 
    /*	Add the pwm4 	*/
    Pega_PWM_config(IFS_PWMGROUP1,IFS_JOIN,ISF_GRP_ADD_4);
    /*	Add the pwm5	*/
    Pega_PWM_config(IFS_PWMGROUP1,IFS_JOIN,ISF_GRP_ADD_5);
    /*	Remove the unused pwm6	*/
    Pega_PWM_config(IFS_PWMGROUP1,IFS_JOIN,ISF_GRP_DEL_6);
    /*	Remove the unused pwm7  */
    Pega_PWM_config(IFS_PWMGROUP1,IFS_JOIN,ISF_GRP_DEL_7);
    /*  Group Period 0 ns       */
    Pega_PWM_config(IFS_PWMGROUP1,IFS_G_PERIOD,      "0");
    /*  Group Duty   0 ns       */
    Pega_PWM_config(IFS_PWMGROUP1,IFS_G_DUTY,        "0");
    /*  Group Shift  0 ns       */
    Pega_PWM_config(IFS_PWMGROUP1,IFS_G_SHIFT,       "0");
    /*  Group Poloarity normal  */
    Pega_PWM_config(IFS_PWMGROUP1,IFS_G_POLARITY,    "0");
    /*  Group Enable : disable  */
    Pega_PWM_config(IFS_PWMGROUP1,IFS_G_ENABLE,      "0");
}

void Pega_PWM_init(void)
{
	printf("\n%s\n", __func__);
	// Unexport GPIO
	Pega_Gpio_pin_state_deinit(PAD_PM_FUART_RX);
	Pega_Gpio_pin_state_deinit(PAD_PM_FUART_TX);
	Pega_Gpio_pin_state_deinit(PAD_OUTN_RX0_CH4);
	Pega_Gpio_pin_state_deinit(PAD_OUTP_RX0_CH4);
	// Init PWM config by IFS Driver
	Pega_IFS_PWM_init();
}

void Pega_PWM_Forward_Fullstep(char* speed, char* round)
{
    char attr[256];
    //printf("speed: %s.\nround: %s.\n",speed,round);
/*  Default Group0 PWM Config   */    
    //  config Group0
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_PERIOD,    "1000000000");
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_POLARITY,           "0");
    //  config pwm 0
    //  attr = pwm0/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM0, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP0, attr, "500000000");
    // attr = pwm0/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM0, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP0, attr,         "0");
    //  config pwm 1
    //  attr = pwm1/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM1, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP0, attr, "1000000000");
    // attr = pwm0/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM1, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP0, attr,  "500000000");
/*  Default Group1 PWM Config   */    
    //  config Group1
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_PERIOD,    "1000000000");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_POLARITY,           "0");
    //  config pwm 4
    //  attr = pwm4/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "750000000");
    // attr = pwm4/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "250000000");
    // attr = pwm4/polarity
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_POLARITY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, IFS_POLARITY_INVERSE);
    //  config pwm 5
    //  attr = pwm5/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM5, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "750000000");
    // attr = pwm5/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM5, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "250000000");
/*  Update the PWM Freq. (Speed)   */
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_PERIOD,    speed);
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_PERIOD,    speed);
/*  Update Freq.                   */
    Pega_PWM_config(IFS_PWMGROUP0, IFS_UPDATE,    "1");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_UPDATE,    "1");
/*  Enable PWM                      */
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_ENABLE,    "1");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_ENABLE,    "1");
/*  Update the PWM Round. (Pulse)   */
    //printf("1. %s \n",round);
    Pega_PWM_config(IFS_PWMGROUP0, IFS_ROUND,    round);
    //printf("2. %s \n",round);
    Pega_PWM_config(IFS_PWMGROUP1, IFS_ROUND,    round);
}

void Pega_PWM_Reverse_Fullstep(char* speed, char* round)
{
    char attr[256];
/*  Default Group0 PWM Config   */    
    //  config Group0
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_PERIOD,    "1000000000");
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_POLARITY,           "0");
    //  config pwm 0
    //  attr = pwm0/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM0, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP0, attr, "1000000000");
    // attr = pwm0/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM0, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP0, attr,  "500000000");
    //  config pwm 1
    //  attr = pwm1/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM1, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP0, attr,  "500000000");
    // attr = pwm0/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM1, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP0, attr,          "0");
/*  Default Group1 PWM Config   */    
    //  config Group1
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_PERIOD,    "1000000000");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_POLARITY,           "0");
    //  config pwm 4
    //  attr = pwm4/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "750000000");
    // attr = pwm4/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "250000000");
    // attr = pwm4/polarity
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_POLARITY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, IFS_POLARITY_INVERSE);
    //  config pwm 5
    //  attr = pwm5/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM5, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "750000000");
    // attr = pwm5/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM5, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "250000000");
/*  Update the PWM Freq. (Speed)   */    
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_PERIOD,    speed);
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_PERIOD,    speed);
/*  Update Freq.                   */
    Pega_PWM_config(IFS_PWMGROUP0, IFS_UPDATE,    "1");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_UPDATE,    "1");
/*  Enable PWM                      */
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_ENABLE,    "1");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_ENABLE,    "1");
/*  Update the PWM Round. (Pulse)   */
    //printf("1. %s \n",round);
    Pega_PWM_config(IFS_PWMGROUP0, IFS_ROUND,    round);
    //printf("2. %s \n",round);
    Pega_PWM_config(IFS_PWMGROUP1, IFS_ROUND,    round);
}

void Pega_PWM_Forward_Halfstep(char* speed, char* round)
{
    char attr[256];
/*  Default Group0 PWM Config   */    
    //  config Group0
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_PERIOD,    "1000000000");
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_POLARITY,           "0");
    //  config pwm 0
    //  attr = pwm0/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM0, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP0, attr, "650000000");
    // attr = pwm0/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM0, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP0, attr,         "0");
    //  config pwm 1
    //  attr = pwm1/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM1, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP0, attr,  "500000000");
    // attr = pwm1/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM1, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP0, attr,  "125000000");
    // attr = pwm1/polarity
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM1, IFS_POLARITY);
    Pega_PWM_config(IFS_PWMGROUP0, attr, IFS_POLARITY_INVERSE);
/*  Default Group1 PWM Config   */    
    //  config Group1
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_PERIOD,    "1000000000");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_POLARITY,           "0");
    //  config pwm 4
    //  attr = pwm4/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "750000000");
    // attr = pwm4/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "375000000");
    // attr = pwm4/polarity
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_POLARITY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, IFS_POLARITY_INVERSE);
    //  config pwm 5
    //  attr = pwm5/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM5, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "875000000");
    // attr = pwm5/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM5, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "250000000");
/*  Update the PWM Freq. (Speed)   */    
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_PERIOD,    speed);
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_PERIOD,    speed);
/*  Update Freq.                   */
    Pega_PWM_config(IFS_PWMGROUP0, IFS_UPDATE,    "1");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_UPDATE,    "1");
/*  Enable PWM                      */
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_ENABLE,    "1");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_ENABLE,    "1");
/*  Update the PWM Round. (Pulse)   */
    //printf("1. %s \n",round);
    Pega_PWM_config(IFS_PWMGROUP0, IFS_ROUND,    round);
    //printf("2. %s \n",round);
    Pega_PWM_config(IFS_PWMGROUP1, IFS_ROUND,    round);
}

void Pega_PWM_Reverse_Halfstep(char* speed, char* round)
{
    char attr[256];
/*  Default Group0 PWM Config   */    
    //  config Group0
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_PERIOD,    "1000000000");
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_POLARITY,           "0");
    //  config pwm 0
    //  attr = pwm0/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM0, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP0, attr, "1000000000");
    // attr = pwm0/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM0, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP0, attr,  "375000000");
    //  config pwm 1
    //  attr = pwm1/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM1, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP0, attr,  "875000000");
    // attr = pwm1/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM1, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP0, attr,  "500000000");
    // attr = pwm1/polarity
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM1, IFS_POLARITY);
    Pega_PWM_config(IFS_PWMGROUP0, attr, IFS_POLARITY_INVERSE);
/*  Default Group1 PWM Config   */    
    //  config Group1
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_PERIOD,    "1000000000");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_POLARITY,           "0");
    //  config pwm 4
    //  attr = pwm4/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "625000000");
    // attr = pwm4/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "250000000");
    // attr = pwm4/polarity
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM4, IFS_POLARITY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, IFS_POLARITY_INVERSE);
    //  config pwm 5
    //  attr = pwm5/duty
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM5, IFS_DUTY);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "750000000");
    // attr = pwm5/shift
    snprintf(attr, sizeof(attr), "%s/%s", IFS_PWM5, IFS_SHIFT);
    Pega_PWM_config(IFS_PWMGROUP1, attr, "125000000");
/*  Update the PWM Freq. (Speed)   */    
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_PERIOD,    speed);
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_PERIOD,    speed);
/*  Update Freq.                   */
    Pega_PWM_config(IFS_PWMGROUP0, IFS_UPDATE,    "1");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_UPDATE,    "1");
/*  Enable PWM                      */
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_ENABLE,    "1");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_ENABLE,    "1");
/*  Update the PWM Round. (Pulse)   */
    //printf("1. %s \n",round);
    Pega_PWM_config(IFS_PWMGROUP0, IFS_ROUND,    round);
    //printf("2. %s \n",round);
    Pega_PWM_config(IFS_PWMGROUP1, IFS_ROUND,    round);
}

void Pega_PWM_Pause(void)
{
    printf("Pause the PWM.\n");
    // Pause Group0, Group1 PWM
    Pega_PWM_config(IFS_PWMGROUP0, IFS_STOP,    "1");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_STOP,    "1");
}

void Pega_PWM_Resume(void)
{
    printf("Resume the PWM.\n");
    // Pause Group0, Group1 PWM
    Pega_PWM_config(IFS_PWMGROUP0, IFS_STOP,    "0");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_STOP,    "0");
}

void Pega_PWM_Start(void)
{
    printf("Start the PWM.\n");
    // Pause Group0, Group1 PWM
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_ENABLE,    "1");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_ENABLE,    "1");
}

void Pega_PWM_Stop(void)
{
    printf("Stpp the PWM.\n");
    // Pause Group0, Group1 PWM
    Pega_PWM_config(IFS_PWMGROUP0, IFS_G_ENABLE,    "0");
    Pega_PWM_config(IFS_PWMGROUP1, IFS_G_ENABLE,    "0");
}

//==============================================================================

/*******************************************************************************
* File Name: pega_gpiod.c
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
#include <sys/sysmacros.h>
#include <linux/gpio.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <gpiod.h>
//==============================================================================
#include "pega_gpiod.h"
//==============================================================================
#define GPIO_CONTROLLER             "/dev/gpiochip0"
#define GPIO_CONTROLLER_PATH        "/sys/class/gpio/gpiochip0/device/dev"
//==============================================================================
static struct gpiod_chip *m_ptChip = NULL;
//==============================================================================
//gpioset -s 5 --mode=time gpiochip0 70=1
//gpioset --mode=wait gpiochip0 70=1
//==============================================================================
#define SYS_GPIO_NUM        sizeof(m_tGpioInitTbl) / sizeof(stGpio_cfg_t)
//==============================================================================
static stGpio_cfg_t m_tGpioInitTbl[] = 
{
     {
     .pad = IO_I_SYNC_BUTTON,
     .pad_desc = "SYNC_BUTTON",     
     .direction = GPIO_DIR_INPUT,
     .value = NOT_THING,
     },
	 {
     .pad = IO_I_MOTOR_nFAULT,
     .pad_desc = "MOTOR_nFAULT",     
     .direction = GPIO_DIR_INPUT,
     .value = NOT_THING,
     },
	 {
     .pad = IO_I_PAN_MOTOR_ERR,
     .pad_desc = "PAN_MOTOR_ERR",     
     .direction = GPIO_DIR_INPUT,
     .value = NOT_THING,
     },
	 {
     .pad = IO_I_RISING_MOTOR_ERR,
     .pad_desc = "RISING_MOTOR_ERR",     
     .direction = GPIO_DIR_INPUT,
     .value = NOT_THING,
     },
	 {
     .pad = IO_O_IW610F_RST_WL,
     .pad_desc = "IW610F_RST_WL",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_HIGH,
     },
	 {
     .pad = IO_O_IW610F_RST_BLE,
     .pad_desc = "IW610F_RST_BLE",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_HIGH,
     },
	 {
     .pad = IO_O_IW610G_RST_WL,
     .pad_desc = "IW610G_RST_WL",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_HIGH,
     },
	 {
     .pad = IO_O_AUDIO_SHUTDOWN,
     .pad_desc = "AUDIO_AMP_SDZ",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_RISING_MOTOR_nSLEEP,
     .pad_desc = "RISING_MOTOR_nSLEEP",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_PAN_MOTOR_nSLEEP,
     .pad_desc = "PAN_MOTOR_nSLEEP",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_MOTOR_CTR0,
     .pad_desc = "MOTOR_CTR0",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_MOTOR_CTR1,
     .pad_desc = "IO_O_MOTOR_CTR1",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_MOTOR_CTR2,
     .pad_desc = "MOTOR_CTR2",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_MOTOR_CTR3,
     .pad_desc = "MOTOR_CTR3",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_IR_CUT_IN1,
     .pad_desc = "IR_CUT_IN1",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_IR_CUT_IN2,
     .pad_desc = "IR_CUT_IN2",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_CAM_1V8_EN,
     .pad_desc = "CAM_1V8_EN",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_HIGH,
     },
	 {
     .pad = IO_O_LED_R,
     .pad_desc = "LED_R",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_LED_G,
     .pad_desc = "LED_G",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_LED_B,
     .pad_desc = "LED_B",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_IR_LED_EN,
     .pad_desc = "IR_LED_EN",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
	 {
     .pad = IO_O_Spotlight_LED_EN,
     .pad_desc = "Spotligh_LED_EN",     
     .direction = GPIO_DIR_OUTPUT,
     .value = GPIO_LOW,
     },
};
//==============================================================================
int pga_gpio_chip_init(void)
{		
    const char *device_path = GPIO_CONTROLLER;
    const char *sysfs_dev_path = GPIO_CONTROLLER_PATH;
    int major = 0, minor = 0;

    if (access(device_path, F_OK) == 0) 
	{
        return 1;
    }

    FILE *fp = fopen(sysfs_dev_path, "r");
    if (!fp) 
	{
        perror("Failed to open sysfs device file");
        return -1;
    }

    if (fscanf(fp, "%d:%d", &major, &minor) != 2) 
	{
        fprintf(stderr, "Failed to parse major:minor from %s\n", sysfs_dev_path);
        fclose(fp);
        return -1;
    }
	
    fclose(fp);

    dev_t dev = makedev(major, minor);	
	
    if (mknod(device_path, S_IFCHR | 0666, dev) == -1) 
	{		
        if (errno == EEXIST)
		{
            printf("%s already exists.\n", device_path);
        }
		else
		{
			perror("Failed to create /dev/gpiochip0");
			return -1;
		}		
    } 
			
	return 0;
}

int pga_gpio_init(void)
{
	struct gpiod_line *line;
    int i,ret;
	
    m_ptChip = gpiod_chip_open(GPIO_CONTROLLER);
		
    if (!m_ptChip) 
	{
        perror("Open gpiochip failed");
        return -1;
    }
    
	printf("\n[%s] SYS_GPIO_NUM=%d \n", __func__, SYS_GPIO_NUM);  
	
	for (i=0;i<SYS_GPIO_NUM;i++)
	{
		 line = gpiod_chip_get_line(m_ptChip, m_tGpioInitTbl[i].pad);
		
         if (m_tGpioInitTbl[i].direction == GPIO_DIR_INPUT)
		 {
			 ret = gpiod_line_request_input(line, m_tGpioInitTbl[i].pad_desc);			  
		 }
		 else
		 {
			 
			 ret = gpiod_line_request_output(line, m_tGpioInitTbl[i].pad_desc, m_tGpioInitTbl[i].value);
		 }
		 
		 if (ret < 0) 
		 {
			 perror("Request line failed.");
			 //printf("Request line failed.(Pad:%d)", m_tGpioInitTbl[i].pad);
			 break;
		 }
	}		
	
End:	
    
	IO_LED_G_CTRL_ON();
	
	sleep(5);
	//gpiod_chip_close(m_ptChip);

    return 0;
}

void pga_gpio_control(const uint8_t pad, const uint8_t value)
{
	 struct gpiod_line *line;
	 
	 if (m_ptChip == NULL) 
	 {
        perror("m_ptChip is Null");
        return;
     }
	
	 line = gpiod_chip_get_line(m_ptChip, pad);
	 
	 if (!line) 
	 {
        perror("Get line failed");
     }
	 else
	 {
		if (gpiod_line_set_value(line, value) < 0) 
		{
			perror("Set line value failed");
		} 	 
	 }
}

void pga_gpio_ircut_control(uint8_t bFlag)
{
	printf("%s[%d]\n", __FUNCTION__, bFlag);	
		
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
//==============================================================================
struct gpiod_chip* pga_gpio_chip_get(void)
{
	return m_ptChip;	
}

int pga_gpio_test(void)
{
	struct gpiod_line *line;
    int ret;
	
    m_ptChip = gpiod_chip_open(GPIO_CONTROLLER);
		
    if (!m_ptChip) 
	{
        perror("Open gpiochip failed");
        return 1;
    }

    // 取得指定的 GPIO 腳位
    line = gpiod_chip_get_line(m_ptChip, IO_O_LED_R);	
    if (!line) 
	{
        perror("Get line failed");        
        goto End;
    }
	
	ret = gpiod_line_request_output(line, "LED_B", 0);
	
    if (ret < 0) 
	{
        perror("Request line as output failed");
        goto End;
    }
	
	// 取得指定的 GPIO 腳位
    line = gpiod_chip_get_line(m_ptChip, IO_O_LED_G);	
    if (!line) 
	{
        perror("Get line failed");        
        goto End;
    }
	
	ret = gpiod_line_request_output(line, "LED_G", 1);
	
    if (ret < 0) 
	{
        perror("Request line as output failed");
        goto End;
    }
	
	// 取得指定的 GPIO 腳位
    line = gpiod_chip_get_line(m_ptChip, IO_O_LED_B);	
    if (!line) 
	{
        perror("Get line failed");        
        goto End;
    }
	
	ret = gpiod_line_request_output(line, "LED_B", 0);
	
    if (ret < 0) 
	{
        perror("Request line as output failed");
        goto End;
    }

End:	

    sleep(5);
	
	gpiod_chip_close(m_ptChip);

    return 0;
}
//==============================================================================
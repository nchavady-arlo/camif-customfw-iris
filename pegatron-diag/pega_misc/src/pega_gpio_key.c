/*******************************************************************************
* File Name: Pega_gpio_key.c
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gpiod.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <poll.h>
#include <sys/prctl.h>
#include <gpiod.h>
//==============================================================================
#include "pega_gpio_key.h"
#include "pega_gpio.h"
#include "pega_schedule.h"
//==============================================================================
#define GPIO_CONTROLLER             "/dev/gpiochip0"
#define GPIO_CONTROLLER_PATH        "/sys/class/gpio/gpiochip0/device/dev"
//==============================================================================
#define CONSUMER 				"sync-button"
#define GPIO_LINE 				IO_I_SYNC_BUTTON  // 你的 GPIO 腳位號
//==============================================================================
static stKeyDetType  m_stKey;
//==============================================================================
static double diff_time(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}
//==============================================================================
static void* Pega_Gpio_Key_Detection_Handler(void* argv)
{	 
     (void)argv;
	 struct gpiod_chip *chip;
     struct gpiod_line *line;
     struct gpiod_line_event event;
     struct timespec press_time, release_time;
     int ret;
     	 
	 pthread_detach(pthread_self());
	 prctl(PR_SET_NAME, THREAD_PROC_4); //set the thread name
	 
	 // 開啟 GPIO chip
    chip = gpiod_chip_open(GPIO_CONTROLLER);
    
	if (!chip) 
	{
        perror("gpiod_chip_open");
        return 0;
    }

    // 取得 GPIO line
    line = gpiod_chip_get_line(chip, GPIO_LINE);
    if (!line) 
	{
        perror("gpiod_chip_get_line");
        gpiod_chip_close(chip);
        return 0;
    }

    // 要偵測上下沿
    ret = gpiod_line_request_both_edges_events(line, CONSUMER);	
    if (ret < 0) 
	{
        perror("gpiod_line_request_both_edges_events");
        gpiod_chip_close(chip);
        return 0;
    }

    printf("Detecting button press duration on GPIO%d...\n", GPIO_LINE);

    while (1) 
	{
        // wait for interrupt event
        ret = gpiod_line_event_wait(line, NULL);
		
        if (ret < 0) 
		{
            perror("gpiod_line_event_wait");
            break;
        } 
		else if (ret == 0) 
		{
            continue;
        }

        ret = gpiod_line_event_read(line, &event);
		
        if (ret < 0) 
		{
            perror("gpiod_line_event_read");
            break;
        }

        if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) 
		{           
            m_stKey.bKeyPressed = 1;
			m_stKey.bKeyPressedCnt++;
			
            press_time = event.ts;
            printf("Button pressed.\n");
        } 
		else if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) 
		{           
            if ( m_stKey.bKeyPressed > 0) 
			{
                release_time = event.ts;
                double duration = diff_time(press_time, release_time);
                printf("Button released. Press duration: %.3f seconds\n", duration);
                 m_stKey.bKeyPressed = 0;
                
                if (duration < 0.5)
                    printf("=> Short press detected\n");
                else if (duration < 2.0)
                    printf("=> Medium press detected\n");
                else
                    printf("=> Long press detected\n");
            }
        }
    }

    //gpiod_line_release(line);
    //gpiod_chip_close(chip);
	 
	return 0;
}

void Pega_Gpio_key_Detection_Start(void)
{	   
	 pthread_t thread_id;    
               
     pthread_create(&thread_id, NULL, &Pega_Gpio_Key_Detection_Handler, NULL); 
          
}
//==============================================================================
void Pega_Gpio_key_Detection_Clear(void)
{	   
	 //printf("\n[%s]\n", __func__);       
     memset(&m_stKey, 0, sizeof(m_stKey));     
}

uint8_t Pega_Gpio_Key_Button_Is_pressed(void)
{
	return (m_stKey.bKeyPressed > 0);
}
//pega_debug debug info key
void Pega_Gpio_Key_Data_Info_Print(void)
{     
#if 1	
	 	 
	 printf("\n-----------------------");
	 printf("\n m_stKey.bKeyPressed    => %d", m_stKey.bKeyPressed);
	 printf("\n m_stKey.bKeyPressedCnt => %d", m_stKey.bKeyPressedCnt);
	 printf("\n-----------------------\n");	 

#endif	   
}
//==============================================================================
/*******************************************************************************
* File Name: pega_gpiod_int.c
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
#include <poll.h>
//==============================================================================
#include "pega_gpiod.h"
#include "pega_gpiod_int.h"
//==============================================================================
#define CONSUMER "dual_interrupt"
#define LINE1 17
#define LINE2 18
//==============================================================================
static pthread_t m_pthread_id = 0;
//==============================================================================
#define SYS_GPIO_INT_NUM        sizeof(m_tGpioIntTbl) / sizeof(stGpio_cfg_t)
//==============================================================================
//#define GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE  1
//#define GPIOD_LINE_REQUEST_EVENT_RISING_EDGE   2
//#define GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES    3
static stGpio_cfg_t m_tGpioIntTbl[] = 
{
     {
     .pad = IO_INT_ALS_IRQ,
     .pad_desc = "ALS_INT",     
     .direction = GPIO_DIR_INT,
     .value = GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE,//GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES,
     },
	 {
     .pad = IO_I_PAN_MOTOR_ERR,
     .pad_desc = "PAN_MOTOR_ERR_INT",     
     .direction = GPIO_DIR_INT,
     .value = GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES,
     },
	 {
     .pad = IO_I_RISING_MOTOR_ERR,
     .pad_desc = "RISING_MOTOR_ERR_INT",     
     .direction = GPIO_DIR_INT,
     .value = GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES,
     },	 
};
//==============================================================================
/*
常數	意義
POLLIN	有資料可讀（read 不會 block）
POLLOUT	可寫入（write 不會 block）
POLLERR	發生錯誤（read/write 均會失敗）
POLLHUP	對端關閉（如 socket 關閉）
POLLPRI	有高優先權資料可讀（如 out-of-band data）
*/
static void* Pega_gpiod_interrupt_TaskHandler(void* argv)
{			 
	(void)argv;
    int i;
    struct gpiod_chip *chip = NULL;
    struct gpiod_line *line[SYS_GPIO_INT_NUM] = {0};
    struct gpiod_line_request_config config;
    struct gpiod_line_event event;
    struct pollfd pfd[SYS_GPIO_INT_NUM];
    int ret;

    chip = pga_gpio_chip_get();
    if (!chip) 
	{
        perror("Open chip failed");
        return NULL;
    }

    for (i = 0; i < SYS_GPIO_INT_NUM; i++) 
	{
        line[i] = gpiod_chip_get_line(chip, m_tGpioIntTbl[i].pad);
        if (!line[i]) 
		{
            fprintf(stderr, "Get line failed: %s\n", m_tGpioIntTbl[i].pad_desc);
            goto cleanup;
        }

        config.consumer = m_tGpioIntTbl[i].pad_desc;
        config.request_type = m_tGpioIntTbl[i].value;
        config.flags = 0;

        if (gpiod_line_request(line[i], &config, 0) < 0) 
		{
            fprintf(stderr, "Request line failed: %s\n", m_tGpioIntTbl[i].pad_desc);
            goto cleanup;
        }

        pfd[i].fd = gpiod_line_event_get_fd(line[i]);
        pfd[i].events = POLLIN;
        printf("Listening on [%s] (GPIO %d)\n", m_tGpioIntTbl[i].pad_desc, m_tGpioIntTbl[i].pad);
    }

    while (1) 
	{
        ret = poll(pfd, SYS_GPIO_INT_NUM, -1);
		
        if (ret < 0) 
		{
            perror("poll");
            break;
        }

        for (i = 0; i < SYS_GPIO_INT_NUM; i++) 
		{
            if (pfd[i].revents & POLLIN) 
			{
                if (gpiod_line_event_read(line[i], &event) == 0) 
				{
                    printf("[%s] %s edge\n",
                           m_tGpioIntTbl[i].pad_desc,
                           event.event_type == GPIOD_LINE_EVENT_RISING_EDGE ?
                               "Rising" : "Falling");
                }
            }
        }
    }

cleanup:

    for (i = 0; i < SYS_GPIO_INT_NUM; i++) 
	{
        if (line[i])
            gpiod_line_release(line[i]);
    }

    if (chip)
        gpiod_chip_close(chip);

    return NULL;
}
//==============================================================================
void Pega_gpiod_interrupt_handler_start(void)
{
	 //pthread_t thread_id;
	                
     //printf("\n%s", __FUNCTION__);	
     pthread_create(&m_pthread_id, NULL, &Pega_gpiod_interrupt_TaskHandler, NULL);     
}

void Pega_gpiod_interrupt_handler_stop(void)
{
	 if (m_pthread_id != 0)
	 {
		 pthread_cancel(m_pthread_id);
		 m_pthread_id = 0;
	 }
}
//==============================================================================
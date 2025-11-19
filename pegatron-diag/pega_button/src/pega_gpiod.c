/*******************************************************************************
* File Name: pega_gpiod.c
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gpiod.h>
#include <time.h>
#include <gpiod.h>
//==============================================================================
#include "pega_gpiod.h"
//==============================================================================
#define GPIO_CONTROLLER             "/dev/gpiochip0"
#define GPIO_CONTROLLER_PATH        "/sys/class/gpio/gpiochip0/device/dev"
//==============================================================================
#define CONSUMER 				"sync-button"
#define GPIO_LINE 				PAD_I2C2_SCL                 // 你的 GPIO 腳位號
//==============================================================================
static double diff_time(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}
//==============================================================================
int pga_syn_button(void)
{
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    struct gpiod_line_event event;
    struct timespec press_time, release_time;
    int ret;
    int pressed = 0;

    // 開啟 GPIO chip
    chip = gpiod_chip_open(GPIO_CONTROLLER);
    if (!chip) {
        perror("gpiod_chip_open");
        return -1;
    }

    // 取得 GPIO line
    line = gpiod_chip_get_line(chip, GPIO_LINE);
    if (!line) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(chip);
        return -1;
    }

    // 要偵測上下沿
    ret = gpiod_line_request_both_edges_events(line, CONSUMER);
	
    if (ret < 0) {
        perror("gpiod_line_request_both_edges_events");
        gpiod_chip_close(chip);
        return -1;
    }

    printf("Detecting button press duration on GPIO%d...\n", GPIO_LINE);

    while (1) 
	{
        // 等待事件 (阻塞直到有事件)
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
            // 假設按鍵按下是下降沿
            pressed = 1;
            press_time = event.ts;
            printf("Button pressed.\n");
        } 
		else if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) 
		{
            // 假設放開是上升沿
            if (pressed) 
			{
                release_time = event.ts;
                double duration = diff_time(press_time, release_time);
                printf("Button released. Press duration: %.3f seconds\n", duration);
                pressed = 0;

                // 依時間分類（可選）
                if (duration < 0.5)
                    printf("→ Short press detected\n");
                else if (duration < 2.0)
                    printf("→ Medium press detected\n");
                else
                    printf("→ Long press detected\n");
            }
        }
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}
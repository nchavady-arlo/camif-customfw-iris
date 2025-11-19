/*******************************************************************************
* File Name: pega_wifi.c
*
*******************************************************************************/
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/un.h>
#include <signal.h>
#include <stddef.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/msg.h>
//==============================================================================
#include "main.h"
//==============================================================================
#include "pega_wifi.h"
#include "pega_utilites.h"
#include "pega_ble_uart.h"
//==============================================================================
static pthread_t m_thread_id = 0;
//==============================================================================
static int Pega_wifi_load_firmware(void)
{		 
	 int bIsReady = 0;
			 
	 if (Pega_Ble_uart_device_open() == TRUE)
	 {
		 Pega_Ble_uart_init(115200, 0);
		 bIsReady = Pega_Ble_uart_BringUp_Ready(3*1000);
		 Pega_Ble_uart_device_close();		
	 }
	 	 
     return bIsReady;
}
//==============================================================================
void Pega_wifi_load_firmware_enable(void)
{		 
	 int bIsReady = 0;
	 char buff[1024]={0};
	 
     bIsReady = Pega_wifi_load_firmware();	
	 
	 if (bIsReady > 0)
	 {
		 //system("sh /scripts/IW610_wifi_on.sh");
		 //using modprobe to load wifi driver. 2025/09/12
		 if (pega_util_shell("sh /scripts/IW610_wifi_enable.sh", buff, sizeof(buff)) != 0)
		  {
			 printf("Error: %s failed\n", "sh /scripts/IW610_wifi_enable.sh");       
		  }  
	 }
	 else
	 {
		 pega_misc_wifi_fw_error();
	 }
	 
	 //printf("\n[%s]%d\n", __func__, bIsReady); 	 
}
//==============================================================================
void Pega_wifi_connect_enable(void)
{	    
     char buff[1024]={0};
	 
     if (pega_util_shell("sh /scripts/wifi_connect.sh", buff, sizeof(buff)) != 0)
	 {
        printf("Error: %s failed\n", "/scripts/wifi_connect.sh");       
     }  
}
//==============================================================================
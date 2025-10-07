/*******************************************************************************
* File Name: pega_ble.c
*
*******************************************************************************/
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <malloc.h>
//==============================================================================
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
//==============================================================================
#include "pega_ble.h"
//==============================================================================
pthread_t m_thread_id = NULL;
//==============================================================================

int pega_ble_is_loading_fw(void)
{
	return (m_thread_id == NULL);
}

void pega_ble_load_fw_Start(void)
{                
     printf("\n%s", __FUNCTION__);
	 #if (BLE_QUICK_FW_LOADING == 0)
	 system("killall brcm_patchram_plus");
	 #endif		
}

int pega_ble_is_loading_fw_ready(int bIsDebug) 
{ 
    int  rtn = 0;
    FILE *ptr; 
    char buff[512] = {0}; 
    char cmd[128]; 
    sprintf(cmd,"hciconfig | grep -c %s", "UART"); 
	
    if((ptr=popen(cmd, "r")) != NULL) 
    { 
        while (fgets(buff, 512, ptr) != NULL) 
        {   
            pclose(ptr); 
			rtn = atoi(buff);
			
			if (bIsDebug > 0)
			{
				printf("\n[%s](%d)\n", __func__, rtn);
			}
			
			sleep(1);
			
            return rtn;
        } 
    } 
	
    pclose(ptr); 
    return 0;
}

int pega_ble_is_ble_running(int bIsDebug) 
{ 
    int  rtn = 0;
    FILE *ptr; 
    char buff[512] = {0}; 
    char cmd[128]; 
    sprintf(cmd,"hciconfig | grep -c %s", "RUNNING"); 
	
    if((ptr=popen(cmd, "r")) != NULL) 
    { 
        while (fgets(buff, 512, ptr) != NULL) 
        {   
            pclose(ptr); 
			rtn = atoi(buff);
			
			if (bIsDebug > 0)
			{
				printf("\n[%s](%d)\n", __func__, rtn);
			}
			
			sleep(1);
			
            return rtn;
        } 
    } 
	
    pclose(ptr); 
    return 0;
}
//==============================================================================
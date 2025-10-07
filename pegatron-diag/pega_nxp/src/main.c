#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
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
#include "pega_uart.h"
//==============================================================================
static char	 m_u8DeviceName[]= "/dev/ttyS2"; //"/dev/ttyS3";
//==============================================================================
//pega_nxp
int main(int argc, char *argv[])
{
	unsigned int u32BaudRate = 115200, timeout = 5;
	int bRTSCTS_En = 0;
	
    if (argc > 1)
    {
    	printf("\n[%s]%s,%s \n", __func__, argv[0],argv[1]);    			 	    	 	
    } 
    
	
	if (Uart_DeviceOpen(m_u8DeviceName) > 0)
	{		
        Uart_InitialParameter(u32BaudRate, 0);
				
		Uart_WaitAndSaveDataToBuffer(timeout*1000);
		sleep(timeout);	
		//PegaAT_SendCommandAndGetResponse(pCommand, pstrResponse, intTimeout_ms);	
		Uart_DeviceClose();		
	}
	
	
    return 0;
}

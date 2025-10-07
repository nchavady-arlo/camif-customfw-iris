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
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <linux/if_link.h>
//==============================================================================
#include "pega_uart.h"
//==============================================================================
static char	 m_u8DeviceName[]= "/dev/ttyS2"; //"/dev/ttyS3";
//==============================================================================
int uart_test(int argc, char *argv[])
{
	unsigned int u32BaudRate = 0;
	int bRTSCTS_En = 0;
	
    argc--;
	argv++;    
	
	if (Uart_DeviceOpen(m_u8DeviceName) > 0)
	{		
        if (argv[2] != NULL)
		{
			u32BaudRate = atoi(argv[2]);
		}
		
		if (argv[3] != NULL)
		{
			bRTSCTS_En = atoi(argv[3]);
		}
		
		Uart_InitialParameter(u32BaudRate, bRTSCTS_En>0);
				
        if (argv[1] != NULL)
		{
			UART_SendBuffer(argv[1], strlen(argv[1]));
		}
		else
		{
			UART_SendBuffer("test",4);		
		}
		
		
		Uart_WaitAndSaveDataToBuffer(3000);
		
		//sleep(3);	
		//PegaAT_SendCommandAndGetResponse(pCommand, pstrResponse, intTimeout_ms);	
		Uart_DeviceClose();		
	}
	
	
    return 0;
}
//pega_test sd env
int env_test(int argc, char *argv[])
{
	char *var, *value;

    argc--;
	argv++;  
	
	if (argv[1] != NULL)
	{
		var = argv[1];
		value = getenv(var);  //第一?????的??值
		
		if (value) //判???值是否存在
		{
			printf("Variable %s has value %s\n", var, value);
		}
		else
		{
			printf("Variable %s has no value\n", var);
		}
	}    
}

//pega_test sdio
int sdio_test(void) 
{  
    FILE *fp;
    char s1[120]={0};  
    
    printf("\n%s\n", __func__);  
    	
    //sprintf(s1,"/sys/devices/platform/soc/1f008400.ifs_sdmmc0/mmc_host/mmc0/mmc0:0001",sGpioNum);  
    //sprintf(s1,"/sys/devices/platform/soc/1f008400.ifs_sdmmc0/mmc_host/mmc0/mmc0:0001/mmc0:0001:1");  
	sprintf(s1,"/sys/devices/platform/soc/1f008400.ifs_sdmmc0/mmc_host/mmc0/mmc0:0001/mmc0:0001:1/net");  
	
    if ((fp = fopen(s1, "r")) == NULL)   
    {  
        printf("Cannot open %s\n",s1);  
        return -1;  
    }              
         
    fclose(fp);  
    
	printf("OK!(%s)\n", s1); 
	
    return 0;  
} 
//pega_test sd0 "mlan0" / "mlan1"
int interface_check_sd0(const char *ifname, int bDebug)
{
	int bRtn = -1;
	
    DIR *pDir;
    struct dirent *de;
    char s1[160]={0};
	
	printf("\n%s\n", __func__);  
	
    if (ifname == NULL) 
	{
        return bRtn;
    }
	
	sprintf(s1,"/sys/devices/platform/soc/1f008400.ifs_sdmmc0/mmc_host/mmc0/mmc0:0001/mmc0:0001:1/net");
	
    pDir = opendir(s1);
	
    if (pDir == NULL) 
	{
		printf("Cannot opendir %s\n",s1); 
        return bRtn;
    }
 
    while (NULL != (de = readdir(pDir))) 
	{
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) 
		{
            continue;
        }
				
		if (!strcmp(de->d_name, ifname))
		{
			if (bDebug > 0)
			{
				printf("Found: Interface %s\n", de->d_name);
			}
			bRtn = 0;
			goto fun_end;
		}
    }

	if (bDebug > 0)
	 {
		printf("Not found: Interface %s\n", ifname);
	 }
			
fun_end:
    
	closedir(pDir);
	
	//printf("closedir(%s)\n", s1); 
	
    return bRtn;
}

//pega_test sd1 "mlan0" / "mlan1"
int interface_check_sd1(const char *ifname, int bDebug)
{
	int bRtn = -1;
	
    DIR *pDir;
    struct dirent *de;
    char s1[160]={0};
	
	printf("\n%s\n", __func__);  
	
    if (ifname == NULL) 
	{
        return bRtn;
    }
		
	sprintf(s1,"/sys/devices/platform/soc/1f282600.ifs_sdmmc1/mmc_host/mmc1/mmc1:0001/mmc1:0001:1/net");
	
    pDir = opendir(s1);
	
    if (pDir == NULL) 
	{
		printf("Cannot opendir %s\n",s1); 
        return bRtn;
    }
 
    while (NULL != (de = readdir(pDir))) 
	{
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) 
		{
            continue;
        }
				
		if (!strcmp(de->d_name, ifname))
		{
			if (bDebug > 0)
			{
				printf("Found: Interface %s\n", de->d_name);
			}
			bRtn = 0;
			goto fun_end;
		}
    }

    if (bDebug > 0)
	 {
		printf("Not found: Interface %s\n", ifname);
	 }
			
fun_end:
    
	closedir(pDir);
	
	//printf("closedir(%s)\n", s1); 
	
    return bRtn;
}

int main(int argc, char *argv[])
{
	unsigned int u32BaudRate = 0;
	int bRTSCTS_En = 0;
	
    if (argc > 1)
    {
    	printf("\n[%s]%s,%s \n", __func__, argv[0],argv[1]);    			 	    	 	
    } 
    
	if (!strcmp(argv[1],"uart"))
	{
		uart_test(argc, argv);
	}
	
	if (!strcmp(argv[1],"env"))
	{
		env_test(argc, argv);
	}
	
	if (!strcmp(argv[1],"sd0"))
	{
		//sdio_test();		
		if (argv[2] != NULL)
		{
			interface_check_sd0(argv[2], 1);
		}
		else
		{
			interface_check_sd0("mlan0", 1);
		}
	}
	
	if (!strcmp(argv[1],"sd1"))
	{
		//sdio_test();		
		if (argv[2] != NULL)
		{
			interface_check_sd1(argv[2], 1);
		}
		else
		{
			interface_check_sd1("mlan0", 1);
		}
	}
	
    return 0;
}

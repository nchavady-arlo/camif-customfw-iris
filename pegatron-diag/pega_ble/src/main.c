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
#include <sys/stat.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
//==============================================================================
#include "pega_ble.h"
//==============================================================================
static const char* m_pWifi_Address=NULL;
//==============================================================================
static void pega_ble_advertisement_enable(void)
{
	   printf("\nhciconfig hci0 up(%d)",     system("hciconfig hci0 up"));
	   printf("\nhciconfig hci0 noscan(%d)", system("hciconfig hci0 noscan")); 
	   printf("\nhciconfig hci0 leadv 3(%d)",system("hciconfig hci0 leadv 3")); 
	   printf("\n");
}
//NURA[0.0.0.0]
static void pega_ble_advertisement_data_default(void)
{
	 //system("hcitool -i hci0 cmd 0x08 0x0008 1f 02 01 06 16 09 4e 55 52 41 5b 31 39 32 2e 31 39 32 2e 31 39 32 2e 31 39 32 5d 04 ff f1 0e fe 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");	   
	   system("hcitool -i hci0 cmd 0x08 0x0008 17 02 01 06 0e 09 4e 55 52 41 5b 30 2e 30 2e 30 2e 30 5d 04 ff f1 0e fe 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
}

static int pega_ble_advertisement_data_update(char* pStrIP)
{
	   int i=0,j=11,len;
       char bleadv[12] = {0}; //space
	   char cBuffer[120] = {0};
	   	   	   
       len = strlen(pStrIP);	   
       	   
	   printf("\n[%s]pStrIP=%s(%d)\n", __func__, pStrIP, len);	
	   
	   if ((len < 7) || (len > 15))
	   {
		   printf("\n[Err]IP foramt error.(%s)(%d)\n", pStrIP, len);
		   pega_ble_advertisement_data_default();
		   return -1;
	   }
	        	   
	   pega_ble_advertisement_enable();
	  
       if (pega_ble_is_ble_running(1) == 0)
	   {
		   pega_ble_advertisement_enable();
		   sleep(1);
	   }
	   
	   memset(bleadv, 0x30, sizeof(bleadv));
	   
	   for (i=len-1; i > -1; i--)
	   {
		   if (pStrIP[i] == '.')
		   {
			   if ((j==9)||(j==10))
			   {
				   j=8;  
			   }
			   else if ((j==6)||(j==7))
			   {
				   j=5;  
			   }
			   else if ((j==3)||(j==4))
			   {
				   j=2;  
			   }
		   }
		   else //if (pStrIP[i] != '.')
		   {		   
			   bleadv[j--] = pStrIP[i];
		   }
	   }
	          
	   #if 0
	   printf("\n");
	   for (i=0;i<sizeof(bleadv);i++)
	   {
           printf("%02x ",bleadv[i]);  
		   //bleadv[i*2] = pStrIP[i];		   
	   }
	   printf("\n");	   
	   #endif
	   
	   printf("\nbleadv=%s\n",bleadv);	   
	   //snprintf(cBuffer, sizeof(cBuffer), "hcitool -i hci0 cmd 0x08 0x0008 17 02 01 06 0e 09 4e 55 52 41 5b %s", bleadv);
	   
	   snprintf(cBuffer, sizeof(cBuffer), "hcitool -i hci0 cmd 0x08 0x0008 1f 02 01 06 16 09 4e 55 52 41 5b %02x %02x %02x 2e %02x %02x %02x 2e %02x %02x %02x 2e %02x %02x %02x 5d 04 ff f1 0e fe 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", bleadv[0],bleadv[1],bleadv[2],bleadv[3],bleadv[4],bleadv[5],bleadv[6],bleadv[7],bleadv[8],bleadv[9],bleadv[10],bleadv[11]);
	   
	   printf("\ncBuffer=%s",cBuffer);
	   	   
	   printf("\nsystem(cBuffer)(%d)",  system(cBuffer));
	   
	   return 0;
}

static int pega_ble_advertisement_data_send(char* pString)
{
	   int i=0,len;
       char bleadv[16] = {0}; 
	   char cBuffer[120] = {0};
	   	   	   
       len = strlen(pString);	   
       	   
	   printf("\n[%s]pString=%s(%d)\n", __func__, pString, len);	
	  	  	        	   
	   pega_ble_advertisement_enable();
	  
       if (pega_ble_is_ble_running(1) == 0)
	   {
		   pega_ble_advertisement_enable();
		   sleep(1);
	   }
	   
	   memset(bleadv, 0x20, sizeof(bleadv)); //space
	   
	   for (i=0; i < len; i++)
	   {
		  bleadv[i] = pString[i];
	   }
	   
       if (len > 15)
	   {
           bleadv[15] = 0x5d;
	   }
	   else
	   {
		   bleadv[len] = 0x5d;
	   }
	   
	   #if 0
	   printf("\n");
	   for (i=0;i<sizeof(bleadv);i++)
	   {
           printf("%02x ",bleadv[i]);  
		   //bleadv[i*2] = pStrIP[i];		   
	   }
	   printf("\n");	   
	   #endif
	   
	   //printf("\nbleadv=%s\n",bleadv);	   
	  
	   snprintf(cBuffer, sizeof(cBuffer), "hcitool -i hci0 cmd 0x08 0x0008 1f 02 01 06 16 09 4e 55 52 41 5b %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x 04 ff f1 0e fe 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", bleadv[0],bleadv[1],bleadv[2],bleadv[3],bleadv[4],bleadv[5],bleadv[6],bleadv[7],bleadv[8],bleadv[9],bleadv[10],bleadv[11],bleadv[12],bleadv[13],bleadv[14],bleadv[15]);
	   
	   //printf("\ncBuffer=%s",cBuffer);
	   	   
	   printf("\nsystem(cBuffer)(%d)",  system(cBuffer));
	   
	   return 0;
}
//==============================================================================
static int pega_ble_check_network_is_ready(void)
{
	 int rtn = 0;
	 struct ifaddrs *ifaddr, *ifa;
	 char host[NI_MAXHOST];
	 int s;
	 struct sockaddr_in *sa;
     char *addr;
		 	 
	 if (getifaddrs(&ifaddr) == -1) 
	 {
		return -1;
	 }
	 
	 for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (NULL == ifa->ifa_addr || AF_INET != ifa->ifa_addr->sa_family)
		{
			continue;
		}

		if (0 != (s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST)))
		{
			continue;
		}
        		
		if ((0 == strcmp(ifa->ifa_name, "usb0")) || (0 == strcmp(ifa->ifa_name, "mlan0"))) //usb0
		{			
			rtn = 1;
			break;
		}
	}
    
	if (rtn > 0)
	{
		sa = (struct sockaddr_in *) ifa->ifa_addr;
        addr = inet_ntoa(sa->sin_addr);
        m_pWifi_Address = addr;				
		printf("Interface: %s\tAddress: %s(%d)\n", ifa->ifa_name, addr,strlen(addr));
		printf("Interface: m_pWifi_Address: %s(%d)\n", m_pWifi_Address,strlen(m_pWifi_Address));
	}
	
	freeifaddrs(ifaddr);
	
	return rtn;
}

static int pega_ble_advertisement_broadcast(void)
{		
	while(1)
	{
		sleep(2);

        if (pega_ble_check_network_is_ready() > 0)
		{
            return pega_ble_advertisement_data_update(m_pWifi_Address);
		}			
	}
	
	return 0;
}
//==============================================================================
static int pega_ble_load_fw(void)
{		
    int rtn = -1;
	int timeoutCnt = 0, fw_reloadCnt = 0;    	
	
	printf("\n------------------");
	printf("\n[%s]", __func__);
	printf("\n------------------");
		
	while(1)
	{
		sleep(2);

        if (pega_ble_check_network_is_ready() > 0)
		{
			break;            
		}			
	}

re_load:
	
	fw_reloadCnt++;
	
	pega_ble_load_fw_Start();
		
	sleep(5);
	
	while(1)
	{
		if (pega_ble_is_loading_fw() == 0)
		{
			continue;
		}
		
		if (pega_ble_is_loading_fw_ready(0) > 0)
		{
			sleep(1);
			
			return pega_ble_advertisement_data_update(m_pWifi_Address);
		}
		else
		{
			if (timeoutCnt++ > 15) //over 30 seconds.
			{
				printf("\n[%s]timeoutCnt=%d", __func__, timeoutCnt);
				goto re_load;
			}
		}
		
        if (fw_reloadCnt >= 3)
		{
            printf("\n[Err][%s]BT fw loading is failed!!(%d)", __func__, fw_reloadCnt);
			return 0;
		}
		
		sleep(2);
	}
	
	return 0;
}

//==============================================================================
int main(int argc, char* argv[])
{	    
    printf("\n[%s](%d) \n", __func__, argc);
	
	if (argc > 1)
    {
    	printf("\n[%s]%s,%s \n", __func__, argv[0],argv[1]);		
    } 
    
    if (argv[1] != NULL)
	{
        if (!strcmp(argv[1],"ip"))
		{			
			return pega_ble_advertisement_data_update(argv[2]);
		}
		else if (!strcmp(argv[1],"fw"))
		{
			return pega_ble_load_fw();
		}
		else if (!strcmp(argv[1],"check"))
		{
			return pega_ble_is_loading_fw_ready(1);
		}
		else if (!strcmp(argv[1],"run"))
		{
			return pega_ble_is_ble_running(1);
		}
		else if (!strcmp(argv[1],"get"))
		{
			return pega_ble_check_network_is_ready();
		}		
		else if (!strcmp(argv[1],"str"))
		{
			return pega_ble_advertisement_data_send(argv[2]);
		}
	}	
	else
	{
		return pega_ble_advertisement_broadcast();
	}
	
    return 0;
}
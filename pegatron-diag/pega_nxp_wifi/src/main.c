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
#include "pega_nxp_wifi.h"
//==============================================================================

//==============================================================================
//pega_nxp_wifi
int main(int argc, char *argv[])
{
	
    if (argc > 1)
    {
    	printf("\n[%s]%s,%s \n", __func__, argv[0], argv[1]);    			 	    	 	
    } 
    
	Pega_nxp_wifi_socket_open();
	
	if (argv[1] == NULL)
	{
		Pega_nxp_wifi_country_code_get(1);
	}
	else
	{	
		Pega_nxp_wifi_country_code_set(1, argv[1]);
	}
	
	Pega_nxp_wifi_socket_close();
	
    return 0;
}

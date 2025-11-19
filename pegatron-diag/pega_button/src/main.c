#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
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
#include <time.h>
#include <stdbool.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/watchdog.h>
//==============================================================================
#include "pega_defines.h"
#include "pega_gpiod.h"
//==============================================================================

int main(int argc, char* argv[])
{
    printf("\n[%s] argc=%d \n", __func__, argc);  
	
	if (argc > 1)
    {
    	printf("\n[%s]%s,%s \n", __func__, argv[0],argv[1]);    			 	    	 	
    } 
   	
	pga_syn_button();
	  
    return 0;
}

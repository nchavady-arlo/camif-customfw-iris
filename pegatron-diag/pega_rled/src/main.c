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
#include "pega_rLed.h"
//==============================================================================
int main(int argc, char *argv[])
{	
    if (argc > 1)
    {
    	printf("\n[%s]%s,%s \n", __func__, argv[0],argv[1]);    			 	    	 	
    } 
    
	if (!strcmp(argv[1],"brightness"))
	{
		if (argv[2] != NULL)
		{
			Pega_rLed_brightness_set(atoi(argv[2]));
		}
	}
	
    return 0;
}

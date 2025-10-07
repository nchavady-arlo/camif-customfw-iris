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
#include "pega_motor_awd8833.h"
//==============================================================================
//pega_motor
int main(int argc, char *argv[])
{
	uint8_t bIsRisingMotor = 0;
	
    if (argc > 1)
    {
    	printf("\n[%s]%s,%s \n", __func__, argv[0],argv[1]);    			 	    	 	
    } 
    
	if (argv[1] != NULL)
	{
		bIsRisingMotor = atoi(argv[1]) > 0 ? 1 : 0;
	}
	
	awd8833c_test(bIsRisingMotor);
	
    return 0;
}

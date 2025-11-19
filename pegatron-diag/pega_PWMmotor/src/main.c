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
	int cmd;
	char speed[256],round[30];
 	Pega_PWM_init();
	while(1)
	{
    	scanf("%d", &cmd);		
		switch(cmd)
		{
			case 1:
		    	scanf("%s %s", speed, round);		
				Pega_PWM_Forward_Fullstep(speed,round);
				break;
				;;

			case 2:
		    	scanf("%s %s", speed, round);		
				Pega_PWM_Reverse_Fullstep(speed,round);
				break;
				;;

			case 3:
		    	scanf("%s %s", speed, round);		
				Pega_PWM_Forward_Halfstep(speed,round);
				break;
				;;

			case 4:
		    	scanf("%s %s", speed, round);		
				Pega_PWM_Reverse_Halfstep(speed,round);
				break;
				;;

			default:
				return 0;
				break;
		}
	}
    return 0;
}

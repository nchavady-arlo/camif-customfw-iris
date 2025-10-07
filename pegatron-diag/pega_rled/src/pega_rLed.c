/*******************************************************************************
* File Name: pega_rLed.c
*
*******************************************************************************/
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
//==============================================================================
#include "pega_rLed.h"
//==============================================================================
#define RLED_DBG(x)          x
//==============================================================================
/*** constants ***/
#define SYSFS_RLED_DIR "/sys/class/leds/ktd2061"
#define MAX_BUF 64
//==============================================================================
//pega_rled brightness xx
int Pega_rLed_brightness_set(unsigned int brightness)
{	
    int fd = -1;
    int len_p = 0;
	char path[MAX_BUF];
	char buf_p[MAX_BUF];
  	/* set rLed brightness */
  	snprintf(path, sizeof(path), "%s/brightness", SYSFS_RLED_DIR);
	
	fd = open(path, O_WRONLY);
		
	if (fd < 0) 
	  {
		printf ("\nFailed set rLed_brightness\n");
		return -1;
	 }
  	
	len_p = snprintf(buf_p, sizeof(buf_p), "%d", brightness);
	
	write(fd, buf_p, len_p);
	  	
	close(fd);
	
	return 0;			
}

/*******************************************************************************
* File Name: pega_pwm.c
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
#include "pega_pwm.h"
//==============================================================================
#define PWM_DBG(x)          x
//==============================================================================
/*** constants ***/
#define SYSFS_PWM_DIR "/sys/class/pwm"
#define MAX_BUF 64
//==============================================================================
/*** PWM functions ***/
/* PWM export */
int Pega_pwm_export(unsigned int pwm)
{
	int fd = -1;
      
	switch(pwm)	
	{
		case PWM_CH6:
			 fd = open(SYSFS_PWM_DIR "/pwmchip6/export", O_WRONLY);
			 break;
		
		case PWM_CH7:
			 fd = open(SYSFS_PWM_DIR "/pwmchip7/export", O_WRONLY);
			 break;		
		
		default:
		     break;		
	}
     	
	if (fd < 0) 
	  {
		printf ("\nFailed export PWM(%s)\n", pwm);
		return -1;
	  }
	
	write(fd, "0", 2);
	close(fd);
    
    return 1;
}

/* PWM unexport */
int Pega_pwm_unexport(unsigned int pwm)
{
	int fd = -1;
      
	switch(pwm)	
	{
		case PWM_CH6:
			 fd = open(SYSFS_PWM_DIR "/pwmchip6/unexport", O_WRONLY);
			 break;
		
		case PWM_CH7:
			 fd = open(SYSFS_PWM_DIR "/pwmchip7/unexport", O_WRONLY);
			 break;		
		
		default:
		     break;		
	}
     	
	if (fd < 0) 
	  {
		printf ("\nFailed unexport PWM(%s)\n", pwm);
		return -1;
	  }
	
	write(fd, "0", 2);
	close(fd);
       
    return 1;
}

/* PWM configuration */
int Pega_pwm_config(unsigned int pwm, unsigned int period, unsigned int duty_percent)
{
	int fd,len_p,len_d;
	char buf_p[MAX_BUF];
	char buf_d[MAX_BUF];
	char path[MAX_BUF];
  	unsigned int duty_cycle = 0;
		
	duty_cycle = period / 100 * duty_percent;
	
	len_p = snprintf(buf_p, sizeof(buf_p), "%d", period);
	len_d = snprintf(buf_d, sizeof(buf_d), "%d", duty_cycle);
  	    
  	/* set pwm period */
  	snprintf(path, sizeof(path), "%s/pwmchip%d/pwm0/period", SYSFS_PWM_DIR, pwm);
	fd = open(path, O_WRONLY);
		
	if (fd < 0) 
	  {
		printf ("\nFailed set PWM_B period\n");
		return -1;
	 }
  	
	write(fd, buf_p, len_p);
	/* set pwm duty cycle */
	snprintf(path, sizeof(path), "%s/pwmchip%d/pwm0/duty_cycle", SYSFS_PWM_DIR, pwm);
	fd = open(path, O_WRONLY);
	if (fd < 0) 
	  {
	    printf ("\nFailed set PWM_B duty cycle\n");
		return -1;
	  }
  	
	write(fd, buf_d, len_d);
  	
	close(fd);
	return 0;			
}

/* PWM enable/disable */
int Pega_pwm_control(unsigned int pwm, int bEnable)
{
	int fd;
  	char path[MAX_BUF];	
  	
  	snprintf(path, sizeof(path), "%s/pwmchip%d/pwm0/enable", SYSFS_PWM_DIR, pwm);
  		
  	fd = open(path, O_WRONLY);
  	
	if (fd < 0) 
	{
		printf ("\nFailed enable PWM_B\n");
		return -1;
	}
		
	if (bEnable == 1)
	{
	    write(fd, "1", 2);
	}
	else    
	{
        write(fd, "0", 2);
	}
        
	close(fd);
		
	return 0;
}
//pega_gpio pwm init
void Pega_pwm_init(unsigned int bIsOn)
{
	 if (bIsOn > 1)
	 {
		 bIsOn = 1;
	 }
	 
	 if (Pega_pwm_export(PWM_CH6) > -1)//for IR led
	 {
	  	 if (Pega_pwm_config(PWM_CH6, PWM_LED_FREQ, PWM_LED_DUTY) > -1)	
		 {				 
	         Pega_pwm_control(PWM_CH6, bIsOn);
		 }
	 }
	 else
	 {
		 ERR_LOG("pwm init failed! (IR Led)");  
	 }
	 
	 #if 1
	 if (Pega_pwm_export(PWM_CH7) > -1)//for Spotlight
	 {
	  	 if (Pega_pwm_config(PWM_CH7, PWM_SPOT_LED_FREQ, PWM_SPOT_LED_DUTY) > -1)	
		 {				 
	         Pega_pwm_control(PWM_CH7, bIsOn);
		 }
	 }
	 else
	 {
		 ERR_LOG("pwm init failed! (Spotlight)");  
	 }
	 #endif
}
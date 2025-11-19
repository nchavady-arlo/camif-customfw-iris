/*******************************************************************************
* File Name: pega_adc.c
*
*******************************************************************************/
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
//==============================================================================
#define SAR_PATH "/dev/sar"
//==============================================================================
#define SARADC_IOC_MAGIC                     'a'
#define IOCTL_SAR_INIT                       _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE     _IO(SARADC_IOC_MAGIC, 1)
//==============================================================================
#include "pega_adc.h"
//==============================================================================
#define ADC_DBG(x)         // x
//==============================================================================
static int mSar_fd = 0;
//==============================================================================

//==============================================================================
//10 bits ADC
//pega_gpio adc init
int Pega_SarADC_init(void)
{		
	ADC_CONFIG_READ_ADC tmp;	
			     			
	mSar_fd = open(SAR_PATH, O_RDWR);
	
	//printf("\n%s(%d)", __FUNCTION__, mSar_fd);
				
	if (mSar_fd < 0)
	  {
		  _LOG_ERROR("can not open sar's node");		  
		  return -1;
	  }
	
	if (ioctl(mSar_fd, IOCTL_SAR_INIT, &tmp) < 0)
	  {
		  _LOG_ERROR("\n sar init err");
		  return -1;
	  }
	
	return 0;
}

int Pega_SarADC_uninit(void)
{
	if (mSar_fd > 0)
	  {
		  close(mSar_fd);
	  }  
	
	return 0x0;
}
//==============================================================================
int Pega_SarADC_Value_Get(const int eChannel, int *val)
{
	ADC_CONFIG_READ_ADC sar_tmp;
	
	*val = 0;
	memset(&sar_tmp, 0x0, sizeof(sar_tmp));
	
	if (eChannel >= SAR_ADC_CH_MAX)
	  {
	  	_LOG_ERROR("invalid adc channel(%d)\n",eChannel);
		return FAILED;	
	  }
	    
	if	(mSar_fd)
	{
		sar_tmp.channel_value = eChannel;
				
		if	(ioctl(mSar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &sar_tmp) < 0)
		{
			 _LOG_ERROR("can not get sar:ch(%d) value", sar_tmp.channel_value);
		 	 return FAILED;							
		}
				
		*val = sar_tmp.adc_value;
				
		//printf("ch(%d), value(%d)\n", sar_tmp.channel_value, sar_tmp.adc_value);				
	}
			
	return SUCCEED;
}
//pega_gpio adc get xx
void Pega_SarADC_Value_Print(const int eChannel)
{
	 int value = 0;
	 
	 Pega_SarADC_Value_Get(eChannel, &value);
	 
	 printf("ch(%d), value(%d)\n",eChannel, value);	
}
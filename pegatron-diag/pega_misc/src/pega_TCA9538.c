/*******************************************************************************
* File Name: pega_TCA9538.c
*
*******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
//==============================================================================
#include "pega_TCA9538.h"
#include "pega_i2c_control.h"
//==============================================================================
static int TCA9538_I2C_WriteToSlave(unsigned char regAddr, unsigned char regData)
{  
//  printc("[MISC.W]:Slave %x, reg:%x,data:%x\r\n",attr->ubSlaveAddr, reg,data);
    #if (DEVICE_IO_EXT_ENABLE == 1)
    return pega_i2c_Write(NULL, TCA9538_ADDRESS, (unsigned int)regAddr, (uint16_t)regData, I2C_FMT_A8D8);
    #else
	return 0;
    #endif
}

static int TCA9538_I2C_ReadFromSlave(unsigned char regAddr, unsigned char *regData)
{
	 int  ret = 0;
	 	  
	 uint16_t value=0;
	  
	 #if (DEVICE_IO_EXT_ENABLE == 1) 
	 ret = pega_i2c_Read(NULL, TCA9538_ADDRESS, (unsigned int)regAddr, &value, I2C_FMT_A8D8);
	  
	 *regData = (unsigned char)value;
	 #endif
  
	 return ret;
}
//==============================================================================
void TCA9538_Device_Init(void)
{
	 /*
	 P0: IW610G_WL_WAKEUP_OUT
	 P1: IW610G_NB_WAKEUP_OUT
	 P2: IW610G_WL_WAKEUP_IN
	 P3: IW610G_NB_WAKEUP_IN
	 
	 P4: IW610F_NB_WAKEUP_IN
	 P5: IW610F_WL_WAKEUP_IN
	 P6: IW610F_NB_WAKEUP_OUT
	 P7: IW610F_WL_WAKEUP_OUT	 
	 */
	 TCA9538_I2C_WriteToSlave(TCA9538_REG_CONFIGURATION, 0x3C); 
}
//==============================================================================
void TCA9538_Data_Print(void)
{
#if 0	
	 	 
	 printf("\r\n-----------------------");
	 
	 printf("\r\n-----------------------");
	 
#endif	 
}

void TCA9538_Register_Info_Print(void)
{   
	  #if 1
	    
	  unsigned char u8Reg, regData;
		
	   printf("\r\n-----------------------");	
	   
	   for (u8Reg=0;u8Reg<=TCA9538_REG_INPUT_PORT;u8Reg++)
	       {
	         switch(u8Reg)
	           {
	           	 case TCA9538_REG_INPUT_PORT:
	             case TCA9538_REG_OUTPUT_PORT:
	             case TCA9538_REG_POLARITY_INV:
	             case TCA9538_REG_CONFIGURATION:	                              
	             	  TCA9538_I2C_ReadFromSlave(u8Reg, &regData);
	             	  printf("\r\nReg(0x%02x) => 0x%04x",u8Reg, regData);
	             	  break;
	             
	             default:
	             	  break;	  	
	           }	
	       }	     
	    printf("\r\n-----------------------");
	 
	 #endif       
}
/*******************************************************************************
* File Name: Pega_ALS_Opt300x.c
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
          
  return pega_i2c_Write(TCA9538_ADDRESS, (unsigned int)regAddr, (unsigned short)regData, I2C_FMT_A8D8);
}

static int TCA9538_I2C_ReadFromSlave(unsigned char regAddr, unsigned char *regData)
{
	 int  ret;
	 	  
	 unsigned short value=0;
	  
	 ret = pega_i2c_Read(TCA9538_ADDRESS, (unsigned int)regAddr, &value, I2C_FMT_A8D8);
	  
	 *regData = (unsigned char)value;
  
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
static void pega_help_print(void)
{	   	   
	printf("\n");

    printf("\n Usage :");
	printf("\n opt300x help info [options]");
	printf("\n options:");
	printf("\n init");	  
    printf("\n read x  (x=regAddr)");
    printf("\n write x0 x1 (x0=regAddr, x1=data)");
	printf("\n dump    (dump register.)");	  
	printf("\n example: opt300x dump"); 
    printf("\n"); 
    
    #if 0
    printf("\n---------------------");
    printf("\nsizeof(short) = %d", sizeof(short)); 
    printf("\nsizeof(int)   = %d", sizeof(int)); 
    printf("\nsizeof(long)  = %d", sizeof(long)); 
    printf("\n---------------------");
    #endif
}

int main(int argc, char* argv[])
{		  
	  pega_i2cOpen();
	  
	  if (argc <= 1)
	      goto Cmd_Err;
	      
	  if (argc > 1)
    	{
    		printf("\n[%s,%d]%s,%s \n", __func__, argc, argv[0],argv[1]);    			 	    	 	
    	} 
    
    if (!strcmp(argv[1],"init"))
  		{  			 	  		 	 
  			 	 TCA9538_Device_Init();
  		}   	
  	else if (!strcmp(argv[1],"dump"))
  		{  			 	  		 	 
  			 	 TCA9538_Register_Info_Print();
  		} 	  	
  	else if (!strcmp(argv[1],"read"))
  	 {
  			 	 unsigned short regAddr=0;
				 unsigned char  ucVal=0;
  			 	   			 	   			 	 
  			 	 if (argc < 3)
				 {
					goto Cmd_Err;
				 }
	   	       			 	 
  			 	 regAddr = strtol(argv[2],NULL,16);
  			 	 
  				 TCA9538_I2C_ReadFromSlave((unsigned char)regAddr, &ucVal);	  
  				 
  				 printf("[%s]:regAddr(0x%#x) = %#x \n", argv[1], regAddr, ucVal);	
  	 }  
	 else if (!strcmp(argv[1],"write"))
  	 {
  			 	 unsigned short regAddr=0;
				 unsigned char  regData=0;
  			 	 
  			 	 if (argc < 4)
				 {
					 goto Cmd_Err;
				 }
	   	           			 	 
  			 	 regAddr = strtol(argv[2],NULL,16);
  			 	 regData = strtol(argv[3],NULL,16);
  			 	 
  				 TCA9538_I2C_WriteToSlave((unsigned char)regAddr, regData);	  
  				 
  				 printf("[%s]:regAddr(0x%#x) = %#x \n", argv[1], regAddr, regData);	
  	 }  					   			    
	 else
	 {
	       pega_help_print();
	 }  
	    
    pega_i2cClose();
    
    //printf("\n[%s]exit....\n", __func__);
    
    return 0;

Cmd_Err: 
  			
    pega_help_print();      
    pega_i2cClose();
    return 0;   
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
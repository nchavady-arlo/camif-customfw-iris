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
#include "Pega_ALS_Opt300x.h"
#include "Pega_i2c_control.h"
//==============================================================================
#define OPT300X_DBG(x)          x
//==============================================================================
static stALSDefType	m_stOpt300x;
//==============================================================================
static MMP_USHORT OPT300X_I2C_Word_Data_Swap(MMP_USHORT usData)
{
       MMP_USHORT val;
       
       val  = (usData & 0x00FF) << 8;
       val += (usData & 0xFF00) >> 8;
       
       //printc("\r\n%s(%x,%x)", __FUNCTION__, usData,val);
       
       return val;	
}

static int OPT300X_I2C_WriteToSlave(unsigned char regAddr, MMP_USHORT regData)
{  
//  printc("[MISC.W]:Slave %x, reg:%x,data:%x\r\n",attr->ubSlaveAddr, reg,data);
          
  return pega_i2c_Write(OPT300x_ADDRESS, (MMP_ULONG)regAddr, (MMP_USHORT)regData, I2C_FMT_A8D16);
}

static int OPT300X_I2C_ReadFromSlave(unsigned char regAddr, MMP_USHORT *regData)
{
	 int  ret;
	 	  
	 unsigned short value=0;
	  
	 ret = pega_i2c_Read(OPT300x_ADDRESS, (MMP_ULONG)regAddr, &value, I2C_FMT_A8D16);
	  
	 *regData = OPT300X_I2C_Word_Data_Swap(value);
  
   return ret;
}

static void OPT300x_set_mode(MMP_USHORT wMode)
{
		m_stOpt300x.wConfReg &= ~OPT300x_CONFIGURATION_M_MASK;
	  m_stOpt300x.wConfReg |= wMode;	
}

static MMP_BOOL OPT300x_Read_Lux(void)
{
	  MMP_BOOL bFlag = MMP_FALSE; 
	  MMP_USHORT wVal,wExponent,wMantissa;
	  float LsbWeightTable[11] = {0.01,0.02,0.04,0.08,0.16,0.32,0.64,1.28,2.56,5.12,10.24};
	  	 
	  
	  if (OPT300X_I2C_ReadFromSlave(OPT300x_REG_SYS_RESULT , &wVal) == SUCCEED)
	    {
	    	bFlag = MMP_TRUE;
	    	 
	    	wExponent = OPT300x_REG_EXPONENT(wVal);
	      wMantissa = OPT300x_REG_MANTISSA(wVal);
	      
	      if (wExponent < 0xB)
	        {	        	
	          m_stOpt300x.fLux = LsbWeightTable[wExponent] * wMantissa; 
	        }  
	      else  
	        {
	          m_stOpt300x.fLux = (float)20.48 * (float)wMantissa; 
	        } 
	      
	      printf("\r\n wExponent = %d", wExponent);	
	      printf("\r\n wMantissa = %d", wMantissa);	
	      printf("\r\n m_stOpt300x.fLux = %.3lf", m_stOpt300x.fLux);		  
	    }
	  
	  return bFlag;   
}

static void OPT300x_ShutDown(void)
{	       
	   OPT300x_set_mode(OPT300x_CONFIGURATION_M_SHUTDOWN);     
	   
	   OPT300X_I2C_WriteToSlave(OPT300x_REG_SYS_CONF, m_stOpt300x.wConfReg);
}
//==============================================================================
void OPT300x_Device_Init(void)
{
	  MMP_USHORT wVal;
	  
	  m_stOpt300x.wDeviceID = 0x0000;
	  m_stOpt300x.wConfReg  = 0x0000;
	  /* Enable automatic full-scale setting mode */
	  m_stOpt300x.wConfReg |= OPT300x_CONFIGURATION_RN_AUTO;
	  m_stOpt300x.wConfReg |= OPT300x_CONFIGURATION_CT;
		/* Ensure device is in shutdown initially */
		OPT300x_set_mode(OPT300x_CONFIGURATION_M_SHUTDOWN);
		 
		 /* Configure for latched window-style comparison operation */	
		m_stOpt300x.wConfReg &= ~OPT300x_CONFIGURATION_ME;
		m_stOpt300x.wConfReg &= ~OPT300x_CONFIGURATION_FC_MASK;
		
		OPT300X_I2C_WriteToSlave(OPT300x_REG_SYS_CONF, m_stOpt300x.wConfReg);
		
		if (OPT300X_I2C_ReadFromSlave(OPT300x_REG_DEVICE_ID , &wVal) == SUCCEED)		
				m_stOpt300x.wDeviceID = wVal;
}

MMP_BOOL OPT300x_Is_Ready_Check(void)
{
	  MMP_BOOL bFlag = MMP_FALSE; 
	  MMP_USHORT wVal;
	  
	  if (OPT300X_I2C_ReadFromSlave(OPT300x_REG_SYS_CONF , &wVal) == SUCCEED)
	    {
	    	if ((wVal & OPT300x_CONFIGURATION_CRF) != 0) //Conversion complete. Ready for reading. 
	    	     bFlag = MMP_TRUE; 
	    }
	  
	  return bFlag;   
}

MMP_BOOL OPT300x_Enable_Read_Lux(void)
{	   
	  MMP_BOOL bFlag = MMP_FALSE; 
	  
	  MMP_USHORT wVal;
	  
	  if (OPT300X_I2C_ReadFromSlave(OPT300x_REG_SYS_CONF , &wVal) == SUCCEED)
	    {
	    	m_stOpt300x.wConfReg = wVal; 
	      
	      printf("\r\n m_stOpt300x.wConfReg = %04x", m_stOpt300x.wConfReg);	
	      
	      OPT300x_set_mode(OPT300x_CONFIGURATION_M_SINGLE);
	      
	      if (OPT300X_I2C_WriteToSlave(OPT300x_REG_SYS_CONF, m_stOpt300x.wConfReg) == SUCCEED)
	          bFlag = MMP_TRUE; 	  
	    }	  
	  
	  return bFlag;   
}

int OPT300x_Processing_Handler(void)
{	   	   
	   if ((m_stOpt300x.wConfReg & OPT300x_CONFIGURATION_M_SINGLE) == 0) 
	     {
	     	  OPT300x_Enable_Read_Lux();
	        return FAILED;
	     }   
	   
	   if (OPT300x_Is_Ready_Check() == MMP_FALSE)
	       return FAILED;
	   
	   if (OPT300x_Read_Lux() == MMP_FALSE)
	     {
	       OPT300x_ShutDown();
	       usleep(1000*100);
	       OPT300x_Enable_Read_Lux();
	       return FAILED;
	     }  
	       
	   OPT300x_ShutDown();
	   
	   return SUCCEED;
}

float OPT300x_Lux_Value_Get(void)
{	   	   
	   return m_stOpt300x.fLux;
}

MMP_USHORT OPT300x_Device_ID_Get(void)
{	   	   
	   return m_stOpt300x.wDeviceID;
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
  			 	 OPT300x_Device_Init();
  		}   	
  	else if (!strcmp(argv[1],"dump"))
  		{  			 	  		 	 
  			 	 OPT300x_Register_Info_Print();
  		} 	  	
  	else if (!strcmp(argv[1],"read"))
  	 {
  			 	 MMP_USHORT regAddr=0, wVal;
  			 	   			 	   			 	 
  			 	 if (argc < 3)
	   	         goto Cmd_Err;
	   	       			 	 
  			 	 regAddr = strtol(argv[2],NULL,16);
  			 	 
  				 OPT300X_I2C_ReadFromSlave((MMP_UBYTE)regAddr, &wVal);	  
  				 
  				 printf("[%s]:regAddr(0x%#x) = %#x \n", argv[1], regAddr, wVal);	
  	 }  
	  else if (!strcmp(argv[1],"write"))
  	 {
  			 	 MMP_USHORT regAddr=0, regData=0;
  			 	 
  			 	 if (argc < 4)
	   	         goto Cmd_Err;
	   	           			 	 
  			 	 regAddr = strtol(argv[2],NULL,16);
  			 	 regData = strtol(argv[3],NULL,16);
  			 	 
  				 OPT300X_I2C_WriteToSlave((MMP_UBYTE)regAddr, regData);	  
  				 
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
void OPT300x_Data_Print(void)
{
#if 1	
	 	 
	 OPT300X_DBG(printf("\r\n-----------------------"));
	 OPT300X_DBG(printf("\r\n m_stOpt300x.wDeviceID = %04x", m_stOpt300x.wDeviceID));	
	 OPT300X_DBG(printf("\r\n m_stOpt300x.wConfReg  = %04x", m_stOpt300x.wConfReg));	 
	 OPT300X_DBG(printf("\r\n m_stOpt300x.fLux      = %.3lf",  m_stOpt300x.fLux));	
	 OPT300X_DBG(printf("\r\n OPT300x_CONFIGURATION_M_MASK       = %04x", OPT300x_CONFIGURATION_M_MASK));
	 OPT300X_DBG(printf("\r\n OPT300x_CONFIGURATION_M_SHUTDOWN   = %04x", OPT300x_CONFIGURATION_M_SHUTDOWN));
	 OPT300X_DBG(printf("\r\n OPT300x_CONFIGURATION_M_SINGLE     = %04x", OPT300x_CONFIGURATION_M_SINGLE));
	 OPT300X_DBG(printf("\r\n OPT300x_CONFIGURATION_M_CONTINUOUS = %04x", OPT300x_CONFIGURATION_M_CONTINUOUS));
	 OPT300X_DBG(printf("\r\n-----------------------"));
	 
#endif	 
}

void OPT300x_Register_Info_Print(void)
{   
	  #if 1
	    
		 MMP_UBYTE u8Reg;
		 
		 MMP_USHORT regData;
		 		    
	   printf("\r\n-----------------------");	
	   
	   for (u8Reg=0;u8Reg<=OPT300x_REG_DEVICE_ID;u8Reg++)
	       {
	         switch(u8Reg)
	           {
	           	 case OPT300x_REG_SYS_RESULT:
	             case OPT300x_REG_SYS_CONF:
	             case OPT300x_REG_LOW_LIMIT:
	             case OPT300x_REG_HIGH_LIMIT:
	             case OPT300x_REG_MANUFACTURER_ID:
	             case OPT300x_REG_DEVICE_ID:	                         
	             	    OPT300X_I2C_ReadFromSlave(u8Reg, &regData);
	             	    printf("\r\nReg(0x%02x) => 0x%04x",u8Reg, regData);
	             	    break;
	             
	             default:
	             	  break;	  	
	           }	
	       }	     
	    printf("\r\n-----------------------");
	 
	 #endif       
}
/*******************************************************************************
* File Name: pega_als_opt300x.c
*
*******************************************************************************/
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
#include "pega_als_opt300x.h"
#include "pega_i2c_control.h"
//==============================================================================
#define ALS_I2C_BUS 	"3" //I2C3 mode 1 for ALS NFC ACC IO_EXPANDER (i2c-3)
//==============================================================================
#define OPT300X_DBG(x)          x
//==============================================================================
// I2C Linux device handle
static int m_Als_i2cFile = -1;
//==============================================================================
static stALSDefType	m_stOpt300x;
//==============================================================================
static uint16_t OPT300X_I2C_Word_Data_Swap(uint16_t usData)
{
       uint16_t val;
       
       val  = (usData & 0x00FF) << 8;
       val += (usData & 0xFF00) >> 8;
       
       //printc("\r\n%s(%x,%x)", __FUNCTION__, usData,val);
       
       return val;	
}

static int OPT300X_I2C_WriteToSlave(uint8_t regAddr, uint16_t regData)
{  
	   int rtn = 0;
	   
       #if (DEVICE_ALS_ENABLE == 1)	   
//  printc("[MISC.W]:Slave %x, reg:%x,data:%x\r\n",attr->ubSlaveAddr, reg,data);
       rtn = pega_i2c_Write(m_Als_i2cFile, OPT300x_ADDRESS, (uint32_t)regAddr, (uint16_t)regData, I2C_FMT_A8D16);
       
       if (rtn == FAILED)
         {
	    	   printf("OPT300X_I2C_WriteToSlave(regAddr=%x)---failed!!", regAddr);	    	   
	     }
	   #endif
	   
  	   return rtn;
}

static int OPT300X_I2C_ReadFromSlave(uint8_t regAddr, uint16_t *regData)
{
	 int  ret = 0;
	 
     #if (DEVICE_ALS_ENABLE == 1)	 
	 uint16_t value=0;
	  
	 ret = pega_i2c_Read(m_Als_i2cFile, OPT300x_ADDRESS, (uint32_t)regAddr, &value, I2C_FMT_A8D16);
	 
	 if (ret == FAILED)
     {
	     printf("OPT300X_I2C_ReadFromSlave(regAddr=%x)---failed!!", regAddr);	 	     
	     return ret;   	   
	 }
	        
	 *regData = OPT300X_I2C_Word_Data_Swap(value);
     #endif
	 
     return ret;
}

static uint16_t OPT300x_lux_to_opt3004_reg(float lux)
{
    uint8_t E = 0;
    uint32_t M;
    
    // 找出最小的 E 使 M <= 0xFFFF
    for (E = 0; E < 16; E++) 
	{
        M = (uint32_t)(lux * 100 / (1 << E) + 0.5); // 四捨五入
        if (M <= 0xFFFF)
            break;
    }
    
    if (E == 16) 
	{
        // 超過範圍
        E = 15;
        M = 0xFFFF;
    }
    
    // 將 E 放到高 4 bits，M 放到低 16 bits
    uint16_t reg = ((E & 0x0F) << 12) | ((M >> 4) & 0x0FFF);
    
    return reg;
}

static void OPT300x_set_mode(uint16_t wMode)
{
	  m_stOpt300x.wConfReg &= ~OPT300x_CONFIGURATION_M_MASK;
	  m_stOpt300x.wConfReg |= wMode;	
}

static int OPT300x_Read_Lux(void)
{
	  int bFlag = FALSE; 
	  uint16_t wVal,wExponent,wMantissa;
	  float LsbWeightTable[11] = {0.01,0.02,0.04,0.08,0.16,0.32,0.64,1.28,2.56,5.12,10.24};
	  	 	  
	  if (OPT300X_I2C_ReadFromSlave(OPT300x_REG_SYS_RESULT , &wVal) == SUCCEED)
	    {
	      bFlag = TRUE;
	    	 
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
	      
	      //printf("\r\n wExponent = %d", wExponent);	
	      //printf("\r\n wMantissa = %d", wMantissa);	
	      //printf("\n m_stOpt300x.fLux = %.3lf", m_stOpt300x.fLux);		  
	    }
	  
	  return bFlag;   
}

static int OPT300x_Continue_Mode_Check(void)
{	   
	  int bFlag = TRUE; 
	  
	  uint16_t wVal, wVal2;
	  
	  if (OPT300X_I2C_ReadFromSlave(OPT300x_REG_SYS_CONF , &wVal) == SUCCEED)
	    {
	    	wVal2 = wVal & 0xFC00; //bit15~bit12 = 0xC, bit11 = 1, bit10~bit9 = 10, Continuous conversions
	    	 
	    	if (wVal2 != 0xCC00)
	    	  {
	    	  	 printf("OPT300X continue mode error!(wVal=%04x, wVal2=%04x)", wVal, wVal2);
	    	  	 
	    	  	 m_stOpt300x.wConfReg = (wVal & 0x03FF) | 0xCC00; //To prevent the data error sometimes.
	    	  	 
	    	  	 OPT300x_set_mode(OPT300x_CONFIGURATION_M_CONTINUOUS);
	      
	             if (OPT300X_I2C_WriteToSlave(OPT300x_REG_SYS_CONF, m_stOpt300x.wConfReg) == SUCCEED)
	               {
	                 bFlag = FALSE; 	  
	               }  
	    	  }	    		        
	      //printc("\r\n m_stOpt300x.wConfReg = %04x", m_stOpt300x.wConfReg);		      
	    }	  
	  
	  return bFlag;   
}
//==============================================================================
// open the Linux device
void OPT300x_Device_I2C_open(void)
{
	 #if (DEVICE_ALS_ENABLE == 1)
	 char devPath[64];
	 sprintf(devPath,"/dev/i2c-%s", ALS_I2C_BUS);
	
	 m_Als_i2cFile = open(devPath, O_RDWR);
	
	 if (m_Als_i2cFile < 0) 
	 {
		 perror("i2cOpen");
	 }
	 #endif
}
// close the Linux device
void OPT300x_Device_I2C_close(void)
{
	 #if (DEVICE_ALS_ENABLE == 1)
	 if (m_Als_i2cFile > 0)
	   {
	       close(m_Als_i2cFile);
	   }
	   
	 m_Als_i2cFile = -1;
	 #endif
}

int OPT300x_Is_Init_Already(void)
{
	  int bFlag = FALSE; 
	  uint16_t wVal;
	  
	  if (OPT300X_I2C_ReadFromSlave(OPT300x_REG_SYS_CONF , &wVal) != FAILED)
	    {
	    	if ((wVal & 0xCC00) == 0xCC00) // 
	    	  {	
	    	     bFlag = TRUE; 
	    	  }   
	    }
	  
	  printf("\n[%s] CONF(%d)=%x (%d)\n", __FUNCTION__, OPT300x_REG_SYS_CONF, wVal, bFlag);
	  
	  return bFlag;   
}

void OPT300x_Device_Init(void)
{
	  uint16_t wVal;
	  
	  m_stOpt300x.wDeviceID = 0x0000;
	  m_stOpt300x.wConfReg  = 0x0000;
	  /* Enable automatic full-scale setting mode */
	  m_stOpt300x.wConfReg |= OPT300x_CONFIGURATION_RN_AUTO; // RN=1100, Auto-range
	  m_stOpt300x.wConfReg |= OPT300x_CONFIGURATION_CT;		 // CT=1, conversion time 800ms
		/* Ensure device is in shutdown initially */	  
	  OPT300x_set_mode(OPT300x_CONFIGURATION_M_CONTINUOUS);	 // 10 or 11, continuous mode 	 
	  /* Configure for latched window-style comparison operation */	
	  m_stOpt300x.wConfReg &= ~OPT300x_CONFIGURATION_ME;
	  m_stOpt300x.wConfReg &= ~OPT300x_CONFIGURATION_FC_MASK;
	  
	  if (OPT300x_Is_Init_Already() == FALSE)
	  	{
	      OPT300X_I2C_WriteToSlave(OPT300x_REG_SYS_CONF, m_stOpt300x.wConfReg);
	    }  

      //Config interrupt by threshold
      //OPT300X_I2C_WriteToSlave(OPT300x_REG_LOW_LIMIT,  OPT300x_lux_to_opt3004_reg(100.0)); //100 lux
      //OPT300X_I2C_WriteToSlave(OPT300x_REG_HIGH_LIMIT, OPT300x_lux_to_opt3004_reg(800.0)); //800 lux
	  
	  if (OPT300X_I2C_ReadFromSlave(OPT300x_REG_DEVICE_ID , &wVal) == SUCCEED)		
	  	{
		  m_stOpt300x.wDeviceID = wVal;
		}  
}

void OPT300x_Comparison_Limit_Set(uint8_t bIsLowLimit, uint16_t u16LimitVal)
{
	 if (bIsLowLimit != 0)
	  	{
	      OPT300X_I2C_WriteToSlave(OPT300x_REG_LOW_LIMIT, u16LimitVal);
	    } 
	 else
		{
	      OPT300X_I2C_WriteToSlave(OPT300x_REG_HIGH_LIMIT, u16LimitVal);
	    } 	
}

float OPT300x_Read_Lux_Handler(void)
{	    
     if (OPT300x_Read_Lux() == FALSE)
	 {
		 return -1;
	 }
	 	
     if (m_stOpt300x.fLux == m_stOpt300x.fLux_old)
      {
       	 OPT300x_Continue_Mode_Check();
      }
     else// if (m_stOpt300x.fLux != m_stOpt300x.fLux_old)    
      {
       	 m_stOpt300x.fLux_old = m_stOpt300x.fLux;
      }		
     
     return m_stOpt300x.fLux;    
}

float OPT300x_Lux_Value_Get(void)
{	   	   
	   return m_stOpt300x.fLux;
}

uint16_t OPT300x_Device_ID_Get(void)
{	   	
	   uint16_t wVal = 0;
	      
	   if (OPT300X_I2C_ReadFromSlave(OPT300x_REG_DEVICE_ID , &wVal) == SUCCEED)		
	     {
		   m_stOpt300x.wDeviceID = wVal;
				 
		   return m_stOpt300x.wDeviceID;
		 }	 
				
	   return 0;
}

float OPT300x_Diag_Lux_Value_Get(void)
{	  
      return OPT300x_Read_Lux_Handler();
}
//==============================================================================
/*
EX:if((access("test.c",F_OK))!=-1)  
F_OK:檔案是否存在
R_OK:檔案是否可讀取
W_OK:檔案是否可寫入
X_OK:檔案是否可執行
*/
//pega_debug debug info als
void OPT300x_Data_Print(void)
{
#if 1	
	 	 
	 OPT300X_DBG(printf("-----------------------\n"));
	 OPT300X_DBG(printf("m_Als_i2cFile = %d\n",   m_Als_i2cFile));	 
	 OPT300X_DBG(printf("-----------------------\n"));
	 OPT300X_DBG(printf("m_stOpt300x.wDeviceID = %04x\n",   m_stOpt300x.wDeviceID));	
	 OPT300X_DBG(printf("m_stOpt300x.wConfReg  = %04x\n",   m_stOpt300x.wConfReg));	 
	 OPT300X_DBG(printf("m_stOpt300x.fLux      = %.3lf\n",  m_stOpt300x.fLux));	
	 OPT300X_DBG(printf("m_stOpt300x.fLux_old  = %.3lf\n",  m_stOpt300x.fLux_old));	
	 OPT300X_DBG(printf("OPT300x_CONFIGURATION_M_MASK       = %04x\n", OPT300x_CONFIGURATION_M_MASK));
	 OPT300X_DBG(printf("OPT300x_CONFIGURATION_M_SHUTDOWN   = %04x\n", OPT300x_CONFIGURATION_M_SHUTDOWN));
	 OPT300X_DBG(printf("OPT300x_CONFIGURATION_M_SINGLE     = %04x\n", OPT300x_CONFIGURATION_M_SINGLE));
	 OPT300X_DBG(printf("OPT300x_CONFIGURATION_M_CONTINUOUS = %04x\n", OPT300x_CONFIGURATION_M_CONTINUOUS));
	 OPT300X_DBG(printf("OPT300x_CONFIGURATION_DEF          = %04x\n", OPT300x_CONFIGURATION_DEF));
	 OPT300X_DBG(printf("-----------------------\n"));
	 
#endif	 
}
//==============================================================================
//pega_debug debug info als_reg
void OPT300x_Register_Info_Print(void)
{   
	  #if 1
	    
	  uint8_t u8Reg;
		 
	  uint16_t regData;
		 		    
	  printf("\n-----------------------");	
	   
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
	             	    printf("\nReg(0x%02x) => 0x%04x",u8Reg, regData);
	             	    break;
	             
	             default:
	             	  break;	  	
	           }	
	       }	     
	       
	    printf("\n-----------------------");		   
	 
	 #endif       
}
//==============================================================================

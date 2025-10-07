/*******************************************************************************
* File Name: pega_nv_mode.c
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
#include <pthread.h>
//==============================================================================
#include "pega_nv_mode.h"
#include "pega_schedule.h"
#include "pega_als_opt300x.h"
#include "pega_gpio.h"
//==============================================================================
#define NV_DELAY_CNT_MS		      (1000*1000) //1s
//==============================================================================
#define NV_DBG(x)          x
//==============================================================================
static int InitialLSValue = 0x7FFFFFFF;
//==============================================================================
static stNVDefType m_stNVmode = 
{
	.bIsIRLedOn = 2,
	.fLux = 0xFFFFFF,
	.fNightModeThd = 2.0,
	.fDayModeThd = 5.0,
	.fLuxCal = 1.0, 
	.bNVModeSet = FALSE,
	.bIsNVModeAuto = TRUE,
	.bIsNightMode = NV_MODE_STATE_MAX,
	.sTriggerCounts = 4,
};
//==============================================================================
static float m_tmpfLux = -1;
//==============================================================================
//pega_camcli misc set irled off/on
void Pega_NV_Mode_IR_Led_Control(int bEnable)
{	   
	 m_stNVmode.bIsIRLedOn = FALSE;	
	 	  
	 if (bEnable == TRUE)
	   {
	   	 if (m_stNVmode.bIRLedEn == TRUE)
	   	   {	   	   	
	   	   	 m_stNVmode.bIsIRLedOn = TRUE;
	   	   }	
	   }		
	
	if (m_stNVmode.bIsIRLedOn == TRUE)
	{
		IO_IRLED_CTRL_ON();
	}
	else
	{
		IO_IRLED_CTRL_OFF();
	}		       
}

static float Pega_NV_Mode_Lux_Update(float fLux)
{
	m_stNVmode.fLux = fLux;
		
	if (m_stNVmode.fLuxCal != 0)
	{
		m_stNVmode.fLux = (float)m_stNVmode.fLux * m_stNVmode.fLuxCal;
	}

	m_stNVmode.fLux = m_stNVmode.fLux + m_stNVmode.fOffset;

	if (InitialLSValue == 0x7FFFFFFF)
	{
		InitialLSValue = (int)m_stNVmode.fLux;
	}
	
	return m_stNVmode.fLux;
}

//==============================================================================
void Pega_NV_Mode_nvram_update(void)
{
	float fLuxGain = 1.696; //230605 Chris calibrate als gain
	int s16NightModeThd = 0, s16DayModeThd = 0;
	char char_fLuxGain [32];
	FILE *fp = NULL;
//	if (pga_env_var_get(ENV_ALS_CALI_GAIN, (char *)&fLuxGain) == SUCCESS)
	//pga_env_get_ALS_CALI_GAIN(&fLuxGain);
	system("echo $(sku_printenv -n ALS_CALI_GAIN) >/tmp/pega/als_cali_gain");
	
    if((fp = fopen("/tmp/pega/als_cali_gain", "r")) == 0)
    {
         printf("/tmp/pega/als_cali_gain open failed!");
         
    }	
	
	fread(char_fLuxGain, sizeof(char), 50, fp);
	fLuxGain = atof(char_fLuxGain);
	
	if (fLuxGain > 0)
	{
	    m_stNVmode.fLuxCal = fLuxGain;
	}  
	
	if (fp != NULL)
	{
       fclose(fp);
	}		
	
	printf("\nPega_NV_Mode_nvram_update(%.3lf)\n", fLuxGain);	
}

int Pega_NV_EnterNVThreshold_Update(int night_thd)
{	
	int rtn = FAILED;
	
	if (night_thd > 0)
	{
		m_stNVmode.fNightModeThd  = (float)night_thd / 100;
		rtn = SUCCEED;
	} 

	return rtn;
}

int Pega_NV_ExitNVThreshold_Update(int day_thd)
{
	int rtn = FAILED;

	if (day_thd > 0)
	{
		m_stNVmode.fDayModeThd	= (float)day_thd / 100;
		rtn = SUCCEED;
	} 

	return rtn;
}
//==============================================================================
void Pega_NV_Mode_Init(void)
{	
#if 0

	 m_stNVmode.bIsIRLedOn     = 2;
	 m_stNVmode.fLux           = 0xFFFFFF;
	 m_stNVmode.fNightModeThd  = 2.0;
	 m_stNVmode.fDayModeThd    = 5.0;
	 m_stNVmode.fLuxCal        = 1.696; //230605 Chris calibrate als gain
	 m_stNVmode.bNVModeSet     = FALSE;	 
	 m_stNVmode.bIsNightMode   = NV_MODE_STATE_MAX;
	 m_stNVmode.sTriggerCounts = 4;
#endif	     
}

void Pega_NV_DayNighMode_Manual_Set(uint8_t bNightMode)
{
	m_stNVmode.bNVModeSet = FALSE;  
	m_stNVmode.bIsNightMode = bNightMode;
	m_stNVmode.bIsNVModeAuto = FALSE;
}

void Pega_NV_Mode_Manual_Set(void)
{
	 if (m_stNVmode.bNVModeSet == TRUE)
	   {	
	     return;	   
	   }  
	   
	 if (m_stNVmode.bIsNightMode == TRUE)
	   {
	   	 m_stNVmode.bIsNightMode = FALSE;
	     Pega_NV_Mode_Night_Mode_Set();
	   }  
	 else
	   {
	   	 m_stNVmode.bIsNightMode = TRUE;
	     Pega_NV_Mode_Day_Mode_Set();  
	   }  
	   
	 m_stNVmode.bNVModeSet = TRUE;  	  	    
}

void Pega_Force_Day_Night_Mode(uint8_t IsNightMode)
{
	 printf("\n%s(IsNightMode=%d)", __FUNCTION__, IsNightMode);  
   	
	 if (IsNightMode)// night mode
	 {
		Pega_Gpio_IRCut_Control(DISABLE); // IRCut off
		IO_IRLED_CTRL_ON();     		
	 }
	 else			// day mode
	 {
		IO_IRLED_CTRL_OFF();			
		Pega_Gpio_IRCut_Control(ENABLE); // IRCut on
	 }		
}

void Pega_NV_Mode_Night_Mode_Set(void)
{	 
	if (m_stNVmode.bIsNightMode == TRUE)
	{	
		return;
	}  
    
	printf("\n[%s]bIsNightMode=%d \n", __func__, m_stNVmode.bIsNightMode);
	
	m_stNVmode.bIsNightMode = TRUE; 

	//Night	       
	
//	fprintf(stdout, "\033[1;31m pga_day_night_mode_handler, %s, line %d, gettid() %ld, name=%s\033[0m\n", __FILE__, __LINE__, gettid(), pga_platform_getThreadName(gettid())); 
#if 0 /// not support on pure Linux platform
	if (pga_stm_QRscan_running_get () == false)
	{
		
	}
#endif		
	
	Pega_Force_Day_Night_Mode(m_stNVmode.bIsNightMode);
}

void Pega_NV_Mode_Day_Mode_Set(void)
{
		 
	if (m_stNVmode.bIsNightMode == FALSE)
	{	
		return;
	}  
    
    printf("\n[%s]bIsNightMode=%d \n", __func__, m_stNVmode.bIsNightMode); 
	
	m_stNVmode.bIsNightMode = FALSE; 	   

//	fprintf(stdout, "\033[1;31m pga_day_night_mode_handler, %s, line %d, gettid() %ld, name=%s\033[0m\n", __FILE__, __LINE__, gettid(), pga_platform_getThreadName(gettid())); 
#if 0 /// not support on pure Linux platform
	if (pga_stm_QRscan_running_get () == false)
	{
		
	}
#endif
	
	Pega_Force_Day_Night_Mode(m_stNVmode.bIsNightMode);
}

void Pega_NV_Mode_Threshold_Set(float fNightModeThd, float fDayModeThd)
{
	 m_stNVmode.fNightModeThd = fNightModeThd;
	 m_stNVmode.fDayModeThd   = fDayModeThd;
}

void Pega_NV_Mode_parameters_Set(stNVDefType para)
{
	 memcpy(&m_stNVmode, &para, sizeof(stNVDefType));	
}

void Pega_NV_Mode_parameters_Get(stNVDefType *para)
{	 
	 memcpy(para, &m_stNVmode, sizeof(stNVDefType));
}

void Pega_NV_Mode_Control_Set(uint8_t bFlag)
{
	 m_stNVmode.bIsNVModeAuto     = bFlag;
	 m_stNVmode.u8DayModeCounts   = 0; 
	 m_stNVmode.u8NightModeCounts = 0;
}

void Pega_NV_Mode_NVModeSet_Flag_Clear(void)
{
	 m_stNVmode.bNVModeSet = FALSE;	 
}

void Pega_NV_Mode_NVModeSet_LuxCal_Set(float fLuxCal)
{
	 m_stNVmode.fLuxCal = fLuxCal;	
}

float Pega_NV_Mode_NVModeSet_LuxCal_Get()
{
	return m_stNVmode.fLuxCal;	
}

void Pega_NV_Mode_NVModeSet_LuxOffset_Set(float fLuxOffset)
{	    
	 m_stNVmode.fOffset = fLuxOffset;	
}
//==============================================================================
void Pega_NV_Mode_Night_IR_Led_Control(uint8_t bFlag)
{
	 m_stNVmode.bIRLedEn = bFlag;
	   
	 if (m_stNVmode.bIsNightMode != TRUE)
       {
	       return;	   
       }
     
     Pega_NV_Mode_IR_Led_Control(TRUE);
              
     printf("\n[%s]%d \n", __func__, m_stNVmode.bIRLedEn);       
}
//==============================================================================
int Pega_NV_Mode_Processing(void)
{	
	int rtn = NOT_THING;	
	
	if (m_stNVmode.bIsNVModeAuto != TRUE)
	{	        
		Pega_NV_Mode_Manual_Set();	   			
		return rtn;	
	}   
	
	//printf("\n[%s]eALSType=%d \n", __func__, eALSType);  
	
	m_tmpfLux = OPT300x_Read_Lux_Handler();
	
	if (m_tmpfLux < 0)
	 {
		return rtn;	
	 }
	 
    Pega_NV_Mode_Lux_Update(m_tmpfLux);
			
	if (m_stNVmode.fLux < m_stNVmode.fNightModeThd)
	{
		if (m_stNVmode.u8NightModeCounts >= (m_stNVmode.sTriggerCounts-1))
		{
			
			m_stNVmode.u8DayModeCounts = 0; 			
			Pega_NV_Mode_Night_Mode_Set();	           
			rtn = NV_MODE_STATE_NIGHT;
		}
		else
		{
			m_stNVmode.u8NightModeCounts++;	
		}  
	}  

	if (m_stNVmode.fLux > m_stNVmode.fDayModeThd)
	{
		if (m_stNVmode.u8DayModeCounts >= (m_stNVmode.sTriggerCounts-1))
		{
			
			m_stNVmode.u8NightModeCounts = 0; 			
			Pega_NV_Mode_Day_Mode_Set();
			rtn = NV_MODE_DAY;
		}
		else
		{
			m_stNVmode.u8DayModeCounts++;	
		}     
	}	 
	
	return rtn;
}
//==============================================================================
void Pega_NV_Mode_Timer_Delay_Set(unsigned int seconds)
{
	struct timeval tv;
	
	tv.tv_sec  = seconds;
	tv.tv_usec = 0;
	select(0, NULL, NULL, NULL, &tv);	
}

uint8_t Pega_VN_Mode_NightMode_Get(void)
{
	return m_stNVmode.bIsNightMode;
}

void Pega_NV_Mode_NightMode_Set(uint8_t day_night)
{
	m_stNVmode.bIsNightMode = day_night;
}

void Pega_NV_Mode_IRLedEn_Set(uint8_t enable)
{
	m_stNVmode.bIRLedEn = enable;
}

//for diag commands
float Pega_NV_Mode_Diag_Lux_Get(void)
{	  
	 return Pega_NV_Mode_Lux_Update(OPT300x_Read_Lux_Handler());
}

int pga_get_InitialLSValue(void)
{
	return InitialLSValue;
}

int pga_get_LastLSValue(void)
{
	return (int)m_stNVmode.fLux;
}

void pga_set_NV_TriggerCounts(int value)
{
	m_stNVmode.sTriggerCounts = value;
}
//==============================================================================
void Pega_Diag_Force_Day_Night_Mode(uint8_t IsNightMode)
{
	 m_stNVmode.bIsNVModeAuto = FALSE;
     m_stNVmode.bNVModeSet    = TRUE;
	 
	 if (m_stNVmode.bIsNightMode != IsNightMode)
	 {	 
         m_stNVmode.bIsNightMode = IsNightMode;
	     Pega_Force_Day_Night_Mode(IsNightMode);
	 }
}
//==============================================================================
//pega_misc_dbg info 8
void Pega_NV_Mode_Data_Print(void)
{			 
     NV_DBG(printf("\n-----------------------"));	 
	 NV_DBG(printf("\n m_stNVmode.bIsNVModeAuto   = %d", m_stNVmode.bIsNVModeAuto));
	 NV_DBG(printf("\n m_stNVmode.bNVModeSet      = %d", m_stNVmode.bNVModeSet));
	 NV_DBG(printf("\n m_stNVmode.bIsNightMode    = %d", m_stNVmode.bIsNightMode));
	 NV_DBG(printf("\n-----------------------"));	 
	 NV_DBG(printf("\n m_stNVmode.fLux            = %.3lf", m_stNVmode.fLux));	
	 NV_DBG(printf("\n m_stNVmode.fOffset         = %.3lf", m_stNVmode.fOffset));	
	 NV_DBG(printf("\n m_stNVmode.fLuxCal         = %.3lf", m_stNVmode.fLuxCal)); 
	 NV_DBG(printf("\n m_stNVmode.fNightModeThd   = %.3lf", m_stNVmode.fNightModeThd));	
	 NV_DBG(printf("\n m_stNVmode.fDayModeThd     = %.3lf", m_stNVmode.fDayModeThd));	 	 
	 NV_DBG(printf("\n m_stNVmode.bIRLedEn        = %d",  m_stNVmode.bIRLedEn));	
	 NV_DBG(printf("\n m_stNVmode.bIsIRLedOn      = %d",  m_stNVmode.bIsIRLedOn));	
	 NV_DBG(printf("\n m_stNVmode.DayModeCounts   = %d",  m_stNVmode.u8DayModeCounts));	
	 NV_DBG(printf("\n m_stNVmode.NightModeCounts = %d",  m_stNVmode.u8NightModeCounts));	
	 NV_DBG(printf("\n m_stNVmode.sTriggerCounts  = %d",  m_stNVmode.sTriggerCounts));	
	 NV_DBG(printf("\n-----------------------"));
	 NV_DBG(printf("\n NV_DELAY_CNT_MS            = %d", NV_DELAY_CNT_MS));	
	 NV_DBG(printf("\n-----------------------\n"));	  
}


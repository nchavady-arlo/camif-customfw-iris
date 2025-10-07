/*******************************************************************************
* File Name: main.c
*
*******************************************************************************/
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
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
#include <time.h>
#include <stdbool.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/watchdog.h>
//==============================================================================
#include "main.h"
//==============================================================================
#include "pega_als_opt300x.h"
#include "pega_amp_tas256x.h"
//==============================================================================
#include "pega_gpio.h"
#include "pega_adc.h"
#include "pega_pwm.h"
//==============================================================================
#include "pega_schedule.h"
#include "pega_nv_mode.h"
#include "pega_wifi.h"
#include "pega_network_ip.h"
#include "pega_led_flash.h"
//==============================================================================
#include "pega_motor_interrupt.h"
//==============================================================================
#include "pega_misc_diag.h"
#include "pega_debug.h"
//==============================================================================
static stMiscType m_stMisc;
//==============================================================================
static void Pega_misc_help_print(void)
{	   	   
	   printf("Usage :\n");	   
	   printf("Pega_misc help info [options]\n");
	   printf("options:\n");	   
	   printf("\t-a : Disable ALS\n");
	   printf("\t-A : Disable ACC\n");
	   printf("\t-w : Disable WiFi\n");
	   printf("\t-n : Disable NFC\n");
	   printf("\t-v : version\n");
	   printf("\t-h : help info\n");
}
//==============================================================================
static void Pega_Misc_system_variables_init(void)
{
	   m_stMisc.bIsALSDisable = 1;
}

static void Pega_Misc_hareware_init(void)
{
	   Pega_Gpio_init();
	   Pega_pwm_init(0);
	   
	   #if (DEVICE_ALS_ENABLE == 1)
	   {
		   OPT300x_Device_Init();
	   }
	   #endif
	   
	   #if (DEVICE_AMP_ENABLE == 1)	  
		   Pega_AMP_Initialize(1,10.0);
	   #endif
}
//==============================================================================
static void Pega_Misc_system_tasks_start(void)
{
		/* create schedule handler */
		pega_schedule_handler_Start();
		
		/* create LED handler */
		Pega_led_flash_handler_Start();	
		
		/* create diag handler */
		Pega_diag_msgq_handler_start();
		/* create debug handler */
		Pega_debug_msgq_handler_start();
		
		if (m_stMisc.bIsMotorDisable == 0)
		{
			Pega_Motor_interrupt_handler_Start();			
		}
}
//==============================================================================
static int pega_misc_cmd_paring(int argc, char* argv[])
{
	int s32Opt = 0,value;
	  
	  // parsing command line
    while ((s32Opt = getopt(argc, argv, "s:P:R::m::M::f::w::a::A::v::n::h::")) != -1)
     {
        switch(s32Opt)
        {	
        	//
            case 'a': 	                 
				m_stMisc.bIsALSDisable = 1;	
			    break;
            
			case 'A': 	                 
				m_stMisc.bIsACCDisable = 1;	
			    break;
				
			case 'w': 	                 
				m_stMisc.bIsWiFiDisable = 1;				    
                break;
			
			case 'n': 	                 
				m_stMisc.bIsNFCDisable = 1;				    
                break;
				
			case 'M': 	                 
				m_stMisc.bIsMotorDisable = 1;				    
                break;
            //==============================================================================
            case 'v':		                 
				fprintf(stdout, "pega_misc --version: %s\n", MISC_VERSTION);
				fprintf(stdout, "pega_misc --date: %s\n", __DATE__);
				fprintf(stdout, "pega_misc --time: %s\n", __TIME__);
			    return FAILED; 
			//==============================================================================
            case 'h':                 
            default:
                 Pega_misc_help_print();
                 return FAILED;  
        }
     }       
  	   
	  return NOT_THING;	
}
//==============================================================================
static void pega_misc_ALS_handler(void)
{
	if (m_stMisc.bIsALSDisable == 0) 
	{
		Pega_NV_Mode_Processing();
	}
}
//==============================================================================
static void pega_misc_wifi_ready_check(void)
{
	if ((m_stMisc.bIsWiFiDisable > 0) || (m_stMisc.bIsWiFiFwReady > 0))
	{
		return;
	}
	
	if (pega_netip_is_interface_exist("mlan0", 0) > -1)
	{
		if (pega_netip_is_interface_exist("mlan1", 0) > -1)
		{
			m_stMisc.bIsWiFiFwReady = 1;
		
			Pega_Misc_Update_WiFi_interface();
		
			Pega_led_flash_state_Set(LED_State_Wifi_Ready);
		}
	}	
}
//==============================================================================
static void pega_misc_wifi_interface_check(void)
{
	if ((m_stMisc.bIsWiFiDisable > 0) || (m_stMisc.bIsWiFiFwReady == 0))
	{
		return;
	}
	
	if (pega_netip_is_interface_exist("mlan0", 0) < 0)
	{
		if (pega_netip_is_interface_exist("mlan1", 0) < 0)
		{
			m_stMisc.bIsWiFiFwReady = 0;
		
			Pega_led_flash_state_Set(LED_State_Wifi_off);
		}
	}	
}
//==============================================================================
/*
pstree
ps -T
cat /proc/pid/status
ex:
cat /proc/692/status
*/
int main(int argc, char* argv[])
{
	memset(&m_stMisc, 0, sizeof(m_stMisc));
	
	//printf("[%s] argc=%d \n", __func__, argc);      
	
    if (argc > 1)
    {
    	//printf("\n[%s,%d]%s,%s \n", __func__, argc, argv[0],argv[1]);     		
    	if (pega_misc_cmd_paring(argc, argv) == FAILED)
    	  {    		  	
    	  	return 0; 
    	  }     		  			 	    	 	
    }    
    	
	OPT300x_Device_I2C_open();
	Pega_AMP_Device_I2C_open();
	
    Pega_Misc_system_variables_init();
	
	Pega_Misc_hareware_init();
	
    Pega_Misc_system_tasks_start();
	
	Pega_led_flash_state_Set(LED_State_System_Start_Up);
	
	if (m_stMisc.bIsWiFiDisable == 0)
	{
		pega_schedule_Event_push(schEVT_Load_wifi_fw, SCH_1Sec);
	}
	
	#if 1
	while(1)
	{
		pega_misc_ALS_handler();
		pega_misc_wifi_ready_check();
		pega_misc_wifi_interface_check();
		
		usleep(MISC_DELAY_CNT_MS); //1s			
	}
	#endif
	
	OPT300x_Device_I2C_close();
	Pega_AMP_Device_I2C_close();
	
    return 0;
}
//==============================================================================
void pega_misc_wifi_fw_error(void)
{
	 m_stMisc.bIsWiFiFwError = 0;
	
	 Pega_led_flash_state_Set(LED_State_Wifi_fw_error);
}
//pega_misc_dbg set 4 0/1
void Pega_Misc_Burn_Enable_Control(uint8_t bIsEnable)
{
	 m_stMisc.bIsBurnInOn = (bIsEnable > 0) ? 1 : 0;
	 printf("(%s)bIsEnable=%d\n", __func__, m_stMisc.bIsBurnInOn); 
	 
}
//pega_misc_dbg set 5 0/1
void Pega_Misc_ALS_Disable_Control(uint8_t bIsDisable)
{
	 m_stMisc.bIsALSDisable = (bIsDisable > 0) ? 1 : 0;
	 printf("(%s)bIsDisable=%d\n", __func__, m_stMisc.bIsALSDisable); 	 
}
//pega_misc_dbg set 6 0/1
void Pega_Misc_WiFi_Disable_Control(uint8_t bIsDisable)
{
	 m_stMisc.bIsWiFiDisable = (bIsDisable > 0) ? 1 : 0;
	 printf("(%s)bIsDisable=%d\n", __func__, m_stMisc.bIsWiFiDisable); 
}
//==============================================================================
//pega_misc_dbg set 2
void Pega_Misc_Update_WiFi_interface(void)
{
	 //printf("(%s)\n", __func__); 
	 
	 m_stMisc.eWIFI1_if = E_WIFI_IF_TYPE_NONE;
	 m_stMisc.eWIFI2_if = E_WIFI_IF_TYPE_NONE;
	 
	 if (pega_netip_interface_check_sd0("mlan0",0) >= 0)
	 {
		 m_stMisc.eWIFI1_if = E_WIFI_IF_TYPE_MLAN0;
	 }
	 else if (pega_netip_interface_check_sd0("mlan1",0) >= 0)
	 {
		 m_stMisc.eWIFI1_if = E_WIFI_IF_TYPE_MLAN1;
	 }
	 
	 if (pega_netip_interface_check_sd1("mlan0",0) >= 0)
	 {
		 m_stMisc.eWIFI2_if = E_WIFI_IF_TYPE_MLAN0;
	 }
	 else if (pega_netip_interface_check_sd1("mlan1",0) >= 0)
	 {
		 m_stMisc.eWIFI2_if = E_WIFI_IF_TYPE_MLAN1;
	 }
}
//==============================================================================
//diag wifi interface
void Pega_Misc_Diag_wifi_interface_Print(void)
{	  
     const char eWiFi_IF[3][6] = {"none","mlan0","mlan1"};
	 
     Pega_Misc_Update_WiFi_interface();
	 
	 PT_DIAG_GET(0, "SDIO_0:%s", eWiFi_IF[m_stMisc.eWIFI1_if]);
	 PT_DIAG_GET(0, "SDIO_1:%s", eWiFi_IF[m_stMisc.eWIFI2_if]);	 
}
//pega_misc_dbg info 1
void Pega_Misc_Data_Print(void)
{	  
     const char eWiFi_IF[3][6] = {"none","mlan0","mlan1"};
	 
	 printf("-----------------------\n");
	 printf("sizeof(uint16_t)            = %d\n", sizeof(uint16_t));
	 printf("-----------------------\n");
	 printf("m_stMisc.bIsBurnInOn        = %d\n", m_stMisc.bIsBurnInOn);
	 printf("m_stMisc.bIsALSDisable      = %d\n", m_stMisc.bIsALSDisable);
	 printf("m_stMisc.bIsWiFiDisable     = %d\n", m_stMisc.bIsWiFiDisable);
	 printf("m_stMisc.bIsACCDisable      = %d\n", m_stMisc.bIsACCDisable);
	 printf("m_stMisc.bIsNFCDisable      = %d\n", m_stMisc.bIsNFCDisable);
	 printf("m_stMisc.bIsMotorDisable    = %d\n", m_stMisc.bIsMotorDisable);
	 printf("-----------------------\n");
	 printf("m_stMisc.bIsWiFiFwReady     = %d\n", m_stMisc.bIsWiFiFwReady);
	 printf("m_stMisc.bIsWiFiFwError     = %d\n", m_stMisc.bIsWiFiFwError);
	 printf("-----------------------\n");
	 printf("m_stMisc.eWIFI1_if(%d)      = %s\n", m_stMisc.eWIFI1_if, eWiFi_IF[m_stMisc.eWIFI1_if]);
	 printf("m_stMisc.eWIFI2_if(%d)      = %s\n", m_stMisc.eWIFI2_if, eWiFi_IF[m_stMisc.eWIFI2_if]);
     printf("-----------------------\n");
}
//==============================================================================

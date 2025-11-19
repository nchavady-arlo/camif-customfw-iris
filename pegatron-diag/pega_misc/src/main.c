/*******************************************************************************
* File Name: main.c
*
*******************************************************************************/
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
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
#include "pega_gpio_key.h"
#include "pega_adc.h"
#include "pega_pwm.h"
#include "pega_i2c_control.h"
//==============================================================================
#include "pega_schedule.h"
#include "pega_nv_mode.h"
#include "pega_wifi.h"
#include "pega_network_ip.h"
#include "pega_led_flash.h"
#include "pega_dtbo.h"
//==============================================================================
#include "pega_linux_mac.h"
#include "pega_nxp_wifi.h"
//==============================================================================
#include "pega_nfc.h"
#include "pega_msgq.h"
#include "pega_utilites.h"
//==============================================================================
#define VOICE_DEMO_FILE 	"/data/cyberon_spk/Guest.bin"
//==============================================================================
static stMiscType m_stMisc;
//==============================================================================
static void Pega_misc_help_print(void)
{	   	   
	   printf("Usage :\n");	   
	   printf("Pega_misc help info [options]\n");
	   printf("options:\n");	   
	   printf("\t-a : Disable ALS\n");
	   printf("\t-A : Disable AMP\n");
	   printf("\t-w : Disable WiFi\n");
	   printf("\t-n : Disable NFC\n");
	   printf("\t-L : Disable RGBLed\n");
	   printf("\t-r : Disable RingLed\n");
	   printf("\t-M : Disable Motor\n");
	   printf("\t-D : Enable Voice Command Demo\n");
	   printf("\t-R : Enable RTtest\n");
	   printf("\t-v : version\n");
	   printf("\t-h : help info\n");
}
//==============================================================================
static void Pega_Misc_system_variables_init(void)
{
	   m_stMisc.bIsALSDisable  = 1;
	   //m_stMisc.bIsWiFiDisable = 1;
	   
	   m_stMisc.bIsDtboReady = (Pega_dtbo_init() > -1);
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
	    if (m_stMisc.bIsAmpDisable == 0)
		{			
		    Pega_AMP_Initialize(1,10.0);
		}
	   #endif
}
//==============================================================================
static void Pega_Misc_system_tasks_start(void)
{
		printf("[pega_misc] Create system task\n");
		/* create schedule handler */
		pega_schedule_handler_Start();
		
		/* create LED handler */
		Pega_led_flash_handler_Start();	
		/* create NFC process handler */
		if (m_stMisc.bIsNFCDisable == 0) 
		{
			Pega_NFC_TaskHandler_Start();
		}
		
		/* create key dtection handler */
		#if (BUTTON_DET_EN == 1)
        Pega_Gpio_key_Detection_Start();
	    #endif
		
		/* create msgQ handler */
		Pega_msgq_listen_handler_start();		
}
//==============================================================================
static int pega_misc_cmd_paring(int argc, char* argv[])
{
	int s32Opt = 0,value;
	  
	  // parsing command line
    while ((s32Opt = getopt(argc, argv, "s:P:L::R::r::m::M::f::w::a::A::D::v::n::R::h::")) != -1)
     {
        switch(s32Opt)
        {	
        	//
            case 'a': 	                 
				m_stMisc.bIsALSDisable = 1;	
			    break;
            
			case 'A': 	                 
				m_stMisc.bIsAmpDisable = 1;	
			    break;
				
			case 'D': 	                 
				m_stMisc.bIsVoiceDemoEnable = 1;	
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
			
			case 'L': 	                 
				m_stMisc.bIsRGBLedDisable = 1;				    
                break;	
				
			case 'r': 	                 
				m_stMisc.bIsRingLedDisable = 1;				    
                break;	
				
			case 'R': 	                 
				m_stMisc.bIsRFTestEnable = 1;				    
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
static void pega_misc_wifi_configurations_update(void)
{
	stDevice_info DevInfo;
		
	if (Pega_dtbo_data_get(&DevInfo) > 0)
	{		
		Pega_linux_mac_address_set(WIFI_IFNAME1, DevInfo.mac1);		
		Pega_linux_mac_address_set(WIFI_IFNAME2, DevInfo.mac2);
	
		Pega_nxp_wifi_country_code_set(WIFI_IFNAME1, DevInfo.wifi_cc);
		Pega_nxp_wifi_country_code_set(WIFI_IFNAME2, DevInfo.wifi_cc);
	}
}
//==============================================================================
static void pega_misc_wifi_ready_check(void)
{
	if ((m_stMisc.bIsWiFiDisable > 0) || (m_stMisc.bIsWiFiFwReady > 0))
	{
		return;
	}
	
	if (pega_netip_is_interface_exist(WIFI_IFNAME1, 0) > -1)
	{
		if (pega_netip_is_interface_exist(WIFI_IFNAME2, 0) > -1)
		{
			m_stMisc.bIsWiFiFwReady = 1;
		
		    pega_misc_wifi_configurations_update();
			
			Pega_Misc_Update_WiFi_interface();
		
			Pega_led_flash_state_Set(LED_State_Wifi_FW_Ready);
		}
	}
	
	//if (m_stMisc.bIsWiFiFwReady > 0) //&& (m_stMisc.bIsVoiceDemoReady > 0)
	if ((m_stMisc.bIsWiFiFwReady > 0) && (m_stMisc.bIsVoiceDemoReady > 0))
	{
		m_stMisc.eWIFI_State = E_WIFI_STATE_CONNECT;
		Pega_led_flash_state_Set(LED_State_Wifi_Connect_trigger);	 
		pega_schedule_Event_push(schEVT_wifi_connect_enable, SCH_1Sec);//SCH_1Sec
	}
}
//==============================================================================
static void pega_misc_wifi_interface_check(void)
{
	if ((m_stMisc.bIsWiFiDisable > 0) || (m_stMisc.bIsWiFiFwReady == 0))
	{
		return;
	}
	
	if (pega_netip_is_interface_exist(WIFI_IFNAME1, 0) < 0)
	{
		if (pega_netip_is_interface_exist(WIFI_IFNAME2, 0) < 0)
		{
			m_stMisc.bIsWiFiFwReady = 0;
			m_stMisc.eWIFI_State = E_WIFI_STATE_NONE;
			Pega_led_flash_state_Set(LED_State_Wifi_off);
		}
	}	
}

static void pega_misc_wifi_connection_check(void)
{
	char ip_addr[128] = {0};
	char buff[1024]={0};
	
	if ((m_stMisc.bIsWiFiDisable > 0) || (m_stMisc.bIsWiFiFwReady == 0))
	{
		return;
	}
	
	switch(m_stMisc.eWIFI_State)
	{
		case E_WIFI_STATE_CONNECT:
		     if (pega_netip_ip_get(WIFI_IFNAME1, ip_addr, 128) > -1)
			 {
				 m_stMisc.eWIFI_State = E_WIFI_STATE_CONNECTED;
				 Pega_led_flash_state_Set(LED_State_Wifi_Connected);				 
				 if (pega_util_shell("sh /scripts/Demo_enable.sh", buff, sizeof(buff)) != 0)
				  {
					printf("Error: %s failed\n", "sh /scripts/Demo_enable.sh");       
				  } 
			 }
		     break;
		
	    case E_WIFI_STATE_CONNECTED:
		     if (pega_netip_ip_get(WIFI_IFNAME1, ip_addr, 128) < 0)
			 {
				 m_stMisc.eWIFI_State = E_WIFI_STATE_DISCONNECT;
				 Pega_led_flash_state_Set(LED_State_Wifi_disconnect);				 
			 }
		     break;
		
        case E_WIFI_STATE_DISCONNECT:
		     if (pega_netip_ip_get(WIFI_IFNAME1, ip_addr, 128) > -1)
			 {
				 m_stMisc.eWIFI_State = E_WIFI_STATE_CONNECTED;
				 Pega_led_flash_state_Set(LED_State_Wifi_Connected);				 
			 }
		     break;
			 
		default:
		     return; 
	}	
}
//==============================================================================
static void pega_misc_Device_I2C_Bus_open(void)
{
	if (m_stMisc.bIsALSDisable == 0) 
	{
		OPT300x_Device_I2C_open();
	}
	
	if (m_stMisc.bIsAmpDisable == 0) 
	{
		Pega_AMP_Device_I2C_open();
	}
}

static void pega_misc_Device_I2C_Bus_close(void)
{
	if (m_stMisc.bIsALSDisable == 0) 
	{
		OPT300x_Device_I2C_close();
	}
	
	if (m_stMisc.bIsAmpDisable == 0) 
	{
		Pega_AMP_Device_I2C_close();
	}
}
//==============================================================================
static void pega_misc_Voice_Demo_Check(void)
{	
	if (m_stMisc.bIsVoiceDemoEnable > 0)
	{	
        char buff[1024]={0};
		
		if (access(VOICE_DEMO_FILE, F_OK) == 0) //is training file exist?
		{			
			m_stMisc.bIsVoiceDemoReady = 1;
		}
		else
		{						
			m_stMisc.bIsWiFiDisable = 1;
			sleep(5);
			Pega_led_flash_state_Set(LED_State_Power_off);
			//Enable Voice command training.			
			if (pega_util_shell("sh /scripts/Demo_training.sh", buff, sizeof(buff)) != 0)
			 {
				printf("Error: %s failed\n", "sh /scripts/Demo_training.sh");       
			 } 
		}
	}
}
//==============================================================================
void daemonize(void) 
{
    pid_t pid;

    // 第一次 fork
    pid = fork();
	
	/*
		pid < 0 : fork failed
		pid = 0 : child process
		pid > 0 : father process
	*/
	
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    // 新的 session leader
    if (setsid() < 0) exit(EXIT_FAILURE);

    // 忽略 SIGHUP
    signal(SIGHUP, SIG_IGN);

    // 第二次 fork
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    // 乾淨環境
    chdir("/");
    umask(0);
    //fclose(stdin);
    //fclose(stdout);
    //fclose(stderr);
}
/*
pstree
ps -T
cat /proc/pid/status
ex:
cat /proc/692/status
*/
int main(int argc, char* argv[])
{
	daemonize();
	
	openlog("PEGA", 0, LOG_DAEMON);
	//openlog("arlod", LOG_PID | LOG_CONS, LOG_DAEMON);
   	
	setlinebuf(stdout);
	
	_LOG_NOTICE("Starting pega_misc");
	
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

    pega_i2c_Init();    
	
	pega_misc_Device_I2C_Bus_open();
				
    Pega_Misc_system_variables_init();
	
	Pega_Misc_hareware_init();
		
	if (m_stMisc.bIsNFCDisable == 0) 
	{
		Pega_NFC_Device_init();
	}
	
    Pega_Misc_system_tasks_start();
	
	Pega_led_flash_state_Set(LED_State_System_Start_Up);
	
	pega_misc_Voice_Demo_Check();
	
	if ((m_stMisc.bIsRFTestEnable == 0) && (m_stMisc.bIsWiFiDisable == 0))
	{
		pega_schedule_Event_push(schEVT_Load_wifi_fw, SCH_300ms);//SCH_1Sec
	}
			
	#if 1
	while(1)
	{
		pega_misc_ALS_handler();
		pega_misc_wifi_ready_check();
		pega_misc_wifi_interface_check();
		pega_misc_wifi_connection_check();
		
		usleep(MISC_DELAY_CNT_MS); //1s			
	}
	#endif

    pega_misc_Device_I2C_Bus_close();
    	
	pega_i2c_Deinit();  
	
    return 0;
}
//==============================================================================
void pega_misc_wifi_fw_error(void)
{
	 m_stMisc.bIsWiFiFwError = 0;
	
	 Pega_led_flash_state_Set(LED_State_Wifi_fw_error);
}
//pega_debug debug set burnin 0/1
void Pega_Misc_BurnIn_Enable_Control(uint8_t bIsEnable)
{
	 m_stMisc.bIsBurnInOn = (bIsEnable > 0) ? 1 : 0;
	 printf("(%s)bIsEnable=%d\n", __func__, m_stMisc.bIsBurnInOn); 
	 
}
//pega_debug debug set dis_als 0/1
void Pega_Misc_ALS_Disable_Control(uint8_t bIsDisable)
{
	 m_stMisc.bIsALSDisable = (bIsDisable > 0) ? 1 : 0;
	 printf("(%s)bIsDisable=%d\n", __func__, m_stMisc.bIsALSDisable); 	 
}
//pega_debug debug set dis_wifi 0/1
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
	 
	 if (pega_netip_interface_check_sd0(WIFI_IFNAME1,0) >= 0)
	 {
		 m_stMisc.eWIFI1_if = E_WIFI_IF_TYPE_MLAN0;
	 }
	 else if (pega_netip_interface_check_sd0(WIFI_IFNAME2,0) >= 0)
	 {
		 m_stMisc.eWIFI1_if = E_WIFI_IF_TYPE_MLAN1;
	 }
	 
	 if (pega_netip_interface_check_sd1(WIFI_IFNAME1,0) >= 0)
	 {
		 m_stMisc.eWIFI2_if = E_WIFI_IF_TYPE_MLAN0;
	 }
	 else if (pega_netip_interface_check_sd1(WIFI_IFNAME2,0) >= 0)
	 {
		 m_stMisc.eWIFI2_if = E_WIFI_IF_TYPE_MLAN1;
	 }
}
//==============================================================================
//diag wifi interface
void Pega_Misc_Diag_wifi_interface_Print(void)
{	  
     const char eWiFi_IF[3][6] = {"none",WIFI_IFNAME1,WIFI_IFNAME2};
	 
     Pega_Misc_Update_WiFi_interface();
	 
	 PT_DIAG_GET(0, "SDIO_0:%s", eWiFi_IF[m_stMisc.eWIFI1_if]);
	 PT_DIAG_GET(0, "SDIO_1:%s", eWiFi_IF[m_stMisc.eWIFI2_if]);	 
}
//pega_debug debug info misc
void Pega_Misc_Data_Print(void)
{	  
     const char strWiFi_IF[3][6] = {"none",WIFI_IFNAME1,"mlan1"};
	 const char strWiFi_State[4][12] = {"none","connect","connected","disconnect"};
	 
	 printf("-----------------------\n");
	 printf("sizeof(uint16_t)            = %d\n", sizeof(uint16_t));
	 printf("-----------------------\n");
	 printf("m_stMisc.bIsBurnInOn        = %d\n", m_stMisc.bIsBurnInOn);
	 printf("-----------------------\n");
	 printf("m_stMisc.bIsRFTestEnable    = %d\n", m_stMisc.bIsRFTestEnable);
	 printf("-----------------------\n");
	 printf("m_stMisc.bIsVoiceDemoEnable = %d\n", m_stMisc.bIsVoiceDemoEnable);
	 printf("m_stMisc.bIsVoiceDemoReady  = %d\n", m_stMisc.bIsVoiceDemoReady);
	 printf("-----------------------\n");
	 printf("m_stMisc.bIsALSDisable      = %d\n", m_stMisc.bIsALSDisable);	 
	 printf("m_stMisc.bIsAmpDisable      = %d\n", m_stMisc.bIsAmpDisable);
	 printf("m_stMisc.bIsWiFiDisable     = %d\n", m_stMisc.bIsWiFiDisable);
	 printf("m_stMisc.bIsNFCDisable      = %d\n", m_stMisc.bIsNFCDisable);
	 printf("m_stMisc.bIsRGBLedDisable   = %d\n", m_stMisc.bIsRGBLedDisable);
	 printf("m_stMisc.bIsRingLedDisable  = %d\n", m_stMisc.bIsRingLedDisable);
	 printf("m_stMisc.bIsMotorDisable    = %d\n", m_stMisc.bIsMotorDisable);
	 printf("-----------------------\n");	 
	 printf("m_stMisc.bIsWiFiFwReady     = %d\n", m_stMisc.bIsWiFiFwReady);
	 printf("m_stMisc.bIsWiFiFwError     = %d\n", m_stMisc.bIsWiFiFwError);
	 printf("-----------------------\n");
	 printf("m_stMisc.bIsDtboReady       = %d\n", m_stMisc.bIsDtboReady);
	 printf("-----------------------\n");
	 printf("m_stMisc.eWIFI_State(%d)     = %s\n", m_stMisc.eWIFI_State, strWiFi_State[m_stMisc.eWIFI_State]);
	 printf("-----------------------\n");
	 printf("m_stMisc.eIW610F_if(%d)     = %s\n", m_stMisc.eWIFI1_if, strWiFi_IF[m_stMisc.eWIFI1_if]);
	 printf("m_stMisc.eIW610G_if(%d)     = %s\n", m_stMisc.eWIFI2_if, strWiFi_IF[m_stMisc.eWIFI2_if]);
     printf("-----------------------\n");
}
//==============================================================================

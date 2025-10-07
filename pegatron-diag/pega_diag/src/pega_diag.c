#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>

#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
//==============================================================================
#include "pega_diag.h"
#include "pega_network_ip.h"
#include "pega_diag_msgq_cmd.h"
//==============================================================================
static stDiagMsgqCmdType m_stDiagMsg;
//==============================================================================
static char audio_file_path[128];
static char audio_record_time[64];
static char audio_play_time[64];
static int mic_record_volume = 0;
//==============================================================================
#define IVA_LICENSE_KEY_LENGTH 28 //iva key 
//==============================================================================
static void print_parameter(int argc, char **argv)
{
#if (ENABLE_PRINTF_PARAMETER_INFO == 1)
	int i;

	if (argc<=0)
	{
		return;
	}
	
	printf("\r\n"); 
	printf("===================================================================\r\n"); 
	printf(" argc =(%d)\r\n",argc); 
	for(i=0; i<argc; i++)
		{
		printf(" argv[%d] = %s \r\n",i,argv[i]); 
		}
	printf("===================================================================\r\n"); 
#else
	argc = argc;
	(void)argv;
#endif	
}

static int pegaDiag_StringBuffGet(char* pCmd, char *pBuff, int pSize, char bDebug)
{
	int bFlag = -1;
	FILE* fd = NULL;
	  
	fd = popen(pCmd,"r");
	
	if (fd != NULL)
	{      
	  while(fgets(pBuff, pSize-1, fd));
    
	  pclose(fd);
	  
	  if (bDebug > 0)
	  {
		  printf("%s",pBuff);
	  }
	  
	  bFlag = 0;
    }
	
	return bFlag;
}
//==============================================================================
//diag burnin on
static int pegaDiag_BurnInOn(int argc, char **argv)
{		
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	    
    m_stDiagMsg.eCmdId = Diag_CmdId_BurnIn;	
	m_stDiagMsg.value1 = 1;
    //--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
		
	return 0;//intResult;
}
//diag burnin off
static int pegaDiag_BurnInOff(int argc, char **argv)
{			
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	    	
    m_stDiagMsg.eCmdId = Diag_CmdId_BurnIn;	
	m_stDiagMsg.value1 = 0;
    //--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	
	return 0;
}
//diag burnin report
static int pegaDiag_BurnInReport(int argc, char **argv)
{			
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	    	
    m_stDiagMsg.eCmdId = Diag_CmdId_BurnIn;	
	m_stDiagMsg.value1 = 2;
    //--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	
	return 0;
}
//diag burnin reset
static int pegaDiag_BurnInReset(int argc, char **argv)
{		
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	    
    m_stDiagMsg.eCmdId = Diag_CmdId_BurnIn;	
	m_stDiagMsg.value1 = 3;
    //--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
		
	return 0;//intResult;
}


static int pegaDiag_BurnInRebootCount(int argc, char **argv)
{
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	    
    m_stDiagMsg.eCmdId = Diag_CmdId_BurnIn;	
	m_stDiagMsg.value1 = 4;
    //--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
		
	return 0;//intResult;
	
	
}
//diag ver sw
static int pegaDiag_IspFwVersionGet(int argc, char **argv)
{
	int intResult=0;	
	
	char cmd[100]={0};
    char buff[100]={0};
	
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	sprintf(cmd,"A=`pega_misc -v | grep 'version' | cut -d' ' -f3`;echo -n \"$A\"");
	
	intResult = pegaDiag_StringBuffGet(cmd, buff, sizeof(buff), 0);
	
	//printf("\n A=%s",buff);
	
	PT_DIAG_GET(0, "%s", buff);
	
	return intResult;
}

//diag ver wifi
static int pegaDiag_WifiVersionGet(int argc, char **argv)
{
	int intResult=0;	
	
	char cmd[120]={0};
    char buff[120]={0};
	
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
    if (pega_netip_is_interface_exist("mlan0", 0) < 0)
	{
		PT_DIAG_GET(0, "%s", "NULL");	
	    return intResult;
	}
	
	sprintf(cmd,"A=`cat /proc/mwlan/adapter0/mlan0/info | grep 'firmware_major_version' | cut -d'=' -f2`;echo -n \"$A\"");
	
	intResult = pegaDiag_StringBuffGet(cmd, buff, sizeof(buff), 0);
	
	//printf("\n A=%s",buff);
	
	PT_DIAG_GET(0, "%s", buff);
	
	return intResult;
}
//diag sn get
static int pegaDiag_SnGet(int argc, char **argv)
{
	int intResult=0;	
	
	char cmd[100]={0};
    char buff[160]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	sprintf(cmd,"A=`sku_printenv sn`;echo -n $A");
	
	intResult = pegaDiag_StringBuffGet(cmd, buff, sizeof(buff), 0);
	
	//printf("\n A=%s",buff);
	
	PT_DIAG_GET(0, "%s", buff);
	
	return intResult;
}
//diag sn set <sn> 
static int pegaDiag_SnSet(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[160]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
	    
	if (argv[0] == NULL)
	{
        printf("\n[error] SN is empty!"); 
	}
    else	
	{	
        intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "sku_setenv sn %s", argv[0]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//------------------------------------------------------------------------------------------
//diag icr day
static int pegaDiag_IcrDayModeSet(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
    m_stDiagMsg.eCmdId = Diag_CmdId_ICR_Control;	
	m_stDiagMsg.value1 = 0;//day	
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//------------------------------------------------------------------------------------------
//diag icr night
static int pegaDiag_IcrNightModeSet(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_ICR_Control;	
	m_stDiagMsg.value1 = 1;//Night	
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//------------------------------------------------------------------------------------------
//diag irled on
static int pegaDiag_IrLedOn(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_IRLed_Control;	
	m_stDiagMsg.value1 = 1; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag irled off
static int pegaDiag_IrLedOff(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_IRLed_Control;	
	m_stDiagMsg.value1 = 0; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag irled set 
static int pegaDiag_IrLedPwmValueSet(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_IRLed_PWM_Control;	
	m_stDiagMsg.value1 = atoi(argv[0]);
	
	if (m_stDiagMsg.value1 <= 0)
	{
		m_stDiagMsg.value1 = 1;
	}
	
	if (m_stDiagMsg.value1 > 100)
	{
		m_stDiagMsg.value1 = 100;
	}
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag spotled on
static int pegaDiag_SpotLedOn(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_SpotLightLed_Control;	
	m_stDiagMsg.value1 = 1; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag spotled off
static int pegaDiag_SpotLedOff(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_SpotLightLed_Control;	
	m_stDiagMsg.value1 = 0; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag spotled set 
static int pegaDiag_SpotLedPwmValueSet(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_SpotLightLed_PWM_Control;	
	m_stDiagMsg.value1 = atoi(argv[0]);
	
	if (m_stDiagMsg.value1 <= 0)
	{
		m_stDiagMsg.value1 = 1;
	}
	
	if (m_stDiagMsg.value1 > 100)
	{
		m_stDiagMsg.value1 = 100;
	}
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag led on
static int pegaDiag_LedAllOn(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_Led_Control;	
	m_stDiagMsg.value1 = 4; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag led off
static int pegaDiag_LedAllOff(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_Led_Control;	
	m_stDiagMsg.value1 = 5; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag led_r on
static int pegaDiag_LedROn(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_Led_Control;	
	m_stDiagMsg.value1 = 1; //Led_R
	m_stDiagMsg.value2 = 1;
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag led_r off
static int pegaDiag_LedROff(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_Led_Control;	
	m_stDiagMsg.value1 = 1; //Led_R
	m_stDiagMsg.value2 = 0;
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag led_g on
static int pegaDiag_LedGOn(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_Led_Control;	
	m_stDiagMsg.value1 = 2; //Led_G
	m_stDiagMsg.value2 = 1;
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag led_g off
static int pegaDiag_LedGOff(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_Led_Control;	
	m_stDiagMsg.value1 = 2; //Led_G
	m_stDiagMsg.value2 = 0;
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag led_b on
static int pegaDiag_LedBOn(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_Led_Control;	
	m_stDiagMsg.value1 = 3; //Led_B
	m_stDiagMsg.value2 = 1;
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag led_b off
static int pegaDiag_LedBOff(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_Led_Control;	
	m_stDiagMsg.value1 = 3; //Led_B
	m_stDiagMsg.value2 = 0;
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//==============================================================================
//diag als id
static int pegaDiag_AlsIdGet(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_ALS_Control;	
	m_stDiagMsg.value1 = 1; 	
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag als day
static int pegaDiag_AlsDayModeSet(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_ALS_Control;	
	m_stDiagMsg.value1 = 2; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag als night
static int pegaDiag_AlsNightModeSet(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_ALS_Control;	
	m_stDiagMsg.value1 = 3; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag als value
static int pegaDiag_AlsValueGet(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_ALS_Control;	
	m_stDiagMsg.value1 = 4; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag als calidata xx xx
static int pegaDiag_AlsCalidata(int argc, char **argv)
{
	int intResult=0;	
	float gain = 0.0, offset = -1000.0;
	char cmd[64];
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_ALS_Control;	
	m_stDiagMsg.value1 = 5; 
	
	if (argv[0] != NULL)
	{
		gain = atof(argv[0]);
		sprintf(cmd, "sh /etc/pega/diag/als_calidata_write.sh %f", gain); // will print messages in script file
	    system(cmd);

	}
	
	if (argv[1] != NULL)
	{
		offset = atof(argv[1]);
		sprintf(cmd, "sh /etc/pega/diag/als_calidata_write.sh %f %f", gain ,offset); // will print messages in script file
	    system(cmd);

	}
	
	m_stDiagMsg.fval1 = gain;
	m_stDiagMsg.fval2 = offset;
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}


//diag als on
static int pegaDiag_AlsOn(int argc, char **argv)
{
	int intResult=0;	
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_ALS_Control;	
	m_stDiagMsg.value1 = 6; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag als off
static int pegaDiag_AlsOff(int argc, char **argv)
{
	int intResult=0;	
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_ALS_Control;	
	m_stDiagMsg.value1 = 7; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}

static int pegaDiag_AlsCalidata_Get(int argc, char **argv)
{
	int intResult=0;	
	
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_ALS_Control;	
	m_stDiagMsg.value1 = 8; 
	
    
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag audio volume xx (xx:8.5 ~ 22.0)
static int pegaDiag_AudioVolumeSet(int argc, char **argv)
{
	int intResult=0;	
	float volume = 0.0;
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_Audio_Control;	
	m_stDiagMsg.value1 = 2;
	
	if (argv[0] != NULL)
	{
		volume = atof(argv[0]);
	}
	
    if (volume < 8.5 || volume > 22.0)
	{
        printf("\n[error] Volume value(%.3lf) is wrong!", volume);
		printf("\n[error] volume range : 8.5 ~ 22.0");
		PT_DIAG_SET(-1);
	}
	else 
	{
		m_stDiagMsg.fval1 = volume;
	
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	}
	
	return intResult;
}
//diag button get
static int pegaDiag_ButtonDetection_Get(int argc, char **argv)
{
	int intResult=0;	
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_Button_Control;
	m_stDiagMsg.value1 = 1; 
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag wifi on
static int pegaDiag_WifiOn(int argc, char **argv)
{	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
        		
	if (pega_netip_is_interface_exist("mlan0", 0) < 0)
	{
        system("sh /scripts/IW610_wifi_on.sh");
		sleep(3);
	}		
   
    PT_DIAG_SET(0);
	
	return 0;//intResult;
}
//diag wifi off
static int pegaDiag_WifiOff(int argc, char **argv)
{	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
        		
	if (pega_netip_is_interface_exist("mlan0", 0) > -1)
	{
        system("sh /scripts/wifi_driver_off.sh");
		sleep(3);
	}		
   
    PT_DIAG_SET(0);
	
	return 0;//intResult;
}
//diag wifi mfg
static int pegaDiag_WifiMfg(int argc, char **argv)
{	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
        		
	if (pega_netip_is_interface_exist("mlan0", 0) < 0)
	{
        system("sh /scripts/IW610_wifi_on.sh mfg");
		sleep(3);
	}		
   
    PT_DIAG_SET(0);
	
	return 0;//intResult;
}
//diag wifi connect <ssid> <password>
//diag wifi connect <ssid> <password> mlan0/mlan1
static int pegaDiag_WifiConnect(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[120]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);

    if (argv[0] == NULL)
	{
		printf("\n[error] SSID is empty!");  
        PT_DIAG_SET(-1);
		return 0;//intResult;
	}	

    if (argv[1] == NULL)
	{
		printf("\n[error] PASSWORD is empty!");  
        PT_DIAG_SET(-1);
		return 0;//intResult;
	}
	
	if (argv[2] != NULL)
	{
		if (!strncmp(argv[2], "mlan1", strlen("mlan1")))
		{
			if (pega_netip_is_interface_exist("mlan1", 0) > -1)
			{
				intResult = 0;
				snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_connect.sh %s %s %s", argv[0], argv[1], "mlan1");
				system(cBuffer); 
				PT_DIAG_SET(intResult);
				return 0;//intResult;
			}			
		}        
	}	

    if (pega_netip_is_interface_exist("mlan0", 0) > -1)
	{	
		intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_connect.sh %s %s", argv[0], argv[1]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}
				
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag wifi status
static int pegaDiag_WifiStatus(int argc, char **argv)
{
	int intResult=0;	
	int rtn = 0;
	char ip_addr[64] = {0};
		 
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
			 	 
	rtn = pega_netip_ip_get("mlan0", ip_addr, 64);
	
	if (rtn > -1)
	{
		PT_DIAG_GET(0, "%s", "Connected");
	}
	else
	{
		PT_DIAG_GET(0, "%s", "No connection");
	}
		
	return intResult;
}
//diag wifi rssi
static int pegaDiag_WifiRssiGet(int argc, char **argv)
{
	int intResult=0;	
	
	char cmd[100]={0};
    char buff[100]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	//sprintf(cmd,"A=`/sdk/bcmdhd/wl rssi`;echo -n $A");
	
	//intResult = pegaDiag_StringBuffGet(cmd, buff, sizeof(buff), 0);
	
	//printf("\n A=%s",buff);
	
	PT_DIAG_GET(0, "%s", buff);
	
	return intResult;
}
//diag wifi mac
static int pegaDiag_WifiMacGet(int argc, char **argv)
{
	int intResult=0;	
	
	char cmd[100]={0};
    char buff[100]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	sprintf(cmd,"A=`ifconfig | grep mlan0 | cut -d' ' -f10`;echo -n \"$A\"");
	
	intResult = pegaDiag_StringBuffGet(cmd, buff, sizeof(buff), 0);
	
	//printf("\n A=%s",buff);
	
	PT_DIAG_GET(0, "%s", buff);
	
	return intResult;
}
//diag wifi interface
static int pegaDiag_WifiInterfaceGet(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_wifi_Control;	
	m_stDiagMsg.value1 = 1; 	
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag wifi ssid get
static int pegaDiag_WifiSsidGet(int argc, char **argv)
{
	int intResult=0;	
	
	char cmd[100]={0};
    char buff[100]={0};
	char pStr[100]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	sprintf(cmd,"A=`cat /config/nxp/wpa.conf | grep ssid | cut -d'=' -f2`;echo -n $A");
	
	intResult = pegaDiag_StringBuffGet(cmd, buff, sizeof(buff), 0);
	
	//printf("\n A=%s",buff);
	memcpy(pStr, &buff[1], strlen(buff)-2);
	
	//PT_DIAG_GET(0, "%s", buff);
	PT_DIAG_GET(0, "%s", pStr);
	
	return intResult;
}
//diag wifi ssid set xx
static int pegaDiag_WifiSsidSet(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[100]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
	    
	if (argv[0] != NULL)
	{	
        intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_ssid.sh %s", argv[0]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag wifi password get
static int pegaDiag_WifiPassGet(int argc, char **argv)
{
	int intResult=0;	
	
	char cmd[100]={0};
    char buff[100]={0};
	char pStr[100]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	sprintf(cmd,"A=`cat /config/nxp/wpa.conf | grep psk | cut -d'=' -f2`;echo -n $A");
	
	intResult = pegaDiag_StringBuffGet(cmd, buff, sizeof(buff), 0);
	
	//printf("\n A=%s",buff);
	
	memcpy(pStr, &buff[1], strlen(buff)-2);
	
	//PT_DIAG_GET(0, "%s", buff);
	PT_DIAG_GET(0, "%s", pStr);
	
	return intResult;
}
//diag wifi password set xx
static int pegaDiag_WifiPassSet(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[100]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
	    
	if (argv[0] != NULL)
	{	
        intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_password.sh %s", argv[0]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag wifi ip_addr get
static int pegaDiag_WifiIpAddrGet(int argc, char **argv)
{
	int intResult=0;	
	int rtn = 0;	
    char ip_addr[64] = {0};
	 
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
			 	 
	rtn = pega_netip_ip_get("mlan0", ip_addr, 64);
	
	if (rtn > -1)
	{		
		PT_DIAG_GET(0, "%s", ip_addr);
	}
	else
	{
		PT_DIAG_GET(0, "%s", "NULL");
	}
		
	return intResult;
}
//diag wifi ip_addr set xx
static int pegaDiag_WifiIpAddrSet(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[100]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
	    
	if (argv[0] != NULL)
	{	
        intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "ifconfig mlan0 %s", argv[0]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag wifi ip_mask get
static int pegaDiag_WifiIpMaskGet(int argc, char **argv)
{
	int intResult=0;	
	int rtn = 0;
	struct ifaddrs *ifaddr, *ifa;
	char host[NI_MAXHOST];
	int s;
	struct sockaddr_in *sa;
    char *addr;
	 
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
			 	 
	if (getifaddrs(&ifaddr) == -1) 
	 {
		return -1;
	 }
	
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (NULL == ifa->ifa_addr || AF_INET != ifa->ifa_addr->sa_family)
		{
			continue;
		}

		if (0 != (s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST)))
		{
			continue;
		}
        				
		if (0 == strcmp(ifa->ifa_name, "mlan0")) //mlan0/eth0
		{	
            rtn = 1;		
			break;
		}	
	}
	
	if (rtn > 0)
	{
		sa = (struct sockaddr_in *) ifa->ifa_netmask;
        addr = inet_ntoa(sa->sin_addr);        
		//printf("Interface: %s, %s\tAddress: %s\n", ifa->ifa_name, ifa->ifa_netmask ,addr);
		//printf("Interface: %s\n", addr);
		PT_DIAG_GET(0, "%s", addr);
	}
	else
	{
		PT_DIAG_GET(0, "%s", "NULL");
	}
		
	return intResult;
}
//diag wifi ip_mask set xx
static int pegaDiag_WifiIpMaskSet(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[100]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
	    
	if (argv[0] != NULL)
	{	
        intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "ifconfig mlan0 netmask %s", argv[0]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//==============================================================================
int pegaDiag_is_ble_fw_ready(int bIsDebug) 
{ 
    int  rtn = 0;
    FILE *ptr; 
    char buff[512] = {0}; 
    char cmd[128]; 
    sprintf(cmd,"hciconfig | grep -c %s", "UART"); 
	
    if((ptr=popen(cmd, "r")) != NULL) 
    { 
        while (fgets(buff, 512, ptr) != NULL) 
        {   
            pclose(ptr); 
			rtn = atoi(buff);
			
			if (bIsDebug > 0)
			{
				printf("\n[%s](%d)\n", __func__, rtn);
			}
			
            return rtn;
        } 
    } 
	
    pclose(ptr); 
    return 0;
}
//diag ble on
static int pegaDiag_BleOn(int argc, char **argv)
{
	int intResult = 0;	
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	if (pega_netip_is_interface_exist("mlan0", 0) > -1)
	{
		if (pegaDiag_is_ble_fw_ready(0) == 0)
		{
			system("/scripts/IW610_ble_on.sh");
		}
	}	  
    else
	{
		printf("\n[error] IW610 fw is not ready!"); 
	}
						
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag ble off
static int pegaDiag_BleOff(int argc, char **argv)
{
	int intResult = 0;	
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);     
    
	//system("sh /sdk/bt_disable.sh");
						
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag ble scan
static int pegaDiag_BleScan(int argc, char **argv)
{
	int intResult = 0;	
	char cBuffer[120] = {0};
	printf("%s\r\n",__FUNCTION__); 	
	//printf("\n%s(%d)\n",__FUNCTION__, argc); 
	//print_parameter(--argc, ++argv);
	
	if (pegaDiag_is_ble_fw_ready(0) == 0)
	{
		system("/scripts/IW610_ble_on.sh");
		sleep(4);
	}
	
	system("hciconfig hci0 up");
	
	if (argc == 2)
	{
		snprintf(cBuffer, sizeof(cBuffer), "hcitool -i hci0 name %s",  argv[1]);
		system(cBuffer);
	}
	else
	{
		//system("hcitool -i hci0 scan");
	}
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);     
    	
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}

//diag ethernet on eth1 xx.xx.xx.xx
static int pegaDiag_EthernetOn(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[100]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
	
	if (argv[0] == NULL)
	{
		printf("\n%s\n","No interface"); 
		PT_DIAG_SET(-1);
		return intResult;
	}
	
	if (argv[1] == NULL)
	{
		printf("\n%s\n","No ip"); 
		PT_DIAG_SET(-1);
		return intResult;
	}
	
	intResult = 0;
	snprintf(cBuffer, sizeof(cBuffer), "/scripts/Ethernet_on.sh %s %s", argv[0], argv[1]);
	system(cBuffer); 
	PT_DIAG_SET(intResult);
	
	return intResult;
}
//diag ota tftp xx.xx.xx.xx
static int pegaDiag_OtaTftp(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[100]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
	    
	if (argv[0] != NULL)
	{	
        intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "/sdk/fota_go.sh %s", argv[0]);
		//intResult = system(cBuffer); 
		//system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	return intResult;
}

static int pegaDiag_FactoryCaliRaw(int argc, char **argv)
{
	int intResult=-1;
	char cmd[64];
    int output_telnet;
	int cali_item;
	int _gain;
	int _exp;
	T_MiCommonMsg snapshot;
	 
	if (argc == 4 || argv[4] == NULL)
	{
	    sprintf(cmd, "sh /etc/pega/diag/tty_path.sh cali %s %s %s", argv[1],argv[2],argv[3]); 
		system(cmd);
		intResult = 0;
		goto end;
	}
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	snapshot.iType = MSG_TYPE_mi_isp;
	snapshot.command = MSG_COMMAND_mi_isp_ctrl;
	snapshot.ctrl_val = isp_factory_cali;
	cali_item = atoi(argv[0]);
	_gain = atoi(argv[1]);
	_exp = atoi(argv[2]);
    output_telnet = atoi(argv[3]);
	if ( _gain >= 1024 && _gain <= 131072 && _exp >= 1 && _exp <= 1000000 )
	{
		printf("\n pegaDiag_FactoryCaliRaw \n");
		snapshot.snapshot_info.cmdid = 0;
		snapshot.snapshot_info.cali_item=cali_item;
        snapshot.snapshot_info.gain=_gain;
        snapshot.snapshot_info.shutter=_exp;
		snapshot.snapshot_info.output_telnet= output_telnet;
        intResult=Pega_diag_msgq_send_mediaserver(snapshot);
	    PT_DIAG_SET(intResult);
	}
	else
	{
		if (!(_gain >= 1024 && _gain <= 131072))
			printf("gain input out of range, should be 1024 ~ 131072\n");

		if (!(_exp >= 1 && _exp <= 1000000))
			printf("exp input out of range, should be 1 ~ 1000000\n");

		
		
		intResult = -1;
		PT_DIAG_SET(intResult);
	}
end:
    printf("\n Get pts\n");

	return intResult;
}


static int pegaDiag_FactoryCaliIsp(int argc, char **argv)
{
	int intResult=-1;
	char cmd[64];
    int output_telnet;
	int  cali_item;
	int _gain;
	int _exp;
	T_MiCommonMsg snapshot;
	
  
	if (argc == 4 || argv[4] == NULL)
	{
		
	    sprintf(cmd, "sh /etc/pega/diag/tty_path.sh isp %s %s %s", argv[1],argv[2],argv[3]); 
		system(cmd);
		intResult = 0;
		goto end;
	}
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	snapshot.iType = MSG_TYPE_mi_isp;
	snapshot.command = MSG_COMMAND_mi_isp_ctrl;
	snapshot.ctrl_val = isp_factory_cali;	
	
	cali_item = atoi(argv[0]);
	_gain = atoi(argv[1]);
	_exp = atoi(argv[2]);
    output_telnet = atoi(argv[3]);
	if ( _gain >= 1024 && _gain <= 131072 && _exp >= 1 && _exp <= 1000000 )
	{
		snapshot.snapshot_info.cmdid = 1;
		snapshot.snapshot_info.cali_item=cali_item;
        snapshot.snapshot_info.gain=_gain;
        snapshot.snapshot_info.shutter=_exp;
		snapshot.snapshot_info.output_telnet= output_telnet;
        intResult=Pega_diag_msgq_send_mediaserver(snapshot);
	    PT_DIAG_SET(intResult);
	}
	else
	{
		if (!(_gain >= 1024 && _gain <= 131072))
			printf("gain input out of range, should be 1024 ~ 131072\n");

		if (!(_exp >= 1 && _exp <= 1000000))
			printf("exp input out of range, should be 1 ~ 1000000\n");

		
		
		intResult = -1;
		PT_DIAG_SET(intResult);
	}
end:
    printf("\n Get pts\n");
	return intResult;
}

static int pegaDiag_FactoryCaliManufactureRead(int argc, char **argv)
{
	int intResult=0;
	uint8_t cali_item;
	char cmd[64];
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	cali_item = atoi(argv[0]);
	if (argc == 1 && cali_item <= 2)
	{
		sprintf(cmd, "sh /etc/pega/diag/cali_read.sh %d", cali_item); // will print messages in script file
		system(cmd);
	}
	else
	{
		intResult = -1;
		PT_DIAG_SET(intResult);
	}
	//--------------------------------------------------------------------------------------
	return intResult;
}

//------------------------------------------------------------------------------------------
static int pegaDiag_FactoryCaliManufactureWrite(int argc, char **argv)
{
	int intResult=0;
	uint8_t cali_item;
	char cmd[64];
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	cali_item = atoi(argv[0]);
	if (argc == 1 && cali_item <= 2)
	{
		sprintf(cmd, "sh /etc/pega/diag/cali_write.sh %d", cali_item); // will print messages in script file
		system(cmd);
	}
	else
	{
		intResult = -1;
		PT_DIAG_SET(intResult);
	}
	//--------------------------------------------------------------------------------------
	return intResult;
}

int	pegaDiag_AudioFileExist(const char* FileName, char *pFn)
{
	int rtn = 1;
	FILE *fp = NULL;
	 
	
//	sprintf(FN, "/etc/pega/diag/%s", FileName);
	sprintf(audio_file_path, "%s", FileName);
	fp = fopen(audio_file_path, "r");

	if(NULL == fp)
	{
		printf("\nfile %s isn't exist!\n", FileName);
		rtn = -1;
	}
	else
	{
		strcpy(pFn, audio_file_path);
	}

	if (NULL != fp)
		fclose(fp);

	return rtn;
}

//diag audio init
static int pegaDiag_AudioInit(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	m_stDiagMsg.eCmdId = Diag_CmdId_Audio_Control;	
	m_stDiagMsg.value1 = 1; 	
	//--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag audio record
static int pegaDiag_AudioRecordFile(int argc, char **argv)
{
	int intResult=0;
	char audioCmd[256];
	char pFn[128];
	char *staticFilePath = "/config/audio/";
	char *staticFileName = "16k_16bit_stereo_ai.json"; /* sound mode 1:MONO, 2:STEREO */
	char staticFile[128];
	printf("%s\r\n",__FUNCTION__);
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	sprintf(audio_record_time ,"10");
	if (argc > 0)
	{
		sprintf(audio_record_time,"%s", argv[0]);
		if (atoi(audio_record_time) <= 0)
			
			sprintf(audio_record_time ,"10");
	}

	sprintf(staticFile, "%s%s", staticFilePath, staticFileName);
	printf("RECORDING FILE >>> %s \n", staticFile);
	if (1 == pegaDiag_AudioFileExist(staticFile, pFn))
	{
		sprintf(audioCmd, "prog_audio_audio -f %s -t %s", staticFile, audio_record_time);
		
		intResult = system(audioCmd);
		printf("\n %s \n",audioCmd);
    }
	else
	{
		intResult = -1;	
	}
	PT_DIAG_SET(intResult);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag audio play
static int pegaDiag_AudioPlayFile(int argc, char **argv)
{
	int intResult=0;
	char audioCmd[256];
	char pFn[128];
	char *staticFilePath = "/config/audio/";
	char *staticFileName = "16k_16bit_mono_ao.json"; /* channel mode 0:STEREO, 1:MONO */
	char staticFile[128];
	char dynamicFile[128];
	char dynamicPlayDuration[16];
	printf("%s\r\n",__FUNCTION__);
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	sprintf(audio_play_time ,"10");  // setup default play time is 10 seconds.
	sprintf(staticFile, "%s%s", staticFilePath, staticFileName);
/*
	if (argc > 0)
	{
		sprintf(audio_play_time,"%s", argv[0]);
		if (atoi(audio_play_time) <= 0)
			sprintf(audio_play_time ,"10");
	}
*/
	switch(argc)
	{
		case 1:
		    sprintf(dynamicFile, "%s", argv[0]);

			if (1 == pegaDiag_AudioFileExist(dynamicFile, pFn))
			{
				sprintf(audioCmd, "prog_audio_audio -f %s", dynamicFile);
				printf("PLAYING FILE >>> %s \n", dynamicFile);

				intResult = system(audioCmd);
				printf("\n %s \n",audioCmd);
			}
			else
			{
				intResult = -1;
			}
			break;
		case 2:
		    sprintf(dynamicFile, "%s", argv[0]);
			sprintf(dynamicPlayDuration, "%s", argv[1]);

			if (atoi(dynamicPlayDuration) <= 0)
			{
				*dynamicPlayDuration = *audio_play_time;
			}

			if (1 == pegaDiag_AudioFileExist(dynamicFile, pFn))
			{
				sprintf(audioCmd, "prog_audio_audio -f %s -t %s", dynamicFile, dynamicPlayDuration);
				printf("PLAYING FILE >>> %s \n", dynamicFile);

				intResult = system(audioCmd);
				printf("\n %s \n",audioCmd);
			}
			else
			{
				intResult = -1;
			}
			break;
		case 0:
		default:
		    sprintf(audioCmd, "prog_audio_audio -f %s", staticFile);

			if (1 == pegaDiag_AudioFileExist(staticFile, pFn))
			{
				printf("PLAYING FILE >>> %s \n", staticFile);
				intResult = system(audioCmd);
				printf("\n %s \n",audioCmd);
			}
			else
			{
				intResult = -1;
			}
			break;
	}

	PT_DIAG_SET(intResult);
	//--------------------------------------------------------------------------------------
	return intResult;
}

//------------------------------------------------------------------------------------------
static int pegaDiag_AudioStop(int argc, char **argv)
{
	int intResult=0;
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	intResult = system("sh /etc/pega/diag/audio_stop.sh");
	/*intResult = system("pid=$(pidof prog_audio_all_test_case)");
	usleep(2000*1000);
	intResult = system("kill -9 $pid");*/
	PT_DIAG_SET(intResult);
	//--------------------------------------------------------------------------------------
	return intResult;
}

//------------------------------------------------------------------------------------------
static int pegaDiag_AudioReStart(int argc, char **argv)
{
	int intResult= 0;
	char audioCmd[256];
	char pFn[128];
	FILE *fp = NULL;
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	
	

    intResult = system("sh /etc/pega/diag/audio_stop.sh");
	/*intResult = system("pid=$(pidof prog_audio_all_test_case)");
	usleep(2000*1000);
	intResult = system("kill -9 $pid");*/
	if((fp = fopen("/tmp/audio_file_path", "r")) == 0)
    {
         printf("audio_file_path open failed!");
         intResult = -1;
    }	
	
	fread(audio_file_path, sizeof(char), 50, fp);
	if (strlen(audio_file_path) == 0)
	{
		intResult = -1;
		goto end_print;
	}
	if((fp = fopen("/tmp/audio_play_time", "r")) == 0)
    {
         printf("audio_file_path open failed!");
         intResult = -1;
    }	
	fread(audio_play_time, sizeof(char), 1, fp);
	printf("\n audio_play_time=%s\n" ,audio_play_time);
	if (strlen(audio_play_time) == 0)
	{
		intResult = -1;
		goto end_print;
	}
	if (1 == pegaDiag_AudioFileExist(audio_file_path, pFn))
	{
		sprintf(audioCmd, "prog_audio_all_test_case -O -D 1 -V 6 -t %s -i %s &", audio_play_time, audio_file_path);
		intResult = system(audioCmd);
	}

end_print:
	PT_DIAG_SET(intResult);
	//--------------------------------------------------------------------------------------
	return intResult;
}

static int pegaDiag_MicRecordStart(int argc, char **argv)
{
	int intResult=0;
    char audioCmd[256];
	printf("%s\r\n",__FUNCTION__);
	print_parameter(--argc, ++argv);
	int intRecordSec = 10;
	//--------------------------------------------------------------------------------------
	
	if (argc == 0 || argv[0] == NULL)
	{
		intRecordSec = 10; // put default time 10 sec.
	}
	else
	{
		uint16_t _sec = atoi(argv[0]);
		
		if (_sec > 60)
		{
			_sec = 60;
		}

		intRecordSec = _sec;
	}
	
	sprintf(audioCmd, "prog_audio_audio -t %d -f /config/audio/16k_16bit_mono_ai.json &", intRecordSec);
	intResult = system(audioCmd);

	//--------------------------------------------------------------------------------------
	return intResult;
}

static int pegaDiag_MicRecordStop(int argc, char **argv)
{
	int intResult=0;
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	intResult = system("sh /etc/pega/diag/audio_stop.sh");
	/*intResult = system("pid=$(pidof prog_audio_all_test_case)");
	usleep(2000*1000);
	intResult = system("kill -9 $pid");*/
	
	PT_DIAG_SET(intResult);
	//--------------------------------------------------------------------------------------
	return intResult;
}

//------------------------------------------------------------------------------------------
static int pegaDiag_MicVolumeGet(int argc, char **argv)
{
	int intResult=0;
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
    if ( mic_record_volume == 0)
		intResult = -1 ;
	printf("\n MicVolume =%d\n",mic_record_volume);
		
	//--------------------------------------------------------------------------------------
	PT_DIAG_SET(intResult);
	return intResult;
}

//------------------------------------------------------------------------------------------
static int pegaDiag_MicVolumeSet(int argc, char **argv)
{
	int intResult=0;
	int vol;
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	vol = atoi(argv[0]);
	
	 
	
	if (vol < -48 || vol > 42) //-48~42dB
	{
		intResult =  -1;
		goto fail_print;
	}
     mic_record_volume = vol;
	printf("%s\n (vol=%d)",__FUNCTION__, vol);
	//sprintf(audioCmd, "prog_audio_all_test_case -I -v %d -w 16000 -d 1 -c 1 -t 10 -s 16000 -o /tmp &", mic_record_volume);
	//intResult = system(audioCmd);
	//--------------------------------------------------------------------------------------

fail_print:
	PT_DIAG_SET(intResult);
	
	return intResult;
}

static int pegaDiag_PrintConsole(int argc, char **argv)
{
	int intResult = 0;
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	m_stDiagMsg.eCmdId = Diag_CmdId_Out2TelnetPegaMisc;	
	m_stDiagMsg.value1 = 0xFF;

    //--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------	
	PT_DIAG_SET(intResult); 	
	//--------------------------------------------------------------------------------------
	return intResult;
}
//------------------------------------------------------------------------------------------
static int pegaDiag_PrintTelnet(int argc, char **argv)
{
	int intResult = 0;
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	if (argc == 0 || argv[0] == NULL)
	{
	  intResult = system("sh /etc/pega/diag/tty_path.sh");
	  return intResult;
	}
    else  
    {
	  m_stDiagMsg.eCmdId = Diag_CmdId_Out2TelnetPegaMisc;	
	  m_stDiagMsg.value1 = atoi(argv[0]);
	}
    //--------------------------------------------------------------------------------------
	Pega_diag_msgq_send_request(m_stDiagMsg);
	//--------------------------------------------------------------------------------------

    
	
	PT_DIAG_SET(intResult); 	
	//--------------------------------------------------------------------------------------
	return intResult;
}

//------------------------------------------------------------------------------------------	
static int pegaDiag_Factory2wayOn(int argc, char **argv)
{
	int intResult = 0;
	int int2WaySec , intRecordSec;
	char pFn[128];
	char audioCmd[256];
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
// record is 5 seconds longer than play
//record file
	int2WaySec = 10;
	intRecordSec = 15;
	if (argc >1)
		{
		uint8_t _sec = atoi(argv[1]);
		if (_sec>0)
			{
			if (_sec > 60)
				_sec = 60;

			int2WaySec = _sec;
			intRecordSec =  int2WaySec +5;
			}
		}

    //--------------------------------------------------------------------------------------
	if (1 == pegaDiag_AudioFileExist(argv[0], pFn))
	{
		
		sprintf(audioCmd, "prog_audio_all_test_case -I -v 16 -w 16000 -d 1 -c 1 -t %d -s 16000 -o /tmp -O -D 1 -V 6 -t %d -i %s &", intRecordSec,int2WaySec, argv[0]);
		intResult = system(audioCmd);
	}
	else
	{
		intResult = -1;	
	}
	PT_DIAG_SET(intResult);
	//--------------------------------------------------------------------------------------

	return intResult;
}
//------------------------------------------------------------------------------------------	
static int pegaDiag_Factory2wayOff(int argc, char **argv)
{
	int intResult=0;
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	intResult = system("sh /etc/pega/diag/audio_stop.sh");
	/*intResult = system("pid=$(pidof prog_audio_all_test_case)");
	usleep(2000*1000);
	intResult = system("kill -9 $pid");*/
	
	PT_DIAG_SET(intResult);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//------------------------------------------------------------------------------------------
static int pegaDiag_FactoryUBootVersionGet(int argc, char **argv)
{
	int intResult=0;
	char UBootVersion[64];
	char start_service_mode[32];
	FILE *fp = NULL;
	print_parameter(--argc, ++argv);
	if((fp = fopen("/config/start_service_mode", "r")) == 0)
    {
         printf("/config/start_service_mode open failed!");
         intResult = -1;
    }	
	
	fread(start_service_mode, sizeof(char), 50, fp);
	sprintf(UBootVersion, "uboot=1.004-%s", start_service_mode);
	PT_DIAG_GET(intResult, "%s", UBootVersion);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag factory reset
static int pegaDiag_FactoryRset(int argc, char **argv)
{
	int intResult=0;
	//--------------------------------------------------------------------------------------
	system("rm -f /config/bcmdhd/*");
	usleep(1000*500); 
	system("cp -f /sdk/bcmdhd/* /config/bcmdhd/");
	usleep(1000*500); 
	system("quectel-at rftest off");
	usleep(1000*500); 
	system("sync");
	usleep(1000*500); 
	PT_DIAG_SET(intResult);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//------------------------------------------------------------------------------------------
static int pegaDiag_FactoryMediaserverOn(int argc, char **argv)
{
	int intResult= 0;
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
    intResult = system("sh /etc/pega/diag/mediaserver_run.sh");
	PT_DIAG_SET(intResult);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//------------------------------------------------------------------------------------------
static int pegaDiag_FactoryMediaserverOff(int argc, char **argv)
{
	int intResult= 0;
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	intResult = system("sh /etc/pega/diag/mediaserver_stop.sh");
	PT_DIAG_SET(intResult);
	//--------------------------------------------------------------------------------------
	return intResult;
}
//diag Factory sn get
static int pegaDiag_FactorySnGet(int argc, char **argv)
{
	int intResult=0;	
	
	char cmd[100]={0};
    char buff[160]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	sprintf(cmd,"A=`sku_printenv factory_sn`;echo -n $A");
	
	intResult = pegaDiag_StringBuffGet(cmd, buff, sizeof(buff), 0);
	
	//printf("\n A=%s",buff);
	
	PT_DIAG_GET(0, "%s", buff);
	
	return intResult;
}
//diag Factory sn set <sn> 
static int pegaDiag_FactorySnSet(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[160]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
	    
	if (argv[0] == NULL)
	{
        printf("\n[error] Factory SN is empty!"); 
	}
    else	
	{	
        intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "sku_setenv factory_sn %s", argv[0]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//------------------------------------------------------------------------------------------
//diag NFC_APP
static int pegaDiag_NFCControl(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[160]={};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(argc, argv);
	    
	if (argv[0] == NULL)
	{
        printf("\n[error]"); 
	}
    else	
	{	
        intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "/scripts/NFC.sh %s", argv[0]);
		system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	
	return intResult;
}

//------------------------------------------------------------------------------------------
//diag Ring LED kernel module
static int pegaDiag_RingLedControl(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[160]={};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(argc, argv);
	    
	if (argv[0] == NULL)
	{
        printf("\n[error]"); 
	}
    else	
	{	
        intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "/scripts/ring_led.sh %s", argv[0]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	
	return intResult;
}
//------------------------------------------------------------------------------------------
//diag Acc kernel module
static int pegaDiag_AccControl(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[160]={};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(argc, argv);
	    
	if (argv[0] == NULL)
	{
        printf("\n[error]"); 
	}
    else	
	{	
        intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "/scripts/acc.sh %s", argv[0]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	
	return intResult;
}

//------------------------------------------------------------------------------------------
//diag motor rising test
static int pegaDiag_rising_motor_test(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[64] = {0};
	 
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	sprintf(cmd,"pega_misc_dbg motor 1 %d", 1);  	
	system(cmd);
		
    PT_DIAG_SET(intResult);
	
	return intResult;
}
//diag motor rising forward xx xx
static int pegaDiag_rising_motor_forward(int argc, char **argv)
{
	int intResult = 0;	
	int speed = 3, time=1000;
	
	char cmd[64] = {0};
	 
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	if (argv[0] != NULL)
	{
        speed = atoi(argv[0]); 
	}
	
	if (argv[1] != NULL)
	{
        time = atoi(argv[1]); 
	}
	
	sprintf(cmd,"pega_misc_dbg motor 14 %d %d", speed, time);  	
	system(cmd);
		
    PT_DIAG_SET(intResult);
	
	return intResult;
}
//diag motor rising forward xx xx
static int pegaDiag_rising_motor_reverse(int argc, char **argv)
{
	int intResult = 0;	
	int speed = 3, time=1000;
	
	char cmd[64] = {0};
	 
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	if (argv[0] != NULL)
	{
        speed = atoi(argv[0]); 
	}
	
	if (argv[1] != NULL)
	{
        time = atoi(argv[1]); 
	}
	
	sprintf(cmd,"pega_misc_dbg motor 13 %d %d", speed, time);  	
	system(cmd);
		
    PT_DIAG_SET(intResult);
	
	return intResult;
}

//diag pan pan forward xx xx
static int pegaDiag_pan_motor_forward(int argc, char **argv)
{
	int intResult = 0;	
	int speed = 3, time=1000;
	
	char cmd[64] = {0};
	 
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	if (argv[0] != NULL)
	{
        speed = atoi(argv[0]); 
	}
	
	if (argv[1] != NULL)
	{
        time = atoi(argv[1]); 
	}
	
	sprintf(cmd,"pega_misc_dbg motor 12 %d %d", speed, time);  	
	system(cmd);
		
    PT_DIAG_SET(intResult);
	
	return intResult;
}
//diag motor pan reverse xx xx
static int pegaDiag_pan_motor_reverse(int argc, char **argv)
{
	int intResult = 0;	
	int speed = 3, time=1000;
	
	char cmd[64] = {0};
	 
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	if (argv[0] != NULL)
	{
        speed = atoi(argv[0]); 
	}
	
	if (argv[1] != NULL)
	{
        time = atoi(argv[1]); 
	}
	
	sprintf(cmd,"pega_misc_dbg motor 11 %d %d", speed, time);  	
	system(cmd);
		
    PT_DIAG_SET(intResult);
	
	return intResult;
}
//diag motor pan test
static int pegaDiag_pan_motor_test(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[64] = {0};
	 
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	sprintf(cmd,"pega_misc_dbg motor 1 %d", 0);  	
	system(cmd);
		
    PT_DIAG_SET(intResult);
	
	return intResult;
}

//diag streaming on
static int pegaDiag_streaming_on(int argc, char **argv)
{
	int intResult = 0;	
	
	char ip_addr[64] = {0};
	 
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	if (pega_netip_ip_get("mlan0", ip_addr, 64) > -1)
	{	        
        system("/scripts/streaming_on.sh"); 
	}
	else if (pega_netip_ip_get("eth0", ip_addr, 64) > -1)
	{	        
        system("/scripts/streaming_on.sh"); 
	}
	else if (pega_netip_ip_get("eth1", ip_addr, 64) > -1)
	{	        
        system("/scripts/streaming_on.sh"); 
	}
	else
	{
        printf("\n[Error]Interface is not ready."); 
	}
	
    PT_DIAG_SET(intResult);
	
	return intResult;
}
//diag mfgbridge test
static int pegaDiag_mfgbridge_test(int argc, char **argv)
{
	int intResult = 0;	
	
	char ip_addr[64] = {0};
	 
	// printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);

	system("/scripts/libgpiod_enable.sh"); 	
	system("/scripts/IW610_mfgbridge_test.sh"); 
	
    PT_DIAG_SET(intResult);
	
	return intResult;
}
//==============================================================================
static command_line_t m_eBurninCommand[] = 
{
	{ "on",		"burnin enable",		pegaDiag_BurnInOn,		NULL},
	{ "off",	"burnin disable",		pegaDiag_BurnInOff,		NULL},
	{ "report",	"burnin report",		pegaDiag_BurnInReport,	NULL},
	{ "reset",	"burnin reset",		    pegaDiag_BurnInReset,	NULL},
	{ "count",	"burnin reboot count",  pegaDiag_BurnInRebootCount, NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eVersionCommand[] = 
{
	{ "sw",		"sw version",		pegaDiag_IspFwVersionGet,		NULL},
	{ "wifi",	"wifi version",		pegaDiag_WifiVersionGet,		NULL},	
	{ NULL, NULL, NULL, NULL }
};
//==============================================================================
static command_line_t m_eSnCommand[] = 
{
	{ "get",	"sn get",		pegaDiag_SnGet,		NULL},
	{ "set",	"sn set",		pegaDiag_SnSet,		NULL},
	{ NULL, NULL, NULL, NULL }
};
//==============================================================================
static command_line_t m_eIcrCommand[] = 
{
	{ "day",	"icr day mode set",				pegaDiag_IcrDayModeSet,			NULL},
	{ "night",	"icr night mode set",			pegaDiag_IcrNightModeSet,		NULL},
	//{ "start",	"icr relibility test start",	pegaDiag_IcrRelibilityStart,	NULL},
	//{ "stop",	"icr relibility test stop",		pegaDiag_IcrRelibilityStop,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eIrLedCommand[] = {
	{ "on",		"ir on",		pegaDiag_IrLedOn,			NULL},
	{ "off",	"ir off",		pegaDiag_IrLedOff,			NULL},
	{ "set",	"ir pwm set",	pegaDiag_IrLedPwmValueSet,	NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eSpotLedCommand[] = {
	{ "on",		"spot on",		pegaDiag_SpotLedOn,			NULL},
	{ "off",	"spot off",		pegaDiag_SpotLedOff,		NULL},
	{ "set",	"spot pwm set",	pegaDiag_SpotLedPwmValueSet,NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eLedCommand[] = {
	{ "on",		"Led_All on",		pegaDiag_LedAllOn,			NULL},
	{ "off",	"Led_All off",		pegaDiag_LedAllOff,			NULL},	
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eLedRCommand[] = {
	{ "on",		"Led_R on",			pegaDiag_LedROn,			NULL},
	{ "off",	"Led_R off",		pegaDiag_LedROff,			NULL},		
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eLedGCommand[] = {
	{ "on",		"Led_G on",			pegaDiag_LedGOn,			NULL},
	{ "off",	"Led_G off",		pegaDiag_LedGOff,			NULL},		
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eLedBCommand[] = 
{
	{ "on",		"Led_B on",			pegaDiag_LedBOn,			NULL},
	{ "off",	"Led_B off",		pegaDiag_LedBOff,			NULL},		
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eAlsCommand[] = 
{
	{ "id",			"als id",			pegaDiag_AlsIdGet,				NULL},
	{ "day",		"als day mode",		pegaDiag_AlsDayModeSet,			NULL},
	{ "night",		"als night mode",	pegaDiag_AlsNightModeSet,		NULL},
	{ "value",		"als value",		pegaDiag_AlsValueGet,			NULL},
	{ "calidata",	"als calidata",		pegaDiag_AlsCalidata,   		NULL},
	{ "on",			"als enable",		pegaDiag_AlsOn,   				NULL},
	{ "off",		"als disable",		pegaDiag_AlsOff,   				NULL},
	{ "gain",	    "als gain get",     pegaDiag_AlsCalidata_Get,        NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eButtonCommand[] = 
{		
	{ "get",	"button status get",			pegaDiag_ButtonDetection_Get,	NULL},	
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eWifiSsidCommand[] = {
	{ "get",	"wifi ssid get",			pegaDiag_WifiSsidGet,	NULL},
	{ "set",	"wifi ssid set",			pegaDiag_WifiSsidSet,	NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eWifiPasswordCommand[] = {
	{ "get",	"wifi password get",			pegaDiag_WifiPassGet,	NULL},
	{ "set",	"wifi password set",			pegaDiag_WifiPassSet,	NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eWifiIPAddrCommand[] = {
	{ "get",	"wifi ip_addr get",			pegaDiag_WifiIpAddrGet,	NULL},
	{ "set",	"wifi ip_addr set",			pegaDiag_WifiIpAddrSet,	NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eWifiIPMaskCommand[] = {
	{ "get",	"wifi ip_mask get",			pegaDiag_WifiIpMaskGet,	NULL},
	{ "set",	"wifi ip_mask set",			pegaDiag_WifiIpMaskSet,	NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eWifiCommand[] = 
{
	{ "on",			"wifi on",						pegaDiag_WifiOn,			NULL},
	{ "off",		"wifi off",						pegaDiag_WifiOff,			NULL},
	{ "mfg",		"wifi mfg fw on",				pegaDiag_WifiMfg,			NULL},
	{ "connect",	"wifi connect ssid password",	pegaDiag_WifiConnect,		NULL},		
	{ "status",		"wifi connection status",		pegaDiag_WifiStatus,		NULL},		
	{ "rssi",		"wifi rssi",					pegaDiag_WifiRssiGet,		NULL},		
	{ "mac",		"wifi mac address",				pegaDiag_WifiMacGet,		NULL},
	{ "interface",	"wifi interface",				pegaDiag_WifiInterfaceGet,	NULL},
	{ "ssid",		"wifi ssid command",			NULL,						m_eWifiSsidCommand},	
	{ "password",	"wifi password",				NULL,						m_eWifiPasswordCommand},	
	{ "ip_addr",	"wifi ip address",				NULL,						m_eWifiIPAddrCommand},	
	{ "ip_mask",	"wifi ip mask",					NULL,						m_eWifiIPMaskCommand},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eBleCommand[] = 
{
	{ "on",			"ble on",						pegaDiag_BleOn,			NULL},
	{ "off",		"ble off",						pegaDiag_BleOff,		NULL},		
	{ "scan",		"ble scan",						pegaDiag_BleScan,		NULL},	
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eEthernetCommand[] = 
{	
	{ "command:",	"[interface:eth0/eth1/ethx] [ip:192.168.xx.xx]",	NULL,			NULL},	
	{ "example:",	"diag ethernet on eth0 192.168.0.5",				NULL,			NULL},	
	{ "on",		"ethernet enable",					pegaDiag_EthernetOn,			NULL},		
	{ NULL, NULL, NULL , NULL }
};

static command_line_t m_eOtaCommand[] = 
{	
	{ "tftp",		"ota via tftp",		pegaDiag_OtaTftp,		NULL},		
	{ NULL, NULL, NULL , NULL }
};

static command_line_t m_eFactoryCaliManufactureCommand[] = {
	{ "read",	"cali manufacture read",		pegaDiag_FactoryCaliManufactureRead,		NULL},
	{ "write",	"cali manufacture write",		pegaDiag_FactoryCaliManufactureWrite,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eFactoryCaliCommand[] = {
	{ "raw",	"Factory image cali raw",		pegaDiag_FactoryCaliRaw,	NULL},
	{ "isp",	"Factory image cali isp",		pegaDiag_FactoryCaliIsp,	NULL},
	{ "manufacture","Factory cali manufacture command",		NULL,			m_eFactoryCaliManufactureCommand},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eFactory2wayCommand[] = {
	{ "on",		"Factory 2way on",		pegaDiag_Factory2wayOn,		NULL},
	{ "off",	"Factory 2way off",		pegaDiag_Factory2wayOff,	NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eFactorymediaserverCommand[] = {
	{ "on",		"Factory mediaserver on",		pegaDiag_FactoryMediaserverOn,		NULL},
	{ "off",	"Factory mediaserver off",		pegaDiag_FactoryMediaserverOff,	    NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eFactorySnCommand[] = {
	{ "get",	"Factory sn get",		pegaDiag_FactorySnGet,		NULL},
	{ "set",	"Factory sn set",		pegaDiag_FactorySnSet,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eFactoryCommand[] = {

	{ "cali",		"factory cali command",			NULL,								m_eFactoryCaliCommand},
	{ "2way",		"factory 2way command",			NULL,								m_eFactory2wayCommand},
	{ "ubootversion","factory uboot version get",	pegaDiag_FactoryUBootVersionGet,	NULL},
	{ "mediaserver", "factory mediaserver command",	NULL,						        m_eFactorymediaserverCommand},
	{ "sn",          "factory sn command",	        NULL,						        m_eFactorySnCommand},
	{ "reset",		 "factory reset to default",	pegaDiag_FactoryRset,				NULL},
	{ NULL, NULL, NULL , NULL }
};

static command_line_t m_eMicRecordCommand[] = {
	{ "start",	"mic record start",	pegaDiag_MicRecordStart,	NULL},
	{ "stop",	"mic record stop",	pegaDiag_MicRecordStop,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eMicVolumeCommand[] = {
	{ "get",	"mic volume get",		pegaDiag_MicVolumeGet,	NULL},
	{ "set",	"mic volume set",		pegaDiag_MicVolumeSet,	NULL},
	{ NULL, NULL, NULL, NULL }
};


static command_line_t m_eAudioCommand[] = {
	{ "init",		"init amplifier",				pegaDiag_AudioInit,			NULL},
	{ "record",		"record audio file",			pegaDiag_AudioRecordFile,	NULL},
	{ "play",		"play audio file",				pegaDiag_AudioPlayFile,		NULL},
	//{ "stop",		"stop play file",				pegaDiag_AudioStop,			NULL},
	//{ "restart",	"restart audio file",			pegaDiag_AudioReStart,		NULL},
	{ "volume xx",	"audio volume Set (xx:0~15)",	pegaDiag_AudioVolumeSet,	NULL},
	{ NULL, NULL, NULL , NULL }
};

static command_line_t m_eMicCommand[] = {
	{ "record",		"mic record command",	NULL,				&m_eMicRecordCommand},
	//{ "volume",		"mic volume command",	NULL,				&m_eMicVolumeCommand},
	{ NULL, NULL, NULL , NULL }
};

static command_line_t m_eOut2TelnetCommand[] = {
	{ "console",	"print to console",		pegaDiag_PrintConsole,			NULL},
	{ "telnet",		"print to telnet",		pegaDiag_PrintTelnet,			NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eRingLEDConfigCommand[] = {
	{ "mode_on",		"Mode on",			pegaDiag_RingLedControl,		NULL},
	{ "mode_off",		"Mode off",			pegaDiag_RingLedControl,		NULL},
	{ "mode_default",	"Default",			pegaDiag_RingLedControl,		NULL},
	{ "mode_night",		"Mode night",		pegaDiag_RingLedControl,		NULL},
	{ "be_on",			"Brightextend on",	pegaDiag_RingLedControl,		NULL},
	{ "be_off",			"brightextend off",	pegaDiag_RingLedControl,		NULL},
	{ "ce_set",			"CoolExtend set",	pegaDiag_RingLedControl,		NULL},
	{ "ce_get",			"CoolExtend get",	pegaDiag_RingLedControl,		NULL},
	{ "fr_set",			"Fade Rate set",	pegaDiag_RingLedControl,		NULL},
	{ "fr_get",			"Fade Rate get",	pegaDiag_RingLedControl,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eRingLEDCommand[] = 
{	
	{ "config",			"RingLED Config",			NULL,							&m_eRingLEDConfigCommand},
	{ "on_breath",		"White light Breathing on",		pegaDiag_RingLedControl,		NULL},
	{ "off_breath",		"White light Breathing off",	pegaDiag_RingLedControl,		NULL},
	{ "breathing",			"breathing",				pegaDiag_RingLedControl,		NULL},
	{ "br2colors",			"br2colors",				pegaDiag_RingLedControl,		NULL},
	{ "br_demo",			"br_demo",					pegaDiag_RingLedControl,		NULL},
	{ "flower",				"flower",					pegaDiag_RingLedControl,		NULL},
	{ "chase",				"chase",					pegaDiag_RingLedControl,		NULL},
	{ "chase_demo",			"chasing_demo",				pegaDiag_RingLedControl,		NULL},
	{ "racing",				"racing",					pegaDiag_RingLedControl,		NULL},
	{ "brlogscale",			"breathing logscale",		pegaDiag_RingLedControl,		NULL},
	{ "br_logscale_demo",	"breathing logscale demo",	pegaDiag_RingLedControl,		NULL},	
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eAccCommand[] = 
{		
	{ "acc_raw",				"Read 3 Axis raw data",		pegaDiag_AccControl,		NULL},
	{ "acc_scale",				"Read 3 Axis scale",		pegaDiag_AccControl,		NULL},
	{ "int_6D_on",				"enable 6D event",			pegaDiag_AccControl,		NULL},
	{ "int_6D_off",				"disable 6D event",			pegaDiag_AccControl,		NULL},	
	{ "int_freefall_on",		"enable freefall event",	pegaDiag_AccControl,		NULL},
	{ "int_freefall_off",		"disable freefall event",	pegaDiag_AccControl,		NULL},	
	{ "int_wake_on",			"enable wakeup event",		pegaDiag_AccControl,		NULL},
	{ "int_wake_off",			"disable wakeup event",		pegaDiag_AccControl,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eNFCCommand[] = 
{		
	{ "i2c_check",		"Dump Register",		pegaDiag_NFCControl,		NULL},
	{ "scan",			"SampleCode Demo",		pegaDiag_NFCControl,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eMotorRisingCommand[] = 
{	
	{ "command:",	"forward [speed:1~4] [time:100ms~]",	NULL,			NULL},	
	{ "example:",	"forward 4 1000",						NULL,			NULL},	
	{ "forward",	"move forward",							pegaDiag_rising_motor_forward,	NULL},
	{ "command:",	"reverse [speed:1~4] [time:100ms~]",	NULL,			NULL},	
	{ "example:",	"reverse 4 1000",						NULL,			NULL},	
	{ "reverse",	"move reverse",							pegaDiag_rising_motor_reverse,	NULL},
	{ "test",		"rising motor self-test",				pegaDiag_rising_motor_test,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eMotorPanCommand[] = 
{	
	{ "command:",	"forward [speed:1~4] [time:100ms~]",	NULL,			NULL},	
	{ "example:",	"forward 4 1000",						NULL,			NULL},
	{ "forward",	"move forward",							pegaDiag_pan_motor_forward,		NULL},	
	{ "command:",	"reverse [speed:1~4] [time:100ms~]",	NULL,			NULL},	
	{ "example:",	"reverse 4 1000",						NULL,			NULL},
	{ "reverse",	"move reverse",							pegaDiag_pan_motor_reverse,		NULL},
	{ "test",		"pan motor self-test",					pegaDiag_pan_motor_test,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eMotorCommand[] = 
{		
	{ "rising",		"rising motor",			NULL,		m_eMotorRisingCommand},
	{ "pan",		"pan motor",			NULL,		m_eMotorPanCommand},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eStreamingCommand[] = {	
	{ "on",			"RTSP streaming output",		pegaDiag_streaming_on,			NULL},	
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eMfgbridgeCommand[] = {
	{ "test",		"Labtool mfgbridge test",		pegaDiag_mfgbridge_test,			NULL},	
	{ NULL, NULL, NULL, NULL }
};
//==============================================================================
static command_line_t m_eDiagMainCommand[] = 
{
	//{ "burnin",		"burnin command",		NULL,				&m_eBurninCommand},
	{ "ver",		"version command",		NULL,				&m_eVersionCommand},	
	//{ "sn",			"sn command",			NULL,				&m_eSnCommand},	
	{ "icr",		"icr command",			NULL,				&m_eIcrCommand},
	{ "irled",		"irled command",		NULL,				&m_eIrLedCommand},
	{ "spotled",	"spotled command",		NULL,				&m_eSpotLedCommand},
	{ "led",		"led command",			NULL,				&m_eLedCommand},
	{ "led_r",		"led red command",		NULL,				&m_eLedRCommand},
	{ "led_g",		"led green command",	NULL,				&m_eLedGCommand},
	{ "led_b",		"led blue command",		NULL,				&m_eLedBCommand},
	{ "als",		"als command",			NULL,				&m_eAlsCommand},
	//{ "mic",		"mic record command",	NULL,				&m_eMicCommand},
	{ "audio",		"audio command",		NULL,				&m_eAudioCommand},
	{ "print",		"print to telnet",		NULL,				&m_eOut2TelnetCommand},	
	{ "button",		"button command",		NULL,				&m_eButtonCommand},
	{ "wifi",		"wifi command",			NULL,				&m_eWifiCommand},
	{ "ble",		"ble command",			NULL,				&m_eBleCommand},		
	{ "ethernet",	"ethernet command",		NULL,				&m_eEthernetCommand},
	//{ "ota",		"ota command",			NULL,				&m_eOtaCommand},	
	//{ "factory",	"factory command",		NULL,				&m_eFactoryCommand},
	{ "ringled", 	"ringled commad",		NULL,				&m_eRingLEDCommand},
	{ "acc", 		"acc commad",			NULL,				&m_eAccCommand},
	{ "nfc", 		"nfc commad",			NULL,				&m_eNFCCommand},
	{ "motor", 		"motor commad",			NULL,				&m_eMotorCommand},
	{ "streaming", 	"streaming commad",		NULL,				&m_eStreamingCommand},
	{ "mfgbridge", 	"mfgbridge commad",		NULL,				&m_eMfgbridgeCommand},
	{ NULL, NULL, NULL , NULL }
};
//==============================================================================
static int pegaDiag_ExecuteCommand(command_line_t *eDiagCommand, int argc, char *argv[])
{
	int intIndex  = 0;
	int intResult = 0;
	command_line_t *eCommand = eDiagCommand;
	
	while (eCommand[intIndex].cmd != NULL)
	{
		if (strncmp(eCommand[intIndex].cmd,	argv[0], strlen(eCommand[intIndex].cmd)) || (strlen(eCommand[intIndex].cmd) !=strlen(argv[0])))
		{
			intIndex++;
			continue;
		}
		
		if(eCommand[intIndex].func != NULL)
		{
			if (0 != (intResult = eCommand[intIndex].func(argc, argv)))
			{
			}
			break;
		}
		else
		{
			eCommand = (command_line_t *)eCommand[intIndex].eNext;	
			intIndex = 0;
            if (argc == 1)
			{
				goto Help_info;
			}				
			argc--;
			argv++;
		}
	}
	
	if (eCommand[intIndex].cmd == NULL) 
	{
		printf("Unknown command - \"%s\"\n", *argv);		
		intResult = -1;
	}

	return intResult;
	
Help_info:

    printf("\nHelp info:\n");
	printf("-----------------------------------\n");
	
    while (eCommand[intIndex].cmd != NULL)
	{
		printf("%16s \t %s\n", eCommand[intIndex].cmd, eCommand[intIndex].doc);
		
		intIndex++;
	}	
	printf("-----------------------------------\n");
	
	return 0;
}

static void pegaDiag_PrintHelp(command_line_t *eDiagCommand)
{
	int intIndex = 0;
	
	command_line_t *eCommand = eDiagCommand;
	
	printf("\nHelp info:");
	printf("\n-----------");
	
	while (eCommand[intIndex].cmd != NULL)
	{
		printf("\n %16s \t %s", eCommand[intIndex].cmd, eCommand[intIndex].doc);
		
		intIndex++;
	}
	
	printf("\n-----------\n");
}
//------------------------------------------------------------------------------------------
// main state function
//------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	int intResult = -1;
	
	argc--;
	argv++;
	
	memset(&m_stDiagMsg, 0, sizeof(m_stDiagMsg));
	
    //printf("\n[%s] argc=%d argv=%s\n", __func__, argc, argv[0]);  
	 
	if (argc <=0 )
	{
		goto HelpInfo;
	}
	
    intResult = pegaDiag_ExecuteCommand(m_eDiagMainCommand, argc, argv);

HelpInfo:
	
	if (intResult != 0)
	{
		pegaDiag_PrintHelp(m_eDiagMainCommand);
	}
	
	return intResult;
} 


//------------------------------------------------------------------------------------------

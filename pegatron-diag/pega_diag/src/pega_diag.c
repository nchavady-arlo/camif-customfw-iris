/*******************************************************************************
* File Name: pega_diag.c
*
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <dirent.h>
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
#include "pega_diag_mq.h"
//==============================================================================
static char audio_file_path[128];
static char audio_record_time[64];
static char audio_play_time[64];
static int mic_record_volume = 0;
//==============================================================================
static uint8_t m_Wifi_IF[E_WIFI_IF_TYPE_MAX-1] = {0};
//==============================================================================
#define RFTEST_ENABLE_FILE 		"/data/enable_rftest"
#define DEMO_ENABLE_FILE 		"/data/enable_demo"
//==============================================================================
#define DEMO_CYBERON_FOLDER		"/data/cyberon_spk"
//==============================================================================
#define ARLO_CERT_FILE			"/data/arlo_cert/dts_config.txt"
//==============================================================================
#define DTBO_FOLDER				"/proc/device-tree/device_info"
//==============================================================================
#define ENABLE_DEBUG_MSG    0
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

static int pegaDiag_util_shell(const char *cmd, char *value, size_t size)
{
    int ret = 0;
    FILE *fp;
    size_t len = 0, total=0;
    char *s = value;

    s[0] = '\0';
   	
    fp = popen(cmd, "r");
    if (!fp) 
	{
        fprintf(stderr, "Error: popen(\"%s\") failed\n", cmd);
        return -1;
    }
	
	while (fgets(s, size - total, fp) != NULL) 
	{
		len = strlen(s);
		total += len;
		if (total >= size) 
		{
			ret = 1;
			break;
		}
		s += len;
	}

    if (len > 0 && s[len-1] == '\n') 
	{
		s[len-1] = '\0';
	}
            
	pclose(fp);
	
    return ret;
}
//
static int pegaDiag_download_file(const char *ipaddr, const char *working, const char *entity, const char *dest_entity)
{
    int ret = -1;
    char cmd[256 + 1] = { '\0' }, log[1024 + 1] = { '\0' };
   
    sprintf(cmd, "tftp -g %s -r %s -l %s/%s 2>&1 | tee /tmp/tftp.log", ipaddr, entity, working, dest_entity);

    if (pegaDiag_util_shell(cmd, log, sizeof(log) - 1) < 0 || log[0] == '\0') 
	{
        printf("Error: TFTP download command failed\n");
        goto func_end;
    }
    else if (strstr(log, "100%") == NULL) 
	{
        printf("%s\n", log);
        printf("Error: Check details in %s\n", "/tmp/tftp.log");
        goto func_end;
    }

    ret = 0;

func_end:

    return ret;
}
//
static int pegaDiag_upload_file(const char *ipaddr, const char *entity)
{
    int ret = -1;
    char cmd[256 + 1] = { '\0' }, log[1024 + 1] = { '\0' };
   
    sprintf(cmd, "tftp -p %s -l %s 2>&1 | tee /tmp/tftp.log", ipaddr, entity);

    if (pegaDiag_util_shell(cmd, log, sizeof(log) - 1) < 0 || log[0] == '\0') 
	{
        printf("Error: TFTP download command failed\n");
        goto func_end;
    }
    else if (strstr(log, "100%") == NULL) 
	{
        printf("%s\n", log);
        printf("Error: Check details in %s\n", "/tmp/tftp.log");
        goto func_end;
    }

    ret = 0;

func_end:

    return ret;
}
//==============================================================================
static int list_files(const char *path) 
{
	int filecnt = 0;
    DIR *dir = opendir(path);
	
    if (dir == NULL) 
	{
        perror("unable to open folder");
        return -1;
    }

    struct dirent *entry;
    char fullpath[1024];
	struct stat statbuf;
	
    printf("=== list the folder : %s ===\n", path);
    while ((entry = readdir(dir)) != NULL) 
	{
        // ignore . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
            continue;
		}

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        
        if (stat(fullpath, &statbuf) == 0) 
		{
            if (S_ISDIR(statbuf.st_mode)) 
			{
                printf("[DIR ] %s\n", entry->d_name);
            } 
			else 
			{
				filecnt++;
                printf("[FILE] %s\n", entry->d_name);
            }
        }
    }

    closedir(dir);
	
	return filecnt;
}

static int remove_dir(const char *path) 
{
    DIR *dir = opendir(path);
    if (!dir) return -1;

    struct dirent *entry;
	struct stat statbuf;
    char fullpath[1024];

    while ((entry = readdir(dir)) != NULL) 
	{
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
            continue;
		}

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        printf("fullpath=%s\n!", fullpath);		
        
        if (stat(fullpath, &statbuf) == 0) 
		{
            if (S_ISDIR(statbuf.st_mode)) 
			{
                // Recursively delete subfolders
                remove_dir(fullpath);
            } else 
			{
                // Delete the file
                remove(fullpath);
            }
        }
    }

    closedir(dir);
    // Delete itself at the end
    return rmdir(path);
}
//==============================================================================
static void pegaDiag_WifiInterfaceCheck(void)
{
	 m_Wifi_IF[0] = E_WIFI_IF_TYPE_NONE;
	 m_Wifi_IF[1] = E_WIFI_IF_TYPE_NONE;
	 
	 if (pega_netip_interface_check_sd0("mlan0",0) >= 0)
	 {
		 m_Wifi_IF[0] = E_WIFI_IF_TYPE_MLAN0;
	 }
	 else if (pega_netip_interface_check_sd0("mlan1",0) >= 0)
	 {
		 m_Wifi_IF[0] = E_WIFI_IF_TYPE_MLAN1;
	 }
	 
	 if (pega_netip_interface_check_sd1("mlan0",0) >= 0)
	 {
		 m_Wifi_IF[1] = E_WIFI_IF_TYPE_MLAN0;
	 }
	 else if (pega_netip_interface_check_sd1("mlan1",0) >= 0)
	 {
		 m_Wifi_IF[1] = E_WIFI_IF_TYPE_MLAN1;
	 }	
}
//==============================================================================
static int pegaDiag_WifiCountrycodeCheck(char *country_code)
{
	int i;
	static char country_code_table[16][3] = 
	{
	{"WW"}, /* World       */
	{"US"}, /* US FCC      */
	{"CA"}, /* IC Canada   */
	{"SG"}, /* Singapore   */
	{"EU"}, /* ETSI        */
	{"AU"}, /* Australia   */
	{"KR"}, /* Republic Of Korea */
	{"JP"}, /* Japan       */
	{"CN"}, /* China       */
	{"TW"}, /* TW support  */
	{"BR"}, /* Brazil      */
	{"RU"}, /* Russia      */
	{"IN"}, /* India       */
	{"MY"}, /* Malaysia    */
	{"NZ"}, /* New Zeland  */
	{"MX"}, /* Mexico */
	};
	
	if ((country_code == NULL) || (strlen(country_code) != 2))
	{
		return -1;
	}	
	
	//printf("%s(%d)\n",__FUNCTION__, strlen(country_code)); 
	
	for (i=0;i<16;i++)
	{
		//printf("country_code[%d]=%s\n", i, country_code_table[i]);
		
		if (!strncmp(country_code_table[i], country_code, strlen(country_code)))
		{			
			return i;
		}
	}

	return -1;
}

static int pegaDiag_WifiCountrycodeShow(int argc, char **argv)
{			
	printf("Country Code :\n"); 
	
	printf("\tWW - World\n");
	printf("\tUS - US FCC\n");
	printf("\tCA - IC Canada\n");
	printf("\tSG - Singapore\n");
	printf("\tEU - ETSI\n");
	printf("\tAU - Australia\n");
	printf("\tKR - Republic Of Korea\n");
	printf("\tJP - Japan\n");
	printf("\tCN - China\n");
	printf("\tTW - Taiwan\n");
	printf("\tBR - Brazil\n");
	printf("\tRU - Russia\n");
	printf("\tIN - India\n");
	printf("\tMY - Malaysia\n");
	printf("\tNZ - New Zeland\n");
	printf("\tMX - Mexico\n");
	
	return 0;
}
//==============================================================================
//diag burnin on
static int pegaDiag_BurnIn_control(int argc, char **argv)
{		
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "burnin", strlen("burnin"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
				
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
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
		
	if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
	{			
		PT_DIAG_GET(0, "%s", buff);
	}
		
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
	
	sprintf(cmd,"A=`cat /proc/mwlan/adapter0/mlan0/info | grep 'driver_version' | cut -d '=' -f 2 | cut -d '-' -f 4`;echo -n \"$A\"");
		
	if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
	{			
		PT_DIAG_GET(0, "%s", buff);
	}
		
	return intResult;
}
//------------------------------------------------------------------------------------------
//diag ircut day
//diag ircut night
static int pegaDiag_ircut_control(int argc, char **argv)
{
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "ircut", strlen("ircut"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));	
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------	
}
//------------------------------------------------------------------------------------------
//diag irled on
//diag irled off
static int pegaDiag_IrLedControl(int argc, char **argv)
{
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "irled", strlen("irled"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
	//stDiagCmd.stData.argv[0][MSGQ_ARGV_LEN - 1] = '\0';			
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//diag irled duty xx (xx:1~100)
static int pegaDiag_IrLedDutySet(int argc, char **argv)
{
	int value = 0;
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	if (argv[1] == NULL)
	{
		return -1;
	}
	
	value = atoi(argv[1]);
	
	if ((value < 1) || (value > 100))
	{
		printf("Value error.(%d)\n",__FUNCTION__, value); 
		return 0;
	}
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "irled", strlen("irled"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
	strncpy(stDiagCmd.stData.argv[2], argv[1], strlen(argv[1]));			
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//diag spotled on
//diag spotled off
static int pegaDiag_SpotLed_Set(int argc, char **argv)
{
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "spotled", strlen("spotled"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
				
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//diag led on
//diag led off
static int pegaDiag_Led_Set(int argc, char **argv)
{
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "led", strlen("led"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
				
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//diag led_r on
//diag led_r off
static int pegaDiag_LedR_Set(int argc, char **argv)
{
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "led_r", strlen("led_r"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
				
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//diag led_g on
//diag led_g off
static int pegaDiag_LedG_Set(int argc, char **argv)
{
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "led_g", strlen("led_g"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
				
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//diag led_b on
//diag led_b off
static int pegaDiag_LedB_Set(int argc, char **argv)
{
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "led_b", strlen("led_b"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
				
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//==============================================================================
//diag als id
//diag als day
//diag als night
//diag als value
//diag als on
//diag als off
static int pegaDiag_Als_Control(int argc, char **argv)
{
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "als", strlen("als"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
				
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//diag als calidata xx xx
static int pegaDiag_AlsCalidata(int argc, char **argv)
{
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "als", strlen("als"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
	
	if (argv[1] != NULL)
		strncpy(stDiagCmd.stData.argv[2], argv[1], strlen(argv[1]));

	if (argv[2] != NULL)
		strncpy(stDiagCmd.stData.argv[3], argv[2], strlen(argv[2]));
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//diag als gain
static int pegaDiag_AlsCalidata_Get(int argc, char **argv)
{
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "als", strlen("als"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
				
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//diag audio volume
//diag audio volume xx (xx:8.5 ~ 22.0)
static int pegaDiag_AudioVolumeSet(int argc, char **argv)
{
	float volume = 0.0;	
	stMsgQdata_t stDiagCmd;   
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "audio", strlen("audio"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
    
    if (argv[1] != NULL)
	{
		strncpy(stDiagCmd.stData.argv[2], argv[1], strlen(argv[1]));
		
		volume = atof(argv[1]);
		
		if (volume < 8.5 || volume > 22.0)
		{
			printf("\n[error] Volume value(%.1f) is wrong!", volume);
			printf("\n[error] volume range : 8.5 ~ 22.0");
			PT_DIAG_SET(-1);
			return 0;
		}
	}
	
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//diag button get
static int pegaDiag_ButtonDetection_Get(int argc, char **argv)
{
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "button", strlen("button"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));				
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
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
        system("sh /scripts/IW610_wifi_enable.sh");
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
	
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__,argc);
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);

    if (argc == 0)
	{
		if (pega_netip_is_interface_exist("mlan0", 0) > -1)
		{	
			intResult = 0;
			snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_connect.sh");
			//intResult = system(cBuffer); 
			system(cBuffer); 
			PT_DIAG_SET(intResult);
			return 0;//intResult;
		}
	}

    if (argc < 2)
	{
		printf("Command error!");  
        PT_DIAG_SET(-1);
		return -1;
	}	
  	
	if (argc == 3) //(argv[2] != NULL)
	{
		if (!strncmp(argv[2], "mlan1", strlen("mlan1")))
		{
			if (pega_netip_is_interface_exist("mlan1", 0) > -1)
			{
				intResult = 0;
				snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_connect.sh \"%s\" \"%s\" %s", argv[0], argv[1], "mlan1");
				system(cBuffer); 
				PT_DIAG_SET(intResult);
				return 0;//intResult;
			}			
		}        
	}	

    if (pega_netip_is_interface_exist("mlan0", 0) > -1)
	{	
		intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_connect.sh \"%s\" \"%s\"", argv[0], argv[1]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}
				
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag wifi connect2 <ssid> <password>
//diag wifi connect2 <ssid> <password> mlan0/mlan1
static int pegaDiag_WifiConnect2(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[120]={0};
	
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	if (argc == 0)
	{
		if (pega_netip_is_interface_exist("mlan1", 0) > -1)
		{	
			intResult = 0;
			snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_connect2.sh");
			//intResult = system(cBuffer); 
			system(cBuffer); 
			PT_DIAG_SET(intResult);
			return 0;//intResult;
		}
	}

    if (argc < 2)
	{
		printf("Command error!");  
        PT_DIAG_SET(-1);
		return -1;
	}	
	
	if (argc == 3)//(argv[2] != NULL)
	{
		if (!strncmp(argv[2], "mlan1", strlen("mlan1")))
		{
			if (pega_netip_is_interface_exist("mlan1", 0) > -1)
			{
				intResult = 0;
				snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_connect2.sh \"%s\" \"%s\" %s", argv[0], argv[1], "mlan1");
				system(cBuffer); 
				PT_DIAG_SET(intResult);
				return 0;//intResult;
			}			
		}        
	}	

    if (pega_netip_is_interface_exist("mlan0", 0) > -1)
	{	
		intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_connect2.sh \"%s\" \"%s\"", argv[0], argv[1]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}
				
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag wifi status mlan0/mlan1
static int pegaDiag_WifiStatus(int argc, char **argv)
{
	int intResult=0;	
	int rtn = 0;
	char ip_addr[64] = {0};
		 
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
    if (argv[0] != NULL)
	{
		if (!strncmp(argv[0], "mlan1", strlen("mlan1")))
		{
            rtn = pega_netip_ip_get("mlan1", ip_addr, 64);
		}
		else if (!strncmp(argv[0], "mlan0", strlen("mlan0")))
		{
            rtn = pega_netip_ip_get("mlan0", ip_addr, 64);
		}
	}	
	else
	{
		rtn = pega_netip_ip_get("mlan0", ip_addr, 64);
	}
	
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
//diag wifi rssi mlan0/mlan1
static int pegaDiag_WifiRssiGet(int argc, char **argv)
{
	int intResult=0;	
	
	char cmd[120]={0};
    char buff[120]={0};
	
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	if (argv[0] != NULL)
	{
		if (!strncmp(argv[0], "mlan1", strlen("mlan1")))
		{
			intResult = 0;
            sprintf(cmd,"A=`cat /proc/net/wireless | grep -i mlan1 | awk '{print $4}'`;echo -n \"$A\"");
		}
		else if (!strncmp(argv[0], "mlan0", strlen("mlan0")))
		{
			intResult = 0;
            sprintf(cmd,"A=`cat /proc/net/wireless | grep -i mlan0 | awk '{print $4}'`;echo -n \"$A\"");
		}
	}
	else
	{
		sprintf(cmd,"A=`cat /proc/net/wireless | grep -i mlan0 | awk '{print $4}'`;echo -n \"$A\"");
	}
		
	if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
	 {			
		PT_DIAG_GET(0, "%s", buff);
	 }		
		
	return intResult;
}
//diag wifi signal mlan0/mlan1
static int pegaDiag_WifiSignalGet(int argc, char **argv)
{
	int intResult = -1;	
		
    char cBuffer[120]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	if (argv[0] != NULL)
	{
		if (!strncmp(argv[0], "mlan1", strlen("mlan1")))
		{
			intResult = 0;
            snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getsignal", "mlan1");
		}
		else if (!strncmp(argv[0], "mlan0", strlen("mlan0")))
		{
			intResult = 0;
            snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getsignal", "mlan0");
		}
	}	
	else
	{
		intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getsignal", "mlan0");
	}
	
	if (intResult == 0)
	{
		system(cBuffer);
	}
	
	//PT_DIAG_GET(0, "%s", buff);
	PT_DIAG_SET(intResult);
	
	return intResult;
}
//diag wifi datarate mlan0/mlan1
static int pegaDiag_WifiDatarateGet(int argc, char **argv)
{
	int intResult = -1;	
		
    char cBuffer[120]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	if (argv[0] != NULL)
	{
		if (!strncmp(argv[0], "mlan1", strlen("mlan1")))
		{
			intResult = 0;
            snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getdatarate", "mlan1");
		}
		else if (!strncmp(argv[0], "mlan0", strlen("mlan0")))
		{
			intResult = 0;
            snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getdatarate", "mlan0");
		}
	}	
	else
	{
		intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getdatarate", "mlan0");
	}
	
	if (intResult == 0)
	{
		system(cBuffer);
	}
	//PT_DIAG_GET(0, "%s", buff);
	PT_DIAG_SET(0);
	
	return intResult;
}
//diag wifi chanstats mlan0/mlan1
static int pegaDiag_WifiChanstatsGet(int argc, char **argv)
{
	int intResult = -1;	
		
    char cBuffer[120]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	if (argv[0] != NULL)
	{
		if (!strncmp(argv[0], "mlan1", strlen("mlan1")))
		{
			intResult = 0;
            snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getchanstats", "mlan1");
		}
		else if (!strncmp(argv[0], "mlan0", strlen("mlan0")))
		{
			intResult = 0;
            snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getchanstats", "mlan0");
		}
	}	
	else
	{
		intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getchanstats", "mlan0");
	}
	
	if (intResult == 0)
	{
		system(cBuffer);
	}
	//PT_DIAG_GET(0, "%s", buff);
	PT_DIAG_SET(0);
	
	return intResult;
}
//diag wifi scantable mlan0/mlan1
static int pegaDiag_WifiScantableGet(int argc, char **argv)
{
	int intResult = -1;	
		
    char cBuffer[120]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	if (argv[0] != NULL)
	{
		if (!strncmp(argv[0], "mlan1", strlen("mlan1")))
		{
			intResult = 0;
            snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getscantable", "mlan1");
		}
		else if (!strncmp(argv[0], "mlan0", strlen("mlan0")))
		{
			intResult = 0;
            snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getscantable", "mlan0");
		}
	}	
	else
	{
		intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s getscantable", "mlan0");
	}
	
	if (intResult == 0)
	{
		system(cBuffer);
	}
	//PT_DIAG_GET(0, "%s", buff);
	PT_DIAG_SET(0);
	
	return intResult;
}
//diag wifi mac mlan0/mlan1
static int pegaDiag_WifiMacGet(int argc, char **argv)
{
	int intResult = -1;	
	
	char cmd[120]={0};
    char buff[128]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
    if (argv[0] != NULL)
	{			
		if (!strncmp(argv[0], "mlan1", strlen("mlan1")))
		{
			intResult = 0;
            sprintf(cmd,"A=`ifconfig | grep mlan1 | cut -d' ' -f10`;echo -n \"$A\"");
		}
		else if (!strncmp(argv[0], "mlan0", strlen("mlan0")))
		{
			intResult = 0;
            sprintf(cmd,"A=`ifconfig | grep mlan0 | cut -d' ' -f10`;echo -n \"$A\"");
		}
	}	
	else
	{		
        intResult = 0;
		sprintf(cmd,"A=`ifconfig | grep mlan0 | cut -d' ' -f10`;echo -n \"$A\"");
	}
			
	if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
	{			
		PT_DIAG_GET(0, "%s", buff);
	}	
	
	return intResult;
}
//diag wifi interface
static int pegaDiag_WifiInterfaceGet(int argc, char **argv)
{
	int intResult=0;	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	const char eWiFi_IF[3][6] = {"none","mlan0","mlan1"};
	
	pegaDiag_WifiInterfaceCheck();
		 
	PT_DIAG_GET(0, "SDIO_0:%s", eWiFi_IF[m_Wifi_IF[0]]);
	PT_DIAG_GET(0, "SDIO_1:%s", eWiFi_IF[m_Wifi_IF[1]]);	 
	 
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
		
	sprintf(cmd,"A=`cat /data/nxp/wpa.conf | grep ssid | cut -d'=' -f2`;echo -n $A");
	
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
		snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_ssid_pass.sh ssid \"%s\"", argv[0]);
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
		
	sprintf(cmd,"A=`cat /data/nxp/wpa.conf | grep psk | cut -d'=' -f2`;echo -n $A");
	
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
		snprintf(cBuffer, sizeof(cBuffer), "/scripts/wifi_ssid_pass.sh psk \"%s\"", argv[0]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag wifi ip_addr get mlan0/mlan1
static int pegaDiag_WifiIpAddrGet(int argc, char **argv)
{
	int intResult = -1;	
	int rtn = -1;	
    char ip_addr[64] = {0};
	 
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
    if (argv[0] != NULL)
	{
		if (!strncmp(argv[0], "mlan1", strlen("mlan1")))
		{
			intResult = 0;	
            rtn = pega_netip_ip_get("mlan1", ip_addr, 64);
		}
		else if (!strncmp(argv[0], "mlan0", strlen("mlan1")))
		{
			intResult = 0;	
            rtn = pega_netip_ip_get("mlan0", ip_addr, 64);
		}
	}	
	else
	{	
        intResult = 0;	
		rtn = pega_netip_ip_get("mlan0", ip_addr, 64);
	}
	
	if (intResult == 0)
	{
		if (rtn > -1)
		{		
			PT_DIAG_GET(0, "%s", ip_addr);
		}
		else
		{
			PT_DIAG_GET(0, "%s", "NULL");
		}
	}
		
	return intResult;
}
//diag wifi ip_addr set xx mlan0/mlan1
static int pegaDiag_WifiIpAddrSet(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[120] = {0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	if (argv[0] == NULL)
	{
		return intResult;
	}
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);

    if (argv[1] != NULL)
	{
		if (!strncmp(argv[1], "mlan1", strlen("mlan1")))
		{
			intResult = 0;
			snprintf(cBuffer, sizeof(cBuffer), "ifconfig mlan1 %s", argv[0]);
		}
		else if (!strncmp(argv[1], "mlan0", strlen("mlan0")))
		{
			intResult = 0;
			snprintf(cBuffer, sizeof(cBuffer), "ifconfig mlan0 %s", argv[0]);
		}	
	}		
	else 	
	{	
        intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "ifconfig mlan0 %s", argv[0]);			
	}	
    			
	if (intResult == 0)
	{
		system(cBuffer); 
	}		
	
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag wifi ip_mask get mlan0/mlan1
static int pegaDiag_WifiIpMaskGet(int argc, char **argv)
{
	int intResult = 0;	
	int rtn = 0;
	struct ifaddrs *ifaddr, *ifa;
	char host[NI_MAXHOST];
	int s;
	struct sockaddr_in *sa;
    char *addr;
	int  eWiFi_IF = -1; 
	 
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	if (argv[0] != NULL)
	{
		if (!strncmp(argv[0], "mlan1", strlen("mlan1")))
		{
			eWiFi_IF = 1;
		}
		else if (!strncmp(argv[0], "mlan0", strlen("mlan1")))
		{
			eWiFi_IF = 0;           
		}
	}	
	else
	{
		eWiFi_IF = 0;
	}
	
	
	if ((eWiFi_IF < 0) || (getifaddrs(&ifaddr) == -1)) 
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
        				
		if (0 == strcmp(ifa->ifa_name, (eWiFi_IF==0)?"mlan0":"mlan1")) //mlan0/mlan1/eth0
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
//diag wifi ip_mask set xx mlan0/mlan1
static int pegaDiag_WifiIpMaskSet(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[120]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	//printf("\n argv[0]=%s",argv[0]);
	//printf("\n argv[1]=%s",argv[1]);
	if (argv[0] == NULL)
	{
		return intResult;
	}

    if (argv[1] != NULL)
	{
		if (!strncmp(argv[1], "mlan1", strlen("mlan1")))
		{
			intResult = 0;
			snprintf(cBuffer, sizeof(cBuffer), "ifconfig mlan1 netmask %s", argv[0]);
		}
		else if (!strncmp(argv[1], "mlan0", strlen("mlan0")))
		{
			intResult = 0;
			snprintf(cBuffer, sizeof(cBuffer), "ifconfig mlan0 netmask %s", argv[0]);
		}	
	}		
	else 	
	{
		intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "ifconfig mlan0 netmask %s", argv[0]);
	}
	
	if (intResult == 0)
	{
		system(cBuffer); 
	}
    			
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag wifi cfgdata get wifi1/wifi2
static int pegaDiag_WifiCfgdataGet(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[120]={0};
	
    pegaDiag_WifiInterfaceCheck();
	
	printf("%s(%d)\r\n",__FUNCTION__, argc); 
	print_parameter(--argc, ++argv);
	
	if (argc > 0)
	{
		if (!strncmp(argv[0], "wifi1", strlen("wifi1")))
		{
			if (m_Wifi_IF[0] == E_WIFI_IF_TYPE_MLAN0)
			{
				intResult = 0;
				snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s cfgdata 2", "mlan0");
			}
			else if (m_Wifi_IF[0] == E_WIFI_IF_TYPE_MLAN1)
			{
				intResult = 0;
				snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s cfgdata 2", "mlan1");
			}
		}
		else if (!strncmp(argv[0], "wifi2", strlen("wifi2")))
		{
			if (m_Wifi_IF[1] == E_WIFI_IF_TYPE_MLAN0)
			{
				intResult = 0;
				snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s cfgdata 2", "mlan0");
			}
			else if (m_Wifi_IF[1] == E_WIFI_IF_TYPE_MLAN1)
			{
				intResult = 0;
				snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s cfgdata 2", "mlan1");
			}
		}
	}
	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	if (intResult == 0)
	{
		system(cBuffer);
	}
	
	PT_DIAG_SET(intResult);
	
	return intResult;
}
//diag wifi cfgdata set xx wifi1/wifi2
static int pegaDiag_WifiCfgdataSet(int argc, char **argv)
{
	int intResult = -1;	
	char cBuffer[160]={0};
	
    pegaDiag_WifiInterfaceCheck();
	
	printf("%s(%d)\r\n",__FUNCTION__, argc); 
	print_parameter(--argc, ++argv);
	
	if (argc > 1) 
	{
		if (!strncmp(argv[0], "wifi1", strlen("wifi1")))
		{
			if (m_Wifi_IF[0] == E_WIFI_IF_TYPE_MLAN0)
			{
				intResult = 0;
				snprintf(cBuffer, sizeof(cBuffer), "mlanutl mlan0 cfgdata 2 %s", argv[1]);
			}
			else if (m_Wifi_IF[0] == E_WIFI_IF_TYPE_MLAN1)
			{
				intResult = 0;
				snprintf(cBuffer, sizeof(cBuffer), "mlanutl mlan1 cfgdata 2 %s", argv[1]);
			}
		}
		else if (!strncmp(argv[0], "wifi2", strlen("wifi2")))
		{
			if (m_Wifi_IF[1] == E_WIFI_IF_TYPE_MLAN0)
			{
				intResult = 0;
				snprintf(cBuffer, sizeof(cBuffer), "mlanutl mlan0 cfgdata 2 %s", argv[1]);
			}
			else if (m_Wifi_IF[1] == E_WIFI_IF_TYPE_MLAN1)
			{
				intResult = 0;
				snprintf(cBuffer, sizeof(cBuffer), "mlanutl mlan1 cfgdata 2 %s", argv[1]);
			}
		}
	}
	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	if (intResult == 0)
	{
		system(cBuffer);
	}
	
	PT_DIAG_SET(intResult);
	
	return 0;//intResult;
}
//diag wifi countrycode get mlan0/mlan1
static int pegaDiag_WifiCountrycodeGet(int argc, char **argv)
{
	int intResult = -1;	
		
    char cBuffer[120]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
	if (argv[0] != NULL)
	{
		if (!strncmp(argv[0], "mlan1", strlen("mlan1")))
		{
			intResult = 0;
            snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s countrycode", "mlan1");
		}
		else if (!strncmp(argv[0], "mlan0", strlen("mlan0")))
		{
			intResult = 0;
            snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s countrycode", "mlan0");
		}
	}	
	else
	{
		intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s countrycode", "mlan0");
	}
	
	if (intResult == 0)
	{
		system(cBuffer);
	}
	//PT_DIAG_GET(0, "%s", buff);
	PT_DIAG_SET(0);
	
	return intResult;
}

//diag wifi countrycode set mlan0/mlan1 xx
static int pegaDiag_WifiCountrycodeSet(int argc, char **argv)
{
	int intResult = -1;	
		
    char cBuffer[120]={0};
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);

    if ((argv[0] == NULL) || (argv[1] == NULL))
	{
		return intResult;
	}		
		
	if (pegaDiag_WifiCountrycodeCheck(argv[1]) < 0)
	{	
        pegaDiag_WifiCountrycodeShow(0, NULL);
		printf("Country code error!(%s)\n", argv[1]); 
		PT_DIAG_SET(0);
		return 0;
	}
	
	if (!strncmp(argv[0], "mlan1", strlen("mlan1")))
	{
		intResult = 0;
        snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s countrycode %s", "mlan1", argv[1]);
	}
	else if (!strncmp(argv[0], "mlan0", strlen("mlan0")))
	{
		intResult = 0;
        snprintf(cBuffer, sizeof(cBuffer), "mlanutl %s countrycode %s", "mlan0", argv[1]);
	}		
		
	if (intResult == 0)
	{
		system(cBuffer);
	}
	//PT_DIAG_GET(0, "%s", buff);
	PT_DIAG_SET(0);
	
	return intResult;
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

//diag ble service_on
static int pegaDiag_BleServiceOn(int argc, char **argv)
{
	int intResult = 0;	
	
	printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	system("/scripts/ble_service_on.sh");

	
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
		
	if (argc == 2)
	{
		system("hciconfig hci0 up");
		snprintf(cBuffer, sizeof(cBuffer), "hcitool -i hci0 name %s",  argv[1]);
		system(cBuffer);
		PT_DIAG_SET(intResult);
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
//diag ethernet on eth1 192.168.50.119
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
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "audio", strlen("audio"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
	if (argv[1] != NULL)
	{
		strncpy(stDiagCmd.stData.argv[2], argv[1], strlen(argv[1]));
	}
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------
}
//diag audio record
static int pegaDiag_AudioRecordFile(int argc, char **argv)
{
	int intResult=0;
	char audioCmd[256];
	char pFn[128];
	char *staticFilePath = "/data/audio/";
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
	char *staticFilePath = "/data/audio/";
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
//diag audio play_wav xx
//Ex: diag audio play_wav /audio/16K_16bit_Stereo_Beep.wav
static int pegaDiag_AudioPlayWavFile(int argc, char **argv)
{
	int intResult = -1;
	char audioCmd[256];	
	printf("%s\r\n",__FUNCTION__);
	print_parameter(--argc, ++argv);
	//--------------------------------------------------------------------------------------
	
	if (argv[0] == NULL)
	{
		PT_DIAG_SET(intResult);
		return intResult;
	}
	
	sprintf(audioCmd, "play_wav %s", argv[0]);    
	intResult = system(audioCmd);
	
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
	
	sprintf(audioCmd, "prog_audio_audio -t %d -f /data/audio/16k_16bit_mono_ai.json &", intRecordSec);
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
	#if 0
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
	#endif
	return intResult;
}
//------------------------------------------------------------------------------------------
static int pegaDiag_PrintTelnet(int argc, char **argv)
{	
	int intResult = 0;
	#if 0
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
	#endif
	return intResult;
}
//------------------------------------------------------------------------------------------
//diag nfc type
//diag nfc uid
//diag nfc data
//diag nfc clear
static int pegaDiag_NFC_Control(int argc, char **argv)
{	
	stMsgQdata_t stDiagCmd;	
	//printf("%s\n",__FUNCTION__); 		
	memset(&stDiagCmd, 0, sizeof(stDiagCmd));
	
	stDiagCmd.stData.argc = argc+1;		
	strncpy(stDiagCmd.stData.argv[0], "nfc", strlen("nfc"));
	strncpy(stDiagCmd.stData.argv[1], argv[0], strlen(argv[0]));
				
	//--------------------------------------------------------------------------------------
	return pga_diag_msgq_command_send(stDiagCmd);
	//--------------------------------------------------------------------------------------	
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
	}else if(strcmp(argv[0], "enable_tap") == 0)
	{
		intResult = 0;
		snprintf(cBuffer, sizeof(cBuffer), "pega_acc");
		system(cBuffer); 
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
//diag Motor kernel module
static int pegaDiag_MotorControl(int argc, char **argv)
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
		snprintf(cBuffer, sizeof(cBuffer), "/scripts/motor.sh %s", argv[0]);
		//intResult = system(cBuffer); 
		system(cBuffer); 
	}	
    			
	PT_DIAG_SET(intResult);
	
	return intResult;
}
//------------------------------------------------------------------------------------------
/* diag function for motor kernel module */
static int pegaDiag_rising_motorko_forward(int argc, char **argv)
{
	int intResult = 0;	
	int step =1000;
	
	char cmd[64] = {0};
	print_parameter(--argc, ++argv);
	
	if (argc < 1)
	{		
	    printf("Motro command error.\r\n"); 	
		return -1;
	}
		
	if (argv[0] != NULL)
	{
        step = atoi(argv[0]); 
	}
	
	sprintf(cmd,"pega_motor_v2 rising forward %d", step);  	
	system(cmd);
		
    PT_DIAG_SET(intResult);
	
	return intResult;
}

static int pegaDiag_rising_motorko_reverse(int argc, char **argv)
{
	int intResult = 0;	
	int step = 1000;
	
	char cmd[64] = {0};
	print_parameter(--argc, ++argv);
	
	if (argc < 1)
	{		
	    printf("Motro command error.\r\n"); 	
		return -1;
	}
	
	if (argv[0] != NULL)
	{
        step = atoi(argv[0]); 
	}
		
	sprintf(cmd,"pega_motor_v2 rising reverse %d", step);  	
	system(cmd);
		
    PT_DIAG_SET(intResult);
	
	return intResult;
}

static int pegaDiag_pan_motorko_forward(int argc, char **argv)
{
	int intResult = 0;	
	int step = 1000;
	
	char cmd[64] = {0};
	print_parameter(--argc, ++argv);
	
	if (argc < 1)
	{		
	    printf("Motro command error.\r\n"); 	
		return -1;
	}
	
	if (argv[0] != NULL)
	{
        step = atoi(argv[0]); 
	}
	
	sprintf(cmd,"pega_motor_v2 pan forward %d", step);  	
	system(cmd);
		
    PT_DIAG_SET(intResult);
	
	return intResult;
}

static int pegaDiag_pan_motorko_reverse(int argc, char **argv)
{
	int intResult = 0;	
	int step = 1000;
	
	char cmd[64] = {0};
	print_parameter(--argc, ++argv);
	
	if (argc < 1)
	{		
	    printf("Motro command error.\r\n"); 	
		return -1;
	}
	
	if (argv[0] != NULL)
	{
        step = atoi(argv[0]); 
	}
	
	sprintf(cmd,"pega_motor_v2 pan reverse %d", step);  	
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
//diag rftest on
static int pegaDiag_rfteset_on(int argc, char **argv)
{
	int intResult = 0;	
	
	FILE *pFile = NULL;	
	
	if (access(RFTEST_ENABLE_FILE, F_OK) == 0)   // Check if file exists	
	{
		printf("\n[error]RFtest on already!");
		PT_DIAG_SET(-1);
		return 0;
	}
	
    //printf("tmob config %s \n",RFTEST_ENABLE_FILE); 	
    pFile = fopen(RFTEST_ENABLE_FILE,"w+");	
	
	if (pFile != NULL)
	{		
		fprintf(pFile,"%d\n", 1);
		fclose(pFile); 
	}
	
    PT_DIAG_SET(intResult);
	
	sleep(2);
	
	system("reboot"); 
	
	return intResult;
}
//diag rftest off
static int pegaDiag_rfteset_off(int argc, char **argv)
{
	int intResult = 0;	
	
	if (access(RFTEST_ENABLE_FILE, F_OK) == 0)   // Check if file exists	
	{
	    remove(RFTEST_ENABLE_FILE);//remove file
	}
	
    PT_DIAG_SET(intResult);
	
	return intResult;
}
//diag rftest wifi1
static int pegaDiag_rfteset_wifi1(int argc, char **argv)
{
	int intResult = 0;	
	
	char ip_addr[64] = {0};
	 
	// printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	    
	system("/scripts/IW610_rftest.sh wifi1"); 
	
    PT_DIAG_SET(intResult);
	
	return intResult;
}

//diag rftest wifi2
static int pegaDiag_rfteset_wifi2(int argc, char **argv)
{
	int intResult = 0;	
	
	char ip_addr[64] = {0};
	 
	// printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	    
	system("/scripts/IW610_rftest.sh wifi2"); 
	
    PT_DIAG_SET(intResult);
	
	return intResult;
}
//diag libgpiod on
static int pegaDiag_libgpiod_on(int argc, char **argv)
{
	int intResult = 0;	
	
	char ip_addr[64] = {0};
	 
	// printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);

	system("/scripts/libgpiod_enable.sh"); 	
	
    PT_DIAG_SET(intResult);
	
	return intResult;
}

//diag cyberon dspotter <keyword model> <0/1>
//diag cyberon dspotter HelloArlo_pack_withTxt.bin 0
static int pegaDiag_dspotter(int argc, char **argv) {
  int intResult = 0;
  FILE *fd = NULL;
  char buff[512] = {0};
  char cmd[128];
  char dspotter_cmd[128];

  sprintf(cmd, "date | awk '{print $6}'");

  if ((fd = popen(cmd, "r")) != NULL) {
    while (fgets(buff, 512, fd) != NULL) {
      pclose(fd);
      // printf("Get date year: %s\r\n", buff);
      if (atoi(buff) == 1970) {
        printf("Please update system date before executing DSpotter demo.\r\n");
        printf("For example: date -s \"2025-09-01 08:00:00\"\r\n");

        intResult = -1;
        PT_DIAG_SET(intResult);

        return intResult;
      }
    }
  }

  // printf("%s: %d, %s\r\n",__FUNCTION__, argc, argv[0]);
  print_parameter(--argc, ++argv);

  if (argc == 2) {
    if (argv[0] == NULL || argv[1] == NULL) {
      goto arg_err;
    } 
    else {
      sprintf(dspotter_cmd, "DSpotter %s %s", argv[0], argv[1]);
      intResult = system(dspotter_cmd);
    }
  } 
  else {
  arg_err:
    printf("Please enter the working model *.bin and variable 0/1.\r\n");
    intResult = -1;
  }

  PT_DIAG_SET(intResult);

  return intResult;
}

//diag cyberon cnsv -train pegatron
//diag cyberon cnsv -test
static int pegaDiag_cnsv(int argc, char **argv) {
    int intResult = 0;
	char cnsv_cmd[128];
	char cnsv_dataset_path[64] = "/cyberon/Data/Setting.ini";

	system("date -s '2025-09-04 12:00:00'");

	print_parameter(--argc, ++argv);
	printf("%s: %d, %s mode\r\n",__FUNCTION__, argc, argv[0]);

    if ((argc == 1) && (strcmp(argv[0], "-test") == 0)) {
		sprintf(cnsv_cmd, "CNSV %s %s", cnsv_dataset_path, argv[0]);
        intResult = system(cnsv_cmd);
    } 
	else if ((argc == 2) && (strcmp(argv[0], "-train") == 0)) {
		sprintf(cnsv_cmd, "CNSV %s %s %s", cnsv_dataset_path, argv[0], argv[1]);
        intResult = system(cnsv_cmd);
	}
    else {
arg_err:
        printf("Please enter the settings .ini file, working modes[-test/-train] or trainer name with training mode.\r\n");
        intResult = -1;
    }

    return intResult;
}

//diag cyberon cclever -train pegatron
//diag cyberon cclever -test
static int pegaDiag_cclever(int argc, char **argv) {
    int intResult = 0;
	char cclever_cmd[128];
	char cclever_dataset_path[64] = "/cyberon/Data/Setting.ini";

	system("date -s '2025-09-12 12:00:00'");

	print_parameter(--argc, ++argv);
#if ENABLE_DEBUG_MSG
	printf("%s: %d, %s mode\r\n",__FUNCTION__, argc, argv[0]);
#endif

    if ((argc == 1) && (strcmp(argv[0], "-test") == 0)) {
		sprintf(cclever_cmd, "CClever %s %s", cclever_dataset_path, argv[0]);
        intResult = system(cclever_cmd);
    } 
	else if ((argc == 2) && (strcmp(argv[0], "-train") == 0)) {
		sprintf(cclever_cmd, "CClever %s %s %s", cclever_dataset_path, argv[0], argv[1]);
        intResult = system(cclever_cmd);
	}
    else {
arg_err:
        printf("Please enter the settings .ini file, working modes[-test/-train] or trainer name with training mode.\r\n");
        intResult = -1;
    }

	//release system cache when finished CClever process
#if ENABLE_DEBUG_MSG
	system("echo 1 > /proc/sys/vm/drop_caches");
	printf("\r\n");
	printf("============================================\r\n");
	printf("=       Checking System Memory Cache       =\r\n");
	printf("============================================\r\n");
	system("cat /proc/meminfo");
#endif

    return intResult;
}

//diag cyberon creaderE2E
static int pegaDiag_creaderE2E(int argc, char **argv) {
    int intResult = 0;
	char creaderE2E_cmd[128];
	char creaderE2E_dataset_path[64] = "/cyberon/Data/TTS_Eng_UTF8.txt";

	system("date -s '2025-09-12 12:00:00'");

	print_parameter(--argc, ++argv);
#if ENABLE_DEBUG_MSG
	printf("%s: %d, %s mode\r\n",__FUNCTION__, argc, argv[0]);
#endif

	sprintf(creaderE2E_cmd, "CReaderE2E %s 0", creaderE2E_dataset_path);
	intResult = system(creaderE2E_cmd);

	//release system cache when finished CClever process
#if ENABLE_DEBUG_MSG
	system("echo 1 > /proc/sys/vm/drop_caches");
	printf("\r\n");
	printf("============================================\r\n");
	printf("=       Checking System Memory Cache       =\r\n");
	printf("============================================\r\n");
	system("cat /proc/meminfo");
#endif

    return intResult;
}

//diag cyberon creaderDNN
static int pegaDiag_creaderDNN(int argc, char **argv) {
    int intResult = 0;
	char creaderDNN_cmd[128];
	char creaderDNN_dataset_path[64] = "/cyberon/Data/TTS_Eng_UTF8.txt";

	system("date -s '2025-09-12 12:00:00'");

	print_parameter(--argc, ++argv);
#if ENABLE_DEBUG_MSG
	printf("%s: %d, %s mode\r\n",__FUNCTION__, argc, argv[0]);
#endif

	sprintf(creaderDNN_cmd, "CReaderDNN %s 0", creaderDNN_dataset_path);
	intResult = system(creaderDNN_cmd);

	//release system cache when finished CClever process
#if ENABLE_DEBUG_MSG
	system("echo 1 > /proc/sys/vm/drop_caches");
	printf("\r\n");
	printf("============================================\r\n");
	printf("=       Checking System Memory Cache       =\r\n");
	printf("============================================\r\n");
	system("cat /proc/meminfo");
#endif

    return intResult;
}

// diag matter commissioning <onnetwork|ble-wifi> <args...>
static int pegaDiag_MatterCommissioning(int argc, char **argv)
{
    int intResult = 0;	
    char cBuffer[512] = {0};

    print_parameter(argc, argv);

    if (argc < 1)
    {		
        printf("Usage: diag matter commissioning <on_network_device|wifi_device> <args...>\r\n");
        return -1;
    }

    int n = snprintf(cBuffer, sizeof(cBuffer), "/scripts/matter_commissioning.sh %s", argv[0]);

    for (int i = 1; i < argc && n < (int)sizeof(cBuffer) - 1; i++) {
        n += snprintf(cBuffer + n, sizeof(cBuffer) - n, " \"%s\"", argv[i]);
    }

    system(cBuffer);

    PT_DIAG_SET(intResult);

    return intResult;
}

// diag matter onoff <on|off> <node-id> <endpoint-id>
static int pegaDiag_MatterMatterOnOff(int argc, char **argv)
{
    int intResult = 0;	
    char *ACTION = NULL;
    int NODE_ID = 1;
    int ENDPOINT_ID = 1;

    char cBuffer[256] = {0};

    print_parameter(argc, argv);

    if (argc < 2)
    {		
        printf("Usage: diag matter onoff <on|off> <node-id> <endpoint-id>\r\n"); 	
        return -1;
    }

    if (argv[0] != NULL)
    {
        ACTION = argv[0];  // "on" | "off"
    }

    if (argv[1] != NULL)
    {
        NODE_ID = atoi(argv[1]); 
    }

    if (argv[2] != NULL)
    {
        ENDPOINT_ID = atoi(argv[2]); 
    }

    snprintf(cBuffer, sizeof(cBuffer),
             "/scripts/matter_onoff.sh %s %d %d",
             ACTION, NODE_ID, ENDPOINT_ID);

    system(cBuffer); 
		
    PT_DIAG_SET(intResult);
	
    return intResult;
}

// diag matter reset
static int pegaDiag_MatterReset(int argc, char **argv)
{
    int intResult = 0;	
    char cBuffer[128] = {0};

    print_parameter(--argc, ++argv);

    snprintf(cBuffer, sizeof(cBuffer), "/scripts/matter_reset.sh");

    system(cBuffer);

    PT_DIAG_SET(intResult);

    return intResult;
}

//diag demo on
static int pegaDiag_VoiceDemo_on(int argc, char **argv)
{
	int intResult = 0;	
	
	FILE *pFile = NULL;	
	
	if (access(DEMO_ENABLE_FILE, F_OK) == 0)   // Check if file exists	
	{
		printf("\n[error]Demo on already!");
		PT_DIAG_SET(-1);
		return 0;
	}
	    
    pFile = fopen(DEMO_ENABLE_FILE,"w+");	
	
	if (pFile != NULL)
	{		
		fprintf(pFile,"%d\n", 1);
		fclose(pFile); 
	}
	
    PT_DIAG_SET(intResult);
	
	sleep(2);
	
	system("reboot"); 
	
	return intResult;
}
//diag demo off
static int pegaDiag_VoiceDemo_off(int argc, char **argv)
{
	int intResult = 0;
	struct stat st = {0};
		
	if (access(DEMO_ENABLE_FILE, F_OK) == 0)   	//Check if file exists	
	 {
	    remove(DEMO_ENABLE_FILE);//remove file
	 }
	
    if (stat(DEMO_CYBERON_FOLDER, &st) == 0) 	//Check if folder exists
	 {	
		remove_dir(DEMO_CYBERON_FOLDER);
	 }	 
	 
    PT_DIAG_SET(intResult);
	
	return intResult;
}
// diag time enable
static int pegaDiag_SetTime(int argc, char **argv)
{
    int intResult = 0;	
    char cBuffer[128] = {0};

    print_parameter(--argc, ++argv);

    snprintf(cBuffer, sizeof(cBuffer), "/scripts/set_time.sh");

    system(cBuffer);

    PT_DIAG_SET(intResult);

    return intResult;
}

// diag bw dump
static int pegaDiag_DumpBandwidth(int argc, char **argv)
{
    int intResult = 0;	
    char cBuffer[128] = {0};

    print_parameter(--argc, ++argv);

    snprintf(cBuffer, sizeof(cBuffer), "cat /sys/devices/system/miu/miu0/bw");

    system(cBuffer);

    PT_DIAG_SET(intResult);

    return intResult;
}

// diag bw set [value]
static int pegaDiag_SetBandwidthLoop(int argc, char **argv)
{
    int intResult = 0;
	int intValue = 0;
    char cBuffer[128] = {0};

    print_parameter(--argc, ++argv);

	intValue = atoi(argv[0]);
	printf("Set BW loops: %d", intValue);

    snprintf(cBuffer, sizeof(cBuffer), "echo %d > /sys/devices/system/miu/miu0/bw", intValue);

    system(cBuffer);

    PT_DIAG_SET(intResult);

    return intResult;
}

// diag arbiter get
static int pegaDiag_ShowArbiter(int argc, char **argv)
{
    int intResult = 0;	
    char cBuffer[128] = {0};

    print_parameter(--argc, ++argv);

    snprintf(cBuffer, sizeof(cBuffer), "cat /sys/devices/system/miu/miu0/client");

    system(cBuffer);

    PT_DIAG_SET(intResult);

    return intResult;
}

// diag arbiter set [variable] [r/w] [id/name] [0/1]
static int pegaDiag_SetArbiter(int argc, char **argv)
{
    int intResult = 0;
	int intValue = 0;
    char cBuffer[128] = {0};

    print_parameter(--argc, ++argv);

	if (argc != 4)
	{
		printf("Error: input variable is invalid.\r\n");
		printf("How to set arbiter:\r\n");
		printf("diag arbiter set mask [r/w] [id/name] [0/1]\r\n");
		printf("diag arbiter set burst [r/w] [id/name] [0/1]\r\n");
		printf("diag arbiter set priority [r/w] [id/name] [0/1]\r\n");
		printf("diag arbiter set vp [r/w] [id/name] [0/1]\r\n");
		printf("diag arbiter set flowctrl [r/w] [id/name] [0/1]\r\n");
		printf("diag arbiter set urgent [r/w] [id/name] [0/1]\r\n");
		printf("diag arbiter set qos [r/w] [id/name] [0/1]\r\n");
		printf("diag arbiter set stall [r/w] [id/name] [0/1]\r\n");
		printf("diag arbiter set qos_init [r/w] [id/name] [0/1]\r\n");

		intResult = -1;
		return intResult;
	}

	intValue = atoi(argv[3]);

    snprintf(cBuffer, sizeof(cBuffer), "echo %s %s %s %d > /sys/devices/system/miu/miu0/client", argv[0], argv[1], argv[2], intValue);

    system(cBuffer);

    PT_DIAG_SET(intResult);

    return intResult;
}
//==============================================================================
//diag tftp ul 192.168.50.103 tmp.json
static int pegaDiag_tftp_file_ul(int argc, char **argv)
{
	int intResult = 0;	
					
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	if (argc == 2)
	{			
		pegaDiag_upload_file(argv[0], argv[1]);
	}
	else
	{
		intResult = -1;
	}		
		
	PT_DIAG_SET(intResult);    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag tftp dl 192.168.50.103 tmp.json
//diag tftp dl 192.168.50.103 tmp.json /data
static int pegaDiag_tftp_file_dl(int argc, char **argv)
{
	int intResult = 0;	
					
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	if (argc == 2)
	{			
		pegaDiag_download_file(argv[0], "/data", argv[1], argv[1]);
	}	
	else if (argc == 3)	
	{			
		pegaDiag_download_file(argv[0], argv[2], argv[1], argv[1]);
	}
	else
	{
		intResult = -1;
	}		
		
	PT_DIAG_SET(intResult);    
	//printf("\n A=%s",buff);	
	return intResult;
}
//==============================================================================
static void pegaDiag_mfg_data_check(void)
{
	struct stat statbuf;
		
	if (stat(ARLO_CERT_FILE, &statbuf) < 0) 
	{
		char tmp[2048] = { '\0' };
		pegaDiag_util_shell("cp -rf /arlo_cert/ /data", tmp, sizeof(tmp));
		//printf("%s\r\n",__FUNCTION__); 
	}
}
//diag mfg init
static int pegaDiag_mfg_init(int argc, char **argv)
{
	int intResult=0;	
	
	char tmp[2048] = { '\0' };
	
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
	
	intResult = pegaDiag_util_shell("cp -rf /arlo_cert/ /data", tmp, sizeof(tmp));
	
	//printf("\n A=%s",buff);
	
	PT_DIAG_SET(0);
	
	return intResult;
}
//diag mfg mac1
//diag mfg mac1 "A1:BB:CC:DD:EE:01"
static int pegaDiag_mfg_mac1(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[120]={0};
    char buff[120]={0};
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	pegaDiag_mfg_data_check();
	
	if (argv[0] != NULL)
	{
		snprintf(buff, sizeof(buff), "/scripts/mfg_item_write.sh MAC1 %s", argv[0]);
		system(buff); 
		PT_DIAG_SET(intResult);
	}	
	else
	{
		//sprintf(cmd, "grep '^MAC1=' /data/arlo_cert/dts_config.txt | cut -d'=' -f2");
		sprintf(cmd, "grep '^MAC1=' %s | cut -d'=' -f2", ARLO_CERT_FILE);
		if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
		{			
			PT_DIAG_GET(0, "%s", buff);
		}
	}
	    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg mac2
//diag mfg mac2 "A1:BB:CC:DD:EE:03"
static int pegaDiag_mfg_mac2(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[120]={0};
    char buff[120]={0};
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	pegaDiag_mfg_data_check();
	
	if (argv[0] != NULL)
	{
		snprintf(buff, sizeof(buff), "/scripts/mfg_item_write.sh MAC2 %s", argv[0]);
		system(buff); 
		PT_DIAG_SET(intResult);
	}	
	else
	{		
		sprintf(cmd, "grep '^MAC2=' %s | cut -d'=' -f2", ARLO_CERT_FILE);
		if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
		{			
			PT_DIAG_GET(0, "%s", buff);
		}
	}
	    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg sn
//diag mfg sn "AB5U247HA003D"
static int pegaDiag_mfg_sn(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[120]={0};
    char buff[120]={0};
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	pegaDiag_mfg_data_check();
	
	if (argv[0] != NULL)
	{
		snprintf(buff, sizeof(buff), "/scripts/mfg_item_write.sh SERIAL %s", argv[0]);
		system(buff); 
		PT_DIAG_SET(intResult);
	}	
	else
	{		
		sprintf(cmd, "grep '^SERIAL=' %s | cut -d'=' -f2", ARLO_CERT_FILE);
		if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
		{			
			PT_DIAG_GET(0, "%s", buff);
		}
	}
	    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg model
//diag mfg model "VMC6060"
static int pegaDiag_mfg_model(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[120]={0};
    char buff[120]={0};
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	pegaDiag_mfg_data_check();
	
	if (argv[0] != NULL)
	{
		snprintf(buff, sizeof(buff), "/scripts/mfg_item_write.sh MODEL %s", argv[0]);
		system(buff); 
		PT_DIAG_SET(intResult);
	}	
	else
	{		
		sprintf(cmd, "grep '^MODEL=' %s | cut -d'=' -f2", ARLO_CERT_FILE);
		if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
		{			
			PT_DIAG_GET(0, "%s", buff);
		}
	}
	    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg hwver
//diag mfg hwver "0.1"
static int pegaDiag_mfg_hwver(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[120]={0};
    char buff[120]={0};
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	pegaDiag_mfg_data_check();
	
	if (argv[0] != NULL)
	{
		snprintf(buff, sizeof(buff), "/scripts/mfg_item_write.sh HW_VERSION %s", argv[0]);
		system(buff); 
		PT_DIAG_SET(intResult);
	}	
	else
	{		
		sprintf(cmd, "grep '^HW_VERSION=' %s | cut -d'=' -f2", ARLO_CERT_FILE);
		if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
		{			
			PT_DIAG_GET(0, "%s", buff);
		}
	}
	    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg sku
//diag mfg sku "NAS-1001"
static int pegaDiag_mfg_sku(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[120]={0};
    char buff[120]={0};
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	pegaDiag_mfg_data_check();
	
	if (argv[0] != NULL)
	{
		snprintf(buff, sizeof(buff), "/scripts/mfg_item_write.sh SKU %s", argv[0]);
		system(buff); 
		PT_DIAG_SET(intResult);
	}	
	else
	{		
		sprintf(cmd, "grep '^SKU=' %s | cut -d'=' -f2", ARLO_CERT_FILE);
		if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
		{			
			PT_DIAG_GET(0, "%s", buff);
		}
	}
	    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg wifi_cc
//diag mfg wifi_cc "US"
static int pegaDiag_mfg_wifi_cc(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[120]={0};
    char buff[120]={0};
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	pegaDiag_mfg_data_check();
	
	if (argv[0] != NULL)
	{
		snprintf(buff, sizeof(buff), "/scripts/mfg_item_write.sh WIFI_CC %s", argv[0]);
		system(buff); 
		PT_DIAG_SET(intResult);
	}	
	else
	{		
		sprintf(cmd, "grep '^WIFI_CC=' %s | cut -d'=' -f2", ARLO_CERT_FILE);
		if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
		{			
			PT_DIAG_GET(0, "%s", buff);
		}
	}
	    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg uuid
//diag mfg uuid xx
//diag mfg uuid "12345678-1234-1234-1234-123456789acc"
static int pegaDiag_mfg_uuid(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[120]={0};
    char buff[320]={0};
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	pegaDiag_mfg_data_check();
	
	if (argv[0] != NULL)
	{		
		if (!strcmp(argv[0],"xx"))//Random to get uuid from linux kernel
		{
			char uuid[120]={0};
			sprintf(cmd, "cat /proc/sys/kernel/random/uuid");			
			if (pegaDiag_util_shell(cmd, uuid, sizeof(uuid)) >= 0)
			{					
				if (strlen(uuid) > 10)
				{
					snprintf(cmd, sizeof(cmd), "/scripts/mfg_item_write.sh UUID %s", uuid);
					pegaDiag_util_shell(cmd, buff, sizeof(buff));				
				}
			}
		}
		else
		{
			snprintf(cmd, sizeof(cmd), "/scripts/mfg_item_write.sh UUID %s", argv[0]);
			pegaDiag_util_shell(cmd, buff, sizeof(buff));		
		}
		
		PT_DIAG_SET(intResult);
	}	
	else
	{		
		sprintf(cmd, "grep '^UUID=' %s | cut -d'=' -f2", ARLO_CERT_FILE);		
		if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
		{			
			PT_DIAG_GET(0, "%s", buff);
		}			
	}
	    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg partnerid
//diag mfg partnerid "1"
static int pegaDiag_mfg_partnerid(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[120]={0};
    char buff[120]={0};
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	pegaDiag_mfg_data_check();
	
	if (argv[0] != NULL)
	{
		snprintf(buff, sizeof(buff), "/scripts/mfg_item_write.sh PARTNER_ID %s", argv[0]);
		system(buff); 
		PT_DIAG_SET(intResult);
	}	
	else
	{		
		sprintf(cmd, "grep '^PARTNER_ID=' %s | cut -d'=' -f2", ARLO_CERT_FILE);		
		if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
		{			
			PT_DIAG_GET(0, "%s", buff);
		}	
	}
	    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg erase
static int pegaDiag_mfg_erase(int argc, char **argv)
{
	int intResult = 0;	
	
	char buff[2048] = { '\0' };
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 	
	
	if (pegaDiag_util_shell("rm -rf /data/arlo_cert", buff, sizeof(buff)) >= 0)
	{			
	    PT_DIAG_SET(intResult);  
	}
		 
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg show
static int pegaDiag_mfg_show(int argc, char **argv)
{
	int intResult = 0;	
	
	char buff[2048] = { '\0' };
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	pegaDiag_mfg_data_check();
	
	if (pegaDiag_util_shell("cat /data/arlo_cert/dts_config.txt", buff, sizeof(buff)) >= 0)
	{			
	    PT_DIAG_GET(0, "%s", buff);
	}
	    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg cfg_dl 192.168.50.103 config_exp.txt
static int pegaDiag_mfg_cfg_file_dl(int argc, char **argv)
{
	int intResult = 0;	
					
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	if (argc == 2)
	{	        		
		pegaDiag_mfg_data_check();
		pegaDiag_download_file(argv[0], "/data/arlo_cert", argv[1], "dts_config.txt");		
	}
	else
	{
		intResult = -1;
	}		
		
	PT_DIAG_SET(intResult);    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg json_dl 192.168.50.103 dsc4.json
static int pegaDiag_mfg_dsc4json_dl(int argc, char **argv)
{
	int intResult = 0;	
					
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc); 
	
	if (argc == 2)
	{	
        struct stat st = {0};
        char pFile[240] = {0};
		char buff[320] = {0};
		char tmp[2048] = { '\0' };
		
		pegaDiag_mfg_data_check();
		pegaDiag_download_file(argv[0], "/data/arlo_cert", argv[1], argv[1]);
		
		snprintf(pFile, sizeof(pFile), "/data/arlo_cert/%s", argv[1]);
		
		if (access(pFile, F_OK) == 0)//Check if file exists	
		{
			snprintf(buff, sizeof(buff), "asl_encrypt %s /data/arlo_cert/dsc4_encrypt.crt", pFile);
			pegaDiag_util_shell(buff, tmp, sizeof(tmp));
		}
	}
	else
	{
		intResult = -1;
	}		
		
	PT_DIAG_SET(intResult);    
	//printf("\n A=%s",buff);	
	return intResult;
}
//diag mfg dtbo
static int pegaDiag_mfg_dtbo(int argc, char **argv)
{
	int intResult = 0;	
	
	char cmd[256] = {0};
			
	print_parameter(--argc, ++argv);	
	//printf("%s(%d)\r\n",__FUNCTION__, argc);     
   
    sprintf(cmd, "sh /scripts/arlo_dtbo_generate.sh");
    system(cmd);
		
	PT_DIAG_SET(intResult);
	    
	//printf("\n A=%s",buff);	
	return intResult;
}
//==============================================================================
static int pegaDiag_dtbo_is_exist(void)
{
	struct stat st = {0};
	
	if (stat(DTBO_FOLDER, &st) == 0) 	//Check if folder exists
	 {	
		return 0;
	 }	 	
	 
	 return -1;
}

//diag dtbo init
static int pegaDiag_dtbo_init(int argc, char **argv)
{
	int intResult = 0;
    char tmp[1024] = {0};	
	//printf("%s\r\n",__FUNCTION__); 
	print_parameter(--argc, ++argv);
		
    pegaDiag_util_shell("modprobe dtbloader", tmp, sizeof(tmp));	
	
    if (pegaDiag_dtbo_is_exist() != 0) 	//Check if folder not exists
	 {	
		printf("DTBO init error.\n"); 
	 }	 	
		
	PT_DIAG_SET(intResult);
		
	return intResult;
}
//diag dtbo deinit
static int pegaDiag_dtbo_deinit(int argc, char **argv)
{
	int intResult = 0;
    char tmp[1024] = {0};
		
	//printf("%s\r\n",__FUNCTION__); 
			
    pegaDiag_util_shell("rmmod dtbloader", tmp, sizeof(tmp));	
	
    if (pegaDiag_dtbo_is_exist() == 0) 	//Check if folder exists
	 {	
		printf("DTBO deinit error.\n"); 
	 }	
	 
	PT_DIAG_SET(intResult);
			
	return intResult;
}
//diag dtbo mac1
//diag dtbo mac2
//diag dtbo model
//diag dtbo sn
//diag dtbo hw
//diag dtbo sku
//diag dtbo wifi_cc
//diag dtbo uuid
//diag dtbo pid
//diag dtbo info
//diag dtbo erase
static int pegaDiag_dtbo_info_read(int argc, char **argv)
{
	int intResult = 0;
    char cmd[120]={0};
    char buff[1024] = {0};
		
	//printf("%s[%s]\r\n",__FUNCTION__, argv[0]); 			
    
    if (pegaDiag_dtbo_is_exist() != 0) 	//Check if folder exists
	 {	
		printf("DTBO is not exist.\n"); 
	 }	
		
    if (!strcmp(argv[0],"mac1"))
	{
		sprintf(cmd, "cat %s/%s", DTBO_FOLDER, "mac1");
	}
	else if (!strcmp(argv[0],"mac2"))
	{
		sprintf(cmd, "cat %s/%s", DTBO_FOLDER, "mac2");
	}
	else if (!strcmp(argv[0],"model"))
	{
		sprintf(cmd, "cat %s/%s", DTBO_FOLDER, "model");
	}
	else if (!strcmp(argv[0],"sn"))
	{
		sprintf(cmd, "cat %s/%s", DTBO_FOLDER, "serial");
	}
	else if (!strcmp(argv[0],"hw"))
	{
		sprintf(cmd, "cat %s/%s", DTBO_FOLDER, "hw_version");
	}
	else if (!strcmp(argv[0],"sku"))
	{
		sprintf(cmd, "cat %s/%s", DTBO_FOLDER, "sku");
	}
	else if (!strcmp(argv[0],"wifi_cc"))
	{
		sprintf(cmd, "cat %s/%s", DTBO_FOLDER, "wifi_cc");
	}
	else if (!strcmp(argv[0],"uuid"))
	{
		sprintf(cmd, "cat %s/%s", DTBO_FOLDER, "uuid");
	}
	else if (!strcmp(argv[0],"pid"))
	{
		sprintf(cmd, "cat %s/%s", DTBO_FOLDER, "partner_id");
	}
	else if (!strcmp(argv[0],"info"))
	{
		sprintf(cmd, "ls -l /proc/device-tree/device_info/");		
	}
	else if (!strcmp(argv[0],"erase"))
	{
		//sprintf(cmd, "flash_eraseall /dev/mtd2");
		sprintf(cmd, "flash_erase /dev/mtd2 %d %d", 0, 0);
	}
	else
	{
		printf("Command(%s) error.\n", argv[0]); 
		return -1;
	}
	
	if (pegaDiag_util_shell(cmd, buff, sizeof(buff)) >= 0)
	{			
	    PT_DIAG_GET(0, "%s", buff);
	}
			
	return intResult;
}
//==============================================================================
static command_line_t m_eBurninCommand[] = 
{
	{ "on",		"burnin enable",		pegaDiag_BurnIn_control,		NULL},
	{ "off",	"burnin disable",		pegaDiag_BurnIn_control,		NULL},	
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eVersionCommand[] = 
{
	{ "sw",		"sw version",		pegaDiag_IspFwVersionGet,		NULL},
	{ "wifi",	"wifi version",		pegaDiag_WifiVersionGet,		NULL},	
	{ NULL, NULL, NULL, NULL }
};
//==============================================================================
static command_line_t m_eIrcutCommand[] = 
{
	{ "day",	"ircut day mode set",			pegaDiag_ircut_control,		NULL},
	{ "night",	"ircut night mode set",			pegaDiag_ircut_control,		NULL},	
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eIrLedCommand[] = {
	{ "on",		"irled on",				pegaDiag_IrLedControl,		NULL},
	{ "off",	"irled off",			pegaDiag_IrLedControl,		NULL},
	{ "duty",	"irled duty percent",	pegaDiag_IrLedDutySet,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eSpotLedCommand[] = {
	{ "on",		"spot on",		pegaDiag_SpotLed_Set,		NULL},
	{ "off",	"spot off",		pegaDiag_SpotLed_Set,		NULL},	
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eLedCommand[] = {
	{ "on",		"Led_All on",		pegaDiag_Led_Set,			NULL},
	{ "off",	"Led_All off",		pegaDiag_Led_Set,			NULL},	
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eLedRCommand[] = {
	{ "on",		"Led_R on",			pegaDiag_LedR_Set,			NULL},
	{ "off",	"Led_R off",		pegaDiag_LedR_Set,			NULL},		
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eLedGCommand[] = {
	{ "on",		"Led_G on",			pegaDiag_LedG_Set,			NULL},
	{ "off",	"Led_G off",		pegaDiag_LedG_Set,			NULL},		
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eLedBCommand[] = 
{
	{ "on",		"Led_B on",			pegaDiag_LedB_Set,			NULL},
	{ "off",	"Led_B off",		pegaDiag_LedB_Set,			NULL},		
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eAlsCommand[] = 
{
	{ "id",			"als id",			pegaDiag_Als_Control,			NULL},
	{ "day",		"als day mode",		pegaDiag_Als_Control,			NULL},
	{ "night",		"als night mode",	pegaDiag_Als_Control,			NULL},
	{ "value",		"als value",		pegaDiag_Als_Control,			NULL},
	{ "on",			"als enable",		pegaDiag_Als_Control,   		NULL},
	{ "off",		"als disable",		pegaDiag_Als_Control,   		NULL},
	{ "calidata",	"als calidata",		pegaDiag_AlsCalidata,   		NULL},	
	{ "gain",	    "als gain get",     pegaDiag_AlsCalidata_Get,       NULL},
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

static command_line_t m_eWifiCfgdataCommand[] = {
	{ "command:",	"wifi cfgdata get [wifi1/wifi2]",			NULL,			NULL},
	{ "get",	"wifi cfgdata get xx",								pegaDiag_WifiCfgdataGet,	NULL},
	{ "command:",	"wifi cfgdata set [wifi1/wifi2] [cfgdata]",	NULL,			NULL},
	{ "set",	"wifi cfgdata set xx xx",							pegaDiag_WifiCfgdataSet,	NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eWifiCountrycodeCommand[] = {
	{ "command:",	"wifi countrycode get [mlan0/mlan1]",					NULL,			NULL},
	{ "get",		"wifi countrycode get xx",								pegaDiag_WifiCountrycodeGet,	NULL},
	{ "command:",	"wifi countrycode set [mlan0/mlan1] [countrycode]",		NULL,			NULL},
	{ "set",		"wifi countrycode set xx xx",							pegaDiag_WifiCountrycodeSet,	NULL},
	{ "show",		"wifi countrycode show",								pegaDiag_WifiCountrycodeShow,	NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eWifiCommand[] = 
{
	{ "on",			"wifi on",						pegaDiag_WifiOn,			NULL},
	{ "off",		"wifi off",						pegaDiag_WifiOff,			NULL},
	{ "mfg",		"wifi mfg fw on",				pegaDiag_WifiMfg,			NULL},
	{ "connect",	"wifi connect ssid password",	pegaDiag_WifiConnect,		NULL},		
	{ "connect2",	"wifi connect ssid password",	pegaDiag_WifiConnect2,		NULL},		
	{ "status",		"wifi connection status",		pegaDiag_WifiStatus,		NULL},		
	{ "rssi",		"wifi rssi show",				pegaDiag_WifiRssiGet,		NULL},		
	{ "signal",		"wifi signal show",				pegaDiag_WifiSignalGet,		NULL},		
	{ "datarate",	"wifi datarate show",			pegaDiag_WifiDatarateGet,	NULL},
	{ "chanstats",	"wifi channel status show",		pegaDiag_WifiChanstatsGet,	NULL},
	{ "scantable",	"wifi scan table show",			pegaDiag_WifiScantableGet,	NULL},
	{ "mac",		"wifi mac address",				pegaDiag_WifiMacGet,		NULL},
	{ "interface",	"wifi interface",				pegaDiag_WifiInterfaceGet,	NULL},
	{ "ssid",		"wifi ssid command",			NULL,						m_eWifiSsidCommand},	
	{ "password",	"wifi password",				NULL,						m_eWifiPasswordCommand},	
	{ "ip_addr",	"wifi ip address",				NULL,						m_eWifiIPAddrCommand},	
	{ "ip_mask",	"wifi ip mask",					NULL,						m_eWifiIPMaskCommand},
	{ "cfgdata",	"wifi cfgdata",					NULL,						m_eWifiCfgdataCommand},
	{ "countrycode","wifi countrycode",				NULL,						m_eWifiCountrycodeCommand},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eBleCommand[] = 
{
	{ "on",			"BLE bring up",					pegaDiag_BleOn,			NULL},
	{ "service_on",	"BLE service on",				pegaDiag_BleServiceOn,	NULL},
	//{ "off",		"BLE off",						pegaDiag_BleOff,		NULL},		
	//{ "scan",		"BLE scan",						pegaDiag_BleScan,		NULL},	
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
	{ "init",		"init amplifier",						pegaDiag_AudioInit,			NULL},
	{ "record",		"record audio file",					pegaDiag_AudioRecordFile,	NULL},
	{ "play",		"play audio file",						pegaDiag_AudioPlayFile,		NULL},
	{ "play_wav",	"play audio wav file",					pegaDiag_AudioPlayWavFile,	NULL},
	//{ "stop",		"stop play file",						pegaDiag_AudioStop,			NULL},
	//{ "restart",	"restart audio file",					pegaDiag_AudioReStart,		NULL},
	{ "volume",		"amplifier gain adjust(xx:8.5~22.0)dB",	pegaDiag_AudioVolumeSet,	NULL},
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
	{ "on_2stage_breath",		"Breathing only depend on hw fade rate",	pegaDiag_RingLedControl,		NULL},
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
	{ "enable_tap",          	"start TAP event",          pegaDiag_AccControl,        NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eNFCCommand[] = 
{		
	{ "type",		"Read NFC type",			pegaDiag_NFC_Control,		NULL},
	{ "uid",		"Read NFC device UID",		pegaDiag_NFC_Control,		NULL},
	{ "data",		"Read NFC device data",		pegaDiag_NFC_Control,		NULL},
	{ "clear",		"Clear NFC data",			pegaDiag_NFC_Control,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t koMotorRisingCommand[] = 
{	
	{ "command:",	"forward [step:0~]",	NULL,			NULL},	
	{ "example:",	"forward 6",			NULL,			NULL},	
	{ "forward",	"move forward",			pegaDiag_rising_motorko_forward,	NULL},
	{ "command:",	"reverse [step:0~]",	NULL,			NULL},	
	{ "example:",	"reverse 4",			NULL,			NULL},	
	{ "reverse",	"move reverse",			pegaDiag_rising_motorko_reverse,	NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t koMotorPanCommand[] = 
{	
	{ "command:",	"forward [step:0~]",	NULL,			NULL},	
	{ "example:",	"forward 34",			NULL,			NULL},
	{ "forward",	"move forward",			pegaDiag_pan_motorko_forward,		NULL},	
	{ "command:",	"reverse [step:0~]",	NULL,			NULL},	
	{ "example:",	"reverse 34",			NULL,			NULL},
	{ "reverse",	"move reverse",			pegaDiag_pan_motorko_reverse,		NULL},
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eMotorKoCommand[] = 
{	
	{ "rising",		"rising motor",			NULL,		koMotorRisingCommand},
	{ "pan",		"pan motor",			NULL,		koMotorPanCommand},
	{ NULL, NULL, NULL, NULL }	
};


static command_line_t m_eStreamingCommand[] = {	
	{ "on",			"RTSP streaming output",		pegaDiag_streaming_on,			NULL},	
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eRFTestCommand[] = {
	{ "on",			"RF test on",					pegaDiag_rfteset_on,			NULL},	
	{ "off",		"RF test off",					pegaDiag_rfteset_off,			NULL},	
	{ "wifi1",		"Labtool mfgbridge wifi1",		pegaDiag_rfteset_wifi1,			NULL},	
	{ "wifi2",		"Labtool mfgbridge wifi2",		pegaDiag_rfteset_wifi2,			NULL},	
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eCyberonDemoAppCommand[] = {
    //{"dspotter", "demo voice wake up",     pegaDiag_dspotter, NULL}, //Removed dspotter program to save rootfs storage.
	//{"cnsv",     "demo voice recognition", pegaDiag_cnsv,     NULL}, //Removed cnsv program to save rootfs storage.
	{"cclever",  "demo voice command",     pegaDiag_cclever,  NULL},
	{"creaderE2E",  "demo text to speech, E2E version",    pegaDiag_creaderE2E,  NULL},
	{"creaderDNN",  "demo text to speech, DNN version",    pegaDiag_creaderDNN,  NULL},
    {NULL, NULL, NULL, NULL}
};

static command_line_t m_eMatterCommissioningCommand[] = {
    {"on_network_device", 	"start commissioning on network device",	pegaDiag_MatterCommissioning, 	NULL},
	{"wifi_device",     	"start commissioning wifi device", 			pegaDiag_MatterCommissioning,     	NULL},
    {NULL, NULL, NULL, NULL}
};

static command_line_t m_eMatterOnOffCommand[] = {
    {"on", 			"turn on device",	pegaDiag_MatterMatterOnOff, 	NULL},
	{"off",     	"turn off device", 	pegaDiag_MatterMatterOnOff,     NULL},
    {NULL, NULL, NULL, NULL}
};


static command_line_t m_eMatterCommand[] = {
    {"commissioning", 	"start commissioning",		 	NULL,     		m_eMatterCommissioningCommand},
	{"onoff",     		"turn on/off matter device",    NULL,			m_eMatterOnOffCommand},
	{"reset",     		"reset matter device", 			pegaDiag_MatterReset,     		NULL},
    {NULL, NULL, NULL, NULL}
};

static command_line_t m_eVoiceDemoCommand[] = {
	{ "on",			"demo test on",					pegaDiag_VoiceDemo_on,			NULL},	
	{ "off",		"demo test off",				pegaDiag_VoiceDemo_off,			NULL},		
	{ NULL, NULL, NULL, NULL }
};

static command_line_t m_eTimeCommand[] = {
    {"enable", 	"set time",     		pegaDiag_SetTime, 	NULL},
    {NULL, NULL, NULL, NULL}
};

static command_line_t m_eBandwidthTestCommand[] = {
    {"dump", 	"dump client bandwidth measurement",							pegaDiag_DumpBandwidth, 	NULL},
	{"set", 	"set client bandwidth loop value, Default:32, Maximum:65536",	pegaDiag_SetBandwidthLoop, 	NULL},
    {NULL, NULL, NULL, NULL}
};

static command_line_t m_eArbiterTestCommand[] = {
	{"get",		 "show client arbiter variable table",							pegaDiag_ShowArbiter,	 	NULL},
	{"set",		 "set client arbiter variable table",							pegaDiag_SetArbiter,	 	NULL},
    {NULL, NULL, NULL, NULL}
};

static command_line_t m_eTftpFileCommand[] = {
	{"ul",	 "Tftp file upload",							pegaDiag_tftp_file_ul,	 	NULL},
	{"dl",	 "Tftp file download",							pegaDiag_tftp_file_dl,	 	NULL},
    {NULL, NULL, NULL, NULL}
};

static command_line_t m_eManufactureCommand[] = {
	{"init",	 "Manufacture data init",							pegaDiag_mfg_init,	 		NULL},
	{"mac1",	 "WiFi mac1 read/write",							pegaDiag_mfg_mac1,	 		NULL},
	{"mac2",	 "WiFi mac2 read/write",							pegaDiag_mfg_mac2,	 		NULL},
	{"sn",		 "Serial number read/write",						pegaDiag_mfg_sn,	 		NULL},
	{"model",	 "Model read/write",								pegaDiag_mfg_model,	 		NULL},
	{"hwver",	 "HW version read/write",							pegaDiag_mfg_hwver,	 		NULL},
	{"sku",	 	 "SKU read/write",									pegaDiag_mfg_sku,	 		NULL},
	{"wifi_cc",	 "WiFi country code read/write",					pegaDiag_mfg_wifi_cc,	 	NULL},	
	{"uuid",	 "UUID read/write",									pegaDiag_mfg_uuid,	 		NULL},
	{"partnerid","Partner ID read/write",							pegaDiag_mfg_partnerid,	 	NULL},
	{"erase",	 "Erase manufacture data",							pegaDiag_mfg_erase,	 		NULL},
	{"show",	 "Show manufacture data",							pegaDiag_mfg_show,	 		NULL},
	{"cfg_dl",   "Download config file",							pegaDiag_mfg_cfg_file_dl,	NULL},
	{"json_dl",  "Download dsc4 json file",							pegaDiag_mfg_dsc4json_dl,	NULL},
	{"dtbo",  	 "To generate the DTBO file",						pegaDiag_mfg_dtbo,			NULL},
    {NULL, NULL, NULL, NULL}
};

static command_line_t m_eDevTreeOverlayCommand[] = {
	{"init",	 "Device tree overlay init",						pegaDiag_dtbo_init,	 		NULL},
	{"deinit",	 "Device tree overlay deinit",						pegaDiag_dtbo_deinit, 		NULL},
	{"mac1",  	 "Read DTBO mac1 address",							pegaDiag_dtbo_info_read,	NULL},
	{"mac2",  	 "Read DTBO mac2 address",							pegaDiag_dtbo_info_read,	NULL},
	{"model",  	 "Read DTBO model name",							pegaDiag_dtbo_info_read,	NULL},
	{"sn",  	 "Read DTBO serial number",							pegaDiag_dtbo_info_read,	NULL},
	{"hw",  	 "Read DTBO hw version",							pegaDiag_dtbo_info_read,	NULL},
	{"sku",  	 "Read DTBO sku",									pegaDiag_dtbo_info_read,	NULL},
	{"wifi_cc",  "Read DTBO wifi country code",						pegaDiag_dtbo_info_read,	NULL},
	{"uuid",  	 "Read DTBO uuid",									pegaDiag_dtbo_info_read,	NULL},
	{"pid",		 "Read DTBO partner id",							pegaDiag_dtbo_info_read,	NULL},
	{"info",	 "Show DTBO items",									pegaDiag_dtbo_info_read,	NULL},
	{"erase",	 "Erase DTBO data",									pegaDiag_dtbo_info_read,	NULL},
    {NULL, NULL, NULL, NULL}
};
//==============================================================================
static command_line_t m_eDiagMainCommand[] = 
{
	//{ "burnin",		"burnin command",		NULL,				&m_eBurninCommand},
	{ "ver",		"version command",		NULL,				&m_eVersionCommand},
	{ "ircut",		"ircut command",		NULL,				&m_eIrcutCommand},
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
	{ "ringled", 	"ringled commad",		NULL,				&m_eRingLEDCommand},
	{ "acc", 		"acc commad",			NULL,				&m_eAccCommand},
	{ "nfc", 		"nfc commad",			NULL,				&m_eNFCCommand},	
	{ "motor_ko", 	"motor kerne module commad",	NULL,		&m_eMotorKoCommand},
	{ "streaming", 	"streaming commad",		NULL,				&m_eStreamingCommand},
	{ "rftest", 	"rftest commad",		NULL,				&m_eRFTestCommand},
	{ "cyberon", 	"Cyberon demo app commad",	NULL,			&m_eCyberonDemoAppCommand},
	{ "matter", 	"matter commad",		NULL,				&m_eMatterCommand},
	{ "demo", 		"voice command demo",	NULL,				&m_eVoiceDemoCommand},
	{ "time", 		"time commad",			NULL,				&m_eTimeCommand},
	{ "bw", 		"bandwidth measurement command",	NULL,	&m_eBandwidthTestCommand},
	{ "arbiter", 	"arbiter measurement command",		NULL,	&m_eArbiterTestCommand},
	{ "tftp", 		"tftp file transmission command",	NULL,	&m_eTftpFileCommand},
	{ "mfg", 		"menufacture items command",		NULL,	&m_eManufactureCommand},
	{ "dtbo", 		"Device tree overlay command",		NULL,	&m_eDevTreeOverlayCommand},
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

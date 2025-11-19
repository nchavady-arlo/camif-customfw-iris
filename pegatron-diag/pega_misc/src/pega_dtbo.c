/*******************************************************************************
* File Name: pega_dtbo.c
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
//==============================================================================
#include "pega_dtbo.h"
//==============================================================================
#define DEVICE_INFO_PATH "/sys/firmware/devicetree/base/device_info"
//==============================================================================
static stDevice_info m_stDevInfo;
//==============================================================================
// 去除 '\n' 或 '\r'
static void trim_newline(char *s) 
{
    for (char *p = s; *p; p++) 
	{
        if (*p == '\n' || *p == '\r') {
            *p = '\0';
            break;
        }
    }
}
// 讀取 sysfs 檔案
static int read_property(const char *path, char *dst, size_t len) 
{
    FILE *fp = fopen(path, "r");
    if (!fp)
        return -1;
    if (!fgets(dst, len, fp)) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    trim_newline(dst);
    return 0;
}
// 解析 device_info
int parse_device_info(stDevice_info *info) 
{
	char path[256];
	
    memset(info, 0, sizeof(*info));

    struct {
        const char *name;
        char *dest;
		size_t len;
    } fields[] = 
	{       
        {"hw_version",  info->hw_ver,	STR_MAX_LEN},
        {"mac1",        info->mac1,		STR_MAC_LEN},
        {"mac2",        info->mac2, 	STR_MAC_LEN},
        {"model",       info->model, 	STR_MAX_LEN},        
        {"partner_id",  info->pid,		STR_PID_LEN},
        {"serial",      info->sn,		STR_MAX_LEN},
        {"sku",         info->sku,		STR_MAX_LEN},
        {"uuid",        info->uuid,		STR_UUID_LEN},
        {"wifi_cc",     info->wifi_cc,	STR_WIFI_CC_LEN},
    };
    
    for (size_t i = 0; i < sizeof(fields)/sizeof(fields[0]); i++) 
	{
        snprintf(path, sizeof(path), "%s/%s", DEVICE_INFO_PATH, fields[i].name);
		
        if (read_property(path, fields[i].dest, fields[i].len) != 0) 
		{                       
			fprintf(stderr, "Failed to parse %s\n", fields[i].name);
			return -1;
        }
    }

    return 0;
}

int Pega_dtbo_init(void)
{   
    struct stat st = {0};
	
	if (stat(DEVICE_INFO_PATH, &st) != 0) 	//Check if folder exists
	 {	
		return -1;
	 }	 
	 
    if (parse_device_info(&m_stDevInfo) != 0) 
	{
        fprintf(stderr, "Failed to parse stDevice_info\n");
        return -1;
    }   

    m_stDevInfo.bDataReady = 1;
	
    return 0;
}

int Pega_dtbo_data_get(stDevice_info* pInfo)
{
	memcpy(pInfo, &m_stDevInfo, sizeof(stDevice_info));
	
	return m_stDevInfo.bDataReady;
}
//pega_debug debug info dtbo
void Pega_dtbo_data_info(void) 
{
	 Pega_dtbo_data_print(m_stDevInfo);	
}

void Pega_dtbo_data_print(stDevice_info info) 
{
	 #if 1
     printf("=== Device Info ===\n");
	 printf("bDataReady      : %d\n", info.bDataReady);
	 printf("-------------------------------\n");
     printf("hw_version(%02d): %s\n", strlen(info.hw_ver), info.hw_ver);
     printf("mac1(%02d)      : %s\n", strlen(info.mac1),info.mac1);
     printf("mac2(%02d)      : %s\n", strlen(info.mac2),info.mac2);
     printf("model(%02d)     : %s\n", strlen(info.model),info.model);    
     printf("partner_id(%02d): %s\n", strlen(info.pid),info.pid);
     printf("serial num(%02d): %s\n", strlen(info.sn),info.sn);
     printf("sku(%02d)       : %s\n", strlen(info.sku),info.sku);
     printf("uuid(%02d)      : %s\n", strlen(info.uuid),info.uuid);
     printf("wifi_cc(%02d)   : %s\n", strlen(info.wifi_cc),info.wifi_cc);
	 #else
	 printf("=== Device Info ===\n");
     printf("hw_version   : %s\n", info.hw_ver);
     printf("mac1         : %s\n", info.mac1);
     printf("mac2         : %s\n", info.mac2);
     printf("model        : %s\n", info.model);    
     printf("partner_id   : %s\n", info.pid);
     printf("serial num   : %s\n", info.sn);
     printf("sku          : %s\n", info.sku);
     printf("uuid         : %s\n", info.uuid);
     printf("wifi_cc      : %s\n", info.wifi_cc);	 
	 #endif
}
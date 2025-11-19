#ifndef _PEGA_DTBO_H
#define _PEGA_DTBO_H
//==============================================================================
#define STR_MAX_LEN 		32
#define STR_MAC_LEN 		19
#define STR_WIFI_CC_LEN 	 3
#define STR_PID_LEN 	 	 2
#define STR_UUID_LEN 	 	42
//==============================================================================
// Device Info 結構
typedef struct
{    
    char bDataReady;
    char hw_ver[STR_MAX_LEN];
    char mac1[STR_MAC_LEN];
    char mac2[STR_MAC_LEN];
    char model[STR_MAX_LEN];    
    char sn[STR_MAX_LEN];
    char sku[STR_MAX_LEN];
    char uuid[STR_UUID_LEN];    
    char wifi_cc[STR_WIFI_CC_LEN];
	char pid[STR_PID_LEN]; //partner id
}stDevice_info;
//==============================================================================
int  Pega_dtbo_init(void);
//==============================================================================
int  Pega_dtbo_data_get(stDevice_info* pInfo);
//==============================================================================
void Pega_dtbo_data_info(void);
//==============================================================================
void Pega_dtbo_data_print(stDevice_info info);
//==============================================================================
#endif//_PEGA_DTBO_H

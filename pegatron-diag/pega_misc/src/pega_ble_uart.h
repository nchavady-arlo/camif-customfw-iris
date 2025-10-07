#ifndef _PEGA_BLE_UART_H_
#define _PEGA_BLE_UART_H_
//==============================================================================
#include "pega_defines.h"
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#define BLE_UART_PORT	    	"/dev/ttyS2"
//==============================================================================
#define UART_BUFFER_MAX_LEN 					64//128
//==============================================================================
int  Pega_Ble_uart_device_open(void);
void Pega_Ble_uart_device_close(void);
void Pega_Ble_uart_init(unsigned int u32BaudRate, int bRTSCTS_En);
int  Pega_Ble_uart_BringUp_Ready(int intTimeout_ms);
//==============================================================================
void Uart_Buffer_Data_Print(int UarRcvLen);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_BLE_UART_H_

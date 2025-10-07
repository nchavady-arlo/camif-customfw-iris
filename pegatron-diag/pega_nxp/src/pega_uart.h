#ifndef _DRV_UART_H
#define _DRV_UART_H
//==============================================================================
#include <stdbool.h>
#include <stdint.h>
//==============================================================================
#define UART_BUFFER_MAX_LEN 					128//2048//1024 
#define DEFAULT_TIMEOUT_MS						300
//==============================================================================
bool Uart_DeviceOpen(char *u8DeviceName);
void Uart_DeviceClose(void);
void Uart_InitialParameter(unsigned int u32BaudRate, int bRTSCTS_En);
int  UART_ReceiveHandler(void);
void Uart_WaitAndSaveDataToBuffer(int intTimeout_ms);
//==============================================================================
void Uart_Buffer_Data_Print(int UarRcvLen);
//==============================================================================

#endif

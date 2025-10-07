#ifndef _DRV_UART_H
#define _DRV_UART_H
//==============================================================================
#include <stdbool.h>
#include <stdint.h>
//==============================================================================
#define UART_BUFFER_MAX_LEN 					128//2048//1024 
#define DEFAULT_TIMEOUT_MS						300
//#define RECEIVE_DONE_STRING					"OK"
#define RECEIVE_DONE_STRING						"sa"
//==============================================================================
bool Uart_DeviceOpen(char *u8DeviceName);
void Uart_DeviceClose(void);
void Uart_InitialParameter(unsigned int u32BaudRate, int bRTSCTS_En);
int  UART_SendBuffer(const char *pCommand, unsigned int length) ;
void UART_ReceiveDataBytesGet(char *pResponse, unsigned int u8Length);
int  UART_ReceiveDataLengthGet(void);
void UART_ReceiveHandler(void);
void Uart_WaitAndSaveDataToBuffer(int intTimeout_ms);
void UART_ReceiveDataLengthSet(int intNewLength) ;
//==============================================================================

#endif

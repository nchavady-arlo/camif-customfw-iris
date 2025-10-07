#ifndef _PEGA_GPIO_KEY_H_
#define _PEGA_GPIO_KEY_H_

#include "pega_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
typedef	char  			MMP_BOOL;
typedef	unsigned char  	MMP_UBYTE;
typedef	unsigned short	MMP_USHORT;
typedef	unsigned int	MMP_ULONG;
//==============================================================================
/*
struct input_event {
struct timeval time;
__u16 type;
__u16 code;
__s32 value;
};
*/
typedef enum 
{
  KEY_DET_BT_RESET = 0, 
  KEY_DET_BT_MAX,
}KeyDetEnumType;

typedef struct
{	  
	  MMP_UBYTE bKeyPressed;
	  MMP_ULONG bKeyPressedCnt;
	  long long Pressed_Time;
	  MMP_ULONG Pressed_Time_us;
	  long long Release_Time;
	  MMP_ULONG Release_Time_us;
}stKeyDetType;
//==============================================================================
void      Pega_Gpio_key_Detection_Start(void);
int  	  Pega_Gpio_Key_Button_Status_Get(MMP_UBYTE button);
MMP_ULONG Pega_Gpio_Key_Button_Count_Get(MMP_UBYTE button);
//==============================================================================
void      Pega_Gpio_key_Detection_Clear(void);
void 	  Pega_Gpio_Key_Data_Info_Print(void);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_GPIO_KEY_H_
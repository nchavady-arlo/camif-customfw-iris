#ifndef _PEGA_GPIO_KEY_H_
#define _PEGA_GPIO_KEY_H_
//==============================================================================
#include "pega_defines.h"
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
typedef struct
{	  
	  uint8_t  bKeyPressed;
	  uint16_t bKeyPressedCnt;	  
}stKeyDetType;
//==============================================================================
void    Pega_Gpio_key_Detection_Start(void);
//==============================================================================
void    Pega_Gpio_key_Detection_Clear(void);
//==============================================================================
uint8_t Pega_Gpio_Key_Button_Is_pressed(void);
//==============================================================================
void    Pega_Gpio_Key_Data_Info_Print(void);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_GPIO_KEY_H_
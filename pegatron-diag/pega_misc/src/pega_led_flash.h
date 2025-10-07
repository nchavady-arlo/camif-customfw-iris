#ifndef _PEGA_LED_FLASH_H_
#define _PEGA_LED_FLASH_H_
//==============================================================================
#include "pega_defines.h"
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
typedef enum 
{
	LED_FLASH_COLOR_NONE = 0,
	LED_FLASH_COLOR_R    = 1,
	LED_FLASH_COLOR_G    = 2,
	LED_FLASH_COLOR_B    = 4,
	LED_FLASH_COLOR_MAX  = 8
}LedColorEnumType;

typedef enum 
{
/*00*/LED_PATTERN_NONE = 0,
/*01*/LED_PATTERN_COLOR_RED,
/*02*/LED_PATTERN_COLOR_GREEN,
/*03*/LED_PATTERN_COLOR_BLUE,
/*04*/LED_PATTERN_COLOR_CYAN,    //G+B
/*05*/LED_PATTERN_COLOR_MAGENTA, //R+B
/*06*/LED_PATTERN_COLOR_YELLOW,  //R+G
/*07*/LED_PATTERN_COLOR_WHITE,   //R+G+B
/*08*/LED_PATTERN_FLASHING_RED,
/*09*/LED_PATTERN_FLASHING_GREEN,
/*10*/LED_PATTERN_FLASHING_BLUE,
/*11*/LED_PATTERN_FLASHING_CYAN,
/*12*/LED_PATTERN_FLASHING_MAGENTA,
/*13*/LED_PATTERN_FLASHING_YELLOW,
/*14*/LED_PATTERN_FLASHING_WHITE,
/*15*/LED_PATTERN_NUM_MAX
}LedPatternEnumType;
//==============================================================================
typedef struct
{	  	
      uint8_t	bIsLedFlashing;	
	  uint8_t	eLedFlashing_Pattern;	  
	  uint8_t	eLedFlashing_Color;	  
	  uint16_t	u16Period;	  //ms
}stLedFlashingType;

typedef struct
{
	int eLedPattery;
	int sPriority;	
}stLedPatternPriorityType;
//==============================================================================
void Pega_led_flash_handler_Start(void);
int  Pega_led_flash_Led_Flashing_Check(void);
int  Pega_led_flash_Pattern_Set(uint16_t ePattern);
int  Pega_led_flash_state_Set(uint16_t estate);
void Pega_led_flash_Chanel(void);
void Pega_led_flash_State_Clear(void); 
void Pega_led_flash_Data_Print(void);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_LED_FLASH_H_
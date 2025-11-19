#ifndef _PEGA_VOICE_CMD_H_
#define _PEGA_VOICE_CMD_H_
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
typedef enum 
{
/*00*/LIGHT_Color_Off = 0,
/*01*/LIGHT_Color_Red,
/*02*/LIGHT_Color_Orange,
/*03*/LIGHT_Color_Yellow,
/*04*/LIGHT_Color_Green,
/*05*/LIGHT_Color_Blue,
/*06*/LIGHT_Color_Purple,
/*07*/LIGHT_Color_Pink,
/*08*/LIGHT_Color_White,
/*09*/LIGHT_Color_MAX,
}LightColorEnumType;

typedef enum 
{
/*00*/LIGHT_Brightness_None = 0,
/*01*/LIGHT_Brightness_Up,
/*02*/LIGHT_Brightness_Down,
/*03*/LIGHT_Brightness_Set,
/*09*/LIGHT_Brightness_MAX,
}LightBrightnessEnumType;

typedef enum 
{
/*00*/PLUG_Power_Off = 0,
/*01*/PLUG_Power_On,
/*02*/PLUG_Power_MAX,
}PlugStateEnumType;

typedef enum 
{
/*00*/MATTER_Dev_None = 0,
/*01*/MATTER_Dev_Light,
/*02*/MATTER_Dev_Plug,
/*03*/MATTER_Dev_All,
/*04*/MATTER_Dev_MAX,
}MatterDevEnumType;
//==============================================================================
void Pega_matter_device_contrl(MatterDevEnumType eDev, int bIsPowerOn);
void Pega_matter_light_contrl(LightColorEnumType eState);
void Pega_matter_light_brightness_contrl(LightBrightnessEnumType eState, int u8Brightness); 
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_VOICE_CMD_H_
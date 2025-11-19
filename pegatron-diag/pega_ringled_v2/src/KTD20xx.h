#ifndef KTD20xx_H
#define KTD20xx_H

#include <stdint.h>
#include <stdbool.h>

#define REG_ID						0x00
#define REG_MONITOR					0x01
#define REG_CONTROL					0x02
#define REG_IRED0					0x03
#define REG_IGRN0					0x04
#define REG_IBLU0					0x05
#define REG_IRED1					0x06
#define REG_IGRN1					0x07
#define REG_IBLU1					0x08
#define REG_ISELA12					0x09
#define REG_ISELA34					0x0A
#define REG_ISELB12					0x0B
#define REG_ISELB34					0x0C
#define REG_ISELC12					0x0D
#define REG_ISELC34					0x0E


#ifdef __cplusplus
extern "C" {
#endif

enum ktd20xx_mode{
	GLOBAL_OFF = 0x00,
	NIGHT_MODE = 0x01,
	NORMAL_MODE = 0x02,
	DEFAULT = 0x03,
};

enum ktd20xx_be{
    BE_dis  = 0,
    BE_en   = 1,
};
enum ktd20xx_ce_temp{
    Temp_0  = 0,
    Temp_1,
    Temp_2,
    Temp_3,
};
enum ktd20xx_fade_rate{
    Fade_Rate_0  = 0,
    Fade_Rate_1,
    Fade_Rate_2,
    Fade_Rate_3,
    Fade_Rate_4,
    Fade_Rate_5,
    Fade_Rate_6,
    Fade_Rate_7,
};

void ktd20xx_chip_init(int fd,uint8_t slave_addr);
void ktd20xx_mode_change(int fd,uint8_t slave_addr,int mode,int fade_rate);
void ktd20xx_select_color(int fd,uint8_t slave_addr,int color, uint8_t sel);
void ktd20xx_set_color0(int fd,uint8_t slave_addr, uint8_t ired, uint8_t igreen, uint8_t iblue);

#ifdef __cplusplus
}
#endif

#endif

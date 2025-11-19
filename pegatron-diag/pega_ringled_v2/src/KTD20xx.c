#include "KTD20xx.h"
#include "pega_i2c.h"

#define CONTROL_REG_VALUE   



void ktd20xx_chip_init(int fd,uint8_t slave_addr)
{
	uint8_t data[2] = {0};
    data[0] = REG_CONTROL;
	data[1] = (BE_dis << 5) | (Temp_0 << 3)| (Fade_Rate_0);
    pega_i2c_write(fd,slave_addr,data,sizeof(data));
}

void ktd20xx_breath(int fd)
{
	uint8_t MAX_BRIGHTNESS = 0xB0;
	uint8_t MIN_BRIGHTNESS = 0x04;
	
    while(1)
    {

    }
    #if 0
	ktd20xx_mode_change(chip,NORMAL_MODE, 0x07);	
	ktd20xx_select_color(chip, 0, 0); //select color 0
	
	ktd20xx_set_color0(chip, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS);
	msleep(2200);

	
	ktd20xx_mode_change(chip,NORMAL_MODE, 0x04);			
	ktd20xx_set_color0(chip, MIN_BRIGHTNESS, MIN_BRIGHTNESS, MIN_BRIGHTNESS);
	msleep(2000);
	#endif
}	

void ktd20xx_mode_change(int fd,uint8_t slave_addr,int mode,int fade_rate)
{
	uint8_t value[2] = {0};
	value[0] = REG_CONTROL;
	/* Read  */
	if (pega_i2c_read(fd, slave_addr, &value[0], sizeof(uint8_t), &value[1], sizeof(uint8_t)) < 0) {
        perror("I2C transfer failed");
        close(fd);
        return 1;
    }  
	value[1] &= 0x3C;
	value[1] = value[1] | mode << 6 | fade_rate;

	/* Write  */
    pega_i2c_write(fd,slave_addr,value,sizeof(value));

}

void ktd20xx_select_color(int fd,uint8_t slave_addr,int color, uint8_t sel)
{
	uint8_t isel;
	int i;
	uint8_t value[7]={0};
	value[0] = REG_ISELA12;
    switch(color)
	{
	   case 0:
	          isel = 0x88; //color0
              break;
	   case 1:
	          isel = 0xFF; //color1	
              break;   
       case 3:
	          isel = 0x00; //OFF
              break;
 	   case 4:
	          isel = sel;
              break;
			  
	   default:
	          isel = 0x00; //OFF
			  printf("invalid value(%d)!\n",color);
			  return 1;	  
	}
	
	for (i = 0; i < REG_ISELC34-REG_ISELA12+1; i++){
		value[i+1] = isel;
	}	
	pega_i2c_write(fd,slave_addr, value, sizeof(value));		
}

void ktd20xx_set_color0(int fd,uint8_t slave_addr, uint8_t ired, uint8_t igreen, uint8_t iblue)
{
	uint8_t value[4]={0};
	value[0] = REG_IRED0;
	value[1] = ired;
	value[2] = igreen;
	value[3] = iblue;
	pega_i2c_write(fd,slave_addr, value, sizeof(value));
}

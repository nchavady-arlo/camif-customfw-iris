#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
//==============================================================================
#include "Pega_Amp2563.h"
#include "Pega_gpio.h"
#include "Pega_i2c_control.h"
//==============================================================================
static MMP_UBYTE cur_book = 0; //current book
static MMP_UBYTE cur_page = 0; //current page
//==============================================================================
//char *psetcConfigFile  = "/etc/audio_ai.cfg";
static char m_ps8audaiConfigFile[50]={0};//  = "/settings/audio.cfg";
//==============================================================================	
/*****************************************************************************************************************
 * @Function:
 *      tas2563_i2c_write
 *
 * @Routine Description:
 *      tas2560 write register
 *
 * @Parameter:
 *      regAddr:    Register address
 *      regData:    Register data
 *
 * @Return:
 *      0:    Success
 *      -1:    Failure
 *
 *****************************************************************************************************************/
static int tas2563_i2c_write(MMP_UBYTE regAddr, MMP_UBYTE regData)
{
	  return pega_i2c_Write(TAS2563_ADDRESS, (MMP_ULONG)regAddr, (MMP_USHORT)regData, I2C_FMT_A8D8);
}

int tas2563_bulk_write_reg(MMP_UBYTE book, MMP_UBYTE page, MMP_UBYTE regAddr, int datalen, MMP_UBYTE *regData)
{
	int  ret;
	int i;
	/* Select the book which the register locates at. */
	if (book != cur_book) {
		cur_book = book;
		ret = tas2563_i2c_write(TAS2563_BOOK, book);
		if (ret != 0) {
			return -1;
		}
	}

	/* Select the page which the register locates at. */
	if (page != cur_page) {
		cur_page = page;
		ret = tas2563_i2c_write(TAS2563_PAGE, page);
		if (ret != 0) {
			return -1;
		}
	}
	
	for (i = 0; i < datalen; i++) {
		/* Write the register */
		ret = tas2563_i2c_write(regAddr + i, regData[i]);
		if (ret != 0) {
			printf("[tas2560] bulk write reg error, book = %d, page = %d, reg = 0x%02x, reg_val = 0x%02x\n", (int)book, (int)page, regAddr + i, regData[i]);
			return -1;
		}
	}
	return 0;
}

static int tas2563_i2c_read(MMP_UBYTE regAddr, MMP_UBYTE *regData)
{
	  int ret;
	  
	  unsigned short value=0;
	  
	  ret = pega_i2c_Read(TAS2563_ADDRESS, (MMP_ULONG)regAddr, &value, I2C_FMT_A8D8);
	  
	  *regData = value & 0xFF;
	  
	  return ret;
}

static int tas2563_update_bits(MMP_UBYTE reg, MMP_UBYTE mask, MMP_UBYTE value)
{
	MMP_UBYTE old, new;
	int change, ret;

	ret = tas2563_i2c_read(reg, &old);
	if (ret < 0) {
		printf("Failed to read private reg: %d\n", ret);
		return ret;
	}

	new = (old & ~mask) | (value & mask);
	change = old != new;
	
	if (change) 
	{
		ret = tas2563_i2c_write(reg, new);
		if (ret < 0) 
		{
			printf("Failed to write private reg: %d\n", ret);
			return ret;
		}
	}
	
	return 0;
}

int Parser_audio_ai_config(void)
{
    FILE *filep = NULL;
    char audpar[60];
    char delim[] = " ";
    char *tok;
	
	uint8_t u8Reg = 0,u8Value = 0;
    //char *psetcConfigFile  = "/etc/audio_ai.cfg";
    //char *ps8audaiConfigFile  = "/config/audio.cfg";
    
	printf("\n[%s]m_ps8audaiConfigFile=%s\n", __func__, m_ps8audaiConfigFile);
	
    if ((access(m_ps8audaiConfigFile, F_OK)) != -1)  //is file exist?
      {
         printf("aud ai config %s \n", m_ps8audaiConfigFile);
         filep = fopen(m_ps8audaiConfigFile, "r");
      }
    else
      {  
        printf("\nCan not load config file! (%s)\n", m_ps8audaiConfigFile);  
        		
        return -1;		
      }  
     
	//printf("\n"); 
	
    if (filep != NULL)
    {
        do
        {
            fgets(audpar, 60, filep);
            audpar[strlen(audpar) - 1] = '\0';
            if (!strcmp(audpar, "end"))
            {
                break;
            }
            else
            {
                //printf("audpar %s \n", audpar);
                tok = strtok(audpar, delim);
                if (!strcmp(tok, "w"))
                {					
                    tok = strtok(NULL, delim);
                    //printf("\n%s", tok); 
					tok = strtok(NULL, delim);
					u8Reg = strtol(tok,NULL,16);
                    //printf("\n%s", tok); 
					tok = strtok(NULL, delim);
                    //printf("\n%s", tok); 
					u8Value = strtol(tok,NULL,16);					
					//printf("\n u8Reg(0x%02x) = (0x%02x)", u8Reg, u8Value);					
					tas2563_i2c_write(u8Reg, u8Value);
                }
                else if (!strcmp(tok, ">"))
                {
                    tok = strtok(NULL, delim);
					u8Reg++;
					u8Value = strtol(tok,NULL,16);
					//printf("\n%s", tok);					
					//printf("\n u8Reg(0x%02x) = (0x%02x)", u8Reg, u8Value);
					tas2563_i2c_write(u8Reg, u8Value);                    
                }
				else if (!strcmp(tok, "d"))
                {
                    tok = strtok(NULL, delim);
					//printf("\n delay(%d)", strtol(tok,NULL,16)); 
                    usleep(10);                    
                }                
            }
        } while (!feof(filep));

        fclose(filep);
    }
    
    //printf("\n");
    
    return 0;
}
//==============================================================================
/*
我的I2S config 如下: (sample rate : 16000)
1.	BCLK 		: 512Khz      (16Khz * 16 bit width * 2 channel = 512Khz) 
2.	LRCLK  	: fs = 16KHz  (sample rate : 16000)
3.	16 bits mode , I2S    (bit width)
*/
//==============================================================================
void Pega_AMP_Initialize(float fdB)
{		
#if 1	
	  printf("\r\n%s(%2.1f)", __FUNCTION__, fdB);
	  
	  Pega_Gpio_init();
	  
	  usleep(1000*300); //d 10
	  
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select
	  tas2563_i2c_write(0x7f, 0x00);//w 98 7f 00 Book Selection
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select
   	  tas2563_i2c_write(0x02, 0x01);//w 98 02 01 Power Up and Mute
	  usleep(1000*10); //d 10
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select
	  tas2563_i2c_write(0x7f, 0x00);//w 98 7f 00 Book Selection
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select	
	  tas2563_i2c_write(0x02, 0x02);//w 98 02 02 Power Up and software shutdown
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select
	  tas2563_i2c_write(0x7f, 0x00);//w 98 7f 00 Book Selection
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select	
	  tas2563_i2c_write(0x01, 0x01);//w 98 01 00 software reset
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select
	  tas2563_i2c_write(0x7f, 0x00);//w 98 7f 00 Book Selection
	  tas2563_i2c_write(0x00, 0xfd);//w 98 00 fd Page Select
	  tas2563_i2c_write(0x0d, 0x0d);//w 98 0d 0d 
	  tas2563_i2c_write(0x32, 0x49);//w 98 32 49 
	  tas2563_i2c_write(0x3f, 0x21);//w 98 3f 21 
	  tas2563_i2c_write(0x19, 0x80);//w 98 19 80 
	  tas2563_i2c_write(0x0d, 0x0d);//w 98 0d 0d 
	  tas2563_i2c_write(0x5f, 0xc1);//w 98 5f c1
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select
	  tas2563_i2c_write(0x0a, 0x03);//w 98 0a 03
	  tas2563_i2c_write(0x1a, 0xfc);//w 98 1a fc
	  tas2563_i2c_write(0x1b, 0xa6);//w 98 1b a6
	  tas2563_i2c_write(0x1c, 0xdf);//w 98 1c df
	  tas2563_i2c_write(0x1d, 0xef);//w 98 1d ef
	  tas2563_i2c_write(0x30, 0x19);//w 98 30 19 
	  tas2563_i2c_write(0x32, 0x80);//w 98 30 19 
	  tas2563_i2c_write(0x38, 0x00);//w 98 38 00 
	  tas2563_i2c_write(0x30, 0x99);//w 98 30 99 //Set P0x00 / R0x30 |= 0x80 (Power Up on BCLK)
	  tas2563_i2c_write(0x33, 0x34);//w 98 33 34 
	  tas2563_i2c_write(0x34, 0x4b);//w 98 34 4b 
	  tas2563_i2c_write(0x35, 0x84);//w 98 35 84 
	  tas2563_i2c_write(0x3c, 0x38);//w 98 3c 38 
	  tas2563_i2c_write(0x00, 0x04);//w 98 00 04 
	  tas2563_i2c_write(0x60, 0x04);//w 98 60 04 
	  tas2563_i2c_write(0x61, 0x04);//w 98 61 cc 
	  tas2563_i2c_write(0x62, 0x04);//w 98 62 cc 
	  tas2563_i2c_write(0x63, 0x04);//w 98 63 cd 
	  tas2563_i2c_write(0x14, 0x1c);//w 98 14 1c 
	  tas2563_i2c_write(0x15, 0x94);//w 98 15 94 
	  tas2563_i2c_write(0x16, 0x7a);//w 98 16 7a 
	  tas2563_i2c_write(0x17, 0xe1);//w 98 17 e1 
	  tas2563_i2c_write(0x18, 0x1f);//w 98 18 1f 
	  tas2563_i2c_write(0x19, 0xa3);//w 98 19 a3 
	  tas2563_i2c_write(0x1a, 0xd7);//w 98 1a d7 
	  tas2563_i2c_write(0x1b, 0x0a);//w 98 1b 0a 
	  tas2563_i2c_write(0x1c, 0x22);//w 98 1c 22 
	  tas2563_i2c_write(0x1d, 0xb3);//w 98 1d b3 
	  tas2563_i2c_write(0x1e, 0x33);//w 98 1e 33 
	  tas2563_i2c_write(0x1f, 0x33);//w 98 1f 33 
	  tas2563_i2c_write(0x20, 0x25);//w 98 20 25 
	  tas2563_i2c_write(0x21, 0xc2);//w 98 21 c2 
	  tas2563_i2c_write(0x22, 0x8f);//w 98 22 8f 
	  tas2563_i2c_write(0x23, 0x5c);//w 98 23 5c 
	  tas2563_i2c_write(0x24, 0x28);//w 98 24 28 
	  tas2563_i2c_write(0x25, 0xd1);//w 98 25 d1 
	  tas2563_i2c_write(0x26, 0xeb);//w 98 26 eb 
	  tas2563_i2c_write(0x27, 0x85);//w 98 27 85 
	  tas2563_i2c_write(0x28, 0x2b);//w 98 28 2b 
	  tas2563_i2c_write(0x29, 0xe1);//w 98 29 e1 
	  tas2563_i2c_write(0x2a, 0x47);//w 98 2a 47 
	  tas2563_i2c_write(0x2b, 0xae);//w 98 2b ae 
	  tas2563_i2c_write(0x2c, 0x2e);//w 98 2c 2e 
	  tas2563_i2c_write(0x2d, 0xf0);//w 98 2d f0 
	  tas2563_i2c_write(0x2e, 0xa3);//w 98 2e a3 
	  tas2563_i2c_write(0x2f, 0xd7);//w 98 2f d7 
	  tas2563_i2c_write(0x30, 0x32);//w 98 30 32
	  tas2563_i2c_write(0x31, 0x00);//w 98 31 00
	  tas2563_i2c_write(0x32, 0x00);//w 98 32 00
	  tas2563_i2c_write(0x33, 0x00);//w 98 33 00
	  tas2563_i2c_write(0x34, 0x35);//w 98 34 35
	  tas2563_i2c_write(0x35, 0x0f);//w 98 35 0f
	  tas2563_i2c_write(0x36, 0x5c);//w 98 36 5c
	  tas2563_i2c_write(0x37, 0x29);//w 98 37 29
	  tas2563_i2c_write(0x38, 0x38);//w 98 38 38
	  tas2563_i2c_write(0x39, 0x1e);//w 98 39 1e
	  tas2563_i2c_write(0x3a, 0xb8);//w 98 3a b8
	  tas2563_i2c_write(0x3b, 0x52);//w 98 3b 52
	  tas2563_i2c_write(0x3c, 0x3b);//w 98 3c 3b
	  tas2563_i2c_write(0x3d, 0x2e);//w 98 3d 2e
	  tas2563_i2c_write(0x3e, 0x14);//w 98 3e 14
	  tas2563_i2c_write(0x3f, 0x7b);//w 98 3f 7b
	  tas2563_i2c_write(0x40, 0x04);//w 98 40 04
	  tas2563_i2c_write(0x41, 0xcc);//w 98 41 cc
	  tas2563_i2c_write(0x42, 0xcc);//w 98 42 cc
	  tas2563_i2c_write(0x43, 0xcd);//w 98 43 cd
	  tas2563_i2c_write(0x00, 0x03);//w 98 00 03 
	  tas2563_i2c_write(0x5c, 0x1e);//w 98 5c 1e 
	  tas2563_i2c_write(0x5d, 0x2e);//w 98 5d 2e 
	  tas2563_i2c_write(0x5e, 0x14);//w 98 5e 14 
	  tas2563_i2c_write(0x5f, 0x7b);//w 98 5f 7b
	  tas2563_i2c_write(0x60, 0x21);//w 98 60 21
	  tas2563_i2c_write(0x61, 0x3d);//w 98 61 3d
	  tas2563_i2c_write(0x62, 0x70);//w 98 62 70
	  tas2563_i2c_write(0x63, 0xa4);//w 98 63 a4
	  tas2563_i2c_write(0x64, 0x24);//w 98 64 24
	  tas2563_i2c_write(0x65, 0x4c);//w 98 65 4c
	  tas2563_i2c_write(0x66, 0xcc);//w 98 66 cc
	  tas2563_i2c_write(0x67, 0xcd);//w 98 67 cd
	  tas2563_i2c_write(0x68, 0x27);//w 98 68 27
	  tas2563_i2c_write(0x69, 0x5c);//w 98 69 5c
	  tas2563_i2c_write(0x6a, 0x28);//w 98 6a 28
	  tas2563_i2c_write(0x6b, 0xf6);//w 98 6b f6
	  tas2563_i2c_write(0x6c, 0x2a);//w 98 6c 2a
	  tas2563_i2c_write(0x6d, 0x6b);//w 98 6d 6b
	  tas2563_i2c_write(0x6e, 0x85);//w 98 6e 85
	  tas2563_i2c_write(0x6f, 0x1f);//w 98 6f 1f
	  tas2563_i2c_write(0x70, 0x2d);//w 98 70 2d
	  tas2563_i2c_write(0x71, 0x7a);//w 98 71 7a
	  tas2563_i2c_write(0x72, 0xe1);//w 98 72 e1
	  tas2563_i2c_write(0x73, 0x48);//w 98 73 48
	  tas2563_i2c_write(0x74, 0x30);//w 98 74 30
	  tas2563_i2c_write(0x75, 0x8a);//w 98 75 8a
	  tas2563_i2c_write(0x76, 0x3d);//w 98 76 3d
	  tas2563_i2c_write(0x77, 0x71);//w 98 77 71
	  tas2563_i2c_write(0x78, 0x33);//w 98 78 33
	  tas2563_i2c_write(0x79, 0x99);//w 98 79 99
	  tas2563_i2c_write(0x7a, 0x99);//w 98 7a 99
	  tas2563_i2c_write(0x7b, 0x9a);//w 98 7b 9a
	  tas2563_i2c_write(0x7c, 0x36);//w 98 7c 36
	  tas2563_i2c_write(0x7d, 0xa8);//w 98 7d a8
	  tas2563_i2c_write(0x7e, 0xf5);//w 98 7e f5
	  tas2563_i2c_write(0x7f, 0xc3);//w 98 7f c3
	  tas2563_i2c_write(0x00, 0x04);//w 98 00 04 
	  tas2563_i2c_write(0x08, 0x39);//w 98 08 39
	  tas2563_i2c_write(0x09, 0xb8);//w 98 09 b8
	  tas2563_i2c_write(0x0a, 0x51);//w 98 0a 51
	  tas2563_i2c_write(0x0b, 0xec);//w 98 0b ec
	  tas2563_i2c_write(0x0c, 0x3c);//w 98 0c 3c
	  tas2563_i2c_write(0x0d, 0xc7);//w 98 0d c7
	  tas2563_i2c_write(0x0e, 0xae);//w 98 0e ae
	  tas2563_i2c_write(0x0f, 0x14);//w 98 0f 14
	  tas2563_i2c_write(0x10, 0x3f);//w 98 10 3f
	  tas2563_i2c_write(0x11, 0xd7);//w 98 11 d7
	  tas2563_i2c_write(0x12, 0x0a);//w 98 12 0a
	  tas2563_i2c_write(0x13, 0x3d);//w 98 13 3d
	  tas2563_i2c_write(0x54, 0x17);//w 98 54 17
	  tas2563_i2c_write(0x55, 0x99);//w 98 55 99
	  tas2563_i2c_write(0x56, 0x99);//w 98 56 99
	  tas2563_i2c_write(0x57, 0x9a);//w 98 57 9a
	  tas2563_i2c_write(0x6c, 0x00);//w 98 6c 00
	  tas2563_i2c_write(0x6d, 0x00);//w 98 6d 00
	  tas2563_i2c_write(0x6e, 0x00);//w 98 6e 00
	  tas2563_i2c_write(0x6f, 0x17);//w 98 6f 17
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select
	  tas2563_i2c_write(0x03, 0x20);//w 98 03 20
	  tas2563_i2c_write(0x04, 0xc6);//w 98 04 c6
	  tas2563_i2c_write(0x0a, 0x03);//w 98 0a 03
	  tas2563_i2c_write(0x12, 0x12);//w 98 12 12
	  tas2563_i2c_write(0x13, 0x76);//w 98 13 76
	  tas2563_i2c_write(0x14, 0x01);//w 98 14 01
	  tas2563_i2c_write(0x15, 0x2e);//w 98 15 2e
	  tas2563_i2c_write(0x17, 0x0e);//w 98 17 0e
	  tas2563_i2c_write(0x19, 0x00);//w 98 19 00
	  tas2563_i2c_write(0x33, 0x34);//w 98 33 34
	  tas2563_i2c_write(0x34, 0x4b);//w 98 34 4b
	  tas2563_i2c_write(0x3b, 0x38);//w 98 3b 38
	  tas2563_i2c_write(0x3d, 0x08);//w 98 3d 08
	  tas2563_i2c_write(0x3e, 0x10);//w 98 3e 10
	  tas2563_i2c_write(0x3f, 0x00);//w 98 3f 00
	  tas2563_i2c_write(0x40, 0xb6);//w 98 40 b6
	  tas2563_i2c_write(0x00, 0x01);//w 98 00 01
	  tas2563_i2c_write(0x08, 0x40);//w 98 08 40
	  tas2563_i2c_write(0x00, 0x02);//w 98 00 02
	  tas2563_i2c_write(0x0c, 0x40);//w 98 0c 40
	  tas2563_i2c_write(0x0d, 0x00);//w 98 0d 00
	  tas2563_i2c_write(0x0e, 0x00);//w 98 0e 00
	  tas2563_i2c_write(0x0f, 0x00);//w 98 0f 00
	  tas2563_i2c_write(0x10, 0x03);//w 98 10 03
	  tas2563_i2c_write(0x11, 0x4a);//w 98 11 4a
	  tas2563_i2c_write(0x12, 0x51);//w 98 12 51
	  tas2563_i2c_write(0x13, 0x6c);//w 98 13 6c
	  tas2563_i2c_write(0x00, 0x04);//w 98 00 04
	  tas2563_i2c_write(0x74, 0x7f);//w 98 74 7f
	  tas2563_i2c_write(0x75, 0xfb);//w 98 75 fb
	  tas2563_i2c_write(0x76, 0xb6);//w 98 76 b6
	  tas2563_i2c_write(0x77, 0x14);//w 98 77 14
	  tas2563_i2c_write(0x78, 0x80);//w 98 78 80
	  tas2563_i2c_write(0x79, 0x04);//w 98 79 04
	  tas2563_i2c_write(0x7a, 0x49);//w 98 7a 49
	  tas2563_i2c_write(0x7b, 0xed);//w 98 7b ed
	  tas2563_i2c_write(0x7c, 0x7f);//w 98 7c 7f
	  tas2563_i2c_write(0x7d, 0xf7);//w 98 7d f7
	  tas2563_i2c_write(0x7e, 0x6c);//w 98 7e 6c
	  tas2563_i2c_write(0x7f, 0x28);//w 98 7f 28
	  tas2563_i2c_write(0x00, 0x02);//w 98 00 02
	  tas2563_i2c_write(0x68, 0x7f);//w 98 68 7f
	  tas2563_i2c_write(0x69, 0xfb);//w 98 69 fb
	  tas2563_i2c_write(0x6a, 0xb6);//w 98 6a b6
	  tas2563_i2c_write(0x6b, 0x14);//w 98 6b 14
	  tas2563_i2c_write(0x6c, 0x80);//w 98 6c 80
	  tas2563_i2c_write(0x6d, 0x04);//w 98 6d 04
	  tas2563_i2c_write(0x6e, 0x49);//w 98 6e 49
	  tas2563_i2c_write(0x6f, 0xed);//w 98 6f ed
	  tas2563_i2c_write(0x70, 0x7f);//w 98 70 7f
	  tas2563_i2c_write(0x71, 0xf7);//w 98 71 f7
	  tas2563_i2c_write(0x72, 0x6c);//w 98 72 6c
	  tas2563_i2c_write(0x73, 0x28);//w 98 73 28
	  tas2563_i2c_write(0x14, 0x2d);//w 98 14 2d
	  tas2563_i2c_write(0x15, 0x6a);//w 98 15 6a
	  tas2563_i2c_write(0x16, 0x86);//w 98 16 86
	  tas2563_i2c_write(0x17, 0x6f);//w 98 17 6f
	  tas2563_i2c_write(0x18, 0x47);//w 98 18 47
	  tas2563_i2c_write(0x19, 0x5c);//w 98 19 5c
	  tas2563_i2c_write(0x1a, 0x28);//w 98 1a 28
	  tas2563_i2c_write(0x1b, 0xf6);//w 98 1b f6
	  tas2563_i2c_write(0x1c, 0x16);//w 98 1c 16
	  tas2563_i2c_write(0x1d, 0x66);//w 98 1d 66
	  tas2563_i2c_write(0x1e, 0x66);//w 98 1e 66
	  tas2563_i2c_write(0x1f, 0x66);//w 98 1f 66
	  tas2563_i2c_write(0x20, 0x1a);//w 98 20 1a
	  tas2563_i2c_write(0x21, 0x66);//w 98 21 66
	  tas2563_i2c_write(0x22, 0x66);//w 98 22 66
	  tas2563_i2c_write(0x23, 0x66);//w 98 23 66
	  tas2563_i2c_write(0x24, 0x08);//w 98 24 08
	  tas2563_i2c_write(0x25, 0x00);//w 98 25 00
	  tas2563_i2c_write(0x26, 0x00);//w 98 26 00
	  tas2563_i2c_write(0x27, 0x00);//w 98 27 00
	  tas2563_i2c_write(0x28, 0x17);//w 98 28 17
	  tas2563_i2c_write(0x29, 0x33);//w 98 29 33
	  tas2563_i2c_write(0x2a, 0x33);//w 98 2a 33
	  tas2563_i2c_write(0x2b, 0x33);//w 98 2b 33
	  tas2563_i2c_write(0x2c, 0x15);//w 98 2c 15
	  tas2563_i2c_write(0x2d, 0x99);//w 98 2d 99
	  tas2563_i2c_write(0x2e, 0x99);//w 98 2e 99
	  tas2563_i2c_write(0x2f, 0x9a);//w 98 2f 9a
	  tas2563_i2c_write(0x00, 0x05);//w 98 00 05
	  tas2563_i2c_write(0x24, 0x7f);//w 98 24 7f
	  tas2563_i2c_write(0x25, 0xfe);//w 98 25 fe
	  tas2563_i2c_write(0x26, 0xfd);//w 98 26 fd
	  tas2563_i2c_write(0x27, 0x47);//w 98 27 47
	  tas2563_i2c_write(0x2c, 0xfe);//w 98 2c fe
	  tas2563_i2c_write(0x2d, 0xfe);//w 98 2d fe
	  tas2563_i2c_write(0x2e, 0xa4);//w 98 2e a4
	  tas2563_i2c_write(0x2f, 0xb5);//w 98 2f b5
	  tas2563_i2c_write(0x40, 0x00);//w 98 40 00
	  tas2563_i2c_write(0x41, 0x00);//w 98 41 00
	  tas2563_i2c_write(0x42, 0x03);//w 98 42 03
	  tas2563_i2c_write(0x43, 0x20);//w 98 43 20
	  tas2563_i2c_write(0x44, 0x02);//w 98 44 02
	  tas2563_i2c_write(0x45, 0x46);//w 98 45 46
	  tas2563_i2c_write(0x46, 0xb4);//w 98 46 b4
	  tas2563_i2c_write(0x47, 0xe4);//w 98 47 e4
	  tas2563_i2c_write(0x1c, 0x01);//w 98 1c 01
	  tas2563_i2c_write(0x1d, 0xc9);//w 98 1d c9
	  tas2563_i2c_write(0x1e, 0x24);//w 98 1e 24
	  tas2563_i2c_write(0x1f, 0x92);//w 98 1f 92
	  tas2563_i2c_write(0x20, 0x00);//w 98 20 00
	  tas2563_i2c_write(0x21, 0x12);//w 98 21 12
	  tas2563_i2c_write(0x22, 0x49);//w 98 22 49
	  tas2563_i2c_write(0x23, 0x25);//w 98 23 25
	  tas2563_i2c_write(0x00, 0x02);//w 98 00 02
	  tas2563_i2c_write(0x5c, 0x00);//w 98 5c 00
	  tas2563_i2c_write(0x5d, 0x01);//w 98 5d 01
	  tas2563_i2c_write(0x5e, 0x09);//w 98 5e 09
	  tas2563_i2c_write(0x5f, 0x45);//w 98 5f 45
	  tas2563_i2c_write(0x64, 0x00);//w 98 64 00
	  tas2563_i2c_write(0x65, 0x00);//w 98 65 00
	  tas2563_i2c_write(0x66, 0x12);//w 98 66 12
	  tas2563_i2c_write(0x67, 0xc0);//w 98 67 c0
	  tas2563_i2c_write(0x40, 0x04);//w 98 40 04
	  tas2563_i2c_write(0x41, 0xcc);//w 98 41 cc
	  tas2563_i2c_write(0x42, 0xcc);//w 98 42 cc
	  tas2563_i2c_write(0x43, 0xcd);//w 98 43 cd
	  tas2563_i2c_write(0x4c, 0x00);//w 98 4c 00
	  tas2563_i2c_write(0x4d, 0x00);//w 98 4d 00
	  tas2563_i2c_write(0x4e, 0x00);//w 98 4e 00
	  tas2563_i2c_write(0x4f, 0x00);//w 98 4f 00
	  tas2563_i2c_write(0x00, 0x03);//w 98 00 03
	  tas2563_i2c_write(0x24, 0x39);//w 98 24 39
	  tas2563_i2c_write(0x25, 0x80);//w 98 25 80
	  tas2563_i2c_write(0x26, 0x00);//w 98 26 00
	  tas2563_i2c_write(0x27, 0x00);//w 98 27 00
	  tas2563_i2c_write(0x18, 0x72);//w 98 18 72
	  tas2563_i2c_write(0x18, 0x14);//w 98 19 14
	  tas2563_i2c_write(0x1a, 0x82);//w 98 1a 82
	  tas2563_i2c_write(0x1b, 0xc0);//w 98 1b c0
	  tas2563_i2c_write(0x1c, 0x00);//w 98 1c 00
	  tas2563_i2c_write(0x1d, 0x00);//w 98 1d 00
	  tas2563_i2c_write(0x1e, 0x00);//w 98 1e 00
	  tas2563_i2c_write(0x1f, 0x64);//w 98 1f 64
	  tas2563_i2c_write(0x20, 0x40);//w 98 20 40
	  tas2563_i2c_write(0x21, 0xbd);//w 98 21 bd
	  tas2563_i2c_write(0x22, 0xb7);//w 98 22 b7
	  tas2563_i2c_write(0x23, 0xc0);//w 98 23 c0
	  tas2563_i2c_write(0x28, 0x2d);//w 98 28 2d
	  tas2563_i2c_write(0x29, 0x6a);//w 98 29 6a
	  tas2563_i2c_write(0x2a, 0x86);//w 98 2a 86
	  tas2563_i2c_write(0x2b, 0x6f);//w 98 2b 6f
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select
	  tas2563_i2c_write(0x04, 0xc6);//w 98 04 c6
	  tas2563_i2c_write(0x0a, 0x03);//w 98 0a 03
	  tas2563_i2c_write(0x31, 0x40);//w 98 31 40
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select
	  tas2563_i2c_write(0x7f, 0x00);//w 98 7f 00 Book Selection
	  tas2563_i2c_write(0x00, 0x00);//w 98 00 00 Page Select
	  tas2563_i2c_write(0x02, 0x00);//w 98 02 00
	  //Volume set		
	  if (fdB > 0)
	  {
	     Pega_AMP_Volume_Control(fdB);			    
	  }
	  //printf("\n%s(%d)", __FUNCTION__, 999);
		
	//	usleep(1000*5000); //d 1s to prevent the bob sound
		
#endif		
}

void Pega_AMP_Software_Reset(void)
{			  
	   int ret;
	/*soft reset*/
	/* 0 : don't reset
	   1 : reset
	*/
	tas2563_i2c_write(TAS2563_PAGE, 0x00);
	tas2563_i2c_write(TAS2563_BOOK, 0x00);

	ret = tas2563_i2c_write(TAS2563_SW_RESET, 0x1);

	if (ret != 0) 
		printf("tas2560 reset fail\n");	
	else 
		printf("tas2560 reset success\n");
	
		 //usleep(1000*50);
}

void Pega_AMP_Software_Mute(MMP_BOOL bMute)
{	
	   printf("\r\n%s", __FUNCTION__);
	   
	   #if 1
	   if ( bMute )
			{
				tas2563_i2c_write(TAS2563_PAGE, 			0x00);
				tas2563_i2c_write(TAS2563_BOOK, 			0x00);
				tas2563_update_bits(TAS2563_PWR_CTL, 0x03,  0x01);
			}
		else
			{
				tas2563_i2c_write(TAS2563_PAGE, 			0x00);
				tas2563_i2c_write(TAS2563_BOOK, 			0x00);
				tas2563_update_bits(TAS2563_PWR_CTL, 0x03,  0x00);
			}
		#endif	
}

void Pega_AMP_Volume_Control(float fdB)
{			   
#if 1 // connected interface 	  
	   
	   MMP_UBYTE ucdB = 0x20;
	   
	   if (fdB < 8.5)
	   {
	   	   fdB = 8.5;
	   }
	   
	   if (fdB > 22.0)
	   {
	   	   fdB = 22.0;
	   }
	   
       ucdB = (MMP_UBYTE)(((fdB - 8.5) / 0.5) + 1);
	   
	   printf("\n%s[%2.1f], ucdB=0x%x \n", __FUNCTION__, fdB, ucdB); //Bit0~3 is Gain control	  
	   
	   tas2563_i2c_write(TAS2563_PAGE, 0x00);
	   tas2563_i2c_write(TAS2563_BOOK, 0x00);
	
	   tas2563_update_bits(TAS2563_PB_CFG1, 0x3e, ucdB<<1); 
	   	   
#endif	   
}

void Pega_AMP_Reg_Write(MMP_UBYTE ucRegID, MMP_UBYTE val)
{        
     tas2563_i2c_write(TAS2563_PAGE, 0x00);
	 tas2563_i2c_write(TAS2563_BOOK, 0x00);
	 tas2563_i2c_write(ucRegID, val);
	  
	 printf("\r\n%s[%d,%d]", __FUNCTION__,ucRegID,val);  
}

void Pega_AMP_Reg_Read(MMP_UBYTE ucRegID, MMP_UBYTE *val)
{        
     tas2563_i2c_write(TAS2563_PAGE, 0x00);
	 tas2563_i2c_write(TAS2563_BOOK, 0x00);
     tas2563_i2c_read(ucRegID, val);
     
	 printf("\r\n%s[%d,%d]", __FUNCTION__,ucRegID,*val);  
}

void Pega_AMP_Data_Print(void)
{
}

void Pega_AMP_Reg_Data_Print(void)
{
#if 1	
	 MMP_UBYTE reg_addr=0;
	 MMP_UBYTE reg_val=0;

	 tas2563_i2c_write(TAS2563_PAGE, 0x00);
	 tas2563_i2c_write(TAS2563_BOOK, 0x00);
		/* Read all registers */
	 printf("page 0 registers :\r\n");
	 for (reg_addr = 0; reg_addr < 0x7f; reg_addr++) {
		//if (tas2560_readable_register(reg_addr)) {
			tas2563_i2c_read(reg_addr, &reg_val);
			printf("Reg %02x: %02x\r\n", reg_addr, reg_val);
		//}
		}
		
	 #if 0
		tas2563_i2c_write(TAS2563_PAGE, 0x01);
		tas2563_i2c_write(TAS2563_BOOK, 0x00);
		/* Read all registers */
		printf("page 1 registers :\r\n");
		for (reg_addr = 0; reg_addr < 0x7f; reg_addr++) {
		//if (tas2560_readable_register(reg_addr)) {
			tas2563_i2c_read(reg_addr, &reg_val);
			printf("Reg %02x: %02x\r\n", reg_addr, reg_val);
		//}
		}
	 #endif	
		
#endif		
}
//==============================================================================
static void pega_help_print(void)
{	   	   
	  printf("\n");

      printf("\n Usage :");
	  printf("\n tas2560 help info [options]");
	  printf("\n options:");
	  printf("\n init x  (x=8.5~22dB)");
	  printf("\n cfg");
	  //printf("\n on      (power on)");
	  //printf("\n off     (power off)");
	  printf("\n mute x  (x=0~1)");
	  printf("\n vol     (8.5~22dB)");
	  printf("\n reset   (reset amp)");
      printf("\n read x  (x=regAddr)");
      printf("\n write x0 x1 (x0=regAddr, x1=data)");
	  printf("\n dump    (dump register.)");	
	  printf("\n example: tas2560 dump");   
      printf("\n"); 
    
    #if 0
    printf("\n---------------------");
    printf("\nsizeof(short) = %d", sizeof(short)); 
    printf("\nsizeof(int)   = %d", sizeof(int)); 
    printf("\nsizeof(long)  = %d", sizeof(long)); 
    printf("\n---------------------");
    #endif
}

int main(int argc, char* argv[])
{		  
	  pega_i2cOpen();
	  
	  if (argc <= 1)
	      goto Cmd_Err;
	      
	  if (argc > 1)
    	{
    		printf("\n[%s,%d]%s,%s \n", __func__, argc, argv[0],argv[1]);    			 	    	 	
    	} 
    
    if (!strcmp(argv[1],"init"))
  		{  			 	  		 	 
  			 	 float fVol = 0.0;
  			 	 
  			 	 if (argc > 2)
	   	          {	   	       			 	 
  			 	     //regData = strtol(argv[2],NULL,16);
  			 	     fVol = atof(argv[2]);;
  			 	   }  
  			 	 
  			 	 Pega_AMP_Initialize(fVol);
  		}
	else if (!strcmp(argv[1],"hw"))
  		{  			 	  		 	 
  			 	 Pega_Gpio_HW_Amp_Reset();//Pega_AMP_Software_Reset();
  		} 	
  	else if (!strcmp(argv[1],"mute"))
  		{  			 	  		 	 
  			 	 MMP_UBYTE regData=0;
  			 	 
  			 	 if (argc < 3)
	   	       	 goto Cmd_Err;
	   	       	   	       			 	 
  			 	 regData = strtol(argv[2],NULL,16);
  			 	   			 	 
  			 	 Pega_AMP_Software_Mute(regData>0?MMP_TRUE:MMP_FALSE);
  		} 				
  	else if (!strcmp(argv[1],"cfg"))
  		{  		
             memcpy(m_ps8audaiConfigFile, "/settings/tas2563.cfg", strlen("/settings/tas2563.cfg"));
			 
			 if (argv[2] != NULL)
		     {					 
		        memcpy(&m_ps8audaiConfigFile, argv[2], strlen(argv[2]));
				printf(" \n argv[2]=%s,%d \n", argv[2], strlen(argv[2]));	
		     }
				  
             Pega_Gpio_init();	
  			 Parser_audio_ai_config();
			 Pega_AMP_Volume_Control(11.5);	
  		} 	 
    else if (!strcmp(argv[1],"vol"))
  		{  			 	  		 	 
  			 	 float fVol = 8.5;
  			 	 
  			 	 if (argv[2] != NULL)
		          {
		             fVol = atof(argv[2]); //16 
		          }
			   
               Pega_AMP_Volume_Control(fVol);	   	       			 	 
  			 	 
  		} 	 		
  	else if (!strcmp(argv[1],"dump"))
  		{  			 	  		 	 
  			 	 Pega_AMP_Reg_Data_Print();
  		} 	
  	else if (!strcmp(argv[1],"reset"))
  		{  			 	  		
                 memcpy(m_ps8audaiConfigFile, "/settings/tas2563.cfg", strlen("/settings/tas2563.cfg")); 	
  			 	 Pega_Gpio_HW_Amp_Reset();//Pega_AMP_Software_Reset();
				 Parser_audio_ai_config();
			     Pega_AMP_Volume_Control(11.5);	
  		} 	
  	else if (!strcmp(argv[1],"read"))
  	 {
  			 	 unsigned short regAddr=0;
  			 	   			 	 
  			 	 MMP_UBYTE regData=0;
  			 	 
  			 	 if (argc < 3)
	   	         goto Cmd_Err;
	   	       			 	 
  			 	 regAddr = strtol(argv[2],NULL,16);
  			 	 
  				 Pega_AMP_Reg_Read((MMP_UBYTE)regAddr, &regData);	  
  				 
  				 printf("[%s]:regAddr(0x%#x) = %#x \n", argv[1], regAddr, regData);	
  	 }  
	  else if (!strcmp(argv[1],"write"))
  	 {
  			 	 unsigned short regAddr=0,regData=0;
  			 	 
  			 	 if (argc < 4)
	   	         goto Cmd_Err;
	   	           			 	 
  			 	 regAddr = strtol(argv[2],NULL,16);
  			 	 regData = strtol(argv[3],NULL,16);
  			 	 
  				 Pega_AMP_Reg_Write((MMP_UBYTE)regAddr, (MMP_UBYTE)regData);	  
  				 
  				 printf("[%s]:regAddr(0x%#x) = %#x \n", argv[1], regAddr, regData);	
  	 }  					   			    
	  else
	    {
	       pega_help_print();
	    }  
	    
    pega_i2cClose();
    
    //printf("\n[%s]exit....\n", __func__);
    
    return 0;

Cmd_Err: 
  			
    pega_help_print();      
    pega_i2cClose();
    return 0;   
}
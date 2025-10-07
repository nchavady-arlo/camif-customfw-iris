/*******************************************
* File Name : i2c_read_write_2.c
* Purpose :
* Creation Date : 18-11-2016
* Created By : Mark Tseng  
**********************************************/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
//==============================================================================
#include "pega_i2c_control.h"
//==============================================================================
// I2C Linux device handle
static int g_i2cFile = -1;

// open the Linux device
int pega_i2cOpen(void)
{
	char devPath[64];
	sprintf(devPath,"/dev/i2c-%s", I2C_BUS);
	g_i2cFile = open(devPath, O_RDWR);
	if (g_i2cFile < 0) {
		perror("i2cOpen");
		
		exit(1);
		
		return FAILED;
	}
	
	//printf("[%s],%d \n",__func__,g_i2cFile);
	
	return SUCCEED;
}

// close the Linux device
void pega_i2cClose(void)
{
	   if (g_i2cFile > 0)
	       close(g_i2cFile);
	   
	   g_i2cFile = -1;
	   
	   //printf("[%s]\n",__func__);    
}

// set the I2C slave address for all subsequent I2C device transfers
void pega_i2cSetAddress(int address)
{
	if (ioctl(g_i2cFile, I2C_SLAVE_FORCE, address) < 0) {
		perror("i2cSetAddress");
		exit(1);
	}
}

// salveAddr : 8 bit savle address 
int pega_i2c_Write(int slaveAddr, unsigned int reg, unsigned short value, ISP_I2C_FMT fmt)
{
	unsigned char data[4];
	
	pega_i2cSetAddress(slaveAddr >> 1);
	//printf("[%s]: slave Addr: %#x \n", __func__, slaveAddr);
   
	memset(data, 0, sizeof(data));
	switch(fmt) {
		default:
		case I2C_FMT_A8D8:
			//printf("[%s]: I2C_FMT_A8D8, reg = %#x, value = %#x \n", __func__, reg, value);
			data[0] = reg & 0xff;
			data[1] = value & 0xff;
			if (write(g_i2cFile, data, 2) != 2) {
				perror("Write Register");
				return FAILED;
			}
			break;
		case I2C_FMT_A16D8:
			//printf("[%s]: I2C_FMT_A16D8, reg = %#x, value = %#x \n", __func__, reg, value);
			data[0] = (reg >> 8) & 0xff;
			data[1] = reg & 0xff;
			data[2] = value & 0xff;
			if (write(g_i2cFile, data, 3) != 3) {
				perror("Write Register");
				return FAILED;
			}
			break;
		case I2C_FMT_A8D16:
			//printf("[%s]: I2C_FMT_A8D16, reg = %#x, value = %#x \n", __func__, reg, value);
			data[0] = reg & 0xff;
			data[1] = (value >> 8) & 0xff;
			data[2] = (value ) & 0xff;
			if (write(g_i2cFile, data, 3) != 3) {
				perror("Write Register");
				return FAILED;
			}
			break;
		case I2C_FMT_A16D16:
			//printf("[%s]: I2C_FMT_A16D16, reg = %#x, value = %#x \n", __func__, reg, value);
			data[0] = (reg >> 8) & 0xff;
			data[1] = (reg ) & 0xff;
			data[2] = (value >> 8) & 0xff;
			data[3] = (value ) & 0xff;
			if (write(g_i2cFile, data, 4) != 4) {
				perror("SetRegisterPair");
				return FAILED;
			}

			break;
	}

	return SUCCEED;
}

int pega_i2c_Read(int slaveAddr, unsigned int reg, unsigned short *val, ISP_I2C_FMT fmt)
{
	unsigned char reg_addr[2];

	pega_i2cSetAddress(slaveAddr >> 1);
	
	//printf("[%s]: slave Addr: %#x, %d \n", __func__, slaveAddr, reg);
	memset(reg_addr, 0, sizeof(unsigned char));

	switch(fmt) {
		default:
		case I2C_FMT_A8D8:
			//printf("[%s]: I2C_FMT_A8D8 \n", __func__);
			reg_addr[0] =  reg & 0xff;
			//printf("reg_addr[0] = %d \n", reg_addr[0]);
			
			if (write(g_i2cFile, reg_addr, 1) != 1) {
				perror("Read RegisterPair set register");
				return -1;
			}
			if (read(g_i2cFile, val, 1) != 1) {
				perror("Read RegisterPair read value");
				return -1;
			}
			//printf("[%s]: read val[0] = %#x \n", __func__, val[0]);

			break;
		case I2C_FMT_A16D8:
			//printf("[%s]: I2C_FMT_A16D8 \n", __func__);
			reg_addr[0] = (reg >> 8) & 0xff;
			reg_addr[1] =  reg & 0xff;
			//printf("reg_addr[0]: %#x\n", reg_addr[0]);
			//printf("reg_addr[1]: %#x\n", reg_addr[1]);
			if (write(g_i2cFile, reg_addr, 2) != 2) {
				perror("Read RegisterPair set register");
				return -1;
			}
			if (read(g_i2cFile, val, 1) != 1) {
				perror("Read RegisterPair read value");
				return -1;
			}
			//printf("[%s]: read val[0] = %#x \n", __func__, val[0]);
			break;
		case I2C_FMT_A8D16:
			//printf("[%s]: I2C_FMT_A8D16 \n", __func__);
			reg_addr[0] =  reg & 0xff;
			//printf("reg_addr[0]: %#x\n", reg_addr[0]);
			if (write(g_i2cFile, reg_addr, 1) != 1) {
				perror("Read RegisterPair set register");
				return -1;
			}
			if (read(g_i2cFile, val, 2) != 2) {
				perror("Read RegisterPair read value");
				return -1;
			}
			//printf("[%s]: read val[0] = %#x \n", __func__, val[0]);
		
			break;
		case I2C_FMT_A16D16:
			//printf("[%s]: I2C_FMT_A16D16 \n", __func__);
			reg_addr[0] = (reg >> 8) & 0xff;
			reg_addr[1] =  reg & 0xff;
			if (write(g_i2cFile, reg_addr, 2) != 2) {
				perror("Read RegisterPair set register");
				return -1;
			}
			if (read(g_i2cFile, val, 2) != 2) {
				perror("Read RegisterPair read value");
				return -1;
			}
			//printf("[%s]: read val[0] = %#x \n", __func__, val[0]);

			break;
	}

	return 0;
}




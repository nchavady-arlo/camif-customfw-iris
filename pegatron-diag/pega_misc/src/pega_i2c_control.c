/*******************************************************************************
* File Name: pega_i2c_control.c
*
*******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
//==============================================================================
#include "pega_i2c_control.h"
//==============================================================================
static pthread_mutex_t m_I2cMutexlock;
//==============================================================================
// set the I2C slave address for all subsequent I2C device transfers
static void pega_i2cSetAddress(int i2cFile, int address)
{
	if (ioctl(i2cFile, I2C_SLAVE_FORCE, address) < 0) 
	{
		perror("i2cSetAddress");
		exit(1);
	}
}
//==============================================================================
void pega_i2c_Init(void)
{
	 pthread_mutex_init(&m_I2cMutexlock, NULL);
}

//==============================================================================
void pega_i2c_Deinit(void)
{
	 pthread_mutex_destroy(&m_I2cMutexlock);
}
//==============================================================================
// salveAddr : 8 bit savle address 
int pega_i2c_Write(int i2cFile, int slaveAddr, unsigned int reg, uint16_t value, ISP_I2C_FMT fmt)
{
	int rtn = FAILED;
	unsigned char data[4] = {0};
	
	pthread_mutex_lock(&m_I2cMutexlock);
	
	if (i2cFile <= 0)
	{
		goto Fun_end;
	}
	
	pega_i2cSetAddress(i2cFile, slaveAddr >> 1);
	//printf("[%s]: slave Addr: %#x \n", __func__, slaveAddr);
   
	memset(data, 0, sizeof(data));
	switch(fmt) {
		default:
		case I2C_FMT_A8D8:
			//printf("[%s]: I2C_FMT_A8D8, reg = %#x, value = %#x \n", __func__, reg, value);
			data[0] = reg & 0xff;
			data[1] = value & 0xff;
			if (write(i2cFile, data, 2) != 2) 
			{
				perror("Write Register");
				goto Fun_end;
			}
			break;
		case I2C_FMT_A16D8:
			//printf("[%s]: I2C_FMT_A16D8, reg = %#x, value = %#x \n", __func__, reg, value);
			data[0] = (reg >> 8) & 0xff;
			data[1] = reg & 0xff;
			data[2] = value & 0xff;
			if (write(i2cFile, data, 3) != 3) 
			{
				perror("Write Register");
				goto Fun_end;
			}
			break;
		case I2C_FMT_A8D16:
			//printf("[%s]: I2C_FMT_A8D16, reg = %#x, value = %#x \n", __func__, reg, value);
			data[0] = reg & 0xff;
			data[1] = (value >> 8) & 0xff;
			data[2] = (value ) & 0xff;
			if (write(i2cFile, data, 3) != 3) 
			{
				perror("Write Register");
				goto Fun_end;
			}
			break;
		case I2C_FMT_A16D16:
			//printf("[%s]: I2C_FMT_A16D16, reg = %#x, value = %#x \n", __func__, reg, value);
			data[0] = (reg >> 8) & 0xff;
			data[1] = (reg ) & 0xff;
			data[2] = (value >> 8) & 0xff;
			data[3] = (value ) & 0xff;
			if (write(i2cFile, data, 4) != 4) 
			{
				perror("SetRegisterPair");
				goto Fun_end;
			}

			break;
	}

   rtn = SUCCEED;
   
Fun_end:

    pthread_mutex_unlock(&m_I2cMutexlock);
	
	return rtn;
}

int pega_i2c_Read(int i2cFile, int slaveAddr, unsigned int reg, uint16_t *val, ISP_I2C_FMT fmt)
{ 
    int rtn = FAILED;
	unsigned char reg_addr[2] = {0};

    pthread_mutex_lock(&m_I2cMutexlock);
	
	if (i2cFile <= 0)
	{
		goto Fun_end;
	}
	
	pega_i2cSetAddress(i2cFile, slaveAddr >> 1);
	
	//printf("[%s]: slave Addr: %#x, %d \n", __func__, slaveAddr, reg);
	memset(reg_addr, 0, sizeof(unsigned char));

	switch(fmt) 
	{
		default:
		case I2C_FMT_A8D8:
			//printf("[%s]: I2C_FMT_A8D8 \n", __func__);
			reg_addr[0] =  reg & 0xff;
			//printf("reg_addr[0] = %d \n", reg_addr[0]);
			
			if (write(i2cFile, reg_addr, 1) != 1) 
			{
				perror("Read RegisterPair set register");
				goto Fun_end;
			}
			if (read(i2cFile, val, 1) != 1) 
			{
				perror("Read RegisterPair read value");
				goto Fun_end;
			}
			//printf("[%s]: read val[0] = %#x \n", __func__, val[0]);

			break;
		case I2C_FMT_A16D8:
			//printf("[%s]: I2C_FMT_A16D8 \n", __func__);
			reg_addr[0] = (reg >> 8) & 0xff;
			reg_addr[1] =  reg & 0xff;
			//printf("reg_addr[0]: %#x\n", reg_addr[0]);
			//printf("reg_addr[1]: %#x\n", reg_addr[1]);
			if (write(i2cFile, reg_addr, 2) != 2) 
			{
				perror("Read RegisterPair set register");
				goto Fun_end;
			}
			if (read(i2cFile, val, 1) != 1) 
			{
				perror("Read RegisterPair read value");
				goto Fun_end;
			}
			//printf("[%s]: read val[0] = %#x \n", __func__, val[0]);
			break;
		case I2C_FMT_A8D16:
			//printf("[%s]: I2C_FMT_A8D16 \n", __func__);
			reg_addr[0] =  reg & 0xff;
			//printf("reg_addr[0]: %#x\n", reg_addr[0]);
			if (write(i2cFile, reg_addr, 1) != 1) 
			{
				perror("Read RegisterPair set register");
				goto Fun_end;
			}
			if (read(i2cFile, val, 2) != 2) 
			{
				perror("Read RegisterPair read value");
				goto Fun_end;
			}
			//printf("[%s]: read val[0] = %#x \n", __func__, val[0]);
		
			break;
		case I2C_FMT_A16D16:
			//printf("[%s]: I2C_FMT_A16D16 \n", __func__);
			reg_addr[0] = (reg >> 8) & 0xff;
			reg_addr[1] =  reg & 0xff;
			if (write(i2cFile, reg_addr, 2) != 2) 
			{
				perror("Read RegisterPair set register");
				goto Fun_end;
			}
			if (read(i2cFile, val, 2) != 2) 
			{
				perror("Read RegisterPair read value");
				goto Fun_end;
			}
			//printf("[%s]: read val[0] = %#x \n", __func__, val[0]);

			break;
	}

   rtn = SUCCEED;
   
Fun_end:

    pthread_mutex_unlock(&m_I2cMutexlock);
	
	return rtn;
}

int pega_i2c_RW_Bytes(int i2cFile, struct i2c_rdwr_ioctl_data *packets)
{
    int rtn = FAILED;
	
	pthread_mutex_lock(&m_I2cMutexlock);

    if (ioctl(i2cFile, I2C_RDWR, packets) < 0) 
	{
        goto Fun_end;
    }
	
	rtn = SUCCEED;

Fun_end:

    pthread_mutex_unlock(&m_I2cMutexlock);

	return rtn;
}
 
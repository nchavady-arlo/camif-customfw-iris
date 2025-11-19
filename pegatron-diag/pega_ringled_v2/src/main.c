#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "KTD20xx.h"
#include "pega_i2c.h"

int pega_i2c_init(const char *i2c_bus)
{   
    int fd = open(i2c_bus, O_RDWR);
    if (fd < 0) {
        perror("Open I2C failed");
        return 1;
    }
    return fd;
}
bool pega_i2c_write(int fd, uint8_t slave_addr, uint8_t* tx_buf, uint32_t tx_len) {
    struct i2c_msg messages[1];
    struct i2c_rdwr_ioctl_data packets;

    messages[0].addr  = slave_addr;
    messages[0].flags = 0;  // Write
    messages[0].len   = tx_len;
    messages[0].buf   = tx_buf;

    packets.msgs  = messages;
    packets.nmsgs = 1;

    if (ioctl(fd, I2C_RDWR, &packets) < 0) {
        perror("i2c_write failed");
        return false;
    }

    return true;
}
bool pega_i2c_read(int fd, uint8_t slave_addr, uint8_t* tx_buf, uint32_t tx_len, uint8_t* rx_buf, uint32_t rx_len) {
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data data;

    msgs[0].addr  = slave_addr;
    msgs[0].flags = 0;  		// Write (register address)
    msgs[0].len   = tx_len;
    msgs[0].buf   = tx_buf;

    msgs[1].addr  = slave_addr;
    msgs[1].flags = I2C_M_RD;  // Read
    msgs[1].len   = rx_len;
    msgs[1].buf   = rx_buf;

    data.msgs  = msgs;
    data.nmsgs = 2;

    if (ioctl(fd, I2C_RDWR, &data) < 0) {
        perror("i2c_read failed");
        return false;
    }

    return true;
}
int main() {
    int fd;
    struct timespec step3_ts,delay,schedule_delay;
    delay.tv_sec = 5;
    delay.tv_nsec = 0;
    scanf("%d,%d",&schedule_delay.tv_sec,&schedule_delay.tv_nsec);
    /* ----------------------- */
    uint8_t write_data[] = { 0x00 };  // e.g., read from register 0x00
    uint8_t read_data[4] = {0};
    /* Init I2C  */
    fd = pega_i2c_init(I2C_BUS);
    /* Write Contorl Reg */
    ktd20xx_chip_init(fd,KTD2064A_SLAVE_ADDR);
    ktd20xx_chip_init(fd,KTD2064B_SLAVE_ADDR);
    /* Breathing  */
    ktd20xx_select_color(fd,KTD2064A_SLAVE_ADDR,0,0);
    ktd20xx_set_color0(fd,KTD2064A_SLAVE_ADDR,0xc0,0xc0,0);
    ktd20xx_select_color(fd,KTD2064B_SLAVE_ADDR,0,0);
    ktd20xx_set_color0(fd,KTD2064B_SLAVE_ADDR,0xc0,0xc0,0);
    while(1){
        ktd20xx_mode_change(fd,KTD2064A_SLAVE_ADDR,NORMAL_MODE,Fade_Rate_7);
        nanosleep(&schedule_delay, NULL);                                   //DELAY
        ktd20xx_mode_change(fd,KTD2064B_SLAVE_ADDR,NORMAL_MODE,Fade_Rate_7);    
        nanosleep(&delay, NULL);
        ktd20xx_mode_change(fd,KTD2064A_SLAVE_ADDR,GLOBAL_OFF,Fade_Rate_5);
        nanosleep(&schedule_delay, NULL);                                   //DELAY
        ktd20xx_mode_change(fd,KTD2064B_SLAVE_ADDR,GLOBAL_OFF,Fade_Rate_5);
        nanosleep(&delay, NULL);
    }    
    close(fd);
    close(fd);
    return 0;
}

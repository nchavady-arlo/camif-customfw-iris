#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <errno.h>

#define I2C_BUS "/dev/i2c-3"
#define LED_ADDR 0x68
#define ACC_ADDR 0x19

int read_i2c_register(int i2c_fd, uint8_t device_addr, uint8_t reg, uint8_t* val)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    messages[0].addr  = device_addr;
    messages[0].flags = 0; // Write
    messages[0].len   = 1;
    messages[0].buf   = &reg;

    messages[1].addr  = device_addr;
    messages[1].flags = I2C_M_RD;
    messages[1].len   = 1;
    messages[1].buf   = val;

    packets.msgs  = messages;
    packets.nmsgs = 2;

    if (ioctl(i2c_fd, I2C_RDWR, &packets) < 0) {
        fprintf(stderr, "無法從裝置 0x%02X 讀取暫存器 0x%02X: %s\n", device_addr, reg, strerror(errno));
        return -1;
    }

    return 0;
}

int write_i2c_register(int i2c_fd, uint8_t device_addr, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    struct i2c_rdwr_ioctl_data packet;
    struct i2c_msg message;

    message.addr  = device_addr;
    message.flags = 0; // Write
    message.len   = 2;
    message.buf   = buf;

    packet.msgs  = &message;
    packet.nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &packet) < 0) {
        fprintf(stderr, "無法寫入裝置 0x%02X 的暫存器 0x%02X: %s\n", device_addr, reg, strerror(errno));
        return -1;
    }

    return 0;
}
int main(int argc, char *argv[])
{
    int i2c_fd = open(I2C_BUS, O_RDWR);
    if (i2c_fd < 0) {
        perror("無法開啟 I2C 裝置");
        return 1;
    }

    printf("Dump 裝置 0x%02X 的暫存器 0x0D 到 0x3F:\n", LED_ADDR);
    for (uint8_t reg = 0x0D; reg <= 0x3F; reg++) {
        uint8_t val = 0;
        if (read_i2c_register(i2c_fd, ACC_ADDR, reg, &val) == 0) {
            printf("  [0x%02X] = 0x%02X\n", reg, val);
        } else {
            printf("  [0x%02X] = <讀取失敗>\n", reg);
        }
    }

    close(i2c_fd);
    return 0;
}


#ifndef PEGA_I2C_H
#define PEGA_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "KTD20xx.h"

#define I2C_BUS         "/dev/i2c-3"
#define KTD2064A_SLAVE_ADDR  0x6c
#define KTD2064B_SLAVE_ADDR  0x6d

#ifdef __cplusplus
extern "C" {
#endif

int pega_i2c_init(const char *i2c_bus);
bool pega_i2c_write(int fd, uint8_t slave_addr, uint8_t *tx_buf, uint32_t tx_len);
bool pega_i2c_read(int fd, uint8_t slave_addr, uint8_t *tx_buf, uint32_t tx_len, uint8_t *rx_buf, uint32_t rx_len);

#ifdef __cplusplus
}
#endif

#endif

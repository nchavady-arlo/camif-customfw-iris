#ifndef PEGA_I2C_H
#define PEGA_I2C_H

#include <stdint.h>
#include <stdbool.h>


#define I2C_BUS         "/dev/i2c-3"
#define ACC_ADDR  0x19

#ifdef __cplusplus
extern "C" {
#endif

bool pega_i2c_write(int fd, uint8_t *tx_buf, uint32_t tx_len);
bool pega_i2c_read(int fd, uint8_t *tx_buf, uint32_t tx_len, uint8_t *rx_buf, uint32_t rx_len);

#ifdef __cplusplus
}
#endif

#endif

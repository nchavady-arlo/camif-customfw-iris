#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <errno.h>

#define SPI_DEVICE "/dev/spidev0.0" // Replace with your SPI device
#define TRANSFER_SIZE 8 // Number of bytes to transfer

int main() {
    int spi_fd;
    uint8_t tx_buffer[TRANSFER_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    uint8_t rx_buffer[TRANSFER_SIZE];
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = TRANSFER_SIZE,
        .speed_hz = 1000000, // Example speed: 1 MHz
        .delay_usecs = 0,
        .bits_per_word = 8,
    };

    // Open the SPI device
    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("Failed to open SPI device");
        return EXIT_FAILURE;
    }

    // Set SPI mode (e.g., SPI mode 0)
    uint8_t mode = SPI_MODE_0;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Failed to set SPI mode");
        close(spi_fd);
        return EXIT_FAILURE;
    }

    // Set bits per word
    uint8_t bits = 8;
     if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        perror("can't set bits per word");
        close(spi_fd);
        return (1);
    }

    // Set SPI speed (Hz)
    uint32_t speed = 1000000; // 1MHz
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("can't set max speed hz");
        close(spi_fd);
        return (1);
    }
    // Perform SPI transfer
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0) {
        perror("SPI transfer failed");
        close(spi_fd);
        return EXIT_FAILURE;
    }

    // Print the received data
    printf("Received data: ");
    for (int i = 0; i < TRANSFER_SIZE; i++) {
        printf("0x%02X ", rx_buffer[i]);
    }
    printf("\n");

    // Close the SPI device
    close(spi_fd);

    return EXIT_SUCCESS;
}

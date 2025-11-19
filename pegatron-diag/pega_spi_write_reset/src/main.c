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
#define TRANSFER_SIZE 32 // Number of bytes to transfer
#define GPIO_CS 13

static void gpio_write(int value) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", GPIO_CS);
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("open gpio value");
        return;
    }
    if (write(fd, value ? "1" : "0", 1) < 0)
        perror("write gpio");
    close(fd);
}

static void gpio_export_init(void) {
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d", GPIO_CS);
    if (access(path, F_OK) != 0) {
        int fd = open("/sys/class/gpio/export", O_WRONLY);
        if (fd >= 0) {
            char buf[8];
            int len = snprintf(buf, sizeof(buf), "%d", GPIO_CS);
            write(fd, buf, len);
            close(fd);
            usleep(100000); // 給 kernel 時間建立節點
        }
    }

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", GPIO_CS);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        write(fd, "out", 3);
        close(fd);
    }
    system("echo '--- Start Init CS ---'");
    system("echo '--- CS value before low ---'; cat /sys/class/gpio/gpio13/direction; cat /sys/class/gpio/gpio13/value");
    gpio_write(0);
    usleep(1000000);
    system("echo '--- CS value after low ---'; cat /sys/class/gpio/gpio13/value");
    gpio_write(1); // 初始狀態：拉高 CS
    usleep(1000000);
    system("echo '--- CS value after high ---'; cat /sys/class/gpio/gpio13/value");
    system("echo '--- End Init CS ---'");
}

int main() {
    int spi_fd;
    uint8_t tx_buffer[TRANSFER_SIZE] = {
        0x82, 0x30, 0x00, 0x03, 0x00, 0x80, 0x01, 0x02, //0x03: 3 spinel frame (0x80, 0x01, 0x02) after SPI header (0x82, 0x30, 0x00, 0x03, 0x00)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //spinel frame 0x01: SPINEL_CMD_RESET
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //spinel frame 0x02: RESET_SOFTWARE
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };          
    

    uint8_t rx_buffer[TRANSFER_SIZE];
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = TRANSFER_SIZE,
        .speed_hz = 1500000, // Example speed: 1.5 MHz
        .delay_usecs = 0,
        .bits_per_word = 8,
        .cs_change = 0,
    };

    gpio_export_init();  // 初始化 GPIO13 為輸出並預設拉高

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
        return EXIT_FAILURE;
    }

    // Set SPI speed (Hz)
    uint32_t speed = 1500000; // 1.5MHz
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("can't set max speed hz");
        close(spi_fd);
        return EXIT_FAILURE;
    }
    // Perform SPI transfer
    printf("[Perform SPI transfer] CS high → low:  start transfer\n");
    gpio_write(0); // 拉低 CS
    usleep(1000000);     // 給一點 setup 時間
    printf("Send data: 0x82 0x30 0x00 0x03 0x00 0x80 0x01 0x02 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00\n");
    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 0)
        perror("SPI transfer failed");
    usleep(1000000);     // 保持片刻
    printf("[Perform SPI transfer] CS low → high:  end transfer\n");
    gpio_write(1); // 拉高 CS
    system("echo '--- after high ---'; cat /sys/class/gpio/gpio13/value");

    // Print the received data
    printf("Expected Received data: 0x82 0xFB 0x01 0x04 0x00 0x80 0x06 0x00 0x72 0x00 0x00 0x00 0x00 0x00 0x00 0x00\n");
    printf("Real Received data: "); //spinel frame 0x06: PROP_VALUE_IS, 0x00: LAST_STATUS, 0x72: SPINEL_STATUS_RESET_SOFTWARE
    for (int i = 0; i < TRANSFER_SIZE; i++) {
        printf("0x%02X ", rx_buffer[i]);
    }
    printf("\n");

    // Close the SPI device
    close(spi_fd);

    return EXIT_SUCCESS;
}

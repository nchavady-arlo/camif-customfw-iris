#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <poll.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include "pega_i2c.h"

#define WHO_AM_I            0x0F
#define CTRL1               0x20
#define CTRL3               0x22
#define CTRL4               0x23
#define STATUS              0x27
#define CTRL6               0x25
#define CTRL7               0x3F
#define TAP_THS_X           0x30
#define TAP_THS_Y           0x31
#define TAP_THS_Z           0x32
#define INT_DUR             0x33
#define WAKE_UP_THS         0x34
#define TAP_SRC             0x39

#define GPIO_PIN            60
#define GPIO_PATH           "/sys/class/gpio/gpio60/value"

#define TAP_IDLE_TIMEOUT 5000
#define TAP_CONT_TIMEOUT 1000

/* Function */
int pega_io_init(int *fd);
int pega_i2c_init(const char *i2c_bus,int i2c_addr,int *fd);
bool pega_i2c_write(int fd, uint8_t* tx_buf, uint32_t tx_len);
bool pega_i2c_read(int fd, uint8_t* tx_buf, uint32_t tx_len, uint8_t* rx_buf, uint32_t rx_len);
int Acc_evnet_init(int fd);
void *poll_event_thread(void *arg);

/* Global Variable */
int i2c_fd;
int tap=0;
int tap_flag;
pthread_t thread;
pthread_mutex_t lock;
pthread_cond_t tap_cond = PTHREAD_COND_INITIALIZER;

long last_timestamp = 0;  // Time of last tap in ms

long get_current_timestamp_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

int main() {
    int ret;

    /* Init I2C  */
    if(pega_i2c_init(I2C_BUS,ACC_ADDR,&i2c_fd))
    {
        perror("Faile to I2C");
        return -1;
    }
    /* Init Acc config and event */
    if(Acc_evnet_init(i2c_fd))
    {
        perror("Fail to config I2C");
        return -1;
    }
    // Init thread mutex
    pthread_mutex_init(&lock, NULL);
    /* Create a thread to poll event */
    if (pthread_create(&thread, NULL, poll_event_thread, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }
    while (1) {
         // Wait for tap event or timeout
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);                                    //Current time Add Timout
        if(!tap)                                                                    //IDLE -- long refresh time
        {
            timeout.tv_sec += TAP_IDLE_TIMEOUT / 1000;
            timeout.tv_nsec += (TAP_IDLE_TIMEOUT % 1000) * 1000000;
        }else{                                                                      //Continous tap -- short refresh time
            timeout.tv_sec += TAP_CONT_TIMEOUT / 1000;
            timeout.tv_nsec += (TAP_CONT_TIMEOUT % 1000) * 1000000;            
        }
        pthread_mutex_lock(&lock);
        // Block the main thread until the tap event is signaled or timeout occurs
        int rc = pthread_cond_timedwait(&tap_cond, &lock, &timeout);
        // Timeout occurred, no new taps within 5 seconds
        if(rc != 0)                         //time out and no more tap
        {
            switch (tap)
            {
                case 1:
                    printf("single tap\n");
                    break;
                case 2:
                    printf("double tap\n");
                    break;
                case 3:
                    printf("triple tap\n");
                    break;    
                default:
                    break;
            }
            tap = 0;  // Reset tap count after timeout
        }
        pthread_mutex_unlock(&lock);

    }
    pthread_join(thread, NULL);
    close(i2c_fd);
    pthread_mutex_destroy(&lock);
    return 0;
}

int pega_io_init(int *fd)
{
    /* Init GPIO */
    if (system("echo in > /sys/class/gpio/gpio60/direction") != 0) {
        perror("Failed to set GPIO 60 as input");
        return -1;
    }

    if (system("echo rising > /sys/class/gpio/gpio60/edge") != 0) {
        perror("Failed to set edge");
        return -1;
    }
    
    // open File
    *fd = open(GPIO_PATH, O_RDONLY);
    if (*fd == -1) {
        perror("Failed to open GPIO file");
        return -1;
    }

    return 0;
}
int pega_i2c_init(const char *i2c_bus,int i2c_addr,int *fd)
{   
    *fd = open(i2c_bus, O_RDWR);
    if (*fd < 0) {
        perror("Open I2C failed");
        return 1;
    }
    return 0;
}
bool pega_i2c_write(int fd, uint8_t* tx_buf, uint32_t tx_len) {
    struct i2c_msg messages[1];
    struct i2c_rdwr_ioctl_data packets;

    messages[0].addr  = ACC_ADDR;
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
bool pega_i2c_read(int fd, uint8_t* tx_buf, uint32_t tx_len, uint8_t* rx_buf, uint32_t rx_len) {
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data data;

    msgs[0].addr  = ACC_ADDR;
    msgs[0].flags = 0;  		// Write (register address)
    msgs[0].len   = tx_len;
    msgs[0].buf   = tx_buf;

    msgs[1].addr  = ACC_ADDR;
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
int Acc_evnet_init(int fd)
{
    uint8_t data_w[2] = {0};
    uint8_t data_r[2] = {0};

    /* Read  WHO AM I  Reg */   
    data_r[0] = WHO_AM_I;
    if (pega_i2c_read(fd, &data_r[0], sizeof(uint8_t), &data_r[1], sizeof(uint8_t)) < 0) {
        perror("I2C transfer failed");
        close(fd);
        return 1;
    } 
    if (data_r[1]!=0x44)
    {
        perror("Chip ID error");
        close(fd);
        return 1;
    }
    /* CTRL1 - Power MODE  */
    data_w[0] = CTRL1;
	data_w[1] = 0x74;                              
    pega_i2c_write(fd,data_w,sizeof(data_w));
     
    /* CTRL1 - Low Noise  */
    data_w[0] = CTRL6;
	data_w[1] = 0x04;                              
    pega_i2c_write(fd,data_w,sizeof(data_w));

    /* CTRL3 - Latch */
    data_w[0] = CTRL3;
	data_w[1] = 0x00;                              
    pega_i2c_write(fd,data_w,sizeof(data_w));

    /* TAP THS X */
    data_w[0] = TAP_THS_X;
	data_w[1] = 0x0C;//wake                          
    pega_i2c_write(fd,data_w,sizeof(data_w));

    /* TAP THS Y */
    data_w[0] = TAP_THS_Y;
	data_w[1] = 0xEC;//wake                          
    pega_i2c_write(fd,data_w,sizeof(data_w));

    /* TAP THS Z */
    data_w[0] = TAP_THS_Z;
	data_w[1] = 0xEC;//wake                          
    pega_i2c_write(fd,data_w,sizeof(data_w));

   /* INT_DUR */
    data_w[0] = INT_DUR;
	data_w[1] = 0x7F;//wake                          
    pega_i2c_write(fd,data_w,sizeof(data_w));

   /* WAKE_UP_THS */
    data_w[0] = WAKE_UP_THS;
	data_w[1] = 0x00;//wake                          
    pega_i2c_write(fd,data_w,sizeof(data_w));
    
   /* CTRL4_INT1_PAD_CTRL */
    data_w[0] = CTRL4;
	data_w[1] = 0x40;//wake                          
    pega_i2c_write(fd,data_w,sizeof(data_w));

    /* CTRL7  Enable the INT */
    data_w[0] = CTRL7;
	data_w[1] = 0x20;//wake                          
    pega_i2c_write(fd,data_w,sizeof(data_w));
    
    return 0;
}
void *poll_event_thread(void *arg) {
    struct pollfd fds[1];
    int gpio_fd, ret;
    char buf;
    uint8_t data_r[2] = {0};

    // Io Init
    if(pega_io_init(&gpio_fd)) {
        perror("Failed to initialize GPIO");
        close(gpio_fd);
        return NULL;
    }
    // Set pollfd struct
    fds[0].fd = gpio_fd;
    fds[0].events = POLLPRI;        // GPIO event (POLLPRI is for priority events like GPIO state change)
    read(gpio_fd, &buf, 1);         // Clear the GPIO signal
    while (1) {
        ret = poll(fds, 1, -1);     // Wait indefinitely for input
        if (ret > 0 && (fds[0].revents & POLLPRI)) {               
            // Clear Event
            data_r[0] = TAP_SRC;
            if (pega_i2c_read(i2c_fd, &data_r[0], sizeof(uint8_t), &data_r[1], sizeof(uint8_t)) < 0) {
                perror("I2C transfer failed");
                close(i2c_fd);
                return;
            }  
            lseek(gpio_fd, 0, SEEK_SET);
            read(gpio_fd, &buf, 1);        
            // Tap detected
            pthread_mutex_lock(&lock);
            if(tap==0)                           //Tap count 0 --> 1 , send a signal to unblock main loop
                pthread_cond_signal(&tap_cond);
            tap = (tap < 3) ? tap + 1 : 3;
            last_timestamp = get_current_timestamp_ms();
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}
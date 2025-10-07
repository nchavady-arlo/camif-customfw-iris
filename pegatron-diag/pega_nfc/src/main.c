#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <poll.h>


/* Device config */
#define GPIO_NUMBER "68"   // 你要用的 GPIO 編號
 
#define I2C_BUS "/dev/i2c-3"
#define NFC_ADDR 0x50
#define ID_REGISTER 0x3F

/* Register configuration*/
/* OPERATION_CONTROL_REG */
#define READYMODE				(1<<7)
#define TX_EN					(1<<6)
#define RX_EN					(1<<3)

/* MODE_DEF_REG */
#define ISO14443A				(1<<3)

/* BIR_RATE_REG */
#define KBIT106					0

/* Register Address */
#define IO_CONFIG_REG			0x00
#define IO_CONFIG_REG2			0x01
#define OPERATION_CONTROL_REG	0x02
#define MODE_DEF_REG			0x03
#define BIR_RATE_REG			0x04

#define MAION_INT_REG			0x1A
#define FIFO_STATUS_REG1		0x1E
#define FIFO_STATUS_REG2		0x1F

#define NUM_OF_TRANS_BYTE_REG1	0x22
#define NUM_OF_TRANS_BYTE_REG2	0x23


/* Command for Direct Command */
#define SET_DEFAULT				0xC0
#define STOP_ALL				0xC2
#define TRANS_WITH_CRC			0xC4
#define TRANS_NO_CRC			0xC5
#define TRANS_REQA				0xC6
#define TRANS_WUPA				0xC7
#define NFC_INITIAL_FIELD_ON	0xC8
#define RST_RX_GAIN				0xD5
#define ADJUST_REGULATORS		0xD6
#define CLEAR_FIFO				0xDB



/* GPIO Function */
int gpio_init();
int polling_irq();

/* NFC Function */ 
int NFCReg_dump(int i2c_fd, int start, int end);
int NFC_command(int i2c_fd, uint8_t command);
int read_nfc_register(int i2c_fd, uint8_t reg, uint8_t* val);
int load_nfc_fifo(int i2c_fd, uint8_t len, uint8_t* val);
int read_nfc_fifo(int i2c_fd, uint8_t len, uint8_t* val);
int write_nfc_register(int i2c_fd, uint8_t reg, uint8_t val);
int maskset_nfc_register(int i2c_fd, uint8_t reg, uint8_t val);


int main(int argc, char *argv[])
{
    int i2c_fd;
	uint8_t reg,val,fifo[20];
	int start,end;
	int status=0;
	
	if (argc < 2) {
        printf("Usage:\n");
        printf("  %s <i2c_bus_device> dump <start_reg> <end_reg>\n", argv[0]);
		printf("  %s <i2c_bus_device> init\n", argv[0]);
		printf("  %s <i2c_bus_device> write <reg> <value>\n", argv[0]);
		printf("  %s <i2c_bus_device> read <reg>\n", argv[0]);
		printf("  %s <i2c_bus_device> command <value>\n", argv[0]);
	
		return 1;
    }
	/* GPIO Init */
	gpio_init();
	
	
	/* Open I2C device */
    if ((i2c_fd = open(I2C_BUS, O_RDWR)) < 0) {
        perror("Failed to open I2C bus");
        return 1;
    }
	
	if (argc == 4 && strcmp(argv[1], "dump") == 0)
	{
        start 	=	strtol(argv[2], NULL, 0);
        end 	=	strtol(argv[3], NULL, 0);
        status = NFCReg_dump(i2c_fd, start, end);
		return status;
		
    }
	else if( argc == 2 && strcmp(argv[1], "init") == 0)
	{
		printf("Set Default\n");
		NFC_command(i2c_fd,SET_DEFAULT);

		printf("READY MODE\n");
		maskset_nfc_register(i2c_fd,OPERATION_CONTROL_REG,READYMODE|TX_EN);
		
		printf("Set mode\n");
		maskset_nfc_register(i2c_fd,MODE_DEF_REG,ISO14443A);

		printf("Bit rate\n");
		maskset_nfc_register(i2c_fd,BIR_RATE_REG,KBIT106);
		
		printf("Trans Byte\n");
		write_nfc_register(i2c_fd,NUM_OF_TRANS_BYTE_REG2,0x07);
		
		
	}
	else if( argc == 2 && strcmp(argv[1], "REQA") == 0)
	{
		printf("STOP ALL\n");
		NFC_command(i2c_fd,STOP_ALL);
		usleep(4 * 1000);
		
		printf("RSET RX Gain\n");
		NFC_command(i2c_fd,RST_RX_GAIN);
		usleep(4 * 1000);

		printf("NFC Initial Field On\n");
		NFC_command(i2c_fd,NFC_INITIAL_FIELD_ON);
		usleep(10 * 1000);

		printf("Trans REQA command\n");
		NFC_command(i2c_fd,TRANS_REQA);
		
	}
	else if( argc == 2 && strcmp(argv[1], "read_fifo") == 0)
	{
		read_nfc_fifo(i2c_fd,2,fifo);
		printf("  fifo =%s \n", fifo);
	}
	else if( argc == 3 && strcmp(argv[1], "read") == 0)
	{
		reg = (uint8_t)strtol(argv[2], NULL, 0);
		val = 0;
		if (read_nfc_register(i2c_fd, reg, &val) == 0) {
			printf("  [0x%02X] = 0x%02X\n", reg, val);
			status = 0;
		} else {
			status = 1;
		}
	}
	else if( argc == 4 && strcmp(argv[1], "write") == 0)
	{
		reg = (uint8_t)strtol(argv[2], NULL, 0);
		val = (uint8_t)strtol(argv[3], NULL, 0);;
		if (write_nfc_register(i2c_fd, reg, val) == 0) {
			printf("  [Done] Register Write\n");
			val = 0;
			read_nfc_register(i2c_fd, reg, &val);
			printf("  [0x%02X] = 0x%02X\n", reg, val);
			status = 0;
		} else {
			status = 1;
		}
	}
	else if( argc == 2 && strcmp(argv[1], "polling") == 0)
	{
		polling_irq();
	}
	else if( argc == 3 && strcmp(argv[1], "command") == 0)
	{
		val = (uint8_t)strtol(argv[2], NULL, 0);
		NFC_command(i2c_fd,val);
	}
	else {
        printf("Invalid arguments!\n");
		status = 1;
    }
	
	
	close(i2c_fd);
    return status;
}

int read_nfc_register(int i2c_fd, uint8_t reg, uint8_t* val)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    
    uint8_t reg_mode = 0x40 | reg; // Register read mode byte, per ST25R3918

    messages[0].addr  = NFC_ADDR;
    messages[0].flags = 0; // Write
    messages[0].len   = 1;
    messages[0].buf   = &reg_mode;

    messages[1].addr  = NFC_ADDR;
    messages[1].flags = I2C_M_RD; // Read
    messages[1].len   = 1;
    messages[1].buf   = val;

    packets.msgs  = messages;
    packets.nmsgs = 2;

    if (ioctl(i2c_fd, I2C_RDWR, &packets) < 0) {
        fprintf(stderr, "Failed to read from device 0x%02X: %s\n", NFC_ADDR, strerror(errno));
        return -1;
    }

    return 0;
}

int write_nfc_register(int i2c_fd, uint8_t reg, uint8_t data)
{
	struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

    uint8_t out_buf[2];
    out_buf[0] = reg;    
    out_buf[1] = data;

    messages[0].addr  = NFC_ADDR;
    messages[0].flags = 0;
    messages[0].len   = sizeof(out_buf);
    messages[0].buf   = out_buf;
	
    packets.msgs  = messages;
    packets.nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &packets) < 0) {
        fprintf(stderr, "Failed to write to device 0x%02X reg 0x%02X: %s\n",
                NFC_ADDR, reg, strerror(errno));
        return -1;
    }

    return 0;
}

int maskset_nfc_register(int i2c_fd, uint8_t reg, uint8_t flag)
{
	struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];
	uint8_t data;
	
	if(read_nfc_register(i2c_fd,reg,&data)!=0)
	{
		return -1;
	}
	data = data | flag;
    uint8_t out_buf[2];
    out_buf[0] = reg;    
    out_buf[1] = data;

    messages[0].addr  = NFC_ADDR;
    messages[0].flags = 0;
    messages[0].len   = sizeof(out_buf);
    messages[0].buf   = out_buf;

    packets.msgs  = messages;
    packets.nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &packets) < 0) {
        fprintf(stderr, "Failed to write to device 0x%02X reg 0x%02X: %s\n",
                NFC_ADDR, reg, strerror(errno));
        return -1;
    }

    return 0;
}

int NFC_command(int i2c_fd, uint8_t command)
{
	struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

    uint8_t out_buf[1];
    out_buf[0] = command;    

    messages[0].addr  = NFC_ADDR;
    messages[0].flags = 0;
    messages[0].len   = sizeof(out_buf);
    messages[0].buf   = out_buf;

    packets.msgs  = messages;
    packets.nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &packets) < 0) {
        fprintf(stderr, "Failed to command 0x%02X device 0x%02X : %s\n",
                NFC_ADDR, command, strerror(errno));
        return -1;
    }

    return 0;
	
}

int NFCReg_dump(int i2c_fd, int start, int end)
{
	if(start < 0x00 || end > 0x3f)
	{
		printf("Invalid Address Range\n");
		return -1;
	}
    printf("Dump Device 0x%02X Reg from 0x%02X to 0x%02X:\n", NFC_ADDR, start, end);
    for (int reg = start; reg <= end; reg++) {
        uint8_t val = 0;
        if (read_nfc_register(i2c_fd, reg, &val) == 0) {
            printf("  [0x%02X] = 0x%02X\n", reg, val);
        } else {
            printf("  [0x%02X] = < fail >\n", reg);
        }
    }
	return 0;
}

int load_nfc_fifo(int i2c_fd, uint8_t reg, uint8_t* val)
{
	struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    
    uint8_t reg_mode = 0x80; 

    messages[0].addr  = NFC_ADDR;
    messages[0].flags = 0; // Write
    messages[0].len   = 1;
    messages[0].buf   = &reg_mode;

    messages[1].addr  = NFC_ADDR;
    messages[1].flags = I2C_M_RD; // Read
    messages[1].len   = 2;
    messages[1].buf   = val;

    packets.msgs  = messages;
    packets.nmsgs = 2;

    if (ioctl(i2c_fd, I2C_RDWR, &packets) < 0) {
        fprintf(stderr, "Failed to read from device 0x%02X: %s\n", NFC_ADDR, strerror(errno));
        return -1;
    }

    return 0;	
}

int read_nfc_fifo(int i2c_fd, uint8_t reg, uint8_t* val)
{
	struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    
    uint8_t reg_mode = 0x9f; 

    messages[0].addr  = NFC_ADDR;
    messages[0].flags = 0; // Write
    messages[0].len   = 1;
    messages[0].buf   = &reg_mode;

    messages[1].addr  = NFC_ADDR;
    messages[1].flags = I2C_M_RD; // Read
    messages[1].len   = 2;
    messages[1].buf   = val;

    packets.msgs  = messages;
    packets.nmsgs = 2;

    if (ioctl(i2c_fd, I2C_RDWR, &packets) < 0) {
        fprintf(stderr, "Failed to read from device 0x%02X: %s\n", NFC_ADDR, strerror(errno));
        return -1;
    }

    return 0;	
}

int gpio_init()
{
    char path[64];

    // 1. export GPIO
    int export_fd = open("/sys/class/gpio/export", O_WRONLY);
    write(export_fd, GPIO_NUMBER, strlen(GPIO_NUMBER));
    close(export_fd);

    // 2. set direction
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", GPIO_NUMBER);
    int dir_fd = open(path, O_WRONLY);
    write(dir_fd, "in", 2);
    close(dir_fd);

    // 3. set edge to rising or falling or both
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/edge", GPIO_NUMBER);
    int edge_fd = open(path, O_WRONLY);
    write(edge_fd, "rising", 6); // or "falling", or "both"
    close(edge_fd);
	
	return 0;
}

int polling_irq() 
{
    char path[64];
    int gpio_fd;
    struct pollfd fdset;
    char buf[8];

    // open value for polling
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", GPIO_NUMBER);
    gpio_fd = open(path, O_RDONLY);

    // 清掉初始值
    read(gpio_fd, buf, sizeof(buf));

    printf("Start polling GPIO %s...\n", GPIO_NUMBER);

    while (1) {
        memset(&fdset, 0, sizeof(fdset));
        fdset.fd = gpio_fd;
        fdset.events = POLLPRI;

        int rc = poll(&fdset, 1, -1); // -1 = 等待直到事件發生
        if (rc < 0) {
            perror("poll failed");
            return 1;
        }

        if (fdset.revents & POLLPRI) {
            lseek(gpio_fd, 0, SEEK_SET);
            read(gpio_fd, buf, sizeof(buf));
            printf("Interrupt detected on GPIO %s, value: %c\n", GPIO_NUMBER, buf[0]);
        }
    }

    close(gpio_fd);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define SYSFS_PATH "/sys/devices/platform/soc/soc:motor/"

int write_sysfs(const char *filename, const char *value) {
    char path[256];
    int fd, ret;

    snprintf(path, sizeof(path), SYSFS_PATH "%s", filename);
    
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    ret = write(fd, value, strlen(value));
    if (ret < 0) {
        perror("write");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [motor: pan|rising] [direction: reverse|forward] [count]\n", argv[0]);
        return 1;
    }
	
    char *motor_name = argv[1];
    char *direction = argv[2];
    char *count = argv[3];
	int value; 
	system("lsmod | grep motor_v2 || insmod /lib/motor_v2.ko");
    
	if (strcmp(motor_name, "pan") == 0) {
        motor_name = "0";
    } else if (strcmp(motor_name, "rising") == 0) {
        motor_name = "1";
    } else {
        fprintf(stderr, "Invalid motor name: %s\n", motor_name);
        return 1;
    }

    if (strcmp(direction, "forward") == 0) {
        direction = "1";
    } else if (strcmp(direction, "reverse") == 0) {
        direction = "0";
    } else {
        fprintf(stderr, "Invalid motor name: %s\n", direction);
        return 1;
    }

	value = atoi(count);
	if (value < 0) {
		fprintf(stderr, "Invalid step count: %s\n", count);
		return 1;
	}

    if (write_sysfs("motor", motor_name) < 0) return 1;
    if (write_sysfs("direction", direction) < 0) return 1;
    if (write_sysfs("count", count) < 0) return 1;
    if (write_sysfs("timer", "1") < 0) return 1;

    printf("Motor command issued successfully.\n");

    return 0;
}


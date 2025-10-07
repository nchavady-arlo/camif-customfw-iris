#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include <linux/types.h>
#include <linux/iio/events.h>

#define DEVICE_PATH "/dev/iio:device0"  // 根據你的實際設備調整

int main(void)
{
    char buf[sizeof(struct iio_event_data)];
    struct pollfd pfd;

    pfd.fd = open(DEVICE_PATH, O_RDONLY);
    if (pfd.fd < 0) {
        perror("Failed to open IIO device");
        return 1;
    }

    pfd.events = POLLIN;

    printf("Listening for IIO events...\n");

    while (1) {
        int ret = poll(&pfd, 1, -1);  // 無限等待
        if (ret < 0) {
            perror("poll");
            break;
        }

        if (pfd.revents & POLLIN) {
            ssize_t len = read(pfd.fd, buf, sizeof(buf));
            if (len < 0) {
                perror("read");
                continue;
            }

            if (len == sizeof(struct iio_event_data)) {
                struct iio_event_data *event = (struct iio_event_data *)buf;
                printf("Event received!\n");
                printf("  Event ID   : 0x%016llx\n", (unsigned long long)event->id);
                printf("  Timestamp  : %llu\n", (unsigned long long)event->timestamp);
            } else {
                fprintf(stderr, "Unexpected event size: %zd bytes\n", len);
            }
        }
    }

    close(pfd.fd);
    return 0;
}


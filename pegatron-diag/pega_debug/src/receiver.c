📩 receiver.c — 接收端
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>    // For O_* constants
#include <sys/stat.h> // For mode constants
#include <unistd.h>

#define QUEUE_NAME "/myqueue"
#define MAX_SIZE 256

int main() {
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_SIZE];

    // 開啟訊息佇列（讀取模式）
    mq = mq_open(QUEUE_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }

    // 取得佇列屬性
    mq_getattr(mq, &attr);
    printf("Receiver ready (max msg size = %ld)\n", attr.mq_msgsize);

    while (1) {
        ssize_t bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        if (bytes_read >= 0) {
            buffer[bytes_read] = '\0';
            printf("Received: %s\n", buffer);
            if (strcmp(buffer, "exit") == 0)
                break;
        } else {
            perror("mq_receive");
        }
    }

    mq_close(mq);
    mq_unlink(QUEUE_NAME); // 刪除佇列
    printf("Receiver closed and queue deleted.\n");
    return 0;
}
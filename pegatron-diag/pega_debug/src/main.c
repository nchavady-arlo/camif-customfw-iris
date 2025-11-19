#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>    // For O_* constants
#include <sys/stat.h> // For mode constants
#include <unistd.h>
//==============================================================================
#include "pega_mq.h"
//==============================================================================
int main(int argc, char *argv[])
{
    int rtn = 0, i;
    stMsgQdata_t data;
	
	memset(&data, 0, sizeof(data));
	
    #if 0
    printf("[%s] argc=%d\n", __func__, argc);	
    for (i = 0; i < argc; i++) 
	{
        printf("argv[%d] = %s\n", i, argv[i]);
    }
	#endif

    data.CmdCount = 0;

    if (argv[1] != NULL)
        strncpy(data.CmdName, argv[1], sizeof(data.CmdName));
    else
        strncpy(data.CmdName, "debug", sizeof(data.CmdName));

    strncpy(data.CmdFrom, "pega_debug", sizeof(data.CmdFrom));
    
    // ? 正確拷貝 argv[]		
    data.stData.argc = argc-2;	
    
    #if 1
    if (data.stData.argc > MSGQ_ARGC_MAX) 
	{
        fprintf(stderr, "argc is over(%d,%d).\n", argc, MSGQ_ARGC_MAX);
        exit(EXIT_FAILURE);
    }
	#endif
	
    for (i = 2; i < argc; i++) 
	{
        strncpy(data.stData.argv[i-2], argv[i], MSGQ_ARGV_LEN - 1);
        data.stData.argv[i-2][MSGQ_ARGV_LEN - 1] = '\0';
    }

    // 打開 message queue
    mqd_t mq = mq_open(MSGQ_QUEUE_NAME, O_WRONLY);
	
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    // 傳送資料
    rtn = mq_send(mq, (const char *)&data, sizeof(data), 1);
    if (rtn == -1) {
        perror("mq_send");
    }

    mq_close(mq);
    return 0;
}

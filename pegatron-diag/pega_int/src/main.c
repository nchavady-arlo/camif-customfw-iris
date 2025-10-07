#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
//==============================================================================
#define GPIO_NUM "6"
#define GPIO_PATH "/sys/class/gpio/gpio" GPIO_NUM "/value"
//==============================================================================
/*
#define POLLIN 		0x0001  
#define POLLPRI 	0x0002  
#define POLLOUT 	0x0004  
#define POLLERR 	0x0008  
#define POLLHUP 	0x0010  
#define POLLNVAL 	0x0020  
  
#define POLLRDNORM 	0x0040  
#define POLLRDBAND 	0x0080  
#define POLLWRNORM 	0x0100  
#define POLLWRBAND 	0x0200  
#define POLLMSG 	0x0400  
#define POLLREMOVE 	0x1000  
#define POLLRDHUP 	0x2000

struct pollfd {  
 int fd;        //ゅン磞瓃才  
 short events;  //璶―琩?ㄆン被?  
 short revents; //ㄆン被?  
};  

琌eventsㄆン被?办POLLIN|POLLPRI?select?ㄆン
POLLPRI Τ候戈璶弄
?竚い?郉?よΑ:
   ??nonerisingfallingbothㄤいぇ??竚い?郉?よΑ?ń猽郉??ㄒ
   echo "falling" > /sys/class/gpio/gpio6/edge
*/
//============================================================================== 
int Pega_Gpio_interrupt_trigger_set(int sGpioNum, char *IntEvt) 
{  
    FILE *fp;
    char s1[50]={0};  
    
    //printf("\n%s[%d,%d]\n", __func__, sGpioNum, bHigh);  
    if (IntEvt == NULL)  
	{
		return -1;  
	}
	
    sprintf(s1,"/sys/class/gpio/gpio%d/edge",sGpioNum);  
  
    if ((fp = fopen(s1, "rb+")) == NULL)   
    {  
        printf("Cannot open %s\n",s1);  
        return -1;  
    }  
               
    fwrite(IntEvt, sizeof(char), strlen(IntEvt), fp);         
    fclose(fp);  
    
    return 0;  
}  

int main(int argc, char *argv[])
{
    int fd;
    struct pollfd pfd;
    char buf[3];
	
	Pega_Gpio_interrupt_trigger_set(6, "falling");
 
    // ゴ?GPIO valueゅン
    if ((fd = open(GPIO_PATH, O_RDONLY)) < 0) {
        perror("Failed to open gpio value");
        return -1;
    }
 
    // ?ゅン磞瓃才睰poll?疼蔨い
    pfd.fd = fd;
    pfd.events = POLLPRI;
 
    // ?Ω睲埃?玡い?
    read(fd, buf, sizeof(buf));
 
    printf("Waiting for GPIO interrupt...\n");
 
    // 单い?
    while (1) {
        // ㄏノpoll单い?ㄆン
        int ret = poll(&pfd, 1, -1);
      
	    // -1: poll error.
		//  0: poll time out.
        
        if (ret > 0) 
		{
            // ?琩い?琌痷郉?
            if (pfd.revents & POLLPRI) {
                lseek(fd, 0, SEEK_SET);  // 竚ゅン?竚
                read(fd, buf, sizeof(buf));
                printf("GPIO interrupt occurred, value: %c\n", buf[0]);
            }
        } else {
            perror("poll");
        }
    }
 
    close(fd);
    return 0;
}
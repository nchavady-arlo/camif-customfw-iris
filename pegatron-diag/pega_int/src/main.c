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
 int fd;        //���y�z��  
 short events;  //�n�D�d?���ƥ�?  
 short revents; //��^���ƥ�?  
};  

�p�W�Oevents�ƥ�?���Ȱ�APOLLIN|POLLPRI?���_select��?�ƥ�
POLLPRI ����檺��ƭnŪ
?�m��?�D?�覡:
   �i�H??none�Brising�Bfalling�Bboth�䤤���@??�m��?�D?�覡�C?���H�U���u�D??�ҡC
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
 
    // ��?GPIO value���
    if ((fd = open(GPIO_PATH, O_RDONLY)) < 0) {
        perror("Failed to open gpio value");
        return -1;
    }
 
    // ?���y�z�ŲK�[��poll?���^��
    pfd.fd = fd;
    pfd.events = POLLPRI;
 
    // ��?���@���H�M��?�e�i�઺��?
    read(fd, buf, sizeof(buf));
 
    printf("Waiting for GPIO interrupt...\n");
 
    // ���ݤ�?
    while (1) {
        // �ϥ�poll���ݤ�?�ƥ�
        int ret = poll(&pfd, 1, -1);
      
	    // -1: poll error.
		//  0: poll time out.
        
        if (ret > 0) 
		{
            // ?�d��?�O�_�u���D?
            if (pfd.revents & POLLPRI) {
                lseek(fd, 0, SEEK_SET);  // ���m���?����m
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
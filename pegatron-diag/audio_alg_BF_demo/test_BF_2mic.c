#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "AudioBfProcess.h"

#define USE_MALLOC   (1)
typedef unsigned char				uint8;
typedef unsigned short				uint16;
typedef unsigned long				uint32;

float AVERAGE_RUN(int a)
{
    static unsigned int num = 0;
    static float avg = 0;
    if(num == 0) avg = 0;
    num++;
    avg = avg + ((float)a - avg) / ((float)num);  
    return avg;
}
unsigned int _OsCounterGetMs(void)
{
	struct  timeval t1;
	gettimeofday(&t1,NULL);
	unsigned int T = ( (1000000 * t1.tv_sec)+ t1.tv_usec ) / 1000;
	return T;
}

int main(int argc, char *argv[])
{
	short input[256];
	short output[128];
	char infileName[512];
	char outfileName[512];

	FILE * fin, * fout;
	ALGO_BF_RET ret;
	int shape = 0;
	float direction = 0.0;
	int delay_sample = 0;
	float avg = 0;
	int counter = 0;
	unsigned int T0,T1;
	AudioBfInit bf_init;
	AudioBfConfig bf_config;
	BF_HANDLE handle;
	bf_init.mic_distance = 8.0;
	bf_init.point_number = 128;
	bf_init.sample_rate = 16000;
	bf_init.channel = 2;
	bf_config.noise_gate_dbfs = -20;
	bf_config.temperature = 20;
	bf_config.noise_estimation = 0;
	bf_config.output_gain = 0.7;
	bf_config.vad_enable = 0;
	bf_config.diagonal_loading = 10;
#if USE_MALLOC
	char *WorkingBuffer2;
	WorkingBuffer2 = (char*)malloc(IaaBf_GetBufferSize());
#endif
	handle = IaaBf_Init((char*)WorkingBuffer2, &bf_init);
	if (handle==NULL)
	{
		printf("BF init error\r\n");
		return -1;
	}
	else
	{
		printf("BF init succeed\r\n");
	}
	ret = IaaBf_SetConfig(handle,&bf_config);
	if (ret)
	{
		printf("Error occured in Config\n");
		return -1;
	}
	ret = IaaBf_SetShape(handle,shape);
	if (ret)
	{
		printf("Error occured in Array shape\n");
		return -1;
	}
	sprintf(infileName,"%s","./Chn-14.wav");
	sprintf(outfileName,"%s","./BFOut.pcm");

	fin = fopen(infileName, "rb");
	if(!fin)
	{		printf("the input file could not be open\n");
		return -1;
	}

	fout = fopen(outfileName, "wb");
	if(!fout)
	{
		printf("the output file could not be open\n");
		return -1;
	}

    fread(input, sizeof(char), 44, fin); // read header 44 bytes
	// fwrite(input, sizeof(char),44, fout); // write 44 bytes output
    // int delay_sample = 0;
	while(fread(input, sizeof(short), bf_init.point_number*bf_init.channel, fin))
	{
		counter++;
		T0  = (long)_OsCounterGetMs();
		ret = IaaBf_Run(handle,input,output,&direction);
		T1  = (long)_OsCounterGetMs();
		avg += (T1 - T0);
		if(ret)
		{
			printf("Error occured in Run\n");
			return -1;
		}
		fwrite(output, sizeof(short), bf_init.point_number, fout);
	}
	avg /= counter;
	printf("AVG is %.2f ms\n",avg);
	IaaBf_Free(handle);
	fclose(fin);
	fclose(fout);
	free(WorkingBuffer2);
	printf("Done\n");
	return 0;
}

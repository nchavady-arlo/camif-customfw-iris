#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "AudioBfProcess.h"

#define MIC_NUM (4)
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
	/*********Input file init*******/
	short input[512];
	short output[128];
	short input_tmp1[128],input_tmp2[128],input_tmp3[128],input_tmp4[128];
	FILE * fin0, * fin1, * fin2, * fin3, * fout;

	char infileName[MIC_NUM][512];
	char outfileName[512];
	ALGO_BF_RET ret;
	int k;
	float avg = 0;
	int counter = 0;
	unsigned int T0, T1;
	/********common setting between SSL and BF********/
	int point_number = 128;
	float microphone_distance = 4.0;
	int temperature = 20;
	int sample_rate = 16000;
	int shape =  0;
	// int delay_sample[MIC_NUM-1] = {0,0,0}; //channel-1
	float direction = 0.0;
	/*******BF data init*********/
#if USE_MALLOC
	char *WorkingBuffer_BF;
	WorkingBuffer_BF = (char*)malloc(IaaBf_GetBufferSize());
#endif

	AudioBfInit bf_init;
    AudioBfConfig bf_config;
    BF_HANDLE bf_handle;
	bf_init.mic_distance = microphone_distance;
	bf_init.point_number = point_number;
	bf_init.sample_rate = sample_rate;
	bf_init.channel = MIC_NUM;
	bf_config.noise_gate_dbfs = -20;
	bf_config.temperature = temperature;
	bf_config.noise_estimation = 0;
	bf_config.output_gain = 0.7;
	bf_config.vad_enable = 0;
	bf_config.diagonal_loading = 10;
	bf_handle = IaaBf_Init((char*)WorkingBuffer_BF, &bf_init);
	if (bf_handle==NULL)
	{
		printf("BF init error\r\n");
		return -1;
	}
	else
	{
		printf("BF init succeed\r\n");
	}
	ret = IaaBf_SetConfig(bf_handle,&bf_config);
	if (ret)
	{
		printf("Error occured in Config\n");
		return -1;
	}
	ret = IaaBf_SetShape(bf_handle,shape);
	if (ret)
	{
		printf("Error occured in Array shape\n");
		return -1;
	}

	/********open input file and output file*****/
	sprintf(infileName[0],"%s","./Chn-01.wav");
	sprintf(infileName[1],"%s","./Chn-02.wav");
	sprintf(infileName[2],"%s","./Chn-03.wav");
	sprintf(infileName[3],"%s","./Chn-04.wav");
	
	sprintf(outfileName,"%s","./BFOut.pcm");

	fin0 = fopen(infileName[0], "rb");
	if(!fin0)
	{		
		printf("the input file 0 could not be open\n");
		return -1;
	}

	fin1 = fopen(infileName[1], "rb");
	if(!fin1)
	{		
		printf("the input file 1 could not be open\n");
		return -1;
	}
	fin2 = fopen(infileName[2], "rb");
	if(!fin2)
	{		
		printf("the input file 2 could not be open\n");
		return -1;
	}
	fin3 = fopen(infileName[3], "rb");
	if(!fin3)
	{		
		printf("the input file 3 could not be open\n");
		return -1;
	}
	fout = fopen(outfileName, "wb");
	if(!fout)
	{
		printf("the output file could not be open\n");
		return -1;
	}
	fread(input, sizeof(char), 44, fin0);
	fread(input, sizeof(char), 44, fin1);
	fread(input, sizeof(char), 44, fin2);
	fread(input, sizeof(char), 44, fin3);
	fwrite(input, sizeof(char),44, fout);
	short * input_ptr;

	while(fread(input_tmp1, sizeof(short), bf_init.point_number, fin0))
	{
		fread(input_tmp2, sizeof(short), bf_init.point_number, fin1);
		fread(input_tmp3, sizeof(short), bf_init.point_number, fin2);
		fread(input_tmp4, sizeof(short), bf_init.point_number, fin3);
		input_ptr = input;
		for(k=0;k<point_number;k++)
		{
			*input_ptr =  input_tmp1[k];
			input_ptr++;
			*input_ptr =  input_tmp2[k];
			input_ptr++;
			*input_ptr =  input_tmp3[k];
			input_ptr++;
			*input_ptr =  input_tmp4[k];
			input_ptr++;
		}
		counter++;
		T0  = (long)_OsCounterGetMs();
		ret = IaaBf_Run(bf_handle,input,output,&direction);
		T1  = (long)_OsCounterGetMs();
		avg += (T1 - T0);
		if(ret)
		{
			printf("Error occured in Run\n");
			return -1;
		}
		fwrite(output, sizeof(short),point_number, fout);
	}
	avg /= counter;
	printf("AVG is %.2f ms\n",avg);
	IaaBf_Free(bf_handle);
	fclose(fin0);
	fclose(fin1);
	fclose(fin2);
	fclose(fin3);
	fclose(fout);
	free(WorkingBuffer_BF);
	printf("Done\n");

	return 0;
}

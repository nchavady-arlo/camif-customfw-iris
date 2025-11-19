#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "AudioSslProcess.h"

#define MIC_NUM (4)
#define USE_MALLOC   (1)
typedef unsigned char				uint8;
typedef unsigned short				uint16;
typedef unsigned long				uint32;

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
	short input_tmp1[128],input_tmp2[128],input_tmp3[128],input_tmp4[128];
	char infileName[MIC_NUM][512];
	char outfileName[512];
	FILE * fin0,* fin1,* fin2,* fin3;
	FILE * fout;
	int k;
	ALGO_SSL_RET ret;
	int counter2 = 0;
	unsigned int T0,T1,T2,T3;
	float avg = 0.0;
	float avg2 = 0.0;
	/********common setting  SSL ********/
	int point_number = 128;
	float microphone_distance = 4.0;
	int temperature = 20;
	int sample_rate = 16000;
	int delay_sample[MIC_NUM-1] = {0,0,0}; //channel-1
	int shape  = 0;
	int direction = 0;
	int frame_number = 32;
	/********SSL data init********/
	int counter = 0;
#if USE_MALLOC
    char *WorkingBuffer_SSL;
    WorkingBuffer_SSL = (char*)malloc(IaaSsl_GetBufferSize());
#endif
	AudioSslInit ssl_init;
    AudioSslConfig ssl_config;
    SSL_HANDLE ssl_handle;
	
	ssl_init.mic_distance = microphone_distance;
    ssl_init.point_number = point_number;
    ssl_init.sample_rate = sample_rate;
	ssl_init.bf_mode = 0;
	ssl_init.channel  = MIC_NUM;
    ssl_config.temperature = temperature;
    ssl_config.noise_gate_dbfs = -80;
	ssl_config.direction_frame_num = frame_number;

	/*******init algorithm *****/
	ssl_handle = IaaSsl_Init((char*)WorkingBuffer_SSL, &ssl_init);
	if (ssl_handle == NULL)
	{
		printf("Init fail\n\r");
		return -1;
	}
	else
	{
		printf("SSL init succeed\n\r");
	}

	ret = IaaSsl_Config(ssl_handle ,&(ssl_config));
	if (ret)
	{
		printf("Error occured in SSL Config\n\r");
		return -1;
	}

	ret = IaaSsl_Set_Shape(ssl_handle,shape);
	if (ret)
	{
		printf("Error occured in Array shape\n\r");
		return -1;
	}

	ret = IaaSsl_Cal_Params(ssl_handle);
	if (ret)
	{
		printf("Error occured in Array matrix calculation\n\r");
		return -1;
	}
	
	/********open input file and input file*****/

	sprintf(infileName[0],"%s","./Chn-01.wav");
	sprintf(infileName[1],"%s","./Chn-02.wav");
	sprintf(infileName[2],"%s","./Chn-03.wav");
	sprintf(infileName[3],"%s","./Chn-04.wav");
	sprintf(outfileName,"%s","./SSL_result.txt");
	fin0 = fopen(infileName[0], "rb");
	if(!fin0)
	{		
		printf("the input file0 could not be open\n\r");
		return -1;
	}
	fin1 = fopen(infileName[1], "rb");
	if(!fin1)
	{		
		printf("the input file 1 could not be open\n\r");
		return -1;
	}
	fin2 = fopen(infileName[2], "rb");
	if(!fin2)
	{		
		printf("the input file 2 could not be open\n\r");
		return -1;
	}
	fin3 = fopen(infileName[3], "rb");
	if(!fin3)
	{		
		printf("the input file 3 could not be open\n\r");
		return -1;
	}
	fout = fopen(outfileName, "w");
	if(!fout)
	{
		printf("the output file could not be open\n\r");
		return -1;
	}

	fread(input, sizeof(char), 44, fin0); // read header 44 bytes
	fread(input, sizeof(char), 44, fin1); // read header 44 bytes
	fread(input, sizeof(char), 44, fin2); // read header 44 bytes
	fread(input, sizeof(char), 44, fin3); // read header 44 bytes
	
	short * input_ptr;
	fprintf(fout,"%s\t%s\t%s\n\r","time","direction","case");
	while(fread(input_tmp1, sizeof(short), point_number, fin0))
	{
		fread(input_tmp2, sizeof(short), point_number, fin1);
		fread(input_tmp3, sizeof(short), point_number, fin2);
		fread(input_tmp4, sizeof(short), point_number, fin3);
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
		ret = IaaSsl_Run(ssl_handle,input,delay_sample);
		if(ret != 0)
		{
			printf("The Run fail\n");
			return -1;
		}
		// low resolution
//		if (ssl_init.bf_mode == 1)
//		{
//			printf("delay_sample: %d,%d,%d\n",delay_sample[0],delay_sample[1],delay_sample[2]);
//		}
		T1  = (long)_OsCounterGetMs();
		avg += (T1-T0);
		
		if(counter == ssl_config.direction_frame_num && ssl_init.bf_mode == 0)
		{
			counter2++;
			counter= 0;
			T2  = (long)_OsCounterGetMs();
			ret = IaaSsl_Get_Direction(ssl_handle, &direction);
			T3  = (long)_OsCounterGetMs();
			avg2 += (T3-T2);
			if(ret != 0 && ret!=ALGO_SSL_RET_RESULT_UNRELIABLE && ret!=ALGO_SSL_RET_BELOW_NOISE_GATE&&ret!=ALGO_SSL_RET_DELAY_SAMPLE_TOO_LARGE)
			{
				printf("The Get_Direction fail\n");
				return -1;
			}
			// write txt file
			fprintf(fout,"%f\t%d",(float)(counter2*ssl_config.direction_frame_num*0.008),direction);
			if (ret==0)
			{
				fprintf(fout,"\t%s\n\r","current time is reliable!");
			}
			else if (ret==ALGO_SSL_RET_BELOW_NOISE_GATE)
			{
				fprintf(fout,"\t%s\n\r","current time volume is too small!");
			}
			else if(ret==ALGO_SSL_RET_DELAY_SAMPLE_TOO_LARGE)
			{
				fprintf(fout,"\t%s\n\r","current time delay_sample is out of range!");
			}
			else
			{
				fprintf(fout,"\t%s\n\r","current time is not reliable!");
			}
			// reset voting
			ret = IaaSsl_Reset_Mapping(ssl_handle);
			if(ret != 0)
			{
				printf("The ResetVoting fail\n");
				return -1;
			}
		}
	}
	avg  = avg / (float)(ssl_config.direction_frame_num*counter2);
	avg2 = avg2 / (float)(counter2);
	printf("AVG for IaaSSL_RUN is %.3f ms\n",avg);
	printf("AVG for IaaSSL_GetDirection is %.3f ms\n",avg2);
	IaaSsl_Free(ssl_handle);
	fclose(fin0);
	fclose(fin1);
	fclose(fin2);
	fclose(fin3);
	fclose(fout);
	free(WorkingBuffer_SSL);
	printf("Done\n");
	
    return 0;
}


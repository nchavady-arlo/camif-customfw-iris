#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "AudioSslProcess.h"
#include "cam_fs_wrapper.h"

#define MIC_NUM (2)
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
	/*******Input file init*********/
	short input[256];
	char infileName[512];
	char outfileName[512];
	FILE * fin;
	FILE * fout;
	ALGO_SSL_RET ret;
	int counter2 = 0;
	unsigned int T0,T1,T2,T3;
	float avg = 0.0;
	float avg2 = 0.0;
	/**********common setting SSL***************/
	int point_number = 128;		//480
	float microphone_distance = 5.5;
	int temperature = 28;
	int sample_rate = 16000;
	int delay_sample[1] = {0};
	int shape = 0;
	int direction = 0;
	int frame_number = 32;
	/**************SSL data init***********/
	int counter = 0;
#if USE_MALLOC
    char *WorkingBuffer2;
	WorkingBuffer2 = (char*)malloc(IaaSsl_GetBufferSize());
#endif
    AudioSslInit ssl_init;
    AudioSslConfig ssl_config;
    SSL_HANDLE handle;
	
    ssl_init.mic_distance = microphone_distance; //cm
    ssl_init.point_number = point_number;
    ssl_init.sample_rate = sample_rate;
    ssl_init.bf_mode = 0;
	ssl_init.channel = MIC_NUM;
    ssl_config.temperature = temperature; //c
    ssl_config.noise_gate_dbfs = -80;
    ssl_config.direction_frame_num = frame_number;
	/******init algorithm********/
	handle = IaaSsl_Init((char*)WorkingBuffer2, &ssl_init);
	if (handle==NULL)
	{
		printf("SSL init error\n\r");
		return -1;
	}
	else
	{
		printf("SSL init succeed\n\r");
	}
	
	ret = IaaSsl_Config(handle ,&(ssl_config));
	if (ret)
	{
		printf("Error occured in SSL Config\n\r");
		return -1;
	}
	ret = IaaSsl_Set_Shape(handle,shape);
	if (ret)
	{
		printf("Error occured in Array shape\n\r");
		return -1;
	}

	ret = IaaSsl_Cal_Params(handle);
	if (ret)
	{
		printf("Error occured in Array matrix calculation\n\r");
		return -1;
	}

	sprintf(infileName,"%s","/tmp/stereo_output.pcm");
	sprintf(outfileName,"%s","/tmp/SSL_result.txt");
	
	fin = fopen(infileName, "rb");
	if(!fin)
	{		printf("the input file 0 could not be open\n\r");
		return -1;
	}

	fout = fopen(outfileName, "w");
	if(!fout)
	{
		printf("the output file could not be open\n\r");
		return -1;
	}

    //fread(input, sizeof(char), 44, fin); // read header 44 bytes
	fprintf(fout,"%s\t%s\t%s\n\r","time","direction","case");
	while(fread(input, sizeof(short), ssl_init.point_number*2, fin))
	{
		counter++;
		T0  = (long)_OsCounterGetMs();
		ret = IaaSsl_Run(handle,input,delay_sample);
		if(ret != 0)
		{
			printf("The Run fail\n");
			return -1;
		}
		// low resolution
//		if (ssl_init.bf_mode == 1)
//		{
//			printf("delay_sample: %d\n",delay_sample[0]);
//		}
		T1  = (long)_OsCounterGetMs();
		avg += (T1-T0);
		if(counter == ssl_config.direction_frame_num && ssl_init.bf_mode == 0)
		{
			counter2++;
			counter= 0;
			T2  = (long)_OsCounterGetMs();
			ret = IaaSsl_Get_Direction(handle, &direction);
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
			ret = IaaSsl_Reset_Mapping(handle);
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
	IaaSsl_Free(handle);
	fclose(fin);
	fclose(fout);
	free(WorkingBuffer2);
	printf("Done\n");
    return 0;
}


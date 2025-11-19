#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AudioBfProcess.h"
#include "AudioSslProcess.h"
#include "cam_fs_wrapper.h"
// MI API
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_ai.h"
#include "mi_ao.h"


/************************************************************************/
// Definitions
/************************************************************************/
#define MAX_LINE_LEN (1024)
#define MAX_BUF_LEN (256)
#define MIN_BUF_LEN (64)

// Thread sleep intervals (microseconds)
#define SLEEP_10MS (10000)
#define SLEEP_500MS (500000)

// Audio Parameters
#define CHANNEL_NUM (1)
#define BIT_PER_SAMPLE (16)
#define SAMPLE_RATE (16000)
#define MAX_RECORD_TIME (-1)
#define PERIOD_SIZE (1020) // samples per frame
#define INTERLEAVED FALSE

#define MI_AI_DEV_ID	0
#define MI_AI_CHN_ID	0
#define MI_AI_PORT_ID	0

// Engine Parameters
#define DSPOTTER_MAX_TIME (500)
#define DSPOTTER_NUM_GROUPS (1)
#define CNSV_MAX_RECORD_TIME (10)
#define CNSV_MAX_TEST_SPEAKERS (10)
#define CNSV_ONNX_THREADS (1)
#define CCLEVER_MAX_CMD_LEN (128)

// Training Parameters
#define TRAINING_SESSIONS (3)

// Command Mode Timeout (10 seconds = 160000 samples at 16kHz)
#define COMMAND_MODE_TIMEOUT_SAMPLES (160000)

// Return Values
#define SUCCESS (0)
#define ERROR (-1)

#define PEGA_PATCH_ENABLE 1

int main(void)
{
    MI_AI_Data_t stAiData;
    MI_AI_Attr_t stAiAttr;
    MI_SYS_ChnPort_t stChnOutputPort;
    int nRet;
  	ALGO_SSL_RET ret;
	MI_S16 s16Gain[] = {18,18};
 
    // MI SYS Init
    nRet = MI_SYS_Init(0);
    if (nRet != MI_SUCCESS)
    {
        printf("Error: MI_SYS_Init Failed, Return Value: %d\n", nRet);
        return -1;
    }

    // File Size = Sample points × Mic readings × 2 channels × 2 bytes per channel
    // MI AI Audio Setting
    memset(&stAiAttr, 0, sizeof(MI_AI_Attr_t));
    stAiAttr.enFormat = E_MI_AUDIO_FORMAT_PCM_S16_LE;       // 16-bit little-endian PCM 
    stAiAttr.enSoundMode = E_MI_AUDIO_SOUND_MODE_STEREO;    // Stereo
    stAiAttr.enSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;   // 16K hz
    stAiAttr.u32PeriodSize = PERIOD_SIZE;                   // samle point
    stAiAttr.bInterleaved = true;                           // DataFrame : L.R.L.R.....
    
    nRet = MI_AI_Open(MI_AI_DEV_ID, &stAiAttr);
    if (nRet != MI_SUCCESS)
    {
        printf("Error: MI_AI_Open Failed, Return Value: %d\n", nRet);
        return -1;
    }

    // DMIC config
    MI_AI_If_e MI_AI_Interface = E_MI_AI_IF_DMIC_A_01;  
    nRet = MI_AI_AttachIf(MI_AI_DEV_ID, &MI_AI_Interface, 1);
    if (nRet != MI_SUCCESS)
    {
        printf("Error: MI_AI_AttachIf Failed, Return Value: %d\n", nRet);
        MI_AI_Close(MI_AI_DEV_ID);
        return -1;
    }

    // MI AI Channel Setting
    memset(&stChnOutputPort, 0, sizeof(stChnOutputPort));
    stChnOutputPort.eModId = E_MI_MODULE_ID_AI;
    stChnOutputPort.u32DevId = MI_AI_DEV_ID;
    stChnOutputPort.u32ChnId = MI_AI_CHN_ID;
    stChnOutputPort.u32PortId = MI_AI_PORT_ID;
    nRet = MI_SYS_SetChnOutputPortDepth(0, &stChnOutputPort, 8, 8); 
    if (nRet != MI_SUCCESS)
    {
        printf("Error: MI_SYS_SetChnOutputPortDepth Failed, Return Value: %d\n", nRet);
        MI_AI_Close(MI_AI_DEV_ID);
        return -1;
    }

    // MI AI Set Gain 
    nRet = MI_AI_SetGain(MI_AI_DEV_ID, MI_AI_CHN_ID, s16Gain, sizeof(s16Gain) / sizeof(s16Gain[0]));
    if (nRet != MI_SUCCESS)
    {
        printf("Error: MI_AI_SetGain Failed, Return Value: %d\n", nRet);
        MI_AI_Close(MI_AI_DEV_ID);
        return -1;
    }

    // MI AI Channel Group Enable (Audio Start)
    nRet = MI_AI_EnableChnGroup(MI_AI_DEV_ID, MI_AI_CHN_ID);
    if (nRet != MI_SUCCESS)
    {
        printf("Error: MI_AI_EnableChnGroup Failed, Return Value: %d\n", nRet);
        MI_AI_Close(MI_AI_DEV_ID);
        return -1;
    }
	FILE *file = fopen("/tmp/stereo_output.pcm", "ab");
    if (!file) {
        fprintf(stderr, "Fail to Create pcm.\n");
        return 1;      
    }

    int times=300;
    printf("Start Recording!---------------------------------\n");
    while(times)
    {
        memset(&stAiData, 0, sizeof(MI_AI_Data_t));
        nRet = MI_AI_Read(MI_AI_DEV_ID, MI_AI_CHN_ID, &stAiData, NULL, 1000);
        if (nRet == MI_SUCCESS)
        {
            // Process the Mi Data
            fwrite(stAiData.apvBuffer[0], 1, stAiData.u32Byte[0], file);
            MI_AI_ReleaseData(MI_AI_DEV_ID, MI_AI_CHN_ID, &stAiData, NULL);
        }
        else if (nRet == MI_AI_ERR_NOBUF)
        {
            usleep(1000);
        }
        else
        {
            printf("MI_AI_Read error: %d\n", nRet);
        }
        times--;
    }
    fclose(file);

    printf("Store the recording as /tmp/stereo_output.pcm.\n");
    MI_AI_DisableChnGroup(MI_AI_DEV_ID, MI_AI_CHN_ID);
    MI_AI_Close(MI_AI_DEV_ID);
    printf("Audio recording stopped\n");
    return 0;
}
/* Pega - 2024 
modified from 2018-2019 Sigmastar Technology Corp. 
*/

/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (��Sigmastar Confidential Information��) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_ai.h"

#define ExecFunc(func, _ret_) \
    printf("%d Start test: %s\n", __LINE__, #func);\
    if (func != _ret_)\
    {\
        printf("AO_TEST [%d] %s exec function failed\n",__LINE__, #func);\
        return 1;\
    }\
    else\
    {\
        printf("AO_TEST [%d] %s  exec function pass\n", __LINE__, #func);\
    }\
    printf("%d End test: %s\n", __LINE__, #func);

#define MI_AUDIO_SAMPLE_PER_FRAME 1024
#define ST_DEFAULT_SOC_ID    0


int main(int argc, char *argv[])
{
    // MI_S32 s32Ret = E_MI_ERR_FAILED;
    MI_AI_Attr_t stSetAttr;
    MI_AI_Attr_t stGetAttr;
    MI_AUDIO_DEV AiDevId = 1;
    //MI_SYS init
    ExecFunc(MI_SYS_Init(ST_DEFAULT_SOC_ID),MI_SUCCESS);
    //set Ao Attr struct
    memset(&stSetAttr, 0, sizeof(MI_AI_Attr_t));
    /*
    stSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stSetAttr.eWorkmode = E_MI_AUDIO_I2S_MODE_I2S_MASTER;
    stSetAttr.u32FrmNum = 6;
    stSetAttr.u32PtNumPerFrm = MI_AUDIO_SAMPLE_PER_FRAME;
    stSetAttr.u32ChnCnt = 2 ;
    stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
    stSetAttr.eSamplerate = E_SAMPLE_RATE_16000;
    */
	
    stSetAttr.enFormat = E_MI_AUDIO_FORMAT_PCM_S16_LE;	//PCM Linear 16bit (Little Endian)
    stSetAttr.enSoundMode = E_MI_AUDIO_SOUND_MODE_STEREO;
    stSetAttr.enSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;
    stSetAttr.u32PeriodSize = MI_AUDIO_SAMPLE_PER_FRAME;
    stSetAttr.bInterleaved = true;
    
    /* open AO Device */   
    ExecFunc(MI_AI_Open(AiDevId,&stGetAttr), MI_SUCCESS);

    /* get ao device*/
    ExecFunc(MI_AI_GetAttr(AiDevId, &stGetAttr), MI_SUCCESS);

    /* disable ao device */
    ExecFunc(MI_AI_Close(AiDevId), MI_SUCCESS);
	
    return 0;
}

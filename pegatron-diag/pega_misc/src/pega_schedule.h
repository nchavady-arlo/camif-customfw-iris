#ifndef _PEGA_5G_SCHEDULE_H_
#define _PEGA_5G_SCHEDULE_H_

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#define SCH_HANDLER_DELAY_CNT		(100*1000) //ms
#define SCH_100ms           		  1 //100ms
#define SCH_200ms           		  2 //100ms
#define SCH_300ms           		  3 //100ms
#define SCH_BUFF_MAX				  12 // 10
//==============================================================================
#define SCH_1Sec           		      SCH_100ms * 10
#define SCH_2Sec           		      SCH_100ms * 20
#define SCH_1Min           		      SCH_1Sec  * 60
#define SCH_1Hour           		  SCH_1Min  * 60
//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================
typedef enum
{
/*000*/schEVT_None = 0,  
/*001*/schEVT_DeleteAll, 
/*002*/schEVT_Load_wifi_fw,
/*003*/schEVT_wifi_connect_enable,

/*200*/schEVT_System_msgq_Cmd_Execution = 200,
/*201*/schEVT_Factory_Reset,
/*202*/schEVT_Max, 
}enScheduleEventType;

typedef struct
{
    unsigned char enEVT;    
    int  		  wTMCount; //ms
    int  		  wTMTime; //ms
}stSchDefType;
//==============================================================================
void pega_schedule_handler_Start(void);
int  pega_schedule_Event_push(unsigned char eEvent, unsigned int wDelayTime);
void pega_schedule_Event_pop(void);
int  pega_schedule_Event_Cancel(unsigned char ucEvent);
int  pega_schedule_Event_Check(unsigned char ucEvent);
int  pega_schedule_Event_DelayTime_Get(unsigned char ucEvent, unsigned int *u16Delayms);
int  pega_schedule_Event_Time_Get(unsigned char ucEvent, unsigned int *u16Timems);
//==============================================================================
int  pega_schedule_Event_Time_Count_Set(unsigned char ucEvent);
int  pega_schedule_Event_Time_Count_Get(unsigned char ucEvent, unsigned int *u16Delayms);
//==============================================================================
void pega_schedule_Data_Info_Print(void);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_5G_SCHEDULE_H_

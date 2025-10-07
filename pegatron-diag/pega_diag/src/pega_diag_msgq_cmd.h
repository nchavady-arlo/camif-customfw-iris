#ifndef _PEGA_DIAG_MSGQ_CMD_H_
#define _PEGA_DIAG_MSGQ_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
#define MSGQ_MISC_DIAG_QUEUE_KEY  85695540  //send msg to pega_misc
//==============================================================================
#define MSG_MI_QUEUE_KEY  83583500  //send msg to mediaserver
//==============================================================================
typedef struct
{
	  int 	msgid;	  
	  int   eCmdId;   
	  int   value1;      
      int   value2;	   
	  int   value3;	  
	  float fval1;	
	  float fval2;	
}stDiagMsgqCmdType;

typedef enum
{
    MSG_TYPE_mi_NULL	        = 0,
    MSG_TYPE_mi_ctrl	        = 1,
    MSG_TYPE_mi_video0	        = 2,
    MSG_TYPE_mi_video1	        = 3,
    MSG_TYPE_mi_video2	        = 4,
    MSG_TYPE_mi_ai0             = 5,
    MSG_TYPE_mi_ai1             = 6,
    MSG_TYPE_mi_ai2             = 7,
    MSG_TYPE_mi_ao              = 8,
    MSG_TYPE_mi_video_client	= 9,
    MSG_TYPE_mi_ai_client       = 10,
    MSG_TYPE_mi_ao_client       = 11,
    MSG_TYPE_mi_aec             = 12,
    MSG_TYPE_mi_ive             = 13,
    MSG_TYPE_mi_isp             = 14,
    MSG_TYPE_mi_snapshot        = 15,
    MSG_TYPE_mi_ctrl_client	    = 16,
    MSG_TYPE_mi_ive_client      = 17,
    MSG_TYPE_mi_snapshot_client = 18,
    MSG_TYPE_mi_framerate_rtn   = 19,
    MSG_TYPE_mi_ao_file_client  = 20,
    MSG_TYPE_mi_ao_siren_client = 21,
    MSG_TYPE_mi_MAX
} T_MsgmiType;

typedef enum
{
    MSG_COMMAND_mi_NULL	        = 0,
    MSG_COMMAND_mi_init	        = 1,
    MSG_COMMAND_mi_uninit	    = 2,
    MSG_COMMAND_mi_vrheader     = 3,
    MSG_COMMAND_mi_vclose       = 4,
    MSG_COMMAND_mi_vresize      = 5,
    MSG_COMMAND_mi_vencctrl     = 6,
    MSG_COMMAND_mi_hdrctrl      = 7,
    MSG_COMMAND_mi_aecctrl      = 8,
    MSG_COMMAND_mi_readpacket   = 9, //read from share memory 
    MSG_COMMAND_mi_writepacket  = 10, //write to share memory
    MSG_COMMAND_mi_mdctrl       = 11,
    MSG_COMMAND_mi_3DNRctrl     = 12,
    MSG_COMMAND_mi_ortn_ctrl    = 13,
    MSG_COMMAND_mi_ao_wheader   = 14,
    MSG_COMMAND_mi_ao_close     = 15,
    MSG_COMMAND_mi_ao_vol_ctrl  = 16,
    MSG_COMMAND_mi_ao_vqe_ctrl  = 17,
    MSG_COMMAND_mi_ai_rheader   = 18,
    MSG_COMMAND_mi_ai_close     = 19,
    MSG_COMMAND_mi_ai_vol_ctrl  = 20,
    MSG_COMMAND_mi_ai_vqe_ctrl  = 21,
    MSG_COMMAND_mi_isp_ctrl     = 22,
    MSG_COMMAND_mi_cap_jpeg     = 23,
    MSG_COMMAND_mi_hc_result    = 24,
    MSG_COMMAND_mi_hd_result    = 25,
    MSG_COMMAND_mi_vmirrorflip  = 26,

    MSG_COMMAND_mi_set_siren_stat       = 97,
    MSG_COMMAND_mi_send_capjpgurl       = 98,
    MSG_COMMAND_mi_set_capresolution    = 99,  // pre set capture resolution mode
    MSG_COMMAND_mi_ive_debug            = 100,    
    MSG_COMMAND_mi_frm_result           = 101,
    MSG_COMMAND_mi_aac_enable_switch    = 102,
    MSG_COMMAND_mi_ao_force_close       = 103,
    MSG_COMMAND_mi_opus_enable_switch   = 104,
    MSG_COMMAND_mi_video_window_crop    = 105,
    MSG_COMMAND_mi_exposure_comp        = 106,
	MSG_COMMAND_mi_MAX
} T_Msgmicommand;

typedef struct
{
    int enable;
    int mi_stream_index;
    int width;
    int hight;
    int bitrate;
    int video_format;
    int frame_rate;
    int GOP;
    int RC_mode;
    int min_qp;
    int max_qp;
    int hdr_enb;
    int e3DNR_level;
    int Rotation;
    int crop_offset_x;
    int crop_offset_y;
    int h264_profile;
}Video_Stream_info;

typedef struct
{
    int stream_num;
    int stream_index;
    Video_Stream_info vstream_info[3];
} T_VstreamInfoMsg;

typedef struct
{
    int stream_index;
    int sample_rate;
    int channels;
    int bitrate;
    int sample_preframe;
} T_AstreamInfoMsg;

typedef enum
{
	PACKET_TYPE_NULL	    = 0,
    PACKET_TYPE_video	    = 1,
    PACKET_TYPE_ai	        = 2,
    PACKET_TYPE_ao	        = 3,
	PACKET_TYPE_MAX
} T_MsgpacketType;

typedef struct
{
    T_MsgpacketType iType;
    int stream_index;
    int64_t pkt_pts;
    unsigned int pkt_size;
    unsigned int shmq_rindex;
    unsigned int shmq_windex;
} T_PacketInfoMsg;

typedef struct
{
	int cmdid;
    int cali_item;
    int gain;
    int shutter;
    int output_telnet;
} T_FactoryCaliMsg;

typedef enum
{
    isp_null            = 0,
    isp_Saturation      = 1,
    isp_Contrast        = 2,
    isp_Brightness      = 3,
    isp_daynightmode    = 4,
    isp_Set_Flicker     = 5,
    isp_Set_Framerate   = 6,
    isp_Get_Framerate   = 7,
    isp_exposure_comp   = 8,
    isp_iva_control = 9,
    iqserver_switch   = 100,
	isp_factory_cali = 101,
    isp_MAx             
} T_Isp_Ctrl;

typedef struct
{
	T_MsgmiType         iType;
    T_Msgmicommand      command;
    int                 ctrl_val;
    int                 ctrl_data;
    T_AstreamInfoMsg    audio_stream_info;
    T_VstreamInfoMsg    video_stream_info;
    T_PacketInfoMsg     packet_info;
	T_FactoryCaliMsg    snapshot_info;
} T_MiCommonMsg;
//==============================================================================
int Pega_diag_msgq_send_request(stDiagMsgqCmdType stDiagMsg);
int Pega_diag_msgq_send_mediaserver(T_MiCommonMsg stDiagMsg);
//==============================================================================
#ifdef __cplusplus
}
#endif
#endif //_PEGA_DIAG_MSGQ_CMD_H_

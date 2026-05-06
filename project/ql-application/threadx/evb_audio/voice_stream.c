/**
  ******************************************************************************
  * @file    aliyun_iot_mqtt.c
  * @author  Xu
  * @version V1.0.0
  * @date    2019/03/01
  * @brief   aliyun iot mqtt func file for C code
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_rtos.h"
#include "ql_audio.h"
#include "ql_tts.h"
#include "ql_fs.h"
#include "prj_common.h"
#include "public_api_interface.h"
#include "tts_yt_task.h"
#include "res.h"
#include "voice_wav_mix.h"


#define LOG_DBG(...)            //do{printf("[DBG WMIX]: "); printf(__VA_ARGS__);}while(0)
#define LOG_INFO(...)           do{usb_log_printf("[INFO WMIX]: "); printf(__VA_ARGS__);}while(0)

#include "HTTPClient/HTTPCUsr_api.h"

#define AUDIO_WAIT_DELAY_TIMES        (20 * 1000)    // wait 20s max

#define QL_MP3_START_FLAG		(0x01)
#define QL_MP3_UNDERRUN_FLAG	(0x02)
#define QL_MP3_OVERRUN_FLAG    	(0x04)
#define QL_MP3_ENDED_FLAG		(0x08)
#define QL_MP3_PAUSE_FLAG		(0x10)
#define QL_MP3_PLAY_FLAG		(0x20)
#define QL_MP3_USE_ENDED_FLAG	(0x80)

static ql_flag_t mp3_stream_flag = NULL;
static Mp3PlaybackHandle handle = 0;

Mp3PlaybackEventCallback ql_Mp3PlaybackEvent(Mp3PlayEventType event, int val)
{
    if(event == MP3_PLAYBACK_EVENT_STATUS){
       if(val == MP3_PLAYBACK_STATUS_STARTED)
       {
            LOG_INFO("mp3 start\r\n");
			ql_rtos_flag_release(mp3_stream_flag, QL_MP3_START_FLAG, QL_FLAG_OR);
       }
       if(val == MP3_PLAYBACK_STATUS_ENDED)
       {
            LOG_INFO("mp3 end\r\n");
			ql_rtos_flag_release(mp3_stream_flag, QL_MP3_ENDED_FLAG, QL_FLAG_OR);
       }
	   if(val == MP3_STREAM_STATUS_NEARLY_UNDERRUN)
	   {
//			printf("mp3 underrun\r\n");
			ql_rtos_flag_release(mp3_stream_flag, QL_MP3_UNDERRUN_FLAG, QL_FLAG_OR);
	   }
    }
}
int voice_stop(void)
{
	LOG_INFO("%s\r\n",__func__);
	if(mp3_stream_flag==NULL)
		return -1;
	
	ql_rtos_flag_release(mp3_stream_flag, QL_MP3_USE_ENDED_FLAG, QL_FLAG_OR);
	return 0;
}
int voice_play(void)
{
	LOG_INFO("%s\r\n",__func__);
	if(mp3_stream_flag==NULL)
		return -1;
	
	ql_rtos_flag_release(mp3_stream_flag, QL_MP3_PLAY_FLAG, QL_FLAG_OR);
	return 0;
}
int voice_pause(void)
{
	LOG_INFO("%s\r\n",__func__);
	if(mp3_stream_flag==NULL)
		return -1;
	
	ql_rtos_flag_release(mp3_stream_flag, QL_MP3_PAUSE_FLAG, QL_FLAG_OR);
	return 0;
}


int voice_state(void)
{
	if(mp3_stream_flag==NULL)
		return 0;
	else
		return 1;
}
int voice_play_stream(char const * url)
{
	int delay_times = AUDIO_WAIT_DELAY_TIMES/50;
	Mp3PlayConfigInfo mp3_configInfo = {0};
	mp3_configInfo.listener = ql_Mp3PlaybackEvent;	
	u32 mp3_play_flag;

	struct {
		HTTPParameters clientParams;
		char rbuf[1024];
	} * info;
	unsigned int recSize;
	unsigned int total;
	uint8_t eof;
	int ret;

	if (url==NULL)
		return -1;

	info=malloc(sizeof(*info));
	if ( !info )
		return -1;
	memset(info,0,sizeof(*info));

	
	strcpy( info->clientParams.Uri, url );
	LOG_INFO("%s_%d play tts over!\n", __func__, __LINE__);
	total=0;
	eof=0;
	LOG_INFO("MP3 Start \r\n");
	ql_rtos_flag_create(&mp3_stream_flag);
	ql_play_mp3(NULL, &handle, &mp3_configInfo);

	while ( 1 )
	{
		recSize=0;
		ret = HTTPC_get( &info->clientParams, (CHAR*) info->rbuf, 1024, (INT32*) &recSize );
		if ( ret == HTTP_CLIENT_SUCCESS )
			eof = 0;
		else if ( ret == HTTP_CLIENT_EOS )
			eof = 1;
		else
		{
			ret = -1;
			break;
		}
		ql_rtos_flag_wait(mp3_stream_flag, QL_MP3_START_FLAG|QL_MP3_UNDERRUN_FLAG|QL_MP3_PLAY_FLAG|QL_MP3_PAUSE_FLAG, QL_FLAG_OR_CLEAR, &mp3_play_flag, AUDIO_WAIT_DELAY_TIMES);
		LOG_INFO("mp3_play_flag=%ld\r\n",mp3_play_flag);
		if(mp3_play_flag & QL_MP3_PAUSE_FLAG)
		{
			LOG_INFO("mp3 enter pause !!\r\n");
			ql_stop_mp3_play(handle, 0);
			ql_rtos_flag_wait(mp3_stream_flag, QL_MP3_PLAY_FLAG|QL_MP3_USE_ENDED_FLAG, QL_FLAG_OR_CLEAR, &mp3_play_flag, QL_WAIT_FOREVER);
			LOG_INFO("mp3_play_flag=%ld\r\n",mp3_play_flag);
			if(mp3_play_flag&QL_MP3_USE_ENDED_FLAG)
			{
				LOG_INFO("mp3 while break !!\r\n");
				break;
			}
			ql_rtos_flag_wait(mp3_stream_flag, (~(QL_MP3_UNDERRUN_FLAG)), QL_FLAG_OR_CLEAR, &mp3_play_flag, QL_NO_WAIT);
			LOG_INFO("mp3_play_flag=%ld\r\n",mp3_play_flag);
			LOG_INFO("mp3 enter play !!\r\n");
			ql_play_mp3(NULL, &handle, &mp3_configInfo);
		}
		if ( recSize )
		{
			total +=recSize;
			LOG_INFO("%s_%d recSize %d,total %d\n", __func__, __LINE__,recSize,total);
			ql_play_mp3_stream_buffer(handle, info->rbuf, recSize);
//			ql_rtos_task_sleep_ms(20);
		}

		if ( eof ||(mp3_play_flag & QL_MP3_USE_ENDED_FLAG))
		{
			ret=0;
			break;
		}
	}
	
	LOG_INFO("%s_%d rec total Size %d\n", __func__, __LINE__,total);
	HTTPC_close( &info->clientParams );
	if(info) free(info);
	if(mp3_play_flag & QL_MP3_USE_ENDED_FLAG)
	{
		 ql_stop_mp3_play(handle, 0);
	}
	else
	{
		ql_stop_mp3_play(handle, 1);
		ql_rtos_flag_wait(mp3_stream_flag, QL_MP3_ENDED_FLAG, QL_FLAG_OR_CLEAR, &mp3_play_flag, AUDIO_WAIT_DELAY_TIMES);
	}
	ql_rtos_flag_delete(mp3_stream_flag);
	mp3_stream_flag=NULL;
	LOG_INFO("mp3_play_flag end %d\r\n",mp3_play_flag);
	return ret;
}


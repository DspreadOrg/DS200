#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "ql_rtos.h"
#include "ql_fs.h"
#include "ql_rtc.h"

#include "prj_common.h"
#include "cjson.h"
#include "record_play.h"
#include "record_store.h"
#include "fixaudio.h"
#include "tts_yt_task.h"
#include "terminfodef.h"
#include "public_api_interface.h"
#include "disp_port.h"

#define LOG_DBG(fmt, arg...) //usb_log_printf("[RECORD DBG]"fmt, ##arg)
#define LOG_INFO(fmt, arg...) usb_log_printf("[RECORD INFO]"fmt, ##arg)

Play_Record_Struct play_history_record = {0,0};

void Enter_Play_Record_Mode(void)
{
	LOG_INFO("%s\n",__func__);
	play_history_record.timeout_count = ql_rtos_get_systicks_to_ms() + 30 * 1000;
	TermInfo.PlayRecordMode = 1;
}

void Exit_Play_Record_Mode(void)
{
	LOG_INFO("%s\n",__func__);
	TermInfo.PlayRecordMode = 0;
	play_history_record.play_record_count = 0;
	play_history_record.timeout_count = 0;
	//tts_play_immediately( AudioTuiccx, AudioTuiccxLen, FixAudioTypeDef );
	tts_play_set_idx(AUD_ID_EXIT_RECODE,0,1);
}

void Play_Record_Chk(void)
{
	if( ql_rtos_get_systicks_to_ms() > play_history_record.timeout_count )
	{
		Exit_Play_Record_Mode();	
	}
}

int display_record_func(uint16_t count) 
{
    //char timestamp[20] = {0};
	//ql_rtc_time_t time;
    cJSON*root = NULL;
	cJSON*psub = NULL;
	uint8_t play_len = 0;
	char play_buf[200] = {0};
	int date[5] = {0};
	uint32_t timestamp;
	struct tm *tm_ptr;
    int64_t money = 0;
	//const char * tts_date_id[5]={AudioMonth, AudioDay, AudioDian, AudioFen, AudioSecond};
	//const int tts_date_len_id[5]={AudioMonthLen,AudioDayLen,AudioDianLen,AudioFenLen,AudioSecondLen};
    //ql_rtc_get_time(&time);
	//sprintf(timestamp, "%d-%d-%d-%d-%d", time.tm_mon, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
    //printf("timestamp %s ,line %d\n", timestamp, __LINE__);
    char *msg = NULL;
    int total_records = Record_Get_Count();
    
    Record_Manage_Read(count, play_buf, &play_len);
	LOG_INFO("%s: read %d/%d play_len:%d\n", __func__, count,total_records, play_len);
	if(play_len>0)
	{
		root=cJSON_Parse((void *)play_buf);
		psub = cJSON_GetObjectItem(root,"TimeStamp");
		timestamp = (uint32_t)psub->valueint;
		tm_ptr = ql_localtime(&timestamp);
		date[0] = tm_ptr->tm_mon;
		date[1] = tm_ptr->tm_mday;
		date[2] = tm_ptr->tm_hour;
		date[3] = tm_ptr->tm_min;
		date[4] = tm_ptr->tm_sec;
		LOG_INFO("%s ,line %d, time: %d-%d %d:%d:%d\n", __func__, __LINE__, date[0], date[1], date[2], date[3], date[4]);

		psub=cJSON_GetObjectItem(root,"Money");
		money = psub->valueint;

		psub=cJSON_GetObjectItem(root,"VoiceMsg");	
		//disp_record_index(count, DISP_HOLDON_FOREVER);
		//disp_set_paymsg(money,timestamp,1);

		if ((TermInfo.disp.tm1604)||(TermInfo.disp.fg00ahk))
		{
			if ( count >= total_records )
			{
				//tts_play_immediately( AudioZhyb, AudioZhybLen, FixAudioTypeDef );
				tts_play_set_idx(AUD_ID_RECODE_LAST,1,1);
			}
			else if( count <= 1 )
			{
				//tts_play_immediately(AudioZjyb,AudioZjybLen,FixAudioTypeDef);
				tts_play_set_idx(AUD_ID_RECODE_FIRST,1,1);
			}
			else
			{
				//tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
				tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
			}
			if( money )
			{
				disp_set_paymsg( money, timestamp, TTS_FLAG_RECORD );
				disp_onoff_request( 1, DISP_HOLDON_MS );
	 		}
		}
		else
		{
			if ( count >= total_records )
			{
			//	//tts_play_set( AudioZhyb, AudioZhybLen, FixAudioTypeDef );
			//	tts_play_set_idx(AUD_ID_RECODE_LAST,1,1);
				count |=0x80;
			}
			//else if( count <= 1 )
			//{
			//	//tts_play_set(AudioZjyb,AudioZjybLen,FixAudioTypeDef);
			//	tts_play_set_idx(AUD_ID_RECODE_FIRST,1,1);
			//}
			//else
			//{
			//	//extern char const * numchangstr;
			//	//char * ptr;
			//	//ptr=play_buf;
			//	//ptr +=sprintf(ptr,"µÚ");
			//	//if (count/10)
			//	//	ptr +=sprintf(ptr,"%.*sÊ®",2,(count/10)>1?&numchangstr[(count/10)*2]:"");
			//	//if (count%10)
			//	//	ptr +=sprintf(ptr,"%.*s",2,&numchangstr[(count%10)*2]);
			//	//ptr +=sprintf(ptr,"±Ê");
			//	//tts_play_set(play_buf,strlen(play_buf), GBK_TEXT);
			//	//short group[4];
			//	tts_play_set_group(group,0,1,1);
			//}
			//tts_led_play_text(psub->valuestring, UTF8_TEXT, money, timestamp, TTS_FLAG_RECORD);
			tts_play_set_record_idx(count,psub->valuestring, money, timestamp);
		}
		cJSON_Delete(root);
	}
	else
	{
		usb_log_printf("enter %s ,line %d, no play record\n", __func__, __LINE__);
		//tts_play_set(AudioZanwskjl,AudioZanwskjlLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_RECODE_NOT_FOUND,1,1);
	}
	if( msg )
	{
	    free(msg);
	}
	return 0;
}


#include <stdio.h>
#include "ql_gpio.h"
#include "ql_rtos.h"
#include "ql_audio.h"
//#include "ql_flash.h"
#include "ql_power.h"
#include "ql_nw.h"
#include "ql_zip.h"
#include "prj_common.h"
#include "tts_yt_task.h"
#include "led.h"
#include "fixaudio.h"
#include "gpio.h"
#include "systemparam.h"
#include "terminfodef.h"
#include "button.h"
#include "module_lte.h"
#include "drv_bat.h"
#include "record_store.h"
#include "record_play.h"
#include "lowpower_mgmt.h"
#include "disp_port.h"
#include "public_api_interface.h"
#include "file_download.h"

#define LOG_DBG(fmt, arg...) //usb_log_printf("[DBG BUTTON]"fmt, ##arg)
#define LOG_INFO(fmt, arg...) usb_log_printf("[INFO BUTTON]"fmt, ##arg)

#define SYSPARAM_DATA_SAVE_TIME_GAP      5 // sysparam will save after x seconds when the data is change
#define ExitPlayRecModeTimeSec	18
 
GPIO_Button_Cmd_Info Gpio_Button_Cmd = {GPIO_BUTTON_CMD_NULL, GPIO_BUTTON_NUM};
PLAYER_CMD read_payer_ctrl_cmd();

//ql_queue_t buttonQueue = NULL;
ql_task_t button_queue_task_thread = NULL;

ql_queue_t keyValueQueue = NULL;
ql_task_t key_value_queue_task_thread = NULL;

#define ModePlayRecord	0
#define ModeVolSet	1
struct {
	char Mode;
	uint32_t Sec;
}playRecordMod=
{
//	ModePlayRecord,0
	ModeVolSet,0
};
//static uint32_t playRecordMod = 0;
static uint8_t playRecordIndex = 0;

extern void device_refactory(void);
extern int MqttExit(void);


const Gpio_Button_Cfg button3_cmd_list[GPIO_BUTTON_CMD_MAX]=
{
	{GPIO_BUTTON_0,{	CMD_POWER_LONG_PRESS,	CMD_POWER_SINGLE_PRESS,		CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_1,{	CMD_VOL_LONG_PRESS,  	CMD_VOL_SINGLE_PRESS,		CMD_VOL_DOUBLE_PRESS,				CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_2,{	CMD_FUN_LONG_PRESS,		CMD_FUN_SINGLE_PRESS,		CMD_FUN_DOUBLE_PRESS,				CMD_PLAYER_NULL,		CMD_PLAYER_NULL}}, 
	{GPIO_BUTTON_3,{	CMD_P_V_UP_LONG_PRESS,	CMD_PLAYER_NULL,   			CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_4,{	CMD_PLAYER_NULL,		CMD_PLAYER_NULL,   			CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
};

const Gpio_Button_Cfg button4_cmd_list[GPIO_BUTTON_CMD_MAX]=
{
	{GPIO_BUTTON_0,{	CMD_POWER_LONG_PRESS,		CMD_POWER_SINGLE_PRESS,					CMD_POWER_DOUBLE_PRESS,			CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_1,{	CMD_VOL_D_LONG_PRESS,		CMD_VOL_D_SINGLE_PRESS,					CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_2,{	CMD_FUN_LONG_PRESS,			CMD_FUN_SINGLE_PRESS,					CMD_FUN_DOUBLE_PRESS,				CMD_PLAYER_NULL,		CMD_PLAYER_NULL}}, 
	{GPIO_BUTTON_3,{	CMD_VOL_U_LONG_PRESS,		CMD_VOL_U_SINGLE_PRESS,					CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_4,{	CMD_P_V_UP_LONG_PRESS,		CMD_PLAYER_NULL,							CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_5,{	CMD_PLAYER_NULL,			CMD_PLAYER_NULL,							CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
//	{GPIO_BUTTON_6,{	CMD_PLAYER_NULL,			CMD_PLAYER_NULL,							CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
};

Gpio_Button_Cfg const * button_cmd_list=NULL;
		
void AtLogSet(int set)
{
	//set=0~2
	int ret = 0;
	char resp_buf[128] = {0};
	char atcmd[64];

	sprintf(atcmd,"at+log=18,%d\r\n",set);
//	ret = ql_atcmd_send_sync("AT+QDSPTABLE=64,48,48,48,48,48,48,48,48,48,48,48,48\r\n", resp_buf, sizeof(resp_buf), NULL, 5);
	ret = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
	usb_log_printf("=============%s , line %d,%s,ret %d===========\n", __func__, __LINE__,atcmd,ret);
}

PLAYER_CMD read_payer_ctrl_cmd(GPIO_Button_Cmd_Info *btinfo)
{
	PLAYER_CMD cmd = CMD_PLAYER_NULL;
	const dev_config_t *pdevconf = get_device_config();
	Gpio_Button_Cmd = *btinfo;

	LOG_INFO("%s: id=%d, cmd=%d\n", __func__, btinfo->id,btinfo->cmd);

#if 0
    if (tts_get_status() == 0)
    {
        return CMD_PLAYER_NULL;
    }
#endif
    
	if ((Gpio_Button_Cmd.cmd != GPIO_BUTTON_CMD_NULL)&&(button_cmd_list!=NULL))
	{
		cmd = button_cmd_list[Gpio_Button_Cmd.id].button_cmd_val[Gpio_Button_Cmd.cmd];;
		Gpio_Button_Cmd.cmd = GPIO_BUTTON_CMD_NULL;
		Gpio_Button_Cmd.id = GPIO_BUTTON_NUM;

		LOG_INFO("%s: id=%d, cmd=%d, func=%d\n",__func__,Gpio_Button_Cmd.id,Gpio_Button_Cmd.cmd,cmd);
	}

	// wakeup display
	disp_onoff_request(1,DISP_HOLDON_MS);

	return cmd;
}


void player_pwroff_func( char saveflag )
{
    LOG_INFO("---- buttons CMD_PLAYER_POWEROFF  \n");
//	if (((get_device_config()->id==ID_DS10)||(get_device_config()->id==ID_DS200))&&get_charge_status())
//	{
//		//tts_play_immediately(AudioChongdz, AudioChongdzLen,FixAudioTypeDef);
//		tts_play_set_idx(AUD_ID_CHARGE_IN,0,0);
//	}
//	else if(ql_rtos_get_systicks_to_s()>5)
	if(ql_rtos_get_systicks_to_s()>5)
	{
#if BT
		if (get_device_config()->id==ID_DS200BT)
	        BT_DISCONNECT();//Disconnect Bluetooth before shutting down
#endif
        disp_poweroff_msg();
        //tts_play_immediately(AudioZhengzaigguanji, AudioZhengzaigguanjiLen,FixAudioTypeDef);
        tts_play_set_idx(AUD_ID_PWR_OFF,0,1);
        audio_clear_all_msg();
        ql_rtos_task_sleep_ms(1000);
        audio_clear_all_msg();
        AudioPlayHalt();
#if BT
		if (get_device_config()->id==ID_DS200BT)
			BT_OFF();//Turn off Bluetooth
#endif
        TermLedShow(TERM_POWEROFF);
        
        TermInfo.Repeat = NULL;
        MqttExit();

        if (saveflag == 1)
        {
            sysparam_save();
            ql_rtos_task_sleep_ms(200);
        }
        disp_onoff_request(0,0);
        //smart_power_down();
        ql_rtos_task_sleep_ms(500);
        ql_power_down(0);
	}
	else
	{
        //tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
		//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
		tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
	}
}

void ExitPlayRecordModChk(void)
{
  		if (playRecordMod.Sec<ql_rtos_get_systicks_to_s())
   		{
			if (playRecordMod.Mode==ModeVolSet)
	    		playRecordMod.Mode=ModePlayRecord;
			disp_set_updown_state( DISP_HIDE_UPDOWN );
			disp_record_index( -1, DISP_HOLDON_FOREVER);
	    	playRecordIndex=0;
    	}
 // 			playRecordMod.Sec=ql_rtos_get_systicks_to_s()+ExitPlayRecModeTimeSec;
}
void player_switch_query_mode_func(void)
{
    if ( ( TermInfo.LowBat ) && ( !get_charge_status( ) ) )
    {
        //tts_play_immediately( AudioDianldqcd, AudioDianldqcdLen, FixAudioTypeDef );
        tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
    }
    else
    {
    	playRecordMod.Sec=ql_rtos_get_systicks_to_s()+ExitPlayRecModeTimeSec;
    	playRecordMod.Mode=ModeVolSet;

 //   	sprintf(buf,"%s%d",AudioYinl,GetCurVolLev());
//    	tts_play_immediately( buf, strlen(buf), GBK_TEXT );
		//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
		tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
		disp_vol_update(1);
//        if ( !playRecordMod )
//        {
//            LOG_INFO( "%s: enter query mode\n", __func__ );
//            playRecordMod = 1;
//            playRecordIndex = 0;
//            if ( 0 < Record_Get_Count( ) )
//            {
//                disp_set_updown_state( DISP_SHOW_UP );
//            }
//            //tts_play_set(AudioQingayljkcx, AudioQingayljkcxLen, GBK_TEXT);
//            tts_play_immediately( AudioQingayljkcx, AudioQingayljkcxLen, FixAudioTypeDef );
//
//            disp_record_index( 0 , DISP_HOLDON_FOREVER);
//        }
//        else
//        {
//            LOG_INFO( "%s: exit query mode\n", __func__ );
//            disp_set_updown_state( DISP_HIDE_UPDOWN );
//            playRecordMod = 0;
//            //tts_play_set(AudioTuiccx, AudioTuiccxLen, GBK_TEXT);
//            tts_play_immediately( AudioTuiccx, AudioTuiccxLen, FixAudioTypeDef );
//            disp_record_index( -1, DISP_HOLDON_FOREVER);
//        }
    }
}

//void player_trans_record_func(void)
//{
//    LOG_INFO("----player_task CMD_PLAYER_TRANS_RECORD, PlayRecordMode = %d, play_record_count = %d\n", TermInfo.PlayRecordMode, play_history_record.play_record_count);
//
//    if(TermInfo.PlayRecordMode)
//    {
//        //tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
//        play_history_record.play_record_count++;
//        play_history_record.timeout_count = ql_rtos_get_systicks_to_ms() + 30 * 1000;
//        int total_records = Record_Get_Count();
//        if( play_history_record.play_record_count > total_records )
//        {
//            play_history_record.play_record_count = 1;
//        }
//        //Play_Record_Func(play_history_record.play_record_count);
//        //display_record_func(play_history_record.play_record_count);
//        playRecordIndex = play_history_record.play_record_count;
////        tts_play_immediately((char *)&playRecordIndex,sizeof(playRecordIndex),RECORD_TEXT);
////        tts_play_immediately((char *)&playRecordIndex,sizeof(playRecordIndex),RECORD_TEXT);
//        tts_play_set_idx(AUD_ID_RECODE_PLAY,1,1);
//    }
//}

void player_mode_func(void)
{
	LOG_INFO("----player_task CMD_PLAYER_MODE_FUNC\n");
	if((TermInfo.LowBat)&&(!get_charge_status()))
		//tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
	else
	{
		short group[10];
		short * ptr;
		int ret;
		
		ptr=group;
#if 0
		//����,electric quantity
		if (battery_read_percent()>=0)
		{
			*ptr++=AUD_ID_PWR_PER;
			ret=num_to_audio_idx(battery_read_percent()*100,ptr,sizeof(group)/sizeof(group[0]) -1);
			if (ret>0)
			{
				ptr +=ret;
			//tts_play_set_group(group,(ptr-group)/sizeof(group[0]),1,1);
	//			tts_play_set_group(group,(ptr-group),1,1);
			}
		}
#endif		
		//����,network   state
		if (sysparam_get_device_type() == 0)
		{
			if((TermInfo.NetStatBak!=NET_DEVICE_STATE_CONNECTED))
			{
				if(TermInfo.SIMState == 0x00)//δ��⵽SIM��
				{
//					tts_play_set(AudioWeijcdsimk,AudioWeijcdsimkLen,FixAudioTypeDef);
//					tts_play_set_idx(AUD_ID_SIM_NOT_FOUND,1,1);
					*ptr++=AUD_ID_SIM_NOT_FOUND;
				}
				else//�����쳣
				{
//					tts_play_set(AudioWanglyc,AudioWanglycLen,FixAudioTypeDef);
//					tts_play_set_idx(AUD_ID_NET_NOT_CONNECT,1,1);
					*ptr++=AUD_ID_NET_NOT_CONNECT;
				}
			}
			else//��������
			{
//				tts_play_set(AudioWanglzc,AudioWanglzcLen,FixAudioTypeDef);
//				tts_play_set_idx(AUD_ID_NET_CONNECT,1,1);
				*ptr++=AUD_ID_NET_CONNECT;
			}
		}
		else
		{
			if((TermInfo.NetMode == GPRS_MODE)||(TermInfo.NetMode == GPRS_BAKE_MODE))
//				tts_play_set(AudioXianzsgprswlms,AudioXianzsgprswlmsLen,FixAudioTypeDef); 
//				tts_play_set_idx(AUD_ID_NET_MODE_GPRS,1,1);
					*ptr++=AUD_ID_NET_MODE_GPRS;
			else
			{
				if(TermInfo.buttonAP == 0)
					//tts_play_set(AudioXianzswifiwlms,AudioXianzswifiwlmsLen,FixAudioTypeDef); 
					//tts_play_set_idx(AUD_ID_NET_MODE_WIFI,1,1);
					*ptr++=AUD_ID_NET_MODE_WIFI;
				else if(TermInfo.buttonAP == 1)
					//tts_play_immediately(AudioXianzswifiklpwms,AudioXianzswifiklpwmsLen,FixAudioTypeDef);
					//tts_play_set_idx(AUD_ID_NET_MODE_WIFI_AIRKISS,1,1);
					*ptr++=AUD_ID_NET_MODE_WIFI_AIRKISS;
				else if(TermInfo.buttonAP == 2)
					//tts_play_immediately(AudioXianzswifiappwms,AudioXianzswifiappwmsLen,FixAudioTypeDef);	
					//tts_play_set_idx(AUD_ID_NET_MODE_WIFI_AP,1,1);
					*ptr++=AUD_ID_NET_MODE_WIFI_AP;
			}	
		}
		tts_play_set_group(group,(ptr-group),1,1);
	}
}

void player_refactory_func(void)
{
	if((TermInfo.LowBat)&&(!get_charge_status()))
		//tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
	else
	{
		device_refactory();
		LOG_INFO("----player_task CMD_PLAYER_REFACTORY\n");
	}
}

void player_sel_chanl_func(void)
{
	LOG_INFO("----player_task CMD_PLAYER_SEL_CHANL\n");
	if (TermInfo.OTAMode)
	{
		//�������������Ͳ����л�����ģʽ��
		return;
	}
				
	if((TermInfo.LowBat)&&(!get_charge_status()))
		//tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
	else
	{
		//tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
		//		tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
		tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);

		if (sysparam_get_device_type() == 1)
		{
			if((TermInfo.NetMode == WIFI_MODE)||(TermInfo.NetMode == WIFI_BAKE_MODE))
			{
				TermInfo.NetMode = GPRS_MODE;
			}
			else if((TermInfo.NetMode == GPRS_MODE)||(TermInfo.NetMode == GPRS_BAKE_MODE))
			{
				TermInfo.NetMode = WIFI_MODE;
			}
			if ((TermInfo.buttonAP)&&((TermInfo.NetMode == GPRS_MODE)||(TermInfo.NetMode == GPRS_BAKE_MODE)))
			{
				if( (TermInfo.buttonAP == 1) || (TermInfo.buttonAP == 2) )
				{
					TermInfo.buttonAP=0;
					//Ext_NetConfig_exit();
				}
			}	
		}
	} 
}


void player_exit_ap_func(void)
{
	LOG_INFO("----player_task CMD_PLAYER_EXIT_AP\n");
	if((TermInfo.LowBat)&&(!get_charge_status()))
		//tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);

	if (sysparam_get_device_type() == 0)
	{
			//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
			tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
	}
	else
	{
		if( (TermInfo.buttonAP == 1) || (TermInfo.buttonAP == 2) )
		{
			TermInfo.buttonAP=0;
			//tts_play_immediately(AudioYitcpw,AudioYitcpwLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_EXIT_WIFI_SET,1,1);
		}
		else
		{
			//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
			tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
		}
	}
}
    
void player_ap_mode_func(void)
{
    LOG_INFO("----player_task CMD_PLAYER_AP_MODE\n");
    if((TermInfo.LowBat)&&(!get_charge_status()))
    {
//        tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
    }
    else
    {
        if (sysparam_get_device_type() == 0)
        {
//			tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
			tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
        }
        else if((TermInfo.NetMode == WIFI_MODE)||(TermInfo.NetMode == WIFI_BAKE_MODE))
        {
            if(TermInfo.buttonAP == 0)
            {
                LOG_INFO("----player_task AD button0 long press enter ap mode\n");
                TermInfo.buttonAP = 2;
            }
            else if(TermInfo.buttonAP == 2)
            {
                //Ext_NetConfig_exit();
                TermInfo.buttonAP=1;
            }
            else if(TermInfo.buttonAP == 1)
            {
                TermInfo.buttonAP=0;
                //Ext_NetConfig_exit();
            }
        }
    }
}
#if BT
int volume_level=0;
#endif

int player_volume_roll_func(void)
{
    static int *current_volume = NULL;
    static int firstFlag = 0;
    static char RollDir = 0;
    int ret = -1;
    
    LOG_INFO("----player_task CMD_PLAYER_VOLUME_ROLL\n");

    if (firstFlag == 0)
    {
        current_volume = &sysparam_get()->volume;
#if BT
		if (get_device_config()->id==ID_DS200BT)
		{
			volume_level = 12-((SPEADKERS_VOLUME_SET_MAX- * current_volume)*3);
			if(volume_level<=0)
				volume_level=1;
		}
		LOG_INFO("VOL=%d\n",volume_level);
#endif
        if (*current_volume >= SPEADKERS_VOLUME_SET_MAX)
        {
            RollDir = 0;
        }
        else if (*current_volume <=1)
        {
            RollDir = 1;
        }
        firstFlag = 1;
    }
    
	if((TermInfo.LowBat)&&(!get_charge_status()))
//		tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
	else
	{
		if (RollDir)
		{
			*current_volume += 1;

			if (*current_volume >= SPEADKERS_VOLUME_SET_MAX) 
			{
				*current_volume = SPEADKERS_VOLUME_SET_MAX;
				AuidoVolumeSet(*current_volume);
				ql_rtos_task_sleep_ms(20);
				RollDir=0;
				//tts_play_immediately(AudioYinlzd,AudioYinlzdLen,FixAudioTypeDef);
				tts_play_set_idx(AUD_ID_VOL_MAX,1,1);
			}
			else
			{
				AuidoVolumeSet(*current_volume);
				ql_rtos_task_sleep_ms(20);
				//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
#if BT
				if ((get_device_config()->id==ID_DS200BT)&&((BT_music_state_check() ==0)||(BT_mute_state_check()==0)))//DS200BT      Do not disturb if playing
					tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
				else if(get_device_config()->id!=ID_DS200BT)//Other models play button tone normally
#endif
				tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
			}
#if BT
				 if (get_device_config()->id==ID_DS200BT)
			 	{
					 volume_level = 12-((SPEADKERS_VOLUME_SET_MAX- * current_volume)*3);
					if(volume_level<=0)
						volume_level=1;
					BT_vol_XX(volume_level);
					LOG_INFO("VOL=%d\n",volume_level);
			 	}
#endif

		}
		else
		{
			*current_volume -= 1;
			if (*current_volume <= 1) 
			{
				*current_volume=1;
				AuidoVolumeSet(*current_volume);
				ql_rtos_task_sleep_ms(20);
                
				RollDir=1;
				//tts_play_immediately(AudioYinlzx,AudioYinlzxLen,FixAudioTypeDef);
				tts_play_set_idx(AUD_ID_VOL_MIN,1,1);
			}
			else
			{
				AuidoVolumeSet(*current_volume);
				ql_rtos_task_sleep_ms(20);
				//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
#if BT
				if ((get_device_config()->id==ID_DS200BT)&&((BT_music_state_check() ==0)||(BT_mute_state_check()==0)))//DS200BT      Do not disturb if playing
					tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
				else if(get_device_config()->id!=ID_DS200BT)//Other models play button tone normally
#endif
				tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
			}
#if BT
		 if (get_device_config()->id==ID_DS200BT)
	 	{
			 volume_level = 12-((SPEADKERS_VOLUME_SET_MAX- * current_volume)*3);
			if(volume_level<=0)
				volume_level=1;
			BT_vol_XX(volume_level);
			LOG_INFO("VOL=%d\n",volume_level);
	 	}
#endif
		}
		LOG_DBG("current_volume %d\n", *current_volume);
		disp_vol_update(0);
        ret = 0;
	}

    return ret;
}

int player_volume_up_func(void)
{
    static int *current_volume = NULL;
    int recordPlayCnt = 0;
    int ret = -1;
    
    current_volume=&sysparam_get()->volume;
    
    if((TermInfo.LowBat)&&(!get_charge_status())) 
    {
      //tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
    } 
    else 
    {
//    	ExitPlayRecordModChk();
   		playRecordMod.Sec=ql_rtos_get_systicks_to_s()+ExitPlayRecModeTimeSec;
    	
    	if (playRecordMod.Mode==ModePlayRecord)
//    	if (!(ql_rtos_get_systicks_to_s()<playRecordMod))
    	{
            //tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
            recordPlayCnt = Record_Get_Count();
            LOG_INFO("recordTotal = %d, current = %d\n", recordPlayCnt,playRecordIndex);
            if ( recordPlayCnt <= 0 ) {
                //ql_rtos_task_sleep_ms(80);
                //tts_play_set(AudioZanwskjl, AudioZanwskjlLen, GBK_TEXT);
                //tts_play_set(AudioZanwskjl,AudioZanwskjlLen,FixAudioTypeDef);
                tts_play_set_idx(AUD_ID_RECODE_NOT_FOUND,1,1);
                disp_set_updown_state(DISP_HIDE_UPDOWN);
                return 0;
            }
            if ( playRecordIndex >= recordPlayCnt )
            {
                playRecordIndex = recordPlayCnt;
            }
            
            if( playRecordIndex > 1 )
            {
                playRecordIndex--;
            }

            if (playRecordIndex == 0 || playRecordIndex == 1) {
                playRecordIndex=1;
                disp_set_updown_state(DISP_SHOW_DOWN);
//                tts_play_immediately(AudioZjyb,AudioZjybLen,FixAudioTypeDef);
#if 0
                display_record_func(playRecordIndex);
                return 0;
#endif
            }
            else
            {
                disp_set_updown_state(DISP_SHOW_UPDOWN);
//                tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
            }

            disp_record_index(playRecordIndex, DISP_HOLDON_FOREVER);
#if 1
            display_record_func(playRecordIndex);
#else
            tts_play_immediately((char *)&playRecordIndex,sizeof(playRecordIndex),RECORD_TEXT);
#endif
            
        } 
        else 
        {
        	*current_volume += 1;
        	if (*current_volume >= SPEADKERS_VOLUME_SET_MAX) 
        	{
        		*current_volume = SPEADKERS_VOLUME_SET_MAX;
        		AuidoVolumeSet(*current_volume);
        		ql_rtos_task_sleep_ms(20);
        		//tts_play_immediately(AudioYinlzd,AudioYinlzdLen,FixAudioTypeDef);
        		tts_play_set_idx(AUD_ID_VOL_MAX,1,1);
        	}
        	else
        	{
        		AuidoVolumeSet(*current_volume);
        		ql_rtos_task_sleep_ms(20);
        		//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
#if BT
				if ((get_device_config()->id==ID_DS200BT)&&((BT_music_state_check() ==0)||(BT_mute_state_check()==0)))//DS200BT      Do not disturb if playing
					tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
				else if(get_device_config()->id!=ID_DS200BT)//Other models play button tone normally
#endif
        		tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
        	}
#if BT
			 if (get_device_config()->id==ID_DS200BT)
		 	{
				 volume_level = 12-((SPEADKERS_VOLUME_SET_MAX- * current_volume)*3);
				if(volume_level<=0)
					volume_level=1;
				BT_vol_XX(volume_level);
				LOG_INFO("VOL=%d\n",volume_level);
		 	}
#endif

        	LOG_DBG("current_volume %d\n", *current_volume);
            ret = 0;

            disp_vol_update(1);
        }

    }

    return ret;
}
int player_volume_max_func(void)
{
    static int *current_volume = NULL;
    int ret = -1;
    
    current_volume=&sysparam_get()->volume;
    
    if((TermInfo.LowBat)&&(!get_charge_status())) 
    {
			tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
    } 
    else 
    {

		*current_volume = SPEADKERS_VOLUME_SET_MAX;
#if BT
			 if (get_device_config()->id==ID_DS200BT)
		 	{
	             volume_level = 12-((SPEADKERS_VOLUME_SET_MAX- * current_volume)*3);
	            if(volume_level<=0)
	                volume_level=1;		
				BT_vol_XX(volume_level);
				LOG_INFO("VOL=%d\n",volume_level);
		 	}
#endif
		AuidoVolumeSet(*current_volume);
		ql_rtos_task_sleep_ms(20);
		tts_play_set_idx(AUD_ID_VOL_MAX,1,1);

    	LOG_DBG("current_volume %d\n", *current_volume);
        ret = 0;
        disp_vol_update(1);
       
    }

    return ret;
}
#define audio_url "http://yuncodeoss.oss-cn-shenzhen.aliyuncs.com/44_1HZ192KB_D.mp3"
http_download_cb_t http_download_cb_test(int state)
{
	printf("%s_%d state=%d\n", __func__, __LINE__,state);

}
void file_download_test(void)
{
	set_http_download_cb(http_download_cb_test);
	http_download(audio_url,"B:/welcome.mp3",1000*1024,20*1024);

}

int player_volume_down_func(void)
{
    static int *current_volume = NULL;
    int recordPlayCnt = 0;
    int ret = -1;
    
    current_volume = &sysparam_get()->volume;
    if ((TermInfo.LowBat)&&(!get_charge_status()))
    {
			//tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
    } 
    else 
    {
//    	ExitPlayRecordModChk();
   		playRecordMod.Sec=ql_rtos_get_systicks_to_s()+ExitPlayRecModeTimeSec;
    	
    	if (playRecordMod.Mode==ModePlayRecord)
//    	if (!(ql_rtos_get_systicks_to_s()<playRecordMod))
    	{
            //tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
            recordPlayCnt = Record_Get_Count();
            LOG_INFO("recordTotal = %d, current = %d\n", recordPlayCnt,playRecordIndex);

            if ( recordPlayCnt <= 0 ) {
                //tts_play_immediately(AudioZanwskjl,AudioZanwskjlLen,FixAudioTypeDef);
                tts_play_set_idx(AUD_ID_RECODE_NOT_FOUND,1,1);
                disp_set_updown_state(DISP_HIDE_UPDOWN);
                return 0;
            }
            
            if (recordPlayCnt>50) 
            	recordPlayCnt=50;
            	
            if (playRecordIndex==0)
            	playRecordIndex=recordPlayCnt;
            else
            	playRecordIndex++;
            	
            if ( playRecordIndex >= recordPlayCnt ) {
//                tts_play_immediately(AudioZhyb,AudioZhybLen,FixAudioTypeDef);
                playRecordIndex = recordPlayCnt;
                disp_set_updown_state(DISP_SHOW_UP);
            }
            else
            {
                disp_set_updown_state(DISP_SHOW_UPDOWN);
//                tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
            }
            
            disp_record_index(playRecordIndex, DISP_HOLDON_FOREVER);
#if 1
            display_record_func(playRecordIndex);
#else
            tts_play_immediately((char *)&playRecordIndex,sizeof(playRecordIndex),RECORD_TEXT);
#endif
        } 
        else
        {
            *current_volume -= 1;
            if (*current_volume <= 1) 
            {
	            *current_volume=1;
	            AuidoVolumeSet(*current_volume);
	            ql_rtos_task_sleep_ms(20);
	            //tts_play_immediately(AudioYinlzx,AudioYinlzxLen,FixAudioTypeDef);
	            tts_play_set_idx(AUD_ID_VOL_MIN,1,1);
            }
            else
            {
	            AuidoVolumeSet(*current_volume);
	            ql_rtos_task_sleep_ms(20);
	            //tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
#if BT
				if ((get_device_config()->id==ID_DS200BT)&&((BT_music_state_check() ==0)||(BT_mute_state_check()==0)))//DS200BT      Do not disturb if playing
					tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
				else if(get_device_config()->id!=ID_DS200BT)//Other models play button tone normally
#endif
	            tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
            }
#if BT
			 if (get_device_config()->id==ID_DS200BT)
		 	{
	             volume_level = 12-((SPEADKERS_VOLUME_SET_MAX- * current_volume)*3);
	            if(volume_level<=0)
	                volume_level=1;
				BT_vol_XX(volume_level);
				LOG_INFO("VOL=%d\n",volume_level);
        	}
#endif
            LOG_DBG("current_volume %d\n", *current_volume);
            ret = 0;
            disp_vol_update(1);
        }    
    }

    return ret;
}

int player_volume_min_func(void)
{
    static int *current_volume = NULL;
    int ret = -1;
    
    current_volume = &sysparam_get()->volume;
    if ((TermInfo.LowBat)&&(!get_charge_status()))
    {
			tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
    } 
    else 
    {
        *current_volume=1;
#if BT
		 if (get_device_config()->id==ID_DS200BT)
	 	{
	         volume_level = 12-((SPEADKERS_VOLUME_SET_MAX- * current_volume)*3);
	        if(volume_level<=0)
	            volume_level=1;
			BT_vol_XX(volume_level);
			LOG_INFO("VOL=%d\n",volume_level);
	 	}
#endif
        AuidoVolumeSet(*current_volume);
        ql_rtos_task_sleep_ms(20);
        tts_play_set_idx(AUD_ID_VOL_MIN,1,1);
        LOG_DBG("current_volume %d\n", *current_volume);
        ret = 0;
        disp_vol_update(1);
    }

    return ret;
}

void player_last_transaction_func(void)
{   
	LOG_INFO( "%s\n",__func__ );
    if ((TermInfo.LowBat)&&(!get_charge_status()))
    {
			tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
    } 
    else 
    {
    	tts_play_set_idx(AUD_ID_KEY_BEEP,0,1);
        display_record_func(1);
    }
}

void player_total_collection_func(void)
{   
	LOG_INFO( "%s\n",__func__ );
	tts_play_set_idx(AUD_ID_KEY_BEEP,0,1);
//	file_download_test();
}

void player_device_info_func(void)
{   
	LOG_INFO( "%s\n",__func__ );
	tts_play_set_idx(AUD_ID_KEY_BEEP,0,1);
}

void player_battery_level_func(void)
{   
	tts_play_set_idx(AUD_ID_KEY_BEEP,0,1);
	battery_read_percent();
	LOG_INFO( "%s   battery percent= %d \n",__func__ ,battery_read_percent());

}
#if BT_DEMO
void switch_Bluetooth_func(void)
{
	static 	uint32_t bt_time = 0;//Prevent switching too often
        LOG_INFO("%s_%d", __func__, __LINE__);
    if(ql_rtos_get_systicks_to_ms()>bt_time)
    {
        bt_time = ql_rtos_get_systicks_to_ms()+2000;
		BT_switch();
    }
    else
		tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
}
void example_BT_read_mac_list(void)
{
	struct BT_CON_info * list=NULL;
	int num=0;
	list=BT_con_list_Read(&num);
	
	for(int i=0;i<num;i++)
	{
	
		usb_log_printf("\n %d: mac:  ",i);
		for(int q=0;q<6;q++)
		{
			usb_log_printf("%02x",list[i].address[q]);
		}
		usb_log_printf(" name:%s\n",list[i].name);
	}
	BT_con_list_Delete(list[0].address);
	if(list)
	free(list);
}
#endif
void player_record_func(void)
{
    LOG_INFO( "%s: call\n",__func__ );
//    tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
    //tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
    tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
    Enter_Play_Record_Mode( );
}
#define FILE_ZIP_NAME		"1KHZ.zip"

#define B_DISK				"B:"

#define B_FILE_UNDER_ZIP 	""B_DISK"/"FILE_ZIP_NAME""

void list_dir(const char *path)
{
	QDIR * dp = NULL;
	struct lfs_info info = {0};
	int ret,sizeee;
	
	dp = ql_opendir(path);
	if(dp == NULL)
	{
		printf("[FS] *** dir open fail: %s ***\r\n", path);
		return;
	}
	usb_log_printf("[FS] dir opened: %s\r\n", path);


	char mV[32] = {0};
	mbedtls_version_get_string(mV);

	usb_log_printf("fmbedtls_version_get_string ver: %s\n",mV);

	usb_log_printf("[FS] ----- start dir list -----\r\n");
	usb_log_printf("[FS] type\tsize\tname\r\n");
	while(1)
	{
		ret = ql_readdir(dp, &info);
		if(ret < 0)
		{
			usb_log_printf("[FS] *** dir read fail: %s ***\r\n", path);
			break;
		}
		else if(ret == 0)
		{
			usb_log_printf("[FS] ----- end of dir list -----\r\n");
			break;
		}
		else
		{
			sizeee += info.size;
			usb_log_printf("[FS] %-4d\t%-4d\t%s\r\n", info.type, info.size, info.name);
		}
	}

	if(dp)
	{
		ql_closedir(dp);
		usb_log_printf("[FS] dir closed: %s,%d\r\n", path,sizeee);
	}
}

extern bool audio_pause_play;
void button_queue_event(void *pvParameters)
{
	char saveflag=0;
	uint8_t recv_net_msg = 0;
	int change_volume_time = 0;
	GPIO_Button_Cmd_Info button_cmd;
	
#if 1
	if( (getAddVolumeButtonGpio() <= GPIO_PIN_NO_NOT_ASSIGNED) || (getAddVolumeButtonGpio() >= GPIO_PIN_NO_MAX) )
		button_cmd_list=button3_cmd_list;	//����
	else
		button_cmd_list=button4_cmd_list;	//�ļ�
#endif
    
	while(1)
	{
//		ExitPlayRecordModChk();
		if(audio_pause_play)
			break ;
		
	    if (ql_rtos_queue_wait(keyValueQueue, (u8*)&button_cmd, sizeof(GPIO_Button_Cmd_Info), 3000) == OS_OK)
        {
	        recv_net_msg = read_payer_ctrl_cmd((GPIO_Button_Cmd_Info *)&button_cmd);
	        if( CMD_PLAYER_NULL == recv_net_msg )
	        {
	            continue;
	        }
        }
	    else
	    {
            if ( tts_get_status( ) != 2 )
            {
                if ( ( saveflag ) && ( ( ql_rtos_get_systicks_to_s( ) - change_volume_time )  >= SYSPARAM_DATA_SAVE_TIME_GAP ) )
                {
                    sysparam_save( );
                    saveflag = 0;
                }
            }
            continue;
	    }

        LOG_INFO("%s__%d, %d\n", __func__, __LINE__, recv_net_msg);
		//current_volume=&sysparam_get()->volume;
		
		switch(recv_net_msg)
		{
			case CMD_POWER_LONG_PRESS:
				player_pwroff_func(saveflag);
				break;
			
			case CMD_POWER_SINGLE_PRESS:
#if BT_DEMO
				{
					const char *unZipFilePath=B_FILE_UNDER_ZIP;

					char *unZip_path=NULL;
					int ret=ql_fs_unzip(unZipFilePath,unZip_path,1);
					if(ret!=0)
					{
						 LOG_INFO("unzip file:[%s] Failed\r\n",unZipFilePath);
					}
					else
					{
						LOG_INFO("unzip file:[%s] Success\r\n",unZipFilePath);
						list_dir("B:/");
						//Use this interface with caution as it may be risky to call outside of tts tasks
						//This call is for demonstration purposes only
						tts_play_local_file("1KHZ.mp3");
					}

				}
#endif
				break;
			
			case CMD_POWER_DOUBLE_PRESS:
#if BT_DEMO
				if (get_device_config()->id==ID_DS200BT)
					switch_Bluetooth_func();
#endif
				break;
				
			case CMD_FUN_LONG_PRESS:
				player_sel_chanl_func();
				break;

			case CMD_FUN_DOUBLE_PRESS:
				player_ap_mode_func();
				break;

			case CMD_FUN_SINGLE_PRESS:
				player_mode_func();
				break;
				
			case CMD_VOL_D_LONG_PRESS:
				player_volume_min_func();
				break;
				
			case CMD_VOL_D_SINGLE_PRESS:
				player_volume_down_func();
				break;
				
			case CMD_VOL_U_LONG_PRESS:
				player_volume_max_func();
				break;
				
			case CMD_VOL_U_SINGLE_PRESS:
				player_volume_up_func();
				break;

			case CMD_P_V_UP_LONG_PRESS:
				player_refactory_func();	
				break;				

			
			case CMD_VOL_LONG_PRESS:
				
				break;
			
			case CMD_VOL_SINGLE_PRESS:
				player_volume_roll_func();
				break;
				



				default:
				//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
				tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
				break;
		}
	}
}

QlOSStatus button_task_init()
{
	if(ql_rtos_queue_create(&keyValueQueue, sizeof(GPIO_Button_Cmd_Info), 1) != OS_OK) {
		LOG_INFO("ql_rtos_queue_create create KeyValQueue error\n");
		return -1;
	}
	
	if (ql_rtos_task_create(&button_queue_task_thread,
						25*1024,
						80,
						"button_queue_event",
						button_queue_event,
						NULL) != OS_OK) {
		LOG_INFO("button_queue_task_thread create error\n");
		return -1;
	}

	return 0;
}


#include <stdio.h>
#include <string.h>
#include "ql_application.h"
#include "ql_rtos.h"
#include "ql_dev.h"
#include "ql_data_call.h"
#include "ql_sim.h"
#include "ql_nw.h"
#include "sockets.h"
#include "netdb.h"
#include "ql_uart.h"
#include "ql_gpio.h"
#include "ql_power.h"
#include "ql_spi.h"
#include "ql_spi_nor.h"
//#include "ql_atcmd.h"
#include "ql_audio.h"
#include "conf_devtype.h"
#include "prj_common.h"
#include "mqtt_aliyun.h"
#include "tts_yt_task.h"
#include "httplayerjson.h"
#include "systemparam.h"
#include "gpio_button_task.h"
#include "button.h"
#include "drv_bat.h"
#include "led.h"
#include "gpio.h"
#include "bat.h"
#include "terminfodef.h"
#include "res.h"
#include "ql_fs.h"
#include "module_lte.h"
#include "record_store.h"
#include "record_play.h"
#include "play_receipt.h"
#include "ext_wifi.h"
#include "ext_wifi_socket.h"
#include "wifi.h"
#include "ql_api_map_common.h"
#include "public_api_interface.h"
#include "lowpower_mgmt.h"
#include "disp_port.h"
#include "gps.h"

#define LOG_DBG(...)            do{usb_log_printf("[DBG MAIN]: "); usb_log_printf(__VA_ARGS__);}while(0)
#define LOG_INFO(...)           do{usb_log_printf("[INFO MAIN]: "); usb_log_printf(__VA_ARGS__);}while(0)

/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
//?汾?????20?????
char firmware_version[50] = CFG_LARKTMS_APP_VERSION;
char g_kernel_version[20] = "8.x";

StructTermInfo TermInfo = {0};
//static unsigned char poweron_switch_modem_min_fun = 0;
static char WifiSetOK = 0;
ql_task_t app_task_ctrl_thread = NULL;
#define APP_TASK_CTRL_THREAD_STACK_SIZE	(10 * 1024)

//???????????????????????????,??????????????????
//Only after the disconnection time exceeds the following duration can the voice prompt be given to prevent the weak network from being broadcast too frequently
#define NetFailAudioTimeDef	60

extern unsigned char mqtt_first_connet;
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
extern uint8_t isEnterFactorymode();

// ???????????
void device_refactory(void)
{
	struct sysparam *psysconfig = sysparam_get();
    
	//tts_play_set(AudioZhenzhfccsz,AudioZhenzhfccszLen,FixAudioTypeDef);
	tts_play_set_idx(AUD_ID_RECOVER_FACTORY,0,0);
#if BT
	if (get_device_config()->id==ID_DS200BT)
		BT_ON();
#endif
	ql_rtos_task_sleep_ms(2000);
#if BT
	if (get_device_config()->id==ID_DS200BT)
		BT_device_refactory();//Clear the bluetooth connection information
#endif
	memset(&psysconfig->wlan_sta_param, 0, sizeof(psysconfig->wlan_sta_param));
    
    #ifndef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	memset(psysconfig->product_key, 0, SYSINFO_PSK_LEN_MAX-1);
	memset(psysconfig->device_name, 0, SYSINFO_PSK_LEN_MAX-1);
	memset(psysconfig->device_secret, 0, SYSINFO_PSK_LEN_MAX-1);
    #endif
    
	psysconfig->register_state = 0;
	psysconfig->volume = SPEADKERS_VOLUME_SET_MAX - 1;
	psysconfig->NetChanlLTE = GPRS_BAKE_MODE;
//  CS10_sysparam->device_type = -1;
	sysparam_save();

    TermInfo.Repeat = NULL;
    MqttExit();
	
	Record_Manage_Clean();
	AudioPlayHalt();
	ql_rtos_task_sleep_ms(800);
	ql_power_reset();
}

// ??????????
void keyboard_input_callback(int input_type, void *param )
{
    static uint8_t last_type = 0;
    char *playlist = NULL;
    int64_t value = *((int64 *)param);

    switch(input_type)
    {
        // ????????
        case KB_INPUT_OK:
            //tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
            tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
            //tts_play_immediately(AudioAnjianyin,AudioAnjianyinLen,MEM_PCM_TEXT);
            break;
        // ??????
        case KB_INPUT_ERR:
            //tts_play_immediately(AudioInputErr,AudioInputErrLen,MEM_PCM_TEXT);
            tts_play_set_idx(AUD_ID_KEY_ERR_BEEP,1,1);
            break;

        // ????
        case KB_INPUT_CLEAR:
            last_type = 0;
            break;

        // ????
        case KB_INPUT_SET:
        {
#if 0
            playlist = create_playlist( );
            if ( playlist )
            {
                playlist_append_content( playlist, AudioZhifuchenggong );
                //playlist_append_str( playlist, AudioYinxjhsbqgjsmsjhsb );
                //playlist_append_money( playlist, value );
                tts_play_set( playlist, strlen( playlist ), PLAYLIST_TEXT );

                ui_custom_msg_t msg;
                memset( &msg, 0, sizeof(ui_custom_msg_t) );
                msg.type = UI_MSG_PAY_OK;
                msg.timeout_ms = 20 * 1000;
                msg.data.pay.money = value;
                disp_set_custom_msg( &msg, sizeof(ui_custom_msg_t) );

                memset(playlist,0,strlen(playlist));
                uint32_t timestamp = read_rtc_time();
                playlist_append_content( playlist, AudioZifbdz );
                playlist_append_num( playlist,value);
                tts_led_play_text(playlist, PLAYLIST_TEXT, value, timestamp, TTS_FLAG_PAYMSG | TTS_FLAG_FLASH);
            }
#endif
        }
            break;

        // ???????????
        case KB_INPUT_REPEAT:
            if( last_type )
            {
                input_type = last_type;
            }
            else
            {
                break;
            }
        case KB_INPUT_RETURN:
        case KB_INPUT_PAY:
        {
 #if 0
            if ( input_type == KB_INPUT_RETURN )
            {
                last_type = KB_INPUT_RETURN;
                playlist = create_playlist( );
                if ( playlist )
                {
                    //playlist_append_str(playlist,AudioZifbdz);
                    playlist_append_content( playlist, AudioQingsaomatuikuan );
                    //playlist_append_money( playlist, value );
                    tts_play_immediately( playlist, strlen( playlist ), PLAYLIST_TEXT );

                    ui_custom_msg_t msg;
                    memset( &msg, 0, sizeof(ui_custom_msg_t) );
                    msg.type = UI_MSG_REFUND_REQUEST;
                    msg.timeout_ms = 20 * 1000;
                    msg.data.pay.money = value;
                    disp_set_custom_msg( &msg, sizeof(ui_custom_msg_t) );
                }
            }
            else if ( input_type == KB_INPUT_PAY )
            {
                last_type = KB_INPUT_PAY;
                playlist = create_playlist( );
                if ( playlist )
                {
                    //playlist_append_str(playlist,AudioZifbdz);
                    playlist_append_content( playlist, AudioQingsaomafukuan );
                    //playlist_append_money( playlist, value );
                    tts_play_immediately( playlist, strlen( playlist ), PLAYLIST_TEXT );

                    ui_custom_msg_t msg;
                    memset( &msg, 0, sizeof(ui_custom_msg_t) );
                    msg.type = UI_MSG_PAY_REQUEST;
                    msg.timeout_ms = 20 * 1000;
                    msg.data.pay.money = value;
                    disp_set_custom_msg( &msg, sizeof(ui_custom_msg_t) );
                }
            }
#endif
        }
            break;
    }// end switch

    if( playlist )
    {
        free(playlist);
    }

}

void AtBlackCellOnOff(int set)
{
	//set=0~2
	int ret = 0;
	char resp_buf[128] = {0};
	char atcmd[64];

	sprintf(atcmd,"AT+QBLACKCELLCFG=%d\r\n",set>0?1:0);
//	ret = ql_atcmd_send_sync("AT+QDSPTABLE=64,48,48,48,48,48,48,48,48,48,48,48,48\r\n", resp_buf, sizeof(resp_buf), NULL, 5);
	ret = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
	usb_log_printf("=============%s , line %d,%s===========ret %d\n", __func__, __LINE__,atcmd,ret);
	sprintf(atcmd,"AT+QBLACKCELLCFG?\r\n");
//	ret = ql_atcmd_send_sync("AT+QDSPTABLE=64,48,48,48,48,48,48,48,48,48,48,48,48\r\n", resp_buf, sizeof(resp_buf), NULL, 5);
	ret = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
	usb_log_printf("=============%s , line %d,%s,ret %d,%s===========\n", __func__, __LINE__,atcmd,ret,resp_buf);
}
void bus_clk_52M(void)
{
	int ret = 0;
	char resp_buf[128] = {0};
	char atcmd[64];
	sprintf(atcmd,"at*regrw=w,d4090118,1\r\n");
//	ret = ql_atcmd_send_sync("AT+QDSPTABLE=64,48,48,48,48,48,48,48,48,48,48,48,48\r\n", resp_buf, sizeof(resp_buf), NULL, 5);
	ret = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
	usb_log_printf("=============%s , line %d,%s===========ret %d\n", __func__, __LINE__,atcmd,ret);
}
#if BT_DEMO
BT_connect_state_cb_t BT_CON_cb(int state)
{
	usb_log_printf("%s , line %d BT%sconnect \n", __func__, __LINE__,state?" ":" dis");
	if(state)
		tts_play_set_idx(AUD_ID_BT_CON,0,0);
	else
		{
			tts_play_set_idx(AUD_ID_BT_DISCON,0,0);
		}
}
switch_Bluetooth_state_cb_t BT_state_cb(int state)
{
	usb_log_printf("%s , line %d BT is %s \n", __func__, __LINE__,state?"open":"close");
	if(state)
		tts_play_set_idx(AUD_ID_BT_OPEN,0,0);
	else
		tts_play_set_idx(AUD_ID_BT_CLOSE,0,0);
}
Bluetooth_OTA_cb_t BT_OTA_cb(void)
{
	usb_log_printf("%s , line %d \n", __func__, __LINE__);
	tts_play_set_idx(AUD_ID_BT_OTA,0,0);
}
BT_config_wifi_ssid_same_cb_t BT_WIFI_ssid_same_cb(void)
{
	usb_log_printf("%s , line %d \n", __func__, __LINE__);
	tts_play_set_idx(AUD_ID_NET_CONNECT,0,0);
}
BT_pair_request_cb_t BT_pair_cb(char * mac,char *name)
{
	usb_log_printf("%s , line %d  \nBT name %s\n", __func__, __LINE__,name);
	usb_log_printf("mac : ");
	for(int q=0;q<6;q++)
	{
		usb_log_printf("%02x	",mac[q]);
	}
	
	usb_log_printf("\n");
	////////This time-delayed pairing is only used when the prototype is displayed. 
	////////For practical debugging, it is not recommended to add a delay function here.
	ql_rtos_task_sleep_ms(5000);
	BT_pair_Ack(1);
	///////end
}
Bluetooth_unpair_cb_t BT_unpair_cb()
{
	usb_log_printf("%s , line %d \n", __func__, __LINE__);
}



#endif
void start(void)
{
    AtBlackCellOnOff(1);
	bus_clk_52M();
#if BT
	if (get_device_config()->id==ID_DS200BT)
	{
		BT_close_time_to_network_fail(7200);
		BT_ble_switch(0);
		set_bt_pair_time(30);//min:10   max:60
		set_bt_faraway_recon_time(0);//0:close   other:  xx minute
#if BT_DEMO
		set_BT_pair_request_cb(BT_pair_cb);
		set_BT_connect_state_cb(BT_CON_cb);
		set_switch_Bluetooth_state_cb(BT_state_cb);
		set_Bluetooth_OTA_cb(BT_OTA_cb);
		set_BT_config_wifi_ssid_same_cb(BT_WIFI_ssid_same_cb);
		set_Bluetooth_unpair_cb(BT_unpair_cb);
#endif
	}
#endif
	disp_ui_init();

	gpio_led_task_init(); 
	TermLedShow(TERM_INIT_START);
	drv_bat_init();
    
//    if (sysparam_get()->NetChanlLTE == WIFI_MODE)
//    {
//        poweron_switch_modem_min_fun = 1;
//        //ql_dev_set_modem_fun(QL_DEV_MODEM_MIN_FUN,0);
//    }
//    else
//    {
//        LTE_CAT1_init();
//    }
    
	tts_play_init(); // tts 
	ResTabChk();
	WifiCtrlInit();

	gpio_button_ctrl_init();
	button_task_init();
    
	if(sysparam_get()->uart_log == 2)
	{
		//usb_log_uart_tts_init(); // ???????????? "USB??????????????" ??????????????????
		usb_log_uart_init(); // ??????????????????????У????????????????????
//	tts_play_text("USB??????????????", 2);
		tts_play_set_idx(AUD_ID_LOG_MODE_USB,0,0);
		while(!usb_log_uart_start())
			ql_rtos_task_sleep_ms(100);
	}
	else if (sysparam_get()->uart_log == 3)
	{
		usb_log_uart_init(); // ??????????????????????У????????????????????
	}

	Record_Manage_Init();
	play_receipt_init();
	Record_Read_MSGID(0);

	if (TermInfo.LowBat>0)
	{
		//tts_play_set(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,0,0);
		//tts_play_set(AudioHuanysylkl,AudioHuanysylklLen,FixAudioTypeDef);
	}
//#ifndef WELCOME_AUDIO_BUILD_IN_APP_SUPPORT
//	//tts_play_set(AudioHuanysyyyx,AudioHuanysyyyxLen,FixAudioTypeDef);
//	//tts_play_set(AudioZhulsyxl,AudioZhulsyxlLen,FixAudioTypeDef);
//	tts_play_set_idx(AUD_ID_WELCOME,0,0);
//#endif
	LOG_INFO("LedModeSet Blink\n");

#if DEV_KEYBOARD_SUPPORT
	if( TermInfo.disp.tm1721 )
	{
		keyboard_init();
		keyboard_set_input_callback(keyboard_input_callback);
	}
#endif
    
	lpm_init();

	TermLedShow(TERM_INIT_END);
}

int FuncWifiErrRepeat(void)
{
#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if ((strlen(sysparam_get()->device_SN) == 0) || (strlen(sysparam_get()->device_name) == 0) ||(strlen(sysparam_get()->device_secret) == 0) || (strlen(sysparam_get()->product_key) == 0))
#else
	if (strlen(sysparam_get()->device_SN) == 0)
#endif
	{
		disp_set_net_connect_error(1);
		TermInfo.RepTim =ql_rtos_get_systicks_to_s()+30;
		//tts_play_set(AudioQingxrsbcs,AudioQingxrsbcsLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		return -1;
	}
	else if (TermInfo.NetStatBak != NET_DEVICE_STATE_CONNECTED) 
	{
		int state=NET_DEVICE_STATE_DISCONNECTED;
		if (Wifi_Que_stat(&state)==0)
		{
			if( state & 0x04 )
			{
				// esp8285
				if( !(state & 0x02) )
				{
					WifiSetOK = 0;
				}
			}
			NetStat = ( state & 0x10 ) ? 1 : 0;
			// MqttStat=state%0x10;
		}

		LOG_INFO("FuncWifiErrRepeat wifi disconnect\n");
		TermInfo.RepTim =ql_rtos_get_systicks_to_s()+30;
		TermLedShow(TERM_NET_DIS);
		disp_set_net_connect_error(1);
		//tts_play_set(AudioWanlljsbqjcwl,AudioWanlljsbqjcwlLen,FixAudioTypeDef);
		//?????????????????????????
		//If the disconnection time exceeds the specified time, the voice prompt will be given
		LOG_INFO("net fail time %d,cur %d\n",TermInfo.FailTime,ql_rtos_get_systicks_to_s());
		if(ql_rtos_get_systicks_to_s()>(TermInfo.FailTime+NetFailAudioTimeDef))
		{
			TermInfo.PlayNetSucc=0;
			tts_play_set_idx(AUD_ID_NET_CONNECT_FAIL,0,0);
		}
		return -3;
	}
	TermInfo.Repeat=NULL;
	return 0;
}

int FuncModuleErrRepeat(void)
{
#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if ((strlen(sysparam_get()->device_SN) == 0) || (strlen(sysparam_get()->device_name) == 0) )//||(strlen(sysparam_get()->device_secret) == 0) || (strlen(sysparam_get()->product_key) == 0))
#else
	if (strlen(sysparam_get()->device_SN) == 0)
#endif
	{
		disp_set_net_connect_error(1);
		TermInfo.RepTim = ql_rtos_get_systicks_to_s()+30;
		//tts_play_set(AudioQingxrsbcs,AudioQingxrsbcsLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		return -1;
	}
	else if (TermInfo.NetStatBak!=NET_DEVICE_STATE_CONNECTED) 
	{
		Get_Let_CregState();
		TermInfo.NetStatBak = Module_Net_State();
		if (TermInfo.NetStatBak != NET_DEVICE_STATE_CONNECTED)
		{
			TermLedShow(TERM_NET_DIS);
			disp_set_net_connect_error(1);
			TermInfo.RepTim = ql_rtos_get_systicks_to_s()+30;
			//tts_play_set(AudioWanlljsbqjcwl,AudioWanlljsbqjcwlLen,FixAudioTypeDef);
			//?????????????????????????
			//If the disconnection time exceeds the specified time, the voice prompt will be given
			LOG_INFO("net fail time %d,cur %d\n",TermInfo.FailTime,ql_rtos_get_systicks_to_s());
			if(ql_rtos_get_systicks_to_s()>(TermInfo.FailTime+NetFailAudioTimeDef))
			{
				TermInfo.PlayNetSucc=0;
				tts_play_set_idx(AUD_ID_NET_CONNECT_FAIL,0,0);
			}
			return -3;
		}
	}
	
	TermInfo.Repeat=NULL;
	return 0;
}

int FuncWifiRetryRepeat(void)
{
	unsigned int address = 0;
	static int index=0;
	int state=NET_DEVICE_STATE_DISCONNECTED;

	if (TermInfo.NetStatBak!=NET_DEVICE_STATE_CONNECTED) 
		return -1;
#if 0
		if (memcmp(sysparam_get()->WifiVer,"EXT_WIFI_1.02",13))
		{
			//tts_play_set("?????????", 10, GBK_TEXT);
			tts_play_set_idx(AUD_ID_UPDATE_START,0,0);
			if (Wifi_Self_Updata("http://server.natappfree.cc:37612/EXT_WIFI_1.02%2020201006.img")==0)
			{
				ql_rtos_task_sleep_ms(3000);
				//tts_play_set("????????", 10, GBK_TEXT);
				tts_play_set_idx(AUD_ID_UPDATE_SUCESS,0,0);
			}
			else
				//tts_play_set("????????", 10, GBK_TEXT);
				tts_play_set_idx(AUD_ID_UPDATE_FAIL,0,0);
		}
		else
				tts_play_set("???????????", 12, GBK_TEXT);
			
#endif

	if (Wifi_Que_stat(&state)==0)
	{
		if ( state & 0x04 )
		{
			// esp8285
			if ( !( state & 0x02 ) )
			{
				WifiSetOK = 0;
			}
		}
		NetStat = ( state & 0x10 ) ? 1 : 0;
		//MqttStat=state%0x10;
	}

	if (!TermInfo.SntpOk)
	{
		LOG_INFO("enter %s , line %d,SntpOk %d\n", __func__, __LINE__, TermInfo.SntpOk);
		if(Wifi_SntpTime(NULL) ) // ?????????  ????task5s????????????
			TermInfo.RepTim =ql_rtos_get_systicks_to_s()+5;
		else 
			TermInfo.SntpOk=1;

		LOG_INFO("enter %s , line %d, read_time: %d\n", __func__, __LINE__, read_rtc_time());
	}
	
	TermInfo.RepTim =ql_rtos_get_systicks_to_s()+60;
	if (Wifi_GetHostByName("www.aliyun.com", &address) != 0) 
	{
		index++;
		LOG_INFO("invalid ping host.\n");
		if (index>6)
		{
			index=0;
			disp_set_net_connect_error(1);
			//tts_play_set(AudioWanglgzqjcwlzt, AudioWanglgzqjcwlztLen, FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_NET_FAULT,0,0);
		}
		return -2;
	}
	LOG_INFO("%s_%d ===ipaddr:%d.%d.%d.%d\n", __func__, __LINE__,(address>>0)&0xff,(address>>8)&0xff,(address>>16)&0xff,(address>>24)&0xff);

#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if ((strlen(sysparam_get()->device_SN) == 0) || (strlen(sysparam_get()->device_name) == 0) )//||(strlen(sysparam_get()->device_secret) == 0) || (strlen(sysparam_get()->product_key) == 0))
#else
	if (strlen(sysparam_get()->device_SN) == 0)
#endif
	{
		TermLedShow(TERM_PARAM_ERR);
		disp_set_net_connect_error(1);
		//tts_play_set(AudioQingxrsbcs,AudioQingxrsbcsLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
		return -3;
	}
	
#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if (0)
#else
	if(!(sysparam_get()->register_state)) 
#endif
	{
		LOG_INFO("enter %s , line %d,register_state %d\n", __func__, __LINE__,sysparam_get()->register_state);
		if (0 != device_sign(TermInfo.NetMode))
		{
			TermLedShow(TERM_REG_FAIL);
			disp_set_net_connect_error(1);
			//tts_play_set(AudioYinxjhsbqgjsmsjhsb,AudioYinxjhsbqgjsmsjhsbLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_ACTIVE_FAIL,0,0);
			TermInfo.RepTim =ql_rtos_get_systicks_to_s()+30;
			return -3;
		}
		else
		{
			//tts_play_set(AudioYinxjhcg,AudioYinxjhcgLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_ACTIVE_SUCESS,0,0);
			sysparam_get()->register_state = 1;
			sysparam_save();
		}
	}

	if ((!TermInfo.MqttIsRuning)&&(!TermInfo.OTAMode))
	{
		disp_service_connecting();
		start_Mqtt_task();
	}

	if ((!TermInfo.SntpOk))
	{
		TermInfo.RepTim = ql_rtos_get_systicks_to_s( ) + 10;
		return -1;
	}
	
	return 0;
}

int FuncLteRetryRepeat(void)
{
	int ret;
	if (TermInfo.NetStatBak!=NET_DEVICE_STATE_CONNECTED) 
		return -1;

	if(!TermInfo.SntpOk)
	{
		LOG_INFO("enter %s , line %d,SntpOk %d\n", __func__, __LINE__, TermInfo.SntpOk);
		if(sntp_get_net_time(NULL) ) // ?????????  ????task5s????????????
			TermInfo.RepTim =ql_rtos_get_systicks_to_s()+5;
		else 
			TermInfo.SntpOk=1;

		LOG_INFO("enter %s , line %d, read_time: %d\n", __func__, __LINE__, read_rtc_time());
	}

#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if ((strlen(sysparam_get()->device_SN) == 0) || (strlen(sysparam_get()->device_name) == 0) ) //||(strlen(sysparam_get()->device_secret) == 0) || (strlen(sysparam_get()->product_key) == 0))
#else
	if (strlen(sysparam_get()->device_SN) == 0)
#endif
	{
		TermLedShow(TERM_PARAM_ERR);
		disp_set_net_connect_error(1);
		//tts_play_set(AudioQingxrsbcs,AudioQingxrsbcsLen,FixAudioTypeDef);
		LOG_INFO("%s: ------------------------AUD_ID_PARAM_EMPTY---------1--------\n",__func__);
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
		return -1;
	}

#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if (0)
#else
	if(!(sysparam_get()->register_state)) 
#endif
	{
		LOG_INFO("enter %s , line %d,register_state %d\n", __func__, __LINE__,sysparam_get()->register_state);
		ret = device_sign(TermInfo.NetMode);
		LOG_INFO("enter %s , line %d,ret = %d\n", __func__, __LINE__,ret );
		if (0 != ret)
		{
			TermLedShow(TERM_REG_FAIL);
			disp_set_net_connect_error(1);
			//tts_play_set(AudioYinxjhsbqgjsmsjhsb,AudioYinxjhsbqgjsmsjhsbLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_ACTIVE_FAIL,0,0);
			TermInfo.RepTim =ql_rtos_get_systicks_to_s()+30;
			return -3;
		}
		else
		{
			//tts_play_set(AudioYinxjhcg,AudioYinxjhcgLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_ACTIVE_SUCESS,0,0);
			sysparam_get()->register_state = 1;
			sysparam_save();
		}
	}

    TermInfo.RepTim =ql_rtos_get_systicks_to_s()+5;

	if ((!TermInfo.MqttIsRuning)&&(!TermInfo.OTAMode))
	{
	    disp_service_connecting();
	    start_Mqtt_task();
	}

	
	if ((!TermInfo.SntpOk))
	{
		TermInfo.RepTim =ql_rtos_get_systicks_to_s()+10;
		return -1;
	}	
	
	return 0;
}

char AudioPlayLevelFlag=0;
void AudioPlayLevel(void)
{
	if (AudioPlayLevelFlag)
	{
		char audio[50];
		AudioPlayLevelFlag=0;
		int level=-1;
					
		if((TermInfo.NetMode == WIFI_MODE)||(TermInfo.NetMode == WIFI_BAKE_MODE))
			level=GetWifiSignalLevel();
		else
			level=GetGsmSignalLevel();
					
		if (level<=0)
		{
			//tts_play_immediately("?????????????",strlen("?????????????"),FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_GET_SIGNAL_FAIL,1,1);
		}
		else
		{
			//sprintf(audio,"???????%d",level);
			//tts_play_immediately(audio,strlen(audio),FixAudioTypeDef);
			short group[10];
			short * ptr;
			int ret;
			ptr=group;
			*ptr++=AUD_ID_CUR_SIGNAL;
			ret=num_to_audio_idx(level,ptr,sizeof(group)/sizeof(group[0]) -1);
			if (ret>0)
			{
				ptr +=ret;
			//tts_play_set_group(group,(ptr-group)/sizeof(group[0]),1,1);
				tts_play_set_group(group,(ptr-group),1,1);
			}
		}
	}
}

int FuncInputChk(void)
{
#if 1
    static uint32_t timer_show_free = 0;
	static uint32_t maxfree = 0,minfree = -1;
    unsigned int heap = 0;

    if( ql_rtos_get_systicks_to_ms() > timer_show_free )
    {
        timer_show_free = ql_rtos_get_systicks_to_ms() + 3000;

        heap = ql_rtos_get_free_heap_size();
        if( heap > maxfree )
        {
            maxfree = heap;
        }
        if( heap < minfree )
        {
            minfree = heap;
        }
        LOG_INFO("<memory> free %d, min free %d, max free %d\n", heap,minfree,maxfree);
    }
#endif

	AudioPlayLevel();
	TermInfo.BatRemain = battery_read_percent();
	TermInfo.Charge = get_charge_status();
	if (TermInfo.Charge != TermInfo.ChargeOld)
	{
		LOG_INFO("enter %s , line %d, charge_status = %d,TermInfo.Charge %d\n", __func__, __LINE__, get_charge_status(), TermInfo.Charge);
		disp_update_request();
		TermInfo.ChargeOld = TermInfo.Charge;
		TermInfo.ChargeFull = 0;
		if (TermInfo.Charge)
		{
			TermLedShow(TERM_CHARGE_START);
			//tts_play_set(AudioChongdz,AudioChongdzLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_CHARGE_IN,0,0);
//			setRedLedOnOff(1);
			ql_rtos_task_sleep_ms(3000);// ??????3s
		}
		else
		{
			//ql_set_pmic_gpio_level(PMIC_GPIO_PIN_209, PIN_LEVEL_LOW);
			//tts_play_set(AudioChongdyyc,AudioChongdyycLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_CHARGE_OUT,0,0);
//			setRedLedOnOff(0);
			dev_config_t *pdevconf = get_device_config();
			if((pdevconf->id==ID_DS200)||(pdevconf->id==ID_DS200BT))
			{
				if (TermInfo.LowBat>0)
				{
					if(get_battery_percent()>5)
						TermLedShow(TERM_LOWBAT);
					else
						TermLedShow(TERM_LOWBAT2);
				}
				else 
					TermLedShow(TERM_CHARGE_FULL);
			}
			else
			{
				if(TermInfo.SIMState == 0x00)
				{
					TermLedShow(TERM_NO_SIM);
				}
				else
				{
					if(TermInfo.NetStatBak == NET_DEVICE_STATE_CONNECTED)
					{
						if ((TermInfo.ServiceOnline&(MqttMaskA|MqttMaskB))==(MqttMaskA|MqttMaskB))
						{
							TermLedShow(TERM_NET_SERVER_CON);
						}
						else
						{
							TermLedShow(TERM_NET_CON);
						}
					}
					else
					{
						TermLedShow(TERM_NET_DIS);
					}
				}

			}

		}

	}
	else if (TermInfo.Charge)
	{
		//if(is_charge_full())
		if(is_charge_full()!=TermInfo.ChargeFull)
		{
			TermInfo.ChargeFull=is_charge_full();
			//if( !TermInfo.ChargeFull )
			if( TermInfo.ChargeFull )
			{
				//TermInfo.ChargeFull = 1;
				//tts_play_set(AudioChongdywc,AudioChongdywcLen,FixAudioTypeDef);
				tts_play_set_idx(AUD_ID_CHARGE_FULL,0,0);
				TermLedShow(TERM_CHARGE_FULL);
			}
		}
	}
	else if (TermInfo.LowBat)
	{
		//LOG_INFO("enter %s , line %d,TermInfo.LowBat %d\n", __func__, __LINE__,TermInfo.LowBat);
		if (TermInfo.LowBat==1)
		{
			if(get_battery_percent()>5)
			TermLedShow(TERM_LOWBAT);
			else
			TermLedShow(TERM_LOWBAT2);
			while((TermInfo.LowBat==1)&&(!TermInfo.Charge))
			{
				//tts_play_set(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
				static int cnt_play_lowbat = 0;
				if(cnt_play_lowbat%2 == 0)
				{
					tts_play_set_idx(AUD_ID_PWR_LOWBAT,0,0);
				}

				for(int ii=0;ii<30;ii++)
				{
					TermInfo.Charge = get_charge_status();
					if ((TermInfo.LowBat!=1)||(TermInfo.Charge==1)) 
						break;
					ql_rtos_task_sleep_ms(1000);
				}

				cnt_play_lowbat++;
				TermInfo.Charge = get_charge_status();
			}
		}
		if ((TermInfo.LowBat==2)&&(!TermInfo.Charge))
		{
			// battery is too low need auto powerdown
			for(int jj=0;jj<50;jj++)
			{
				if (tts_get_status() == 1)
				{
					break;
				}
				ql_rtos_task_sleep_ms(100);
			}
			disp_poweroff_msg();
			LOG_INFO("%s_%d ===battery is too low need auto powerdown\n", __func__, __LINE__);
			//tts_play_immediately(AudioDianliangdizzgj, AudioDianliangdizzgjLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_PWR_LOWBATOFF,0,1);
			audio_clear_all_msg();
			ql_rtos_task_sleep_ms(3000);
			audio_clear_all_msg();
			AudioPlayHalt();
			TermInfo.Repeat = NULL;
			MqttExit();
            
			ql_power_down(0);
		}
	}
	return 0;
}

extern void Module_Net_State_Set(int state);
int Get_Let_CregState(void){
	int ret = 0;
	QL_NW_REG_STATUS_INFO_T  reg_status;

	ret=ql_nw_get_reg_status(&reg_status);

	LOG_INFO("The nw state: %d\n", reg_status.data_reg.state);
	if(ret==0)
	{
		switch(reg_status.data_reg.state)
		{
			case QL_NW_REG_STATE_HOME_NETWORK:
			case QL_NW_REG_STATE_ROAMING:
				Module_Net_State_Set(1);
				return 1;
			default:
				return 0;
		}
	}
	return 0;
}

void NetChanlLTE(void)
{
	int i=0;
	int state = NET_DEVICE_STATE_DISCONNECTED;

	set_net_led_state(0);
	disp_gprs_mode();
	disp_set_net_connect_error(0);
	//wlan_sta_disable();
	lpm_set(LPM_LOCK_NETWORK,1);

	if (sysparam_get_device_type() == 1)
	{
		//tts_play_set(AudioXianzsgprswlms,AudioXianzsgprswlmsLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_NET_MODE_GPRS,0,0);

		Ext_wifi_Off();		
//    	ql_dev_set_modem_fun(QL_DEV_MODEM_FULL_FUN,0);
	}

    AudioPlayHalt();
    ModuleNetFunc(1);
    AudioPlayContinue();

	TermInfo.NetStatBak = NET_DEVICE_STATE_DISCONNECTED;
	TermInfo.PlayNetSucc = 0;

	disp_net_connecting();
	for(i=0;i<300;i++)
	{
		if(SIM_care_check_state())
			break;
		ql_rtos_task_sleep_ms(10);
	}
	printf("MAIN TermInfo.SIMState = %d \n",TermInfo.SIMState);
	switch(TermInfo.SIMState)
	{
		case 0x00:
		{
            short group[4];
            int cnt=0;
            TermLedShow(TERM_NO_SIM);
            disp_set_net_connect_error(1);
            group[cnt++]=AUD_ID_SIM_INSERT_REQ;
            if ( ( sysparam_get( )->NetChanlLTE == Mode_NULL )
                 || ( sysparam_get( )->NetChanlLTE == WIFI_BAKE_MODE )
                 || ( sysparam_get( )->NetChanlLTE == WIFI_MODE ) )
			{
					group[cnt++]=AUD_ID_OR;
					group[cnt++]=AUD_ID_HOW_GPRS_TO_WIFI;
			}
            tts_play_set_group(group,cnt,0,0);
            uint32_t timer_notify = ql_rtos_get_systicks_to_ms() + 30 * 1000;
			while(1)
			{
				ql_rtos_task_sleep_ms(1000);
				if((TermInfo.NetMode != GPRS_MODE)&&(TermInfo.NetMode != GPRS_BAKE_MODE)) 
					break;
				if(TermInfo.SIMState)
					break;
				FuncInputChk();
				if( ql_rtos_get_systicks_to_ms() > timer_notify )
				{
				    timer_notify = ql_rtos_get_systicks_to_ms() + 60 * 1000;
				    tts_play_set_idx(AUD_ID_SIM_NOT_FOUND,0,0);
				}
			}
		}	
			break;
		case 0x01:
		{
				short group[4];
				int cnt=0;
				
				TermLedShow(TERM_NET_START);
				//tts_play_set(AudioZhenzljwl,AudioZhenzljwlLen,FixAudioTypeDef); 
				//tts_play_set(AudioQinsh,AudioQinshLen,FixAudioTypeDef); 
				//tts_play_set_idx(AUD_ID_NET_CONNECTING,0,0);
				//tts_play_set_idx(AUD_ID_PLEASE_WAITING,0,0);
				group[cnt++]=AUD_ID_NET_CONNECTING;
				group[cnt++]=AUD_ID_PLEASE_WAITING;
				if ( ( sysparam_get( )->NetChanlLTE == Mode_NULL )
					|| ( sysparam_get( )->NetChanlLTE == WIFI_BAKE_MODE )
					|| ( sysparam_get( )->NetChanlLTE == WIFI_MODE ) )
				{
					//tts_play_set(AudioHuo,AudioHuoLen,FixAudioTypeDef); 
					//tts_play_set(AudioChangagnjqhzwifiwlms,AudioChangagnjqhzwifiwlmsLen,FixAudioTypeDef); 
					//tts_play_set_idx(AUD_ID_OR,0,0);
					//tts_play_set_idx(AUD_ID_HOW_GPRS_TO_WIFI,0,0);
					group[cnt++]=AUD_ID_OR;
					group[cnt++]=AUD_ID_HOW_GPRS_TO_WIFI;
				}
				tts_play_set_group(group,cnt,0,0);
		}
			break;
		case 0x45:
		{
            TermLedShow(TERM_ABNORMAL_EXT);
            disp_set_net_connect_error(1);
			TermInfo.Repeat=FuncModuleErrRepeat;
			FuncModuleErrRepeat();
		}	
			break;
		default:
			break; 
	}

	TermInfo.Repeat=FuncModuleErrRepeat;
	
	LOG_INFO("device_SN: %s\n", sysparam_get()->device_SN);
#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if ((strlen(sysparam_get()->device_SN) == 0) || (strlen(sysparam_get()->device_name) == 0) )// ||(strlen(sysparam_get()->device_secret) == 0) || (strlen(sysparam_get()->product_key) == 0))
#else
	if (strlen(sysparam_get()->device_SN) == 0)
#endif
	{
	    disp_set_service_connect_error(1);
		//tts_play_set(AudioQingxrsbcs,AudioQingxrsbcsLen,FixAudioTypeDef);
		LOG_INFO("%s: ------------------------AUD_ID_PARAM_EMPTY---------2--------\n",__func__);
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
	}
	else 
	{
		TermLedShow(TERM_ABNORMAL);
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+60;
	}
    
	LOG_INFO("%s: loop start\n",__func__);
	while(1)
	{
		ql_rtos_task_sleep_ms(500);
		if((TermInfo.NetMode != GPRS_MODE)&&(TermInfo.NetMode != GPRS_BAKE_MODE)) break;
		FuncInputChk();
		if (TermInfo.Repeat!=NULL)
		{
			if (ql_rtos_get_systicks_to_s()>TermInfo.RepTim)
				TermInfo.Repeat();
		}

		if(TermInfo.PlayRecordMode)
		{
			Play_Record_Chk();
		}

		state = Module_Net_State();
		//LOG_INFO("Module_Net_State state = %d\n",state);
		//network status changed
		if (state==TermInfo.NetStatBak)  continue;
	
		if(state)
		{
			if ((sysparam_get()->NetChanlLTE == WIFI_MODE) || (sysparam_get()->NetChanlLTE == WIFI_BAKE_MODE) )
			{
				TermInfo.NetMode = GPRS_MODE;
				sysparam_get()->NetChanlLTE = GPRS_MODE;
				sysparam_save();
			}
			else if(sysparam_get()->NetChanlLTE == Mode_NULL)
			{
				TermInfo.NetMode = GPRS_BAKE_MODE;
				sysparam_get()->NetChanlLTE = GPRS_BAKE_MODE;	
				sysparam_save();
			}
		}
			
		TermInfo.NetStatBak=state;
		if (state!=NET_DEVICE_STATE_CONNECTED) 
		{
			TermInfo.FailTime=ql_rtos_get_systicks_to_s();
			LOG_INFO("net fail time %d\n",TermInfo.FailTime);
			int module_state = NET_DEVICE_STATE_DISCONNECTED;
			set_net_led_state(0);
			lpm_set(LPM_LOCK_NETWORK,1);
			ql_rtos_task_sleep_ms(20000);
			//查询注网 ，确实是否真实断网
			Get_Let_CregState();
			module_state = Module_Net_State();
			if (module_state != NET_DEVICE_STATE_CONNECTED) 
			{
				TermInfo.NetStatBak=module_state;
				TermLedShow(TERM_NET_DIS);
				disp_set_net_connect_error(1);
				disp_sleep_enable( 0 );
				TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
				TermInfo.Repeat=FuncModuleErrRepeat;
				FuncModuleErrRepeat();
				continue;				
			}
			else
				continue;
		}
		else
			set_net_led_state(1);

		lpm_set(LPM_LOCK_NETWORK,0);
		if ((TermInfo.ServiceOnline&(MqttMaskA|MqttMaskB))==(MqttMaskA|MqttMaskB))
		{
			TermLedShow(TERM_NET_SERVER_CON);
		}
		else
		{
			TermLedShow(TERM_NET_CON);
		}
		//tts_play_set(AudioWanglljcg,AudioWanglljcgLen,FixAudioTypeDef);
		if (TermInfo.PlayNetSucc==0)
		{
			TermInfo.PlayNetSucc=1;
			tts_play_set_idx(AUD_ID_NET_CONNECT_SUCESS,0,0);
		}
		mqtt_first_connet = 0;
#ifdef DUAL_MQTT_SUPPORT
		if ((TermInfo.ServiceOnline&(MqttMaskA|MqttMaskB))==(MqttMaskA|MqttMaskB))
#else
		if( TermInfo.ServiceOnline )
#endif
		{
			// ?????????????????????????????
			disp_onoff_request( 1, DISP_HOLDON_MS );
			ql_rtos_task_sleep_ms( 50 );
			disp_set_service_connect_error( 0 );
			disp_sleep_enable( 1 );
		}

		TermInfo.Repeat=FuncLteRetryRepeat;
		FuncLteRetryRepeat();
	}
	printf("voice stop = %d\n",voice_stop());
	TermInfo.Repeat=NULL;
	FuncInputChk();

	MqttExit();
}
#if BT
	//Indicates getting wifi networking information from Bluetooth
	extern uint8 wifi_BT_flag;
#endif

void NetChanlWifi(void)
{
	set_net_led_state(0);
	disp_wifi_mode();
	disp_set_net_connect_error(0);
	lpm_set(LPM_LOCK_WIFI,1);
	lpm_set(LPM_LOCK_NETWORK,1);

	int state=NET_DEVICE_STATE_DISCONNECTED;

	TermInfo.buttonAP = 0;
	Ext_wifi_Off();
	TermInfo.NetStatBak=NET_DEVICE_STATE_DISCONNECTED;
	TermInfo.PlayNetSucc = 0;

#if BT
		if((sysparam_get()->NetChanlLTE != Mode_NULL)&&!wifi_BT_flag)
#else
		if(sysparam_get()->NetChanlLTE != Mode_NULL)
#endif
		//tts_play_set(AudioXianzswifiwlms,AudioXianzswifiwlmsLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_NET_MODE_WIFI,0,0);

    set_LTE_module_low_power();
    
//    if (poweron_switch_modem_min_fun == 0)
//    {
//        ql_rtos_task_sleep_ms(1000);
//        AudioPlayHalt();
//    	ql_dev_set_modem_fun(QL_DEV_MODEM_MIN_FUN,0);
//        AudioPlayContinue();
//    }
    AudioPlayHalt();
    //ql_dev_set_modem_fun(QL_DEV_MODEM_MIN_FUN,0);
    ModuleNetFunc(0);
    AudioPlayContinue();
	
	TermInfo.Repeat=FuncWifiErrRepeat;

	Ext_wifi_Open();		
	
	if (strlen(sysparam_get()->device_SN) == 0)
	{
		disp_set_net_connect_error(1);
		//tts_play_set(AudioQingxrsbcs,AudioQingxrsbcsLen,FixAudioTypeDef);
		LOG_INFO("%s: ------------------------AUD_ID_PARAM_EMPTY---------3--------\n",__func__);
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
	}
	else if (((sysparam_get()->wlan_sta_param.ssid_len) <= 0))
	{
		TermLedShow(TERM_ABNORMAL);
		disp_net_connecting();
		if(sysparam_get()->NetChanlLTE == Mode_NULL)
		{
			TermInfo.buttonAP = 2;
		}
		else
		{
			short group[4];
			int cnt=0;
			//tts_play_set(AudioShuangjgnjqhzappwms,AudioShuangjgnjqhzappwmsLen,FixAudioTypeDef);
			//tts_play_set(AudioHuo,AudioHuoLen,FixAudioTypeDef); 
			//tts_play_set(AudioChangagnjqhzgprswlms,AudioChangagnjqhzgprswlmsLen,FixAudioTypeDef); 
			//tts_play_set_idx(AUD_ID_HOW_WIFI_TO_AP,0,0);
			//tts_play_set_idx(AUD_ID_OR,0,0);
			//tts_play_set_idx(AUD_ID_HOW_WIFI_TO_GPRS,0,0);
			group[cnt++]=AUD_ID_HOW_WIFI_TO_AP;
			group[cnt++]=AUD_ID_OR;
			group[cnt++]=AUD_ID_HOW_WIFI_TO_GPRS;
			tts_play_set_group(group,cnt,0,0);
			
			TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
		}
	}
	else 
	{
	    disp_net_connecting();
		if ((sysparam_get()->wlan_sta_param.ssid_len) > 0)
		{
			if (wlan_sta_set((uint8_t *)sysparam_get()->wlan_sta_param.ssid, sysparam_get()->wlan_sta_param.ssid_len, (uint8_t *)sysparam_get()->wlan_sta_param.psk)==0)
			{
				ql_rtos_task_sleep_ms(500);
				WifiSetOK=1;
#if BT
                if(wifi_BT_flag)
                {
                    wifi_BT_flag=0;
                }
#endif
				Ext_wifi_reBoot();
			}
			else
			{
			    disp_set_net_connect_error(1);
			}
			
//			wlan_sta_enable();
			LOG_INFO("enter %s , line %d,%s,%s\n", __func__, __LINE__,sysparam_get()->wlan_sta_param.ssid,sysparam_get()->wlan_sta_param.psk);
		}
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+60;
	}

	LOG_INFO("%s: loop start,wifi ver=%s\n", __func__,sysparam_get()->WifiVer);
	while(1)
	{
		ql_rtos_task_sleep_ms(500);
#if BT
				if((TermInfo.NetMode != WIFI_MODE)&&(TermInfo.NetMode != WIFI_BAKE_MODE)||wifi_BT_flag) break;
#else
				if((TermInfo.NetMode != WIFI_MODE)&&(TermInfo.NetMode != WIFI_BAKE_MODE)) break;
#endif
		
		if (TermInfo.buttonAP)
		{
			TermLedShow(TERM_NET_ABNORMAL);
			disp_net_connecting();
			MqttExit();
			WifiConfigSet();
			TermInfo.buttonAP = 0;
			TermInfo.NetStatBak=NET_DEVICE_STATE_DISCONNECTED;
			TermInfo.Repeat=FuncWifiErrRepeat;
			TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
			WifiSetOK=0;
		}
		if (Ext_wifi_reBoot()||(WifiSetOK==0))
		{
			WifiSetOK=0;
			NetStat=NET_DEVICE_STATE_DISCONNECTED;
			TermInfo.NetStatBak=NET_DEVICE_STATE_DISCONNECTED;
			if ((sysparam_get()->wlan_sta_param.ssid_len) > 0)
			{
				if (wlan_sta_set((uint8_t *)sysparam_get()->wlan_sta_param.ssid, sysparam_get()->wlan_sta_param.ssid_len, (uint8_t *)sysparam_get()->wlan_sta_param.psk)==0)
				{
					ql_rtos_task_sleep_ms(500);
					WifiSetOK=1;
				}	
	//			wlan_sta_enable();
				LOG_INFO("enter %s , line %d,%s,%s\n", __func__, __LINE__,sysparam_get()->wlan_sta_param.ssid,sysparam_get()->wlan_sta_param.psk);
			}
		}
		
		FuncInputChk();
		if (TermInfo.Repeat!=NULL)
		{
			if (ql_rtos_get_systicks_to_s()>TermInfo.RepTim)
				TermInfo.Repeat();
		}
		if(TermInfo.PlayRecordMode)
		{
			Play_Record_Chk();
		}

		state=NetStat;
		//???????б仯
		if (state==TermInfo.NetStatBak)  continue;
		if(state)
		{
			if( (sysparam_get()->NetChanlLTE == GPRS_MODE)||(sysparam_get()->NetChanlLTE == GPRS_BAKE_MODE) )
			{
				TermInfo.NetMode = WIFI_MODE;
				sysparam_get()->NetChanlLTE = WIFI_MODE;
				sysparam_save();
			}
			else if(sysparam_get()->NetChanlLTE == Mode_NULL)
			{
				TermInfo.NetMode = WIFI_BAKE_MODE;
				sysparam_get()->NetChanlLTE = WIFI_BAKE_MODE;	
				sysparam_save();
			}
		}

		if (state!=NET_DEVICE_STATE_CONNECTED) 
		{
			set_net_led_state(0);
			TermInfo.FailTime=ql_rtos_get_systicks_to_s();
			LOG_INFO("net fail time %d\n",TermInfo.FailTime);
			setNetLedOnOff(0);
			lpm_set(LPM_LOCK_NETWORK,1);
			ql_rtos_task_sleep_ms(10000);
			state=NetStat;
			if (state != NET_DEVICE_STATE_CONNECTED) 
			{
				TermInfo.NetStatBak = state;
				TermLedShow( TERM_NET_DIS );
				disp_set_net_connect_error( 1 );
				disp_sleep_enable( 0 );
				TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
				TermInfo.Repeat=FuncWifiErrRepeat;
				FuncWifiErrRepeat();
				continue;				
			}
			else
				continue;
		}
		else
			set_net_led_state(1);

		TermInfo.NetStatBak=state;
		lpm_set(LPM_LOCK_NETWORK,0);
		TermLedShow(TERM_NET_CON);
		//tts_play_set(AudioWanglljcg,AudioWanglljcgLen,FixAudioTypeDef);
		if (TermInfo.PlayNetSucc==0)
		{
			TermInfo.PlayNetSucc=1;
			tts_play_set_idx(AUD_ID_NET_CONNECT_SUCESS,0,0);
		}
		mqtt_first_connet = 0;
#ifdef DUAL_MQTT_SUPPORT
		if ((TermInfo.ServiceOnline&(MqttMaskA|MqttMaskB))==(MqttMaskA|MqttMaskB))
#else
		if( TermInfo.ServiceOnline )
#endif
		{
			// ?????????????????????????????
			disp_onoff_request( 1, DISP_HOLDON_MS );
			ql_rtos_task_sleep_ms( 50 );
			disp_set_service_connect_error( 0 );
			disp_sleep_enable( 1 );
		}
		TermInfo.Repeat=FuncWifiRetryRepeat;
		FuncWifiRetryRepeat();
	} 

//    if (poweron_switch_modem_min_fun == 1)
//    {
//        LTE_CAT1_init();
//    }
//    poweron_switch_modem_min_fun = 0;
	printf("voice stop = %d\n",voice_stop());

	TermInfo.Repeat=NULL;
	MqttExit();

	lpm_set(LPM_LOCK_WIFI,0);
}

void NetChanlNull(void)
{

    disp_none_mode();
#if 1
    TermInfo.NetMode = GPRS_BAKE_MODE;
#else

    if(i<200)
    {
        if(TermInfo.SIMState != 1) // ???SIM??
        {
            if(TermInfo.SIMState == 0)
                //tts_play_set(AudioWeijcdsimk,AudioWeijcdsimkLen,FixAudioTypeDef);
                tts_play_set_idx(AUD_ID_SIM_INSERT_REQ,0,0);
//            else if(TermInfo.SIMState == 0x45)
//                tts_play_set(AudioShebgz,AudioShebgzLen,FixAudioTypeDef);
            TermInfo.NetMode = WIFI_BAKE_MODE;
        }
        else
        {
            //NetChanlLTE();
            TermInfo.NetMode = GPRS_BAKE_MODE;
        }
    }
    else
    {
        //NetChanlWifi();
        TermInfo.NetMode = WIFI_BAKE_MODE;
    }
#endif
}

void App_task(void *pvParameters)
{
	start();

	list_dir("B:/");
	uint32_t disk_empty_size = ql_fs_free_size(U_DISK_SYM);
    int disk_total_size = ql_fs_size(U_DISK_SYM);
    usb_log_printf("%s_%d : ????? %d\r\n", __func__, __LINE__, disk_empty_size);
    usb_log_printf("%s_%d : ????????? %d\r\n", __func__, __LINE__, disk_total_size);
	disk_empty_size = ql_fs_free_size(B_DISK_SYM);
	disk_total_size = ql_fs_size(B_DISK_SYM);
    usb_log_printf("%s_%d : ????? %d\r\n", __func__, __LINE__, disk_empty_size);
    usb_log_printf("%s_%d : ????????? %d\r\n", __func__, __LINE__, disk_total_size);
	usb_log_printf("\r\n**********");

	const dev_config_t *pdevconf = get_device_config();
	usb_log_printf("\n");
	usb_log_printf("======================================================================================\n\n");
	usb_log_printf(" device: %s-%s, hw: %s, sdk: %s, fw: %s, build: %s %s\n\n",
			pdevconf->devname,
			sysparam_get_device_type() == 0 ? "4G":"4GW",
			pdevconf->module,g_kernel_version,firmware_version + strlen(pdevconf->devname)+ strlen(g_kernel_version) + 2,
			__DATE__, __TIME__);
	usb_log_printf("======================================================================================\n\n");

	aging_test_task();
	
    if (sysparam_get_device_type() == 1)
	{
		TermInfo.NetMode = sysparam_get()->NetChanlLTE;
	}
	else
	{
		TermInfo.NetMode = GPRS_BAKE_MODE;
//		if (poweron_switch_modem_min_fun == 1)
//		{
//			LTE_CAT1_init();
//		}
//		poweron_switch_modem_min_fun = 0;
	}

	while(1)
	{
		switch(TermInfo.NetMode)
		{
			case GPRS_MODE:
			case GPRS_BAKE_MODE:
			{
				LOG_INFO("<--NetChanlLTE-->\n");
				sockets_funcset(1);
				NetChanlLTE();
			}
			break;
			case WIFI_MODE:
			case WIFI_BAKE_MODE:
			{
				LOG_INFO("<--NetChanlWifi-->\n");
				sockets_funcset(0);
				NetChanlWifi();
			}
			break;
			case Mode_NULL:
			{
				LOG_INFO("<--Net Mode_NULL-->\n");
				NetChanlNull();
			}
			break;
			default:
				break;
		}
	}
}

void AppTaskInit(void)
{
	if (ql_rtos_task_create(&app_task_ctrl_thread,
						APP_TASK_CTRL_THREAD_STACK_SIZE,
						100,
						"App_task",
						App_task,
						NULL) != OS_OK) {
		LOG_INFO("--thread create error\n");
	}
}

void AppTaskDeInit(void)
{
	ql_rtos_task_delete(app_task_ctrl_thread);
	LOG_INFO("app_task_ctrl_thread delete\n");
}

void app_generate_kernel_app_version(void)
{
    char temp_buff[40];
    
    memset(temp_buff, 0, sizeof(temp_buff));
    ql_api_map_caller(QL_API_MAP_GET_KERNEL_VERSION, temp_buff);

    if (strlen(temp_buff) > 0)
    {
        memset(g_kernel_version, 0, sizeof(g_kernel_version));
        strcpy(g_kernel_version, temp_buff);

        memset(temp_buff, 0, sizeof(temp_buff));
        sprintf(temp_buff, "%s_%s_%s", get_device_config()->devname,g_kernel_version, firmware_version);
        memset(firmware_version, 0, sizeof(firmware_version));
        strcpy(firmware_version, temp_buff);

        memset(temp_buff,0,sizeof(temp_buff));
        ql_api_map_caller(QL_API_MAP_GET_KERNEL_BUILD_TIME, temp_buff);
        LOG_INFO("firmware info: app: %s, kernel: %s, build: %s\n",firmware_version,g_kernel_version,temp_buff);
    }
}
//External fs init
void ex_fs_init(void)
{
	int ret;
	int port_index = EXTERNAL_NORFLASH_PORT33_36;
	int clk = _APBC_SSP_FNCLKSEL_1_625MHZ_;
	LOG_INFO("[FS] ========== exflash init  \r\n");
	ql_spi_nor_init(port_index, clk);	

	ret = qextfs_init('B', "external_fs", 0, port_index, 0, 0x800000);
	LOG_INFO("[FS] ========== exfs init : %d  \r\n", ret);
}
void sys_disk_init(void)
{
	int ret;
	ret = qfs_init('S', "ymzn_param",  0);
	LOG_INFO("[FS] ========== exfs init : %d  \r\n", ret);
}

#define FILE_NAME			"testFile.txt"
#define FILE_CONTENT 		"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIR_NAME			"testDir"

#define B_DISK				"S:"
#define B_ROOT_PATH			""B_DISK"/"
#define B_FILE_UNDER_ROOT 	""B_DISK"/"FILE_NAME""
#define B_DIR_PATH 			""B_DISK"/"DIR_NAME""
#define B_FILE_UNDER_DIR	""B_DIR_PATH"/"FILE_NAME""
//External fs test
static void ex_file_test(void)
{
	QFILE * fp = NULL;
	char buf[100] = {0};
	int ret;

	printf("[EXFS] ========== file test satrt\r\n");
	
	fp = ql_fopen(B_FILE_UNDER_ROOT, "w+");
	if(fp == NULL)
	{
		printf("[EXFS] *** file create fail: %s ***\r\n", B_FILE_UNDER_ROOT);
		goto exit;
	}
	printf("[EXFS] file created: %s\r\n", B_FILE_UNDER_ROOT);
	
	ret = ql_fwrite((void *)FILE_CONTENT, strlen(FILE_CONTENT), 1, fp);
	if(ret < 0) 
	{
		printf("[EXFS] *** file write fail: %s ***\r\n", B_FILE_UNDER_ROOT);
		goto exit;
	}
	printf("[EXFS] file write: [%d]%s\r\n", ret, FILE_CONTENT);
	
	memset(buf ,0 ,sizeof(buf));
	
	ret = ql_fseek(fp,0,0);
	if (ret < 0)
	{
		printf("[EXFS] file seek err\r\n");
	}
	
	ret = ql_fread((void *)buf, strlen(FILE_CONTENT),1,fp);
	if(ret < 0) 
	{
		printf("[EXFS] *** file read fail: %s ***\r\n", B_FILE_UNDER_ROOT);
		goto exit;
	}
	printf("[EXFS] file read: [%d]%s\r\n", ret, buf);
	
exit:
	if(fp)
	{
		ql_fclose(fp);
		printf("[EXFS] file closed: %s\r\n", B_FILE_UNDER_ROOT);
	}
	printf("[EXFS] ========== file test finished\r\n");
}

uint8 t_aging_test_flag = 0;
void aging_test_task(void)
{
	unsigned char level = 0;
	unsigned char keypress=0;
	sysparam_get();
	static uint32_t bat_charge_start_time=0;
	static uint8_t bat_charge_start_flag=0;
	static uint32_t audio_play_start_time=0;
	static uint8_t audio_play_start_flag=0;	
	uint8_t step=0;
	uint8_t ret = 0;	
	ql_gpio_init(GPIO_PIN_NO_18, PIN_DIRECTION_IN, PIN_PULL_PU, PIN_LEVEL_HIGH);
	ql_gpio_init(GPIO_PIN_NO_17, PIN_DIRECTION_IN, PIN_PULL_PU, PIN_LEVEL_HIGH);
	ql_gpio_get_level(GPIO_PIN_NO_18, &level);	
	ql_rtos_task_sleep_ms(100);
	printf("++++ql_gpio_get_level(GPIO_PIN_NO_18, &level)=%d\r\n",level);
	if(level==0)
	{
		keypress = 1;
	}
	uart_task_init();
	while(1)
	{
		if(keypress)
		{
			ret = 1;
			break;
		}
		else 
		{
			ret = 0;
			break;	
		}
		ql_rtos_task_sleep_ms(10);
	}
	if(ret == 1)
	{
		setBlueLedOnOff(0);
		setGreenLedOnOff(0);
		drv_bat_init();
		t_aging_test_flag = 1;
		tts_play_init(); // tts 
	}
	else
	{
		t_aging_test_flag =0;
		return ;
	}
	tts_play_set_idx(AUD_ID_LAOHUA_TEST,0,0);
	while(1)
	{
		switch(step)
		{
			case 0:
			{
				if(ql_get_vbus_state())//插入充电线
				{
					if(bat_charge_start_flag==0)
					{
						setBlueLedOnOff(0);
						setGreenLedOnOff(0);
						bat_charge_start_time=ql_rtos_get_systicks_to_s();
						printf("--------bat_charge_start_time=ql_rtos_get_systicks_to_s();=%d-------\r\n",bat_charge_start_time);
					}
					bat_charge_start_flag=1;
					strcpy(sysparam_get()->aging_state,"start");
					strcpy(sysparam_get()->aging_time,"0");					
					sysparam_save();	
				}
				else
				{
					bat_charge_start_flag = 0;
					bat_charge_start_time = 0;
					printf("--------bat_charge_start_flag=0;--------\r\n");
				}		
				//-------------------------------------------------------------------------------------------------------------
				if(bat_charge_start_flag)
				{
					//setRedLedOnOff(0);
					static uint32_t cnt_play1 = 0;
					static uint8_t flag_deng =0;

					cnt_play1++;
					if(cnt_play1%100==0)
					{
						uint32_t ret_sum=ql_rtos_get_systicks_to_s()-bat_charge_start_time;
						char buf[32]={0};
						sprintf(buf,"%d",ret_sum);
						strcpy(sysparam_get()->aging_time,buf);
						sysparam_save();						
					}					

					if(flag_deng)
					{
						setBlueLedOnOff(1);
						setGreenLedOnOff(0);
						flag_deng=0;
					}
					else
					{
						setBlueLedOnOff(0);
						setGreenLedOnOff(1);
						flag_deng=1;						
					}

					if((ql_rtos_get_systicks_to_s()-bat_charge_start_time)>=900)
					{
						printf("完成第一阶段，到第二阶段\r\n");
						bat_charge_start_flag = 0;
						bat_charge_start_time = 0;						
						step =1;
					}
				}
				break;
			}
			case 1:
			{
				if(audio_play_start_flag==0)
				{
					audio_play_start_time=ql_rtos_get_systicks_to_s();	
					audio_play_start_flag = 1;
					extern void bat_Charge_OnOff(int OnOff);
					bat_Charge_OnOff(0);//关闭充电
					printf("关闭充电\r\n");
					strcpy(sysparam_get()->aging_state,"charge complete");
					strcpy(sysparam_get()->aging_time,"1800");					
					sysparam_save();					
				}
				//---------------------------------------------------------------------------GPIO_PIN_NO_17
				static uint8_t flag_deng1 =0;
				static uint32_t cnt_play = 0;
				if(audio_play_start_flag ==1)
				{
					if(cnt_play%10==0)
					{
						tts_play_set_idx(AUD_ID_LAOHUA_TEST,0,0);
					}
					
					cnt_play++;
					if(cnt_play%100==0)
					{
						//tts_play_set_idx(AUD_ID_LAOHUA_TEST,0,0);
						uint32_t ret_sum=ql_rtos_get_systicks_to_s()-bat_charge_start_time;
						char buf[32]={0};
						sprintf(buf,"%d",ret_sum);
						strcpy(sysparam_get()->aging_time,buf);
						sysparam_save();						
					}					
					if(flag_deng1)
					{
						setBlueLedOnOff(1);
						setGreenLedOnOff(0);
						flag_deng1=0;
					}
					else
					{
						setBlueLedOnOff(0);
						setGreenLedOnOff(1);
						flag_deng1=1;						
					}				
					
					if((ql_rtos_get_systicks_to_s()-audio_play_start_time)>=2700)
					{
						printf("完成第二阶段，到第三阶段\r\n");
						audio_play_start_time = 0;
						audio_play_start_flag = 0;						
						step =2;
					}

				}
				break;
			}
			case 2:
			{
				printf("第三阶段，完成老化测试\r\n");
				strcpy(sysparam_get()->aging_state,"success");
				strcpy(sysparam_get()->aging_time,"3600");
				sysparam_save();
				tts_play_set_idx(AUD_ID_LAOHUA_TEST_wancheng,0,0);
				ql_rtos_task_sleep_ms(2500);
				ql_power_down(0);				
				break;
			}	
		}	
		static int cnt=0;
		ql_rtos_task_sleep_ms(500);
		ql_gpio_get_level(GPIO_PIN_NO_17, &level);
		if(level==0)
		{
			printf("++++ql_gpio_get_level(GPIO_PIN_NO_18, &level)=%d\r\n",level);
			cnt++;
			if(cnt>=4)
			{
				step = 0;
				bat_charge_start_time=0;
				bat_charge_start_flag=0;
				audio_play_start_time=0;
				audio_play_start_flag=0;
				setBlueLedOnOff(0);
				setGreenLedOnOff(0);									
				printf("退出老化测试\r\n");
				strcpy(sysparam_get()->aging_state,"exit");
				uint32_t ret_sum=ql_rtos_get_systicks_to_s()-bat_charge_start_time;
				char buf[32]={0};
				sprintf(buf,"%d",ret_sum);
				strcpy(sysparam_get()->aging_time,buf);
				sysparam_save();
				tts_play_set_idx(AUD_ID_LAOHUA_TEST_tuichu,0,0);
				return ;
			} 
		}
		else
		{
			cnt = 0;	
		}
	}
}

void evb_audio_main(void * param)
{
    mbedtls_platform_setup(NULL);
    memset(&TermInfo,0,sizeof(TermInfo));
    load_device_config(CONF_DEVTYPE_DEFAULT);
	ex_fs_init();
	sys_disk_init();
//	ex_file_test();

    app_generate_kernel_app_version();
    sysparam_init();
    ql_pwrkey_intc_enable(0);
    disp_port_init();
#if BT
		Bt_task_init();
#else
	//Do not turn off the power, Bluetooth power on to increase power consumption
	if (get_device_config()->id==ID_DS200BT)
		BT_POW_OFF();
#endif
	
	isEnterFactorymode();

//	{
//		char resp_buf[128] = {0};
//		char atcmd[64];
//	
//		sprintf(atcmd,"AT+LOG=15,13\r\n");
//		ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
//	}


	if(sysparam_get()->uart_log == 0)
	{
		ql_debug_log_disable();
	}
    else
    {
        ql_debug_log_enable();
    }
	AppTaskInit();
//	return 0;
}

application_init(evb_audio_main, "evb_audio_main", 10, 1);

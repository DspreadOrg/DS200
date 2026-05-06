#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ql_rtos.h"
#include "ql_spi.h"
#include "ql_fs.h"
#include "prj_common.h"
#include "res.h"
#include "fixaudio.h"
#include "public_api_interface.h"
#include "systemparam.h"

#define LOG_DBG(...)            //do{printf("[DBG PCM]: "); printf(__VA_ARGS__);}while(0)
#define LOG_ERR(fmt, arg...) usb_log_printf("[RES ERR]"fmt, ##arg)
#define LOG_INFO(fmt, arg...) usb_log_printf("[RES INFO]"fmt, ##arg)


AudioResInfo auds[] = {

	{AUD_ID_FAC_TEST_MODE,						STR_ID_FAC_TEST_MODE,						&AudioGcmsInfo,					NULL  },
	{AUD_ID_FAC_TEST_COMPLETE,					STR_ID_FAC_TEST_COMPLETE,					NULL,							NULL},
	{AUD_ID_FAC_TEST_AUDIO,						STR_ID_FAC_TEST_AUDIO,						&AudioYlcsInfo,					NULL	},
//	{AUD_ID_FAC_TEST_AUDIO,						STR_ID_FAC_TEST_AUDIO,						&AudioYlcsInfo,					NULL},
//	{AUD_ID_FAC_TEST_TTS,						STR_ID_FAC_TEST_TTS,						NULL,							NULL},
	{AUD_ID_FAC_TEST_PWROFF,					STR_ID_FAC_TEST_PWROFF,						&AudioZhengzgjInfo,				NULL },
	{AUD_ID_FAC_TEST_GET_VESION_FAIL,			STR_ID_FAC_TEST_GET_VESION_FAIL,			&AudioInputErrInfo,				NULL},
	{AUD_ID_FAC_TEST_SIM_NOT_FOUND,				STR_ID_FAC_TEST_SIM_NOT_FOUND,				NULL,							"PleaseInsertSIMCard.mp3"			},
	{AUD_ID_FAC_TEST_GET_LEV_FAIL,				STR_ID_FAC_TEST_GET_LEV_FAIL,				&AudioInputErrInfo,				NULL 		},
//	{AUD_ID_FAC_TEST_TTS_ACTIVEOK,				STR_ID_FAC_TEST_TTS_ACTIVEOK,				NULL,							NULL },
//	//用户取消交
	{AUD_ID_SALE_CANCLE,						STR_ID_SALE_CANCLE,							NULL,							NULL		},
	{AUD_ID_NUM_0,								STR_ID_NUM_0,								NULL,							"wav/0.wav" 	},
	{AUD_ID_NUM_1,								STR_ID_NUM_1,								NULL,							"wav/1.wav" 	},
	{AUD_ID_NUM_2,								STR_ID_NUM_2,								NULL,							"wav/2.wav" 	},
	{AUD_ID_NUM_3,								STR_ID_NUM_3,								NULL,							"wav/3.wav" 	},
	{AUD_ID_NUM_4,								STR_ID_NUM_4,								NULL,							"wav/4.wav" 	},
	{AUD_ID_NUM_5,								STR_ID_NUM_5,								NULL,							"wav/5.wav" 	},
	{AUD_ID_NUM_6,								STR_ID_NUM_6,								NULL,							"wav/6.wav" 	},
	{AUD_ID_NUM_7,								STR_ID_NUM_7,								NULL,							"wav/7.wav" 	},
	{AUD_ID_NUM_8,								STR_ID_NUM_8,								NULL,							"wav/8.wav" 	},
	{AUD_ID_NUM_9,								STR_ID_NUM_9,								NULL,							"wav/9.wav" 	},
	{AUD_ID_NUM_10, 							STR_ID_NUM_10,								NULL,							"wav/ten.wav"		},
	{AUD_ID_NUM_11, 							STR_ID_NUM_11,								NULL,							"wav/eleven.wav" 	},
	{AUD_ID_NUM_12, 							STR_ID_NUM_12,								NULL,							"wav/twelve.wav" 	},
	{AUD_ID_NUM_13, 							STR_ID_NUM_13,								NULL,							"wav/thirteen.wav"		},
	{AUD_ID_NUM_14, 							STR_ID_NUM_14,								NULL,							"wav/fourteen.wav" 	},
	{AUD_ID_NUM_15, 							STR_ID_NUM_15,								NULL,							"wav/fifteen.wav" 	},
	{AUD_ID_NUM_16,								STR_ID_NUM_16,								NULL,							"wav/sixteen.wav"		},
	{AUD_ID_NUM_17, 							STR_ID_NUM_17,								NULL,							"wav/seventeen.wav" 	},
	{AUD_ID_NUM_18,				 				STR_ID_NUM_18,								NULL,							"wav/eighteen.wav" 	},
	{AUD_ID_NUM_19, 							STR_ID_NUM_19,								NULL,							"wav/nineteen.wav"		},
	
	{AUD_ID_NUM_20, 							STR_ID_NUM_20,								NULL,							"wav/twenty.wav" 	},
	{AUD_ID_NUM_30, 							STR_ID_NUM_30,								NULL,							"wav/thirty.wav"		},
	{AUD_ID_NUM_40, 							STR_ID_NUM_40,								NULL,							"wav/forty.wav" 	},
	{AUD_ID_NUM_50, 							STR_ID_NUM_50,								NULL,							"wav/fifty.wav" 	},
	{AUD_ID_NUM_60, 							STR_ID_NUM_60,								NULL,							"wav/sixty.wav"		},
	{AUD_ID_NUM_70, 							STR_ID_NUM_70,								NULL,							"wav/seventy.wav" 	},
	{AUD_ID_NUM_80, 							STR_ID_NUM_80,								NULL,							"wav/eighty.wav" 	},
	{AUD_ID_NUM_90, 							STR_ID_NUM_90,								NULL,							"wav/ninety.wav"		},
	{AUD_ID_HUNDRED,							STR_ID_HUNDRED, 							NULL,							"wav/hundred.wav"		},
	{AUD_ID_THOUSAND,							STR_ID_THOUSAND,							NULL,							"wav/thousand.wav"		},
	{AUD_ID_MILLION,								STR_ID_MILION,						NULL,									"wav/million.wav"	},
	{AUD_ID_DOT,								STR_ID_DOT,									NULL,							"wav/dot.wav"		},
	{AUD_ID_UNIT,								STR_ID_UNIT,								NULL,							"wav/dollar.wav"		},

	{AUD_ID_PAYMENT,							STR_PAY_MSG_SK, 							NULL,							"wav/collection.wav"		},

	{AUD_ID_KEY_BEEP,							STR_ID_KEY_BEEP,							&AudioKeyBeepInfo,				NULL	},		
	{AUD_ID_KEY_ERR_BEEP,						STR_ID_KEY_ERR_BEEP,						&AudioInputErrInfo,				NULL		},	
	{AUD_ID_OR,									STR_ID_OR,									NULL,							"or.mp3"		},	
	{AUD_ID_LOG_MODE_USB,						STR_ID_LOG_MODE_USB,						&AudioInputErrInfo,				NULL		},	
	{AUD_ID_WELCOME,							STR_ID_WELCOME,								NULL,							"welcome.mp3"		},	
	{AUD_ID_PLEASE_WAITING,						STR_ID_PLEASE_WAITING,						NULL,							"PleaseWait.mp3"		},  

	{AUD_ID_SIM_NOT_FOUND,						STR_ID_SIM_NOT_FOUND,						NULL,							"SIMNotFound.mp3"		},
	{AUD_ID_SIM_INSERT_REQ,						STR_ID_SIM_INSERT_REQ,						NULL,							"PleaseInsertSIMCard.mp3"		},

	{AUD_ID_NET_FAULT,							STR_ID_NET_FAULT,							NULL,							"NetFault.mp3"		},
	{AUD_ID_NET_CONNECTING,						STR_ID_NET_CONNECTING,						NULL,							"NetConnecting.mp3"		},
	{AUD_ID_NET_CONNECT_FAIL,					STR_ID_NET_CONNECT_FAIL,					NULL,							"NetConnectFail.mp3"		},
	{AUD_ID_NET_CONNECT_SUCESS,					STR_ID_NET_CONNECT_SUCESS,					NULL,							"NetConnectSucess.mp3"		},
	{AUD_ID_NET_NOT_CONNECT,					STR_ID_NET_NOT_CONNECT,						NULL,							"NetDisonnect.mp3"		},
	{AUD_ID_NET_CONNECT,						STR_ID_NET_CONNECT,							NULL,							"NetCon.mp3"		},
	{AUD_ID_NET_MODE_GPRS,						STR_ID_NET_MODE_GPRS,						NULL,							"NewIsGPRS.mp3"		},
	{AUD_ID_NET_MODE_WIFI,						STR_ID_NET_MODE_WIFI,						NULL,							"NewIsWiFi.mp3"		},
	{AUD_ID_NET_MODE_WIFI_AIRKISS,				STR_ID_NET_MODE_WIFI_AIRKISS,				NULL,							"WiFiAirkiss.mp3"		},
	{AUD_ID_NET_MODE_WIFI_AP,					STR_ID_NET_MODE_WIFI_AP,					NULL,							"WiFiAP.mp3"		},
	{AUD_ID_MQTT_CONNECT_SUCESS,				STR_ID_MQTT_CONNECT_SUCESS,					NULL,							"MqttConnectSucess.mp3"		},
	{AUD_ID_MQTT_CONNECT_FAIL,					STR_ID_MQTT_CONNECT_FAIL,					NULL,							"MqttConnectFail.mp3"		},

	{AUD_ID_HOW_GPRS_TO_WIFI,					STR_ID_HOW_GPRS_TO_WIFI,					NULL,							"GoToWiFi.mp3"		},
	{AUD_ID_HOW_WIFI_TO_GPRS,					STR_ID_HOW_WIFI_TO_GPRS,					NULL,							"GoToGPRS.mp3"		},
	{AUD_ID_HOW_WIFI_TO_AP,						STR_ID_HOW_WIFI_TO_AP,						NULL,							"GoToWiFiAP.mp3"		},
	{AUD_ID_HOW_WIFI_TO_AIRKISS,				STR_ID_HOW_WIFI_TO_AIRKISS,					NULL,							"GoToWiFiAirkiss.mp3"		},
	{AUD_ID_EXIT_WIFI_SET,						STR_ID_EXIT_WIFI_SET,						NULL,							"ExitWiFiSet.mp3"		},

	{AUD_ID_PWR_LOWBAT,							STR_ID_PWR_LOWBAT,							NULL,							"LowBat.mp3"		},
	{AUD_ID_PWR_OFF,							STR_ID_PWR_OFF,								&AudioZhengzgjInfo,				"PowerOFF.mp3"		},
	{AUD_ID_PWR_LOWBATOFF,						STR_ID_PWR_LOWBATOFF,						NULL,							"LowBatOFF.mp3"		},
	{AUD_ID_PWR_PER,							STR_ID_PWR_PER,								NULL,							NULL		},

	{AUD_ID_CHARGE_IN,							STR_ID_CHARGE_IN,	  						NULL,							"Charging.mp3"		},
	{AUD_ID_CHARGE_OUT,							STR_ID_CHARGE_OUT,							NULL,							"ChargOur.mp3"		},
	{AUD_ID_CHARGE_FULL,						STR_ID_CHARGE_FULL, 						NULL,							"ChargFull.mp3"		},
	{AUD_ID_UPDATE_START,						STR_ID_UPDATE_START,						NULL,							"UpdateStart.mp3"		},
	{AUD_ID_UPDATE_SUCESS,						STR_ID_UPDATE_SUCESS,						NULL,							"UpdateSucess.mp3"		},
	{AUD_ID_UPDATE_FAIL,						STR_ID_UPDATE_FAIL,							NULL,							"UpdateFail.mp3"		},

	{AUD_ID_ACTIVE_FAIL,						STR_ID_ACTIVE_FAIL,							NULL,							"ActiveFail.mp3"		},
	{AUD_ID_ACTIVE_SUCESS,						STR_ID_ACTIVE_SUCESS,						NULL,							"ActiveSucess.mp3"		},

	{AUD_ID_VOL_MAX,							STR_ID_VOL_MAX,								NULL,							"VolMax.mp3"		},
	{AUD_ID_VOL_MIN,							STR_ID_VOL_MIN,								NULL,							"VolMin.mp3"		},

	{AUD_ID_EXIT_RECODE,						STR_ID_EXIT_RECODE,							NULL,							"ExitRecode.mp3"		},
	{AUD_ID_RECODE_NOT_FOUND,					STR_ID_RECODE_NOT_FOUND,					NULL,							"RecodeNotFound.mp3"		},
	{AUD_ID_RECOVER_FACTORY,					STR_ID_RECOVER_FACTORY,						NULL,							"RecoverFactory.mp3"		},
	{AUD_ID_PARAM_EMPTY,						STR_ID_PARAM_EMPTY,							NULL,							"PaRamEmpty.mp3"		},
	{AUD_ID_AND,								STR_ID_AND,							NULL,								"wav/and.wav"		},
#if BT_DEMO
	{AUD_ID_BT_CON, 							STR_ID_BT_CON,								NULL,							"BTcon.mp3"		},
	{AUD_ID_BT_DISCON,							STR_ID_BT_DISCON,							NULL,							"BTdiscon.mp3"		},
	{AUD_ID_BT_OPEN,							STR_ID_BT_OPEN, 							NULL,							"BTopen.mp3"		},
	{AUD_ID_BT_CLOSE,							STR_ID_BT_CLOSE,							NULL,							"BTclose.mp3" 	},
	{AUD_ID_BT_OTA, 							STR_ID_BT_OTA,								NULL,							"BTOTA.mp3" 	},
#endif
	{AUD_ID_LAOHUA_TEST,						STR_ID_LAOHUA_CESHI,						NULL,							"laohuaceshi.mp3"		},
	{AUD_ID_LAOHUA_TEST_tuichu,						STR_ID_LAOHUA_CESHI_tuichu,						NULL,							"tuichulaohuaceshi.mp3"		},	
	{AUD_ID_LAOHUA_TEST_wancheng,						STR_ID_LAOHUA_CESHI_wancheng,						NULL,							"laohuaceshiwancheng.mp3"		},
};


#if 0
	{ AudioHuanysylkl, "欢迎使用拉卡拉.mp3", "huanyingshiyonglakala.mp3" },
	{ AudioShebgz, "设备故障.mp3", "shebeiguzhang.mp3" },
	{ AudioQingayljkcx, "请按音量键查询最近10笔交易，单击功能键退出查询.mp3", "qayljcxzj10bjy,djgnjtccx.mp3"},
	{ AudioYear, "年.mp3", "nian.mp3" },
	{ AudioMonth, "月.mp3", "yue.mp3" },
	{ AudioDay, "日.mp3", "ri.mp3" },
	{ AudioHour, "时.mp3", "shi.mp3" },
	{ AudioSecond, "秒.mp3", "miao.mp3" },
	{ AudioFen, "分.mp3", "fen.mp3" },

	{ AudioQingsaomafukuan, "请扫码付款", "qingsaomafukuan.mp3" },
	{ AudioQingsaomatuikuan, "请扫码退款", "qingsaomatuikuan.mp3" },
	{ AudioZhifuchenggong, "支付成功", "zhifuchenggong.mp3" },
	{ AudioZhifushibai, "支付失败", "zhifushibai.mp3" },
	{ AudioZhifuchaoshi, "支付超时", "zhifuchaoshi.mp3" },

#endif

const AudioResInfo *GetRecInfo(int Idx)
{
	int ii;
	for(ii=0;ii<sizeof(auds)/sizeof(auds[0]);ii++)
	{
		if (Idx==auds[ii].Idx)
			return &auds[ii];
	}

	LOG_INFO("====Audio Idx=%d not found!====\n",Idx);
	return NULL;
}

uint32_t getResVer(void)
{
	return 0;
}

int getResCount(uint16_t type)
{
	return 0;
}

//int resource_init(void)
int IdxRangChk(void)
{
	int ii;
	int jj;
	
	for(ii=AUD_ID_FIX_AUDIO_BASE+1;ii<AUD_ID_MAX;ii++)
	{
		for(jj=0;jj<sizeof(auds)/sizeof(auds[0]);jj++)
		{
			if (ii==auds[jj].Idx)
				break;
		}
		if (jj<sizeof(auds)/sizeof(auds[0]))
			continue;

		LOG_INFO("=========Audio Idx %d defined,but not found at res tab!===========\n",ii);
	}
	
	return 0;
}

int ResTabChk(void)
{	
	int ii;
	char path[128] = {0};
	int total=0;
	int find=0;
	int not=0;

	for(ii=0;ii<sizeof(auds)/sizeof(auds[0]);ii++)
	{
		if ((auds[ii].Array==NULL)&&(auds[ii].filePinyinName==NULL))
		{
			LOG_INFO("=============Audio Def Err:==========\n");
			LOG_INFO("	Idx %d,TTsStr:%s\n",auds[ii].Idx,auds[ii].TTSstr);
			LOG_INFO("	info is empty,will play tts str !\n");
			LOG_INFO("=====================================\n");
			not ++;
		}
		if (auds[ii].filePinyinName)
		{
			total++;
			snprintf(path,sizeof(path),"%s%s",VOICE_PATH_PREFIX,auds[ii].filePinyinName);
			if ( ql_access(path,0) != 0 )
			{
				// fallback to U:/
				memset(path,0,sizeof(path));
				snprintf(path,sizeof(path),"%s%s",AUDIO_RESOURCE_ROOT_PATH,auds[ii].filePinyinName);
				if ( ql_access(path,0) != 0 )
				{
					LOG_INFO("=============Audio file not found:==========\n");
					LOG_INFO("	Idx %d,TTsStr:%s\n",auds[ii].Idx,auds[ii].TTSstr);
					LOG_INFO("	file \"%s\" not found!\n",auds[ii].filePinyinName);
					if ((auds[ii].Array==NULL))
					{
						LOG_INFO("	and Array is empty,will play tts str!\n");
						not ++;
					}
					else
						LOG_INFO("	and Array defined!\n");
					LOG_INFO("============================================\n");
				}
			}
			else find++;
		}
	}

	LOG_INFO("========item total %d,tts play %d ================\n",sizeof(auds)/sizeof(auds[0]),not);
	LOG_INFO("========file def total %d,found %d==========\n",total,find);

	return 0;
}



int AllVoiceTest(void)
{	
	int ii;
	char path[128] = {0};
	int total=0;
	int find=0;
	int not=0;

	for(ii=0;ii<sizeof(auds)/sizeof(auds[0]);ii++)
	{
		if (auds[ii].filePinyinName)
		{
			memset(path,0,sizeof(path));
			snprintf(path,sizeof(path),"%s%s",VOICE_PATH_PREFIX,auds[ii].filePinyinName);
			if ( ql_access(path,0) == 0 )
			{
				LOG_INFO("===============Play Aduio=============================\n");
				LOG_INFO("	Idx %d,fileName:%s\n",auds[ii].Idx,auds[ii].filePinyinName);
				tts_play_set_idx(auds[ii].Idx,0,0);
				total ++;
				ql_rtos_task_sleep_ms(2000);
			}
			else 
			{
				LOG_INFO("=============Audio file not found:==========\n");
				LOG_INFO("	Idx %d,file:%s\n",auds[ii].Idx,auds[ii].filePinyinName);
			}
		}
	}

	LOG_INFO("========item total %d,tts play %d ================\n",sizeof(auds)/sizeof(auds[0]),total);

	return 0;
}

const char *  getImgDataAddr(char const * pfileName)//,uint32_t *fileLen)
{
	char fileName[32+1];
	char buf[32+1];

	extern unsigned char acstatimg[];
	uint32_t ResBaseAddr =acstatimg;

	int sAddr =0;// ResBaseAddr;
	uint8_t dataHeader[HEADER_SIZE+1] = {0};

	if (strlen(pfileName)>32) return -1;
	memset(fileName,0,sizeof(fileName));
	memcpy(fileName,pfileName,strlen(pfileName));

	strupr(fileName);
	pfileName=fileName;
	//printf("enter %s , line %d,name:%s\n", __func__, __LINE__,pfileName);
	while(1)
	{
		memset(dataHeader,0,sizeof(dataHeader));
		memcpy(dataHeader,sAddr+ResBaseAddr,HEADER_SIZE);

		if(strncmp((char *)dataHeader,"YMZN",strlen("YMZN")) == 0)
		{
			struct Download_header *file_header = (struct Download_header *)dataHeader;
			if(file_header->attribute==1)
			{
				int len1;

				len1=strlen((const char *)file_header->FileName);
				if (len1>32) len1=32;
				memset(buf,0,sizeof(buf));
				memcpy(buf,file_header->FileName,len1);
				strupr(buf);
				if (strcmp(pfileName,buf)==0)
				{
					// printf("%s , line %d find file ok\n", __func__, __LINE__);
					sAddr = sAddr + HEADER_SIZE;
//					*fileLen = file_header->data_size;
					break;
				}
			}
			sAddr = file_header->next_addr;
			if (sAddr==0) return NULL;
		}
		else return NULL;
	}

	return (void *)(sAddr+ResBaseAddr);
}

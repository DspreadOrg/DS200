#ifndef __FIXAUDIO_H__
#define __FIXAUDIO_H__


////#define PlayBuildinAudio
typedef struct{
	char const * Data;
	unsigned int dlen;
}AudioArrayInfo;

typedef struct {
	const unsigned int Idx;
	const char * TTSstr;
	const AudioArrayInfo * Array;
	const char *filePinyinName;
}AudioResInfo;

#define VOICE_PATH_PREFIX       "B:/"

extern const AudioResInfo *GetRecInfo(int Idx);

extern const AudioArrayInfo AudioGcmsInfo;
extern const AudioArrayInfo AudioYlcsInfo;
extern const AudioArrayInfo AudioZhengzgjInfo;
extern const AudioArrayInfo AudioKeyBeepInfo;
extern const AudioArrayInfo AudioWelcomeInfo;
extern const AudioArrayInfo AudioInputErrInfo;
extern const AudioArrayInfo AudioCswcInfo;

#define AudioKeyErrBeepInfo AudioKeyBeepInfo

typedef enum
{
	AUD_ID_GROUP_IDX,
	AUD_ID_SALE_ID_INFO,
	AUD_ID_SALE_STR_INFO,
	AUD_ID_USER_DEF,
	AUD_ID_RECODE_PLAY,

	AUD_ID_FIX_AUDIO_BASE=2000,
	AUD_ID_FAC_TEST_MODE,
	AUD_ID_FAC_TEST_COMPLETE,
	AUD_ID_FAC_TEST_AUDIO,
	AUD_ID_FAC_TEST_PWROFF,
	AUD_ID_FAC_TEST_GET_VESION_FAIL,
	AUD_ID_FAC_TEST_SIM_NOT_FOUND,
	AUD_ID_FAC_TEST_GET_LEV_FAIL,
	AUD_ID_CUR_SIGNAL,

	AUD_ID_SALE_CANCLE,
	AUD_ID_NUM_0,
	AUD_ID_NUM_1,
	AUD_ID_NUM_2,
	AUD_ID_NUM_3,
	AUD_ID_NUM_4,
	AUD_ID_NUM_5,
	AUD_ID_NUM_6,
	AUD_ID_NUM_7,
	AUD_ID_NUM_8,
	AUD_ID_NUM_9,
	AUD_ID_NUM_10,
	AUD_ID_NUM_11,
	AUD_ID_NUM_12,
	AUD_ID_NUM_13,
	AUD_ID_NUM_14,
	AUD_ID_NUM_15,
	AUD_ID_NUM_16,
	AUD_ID_NUM_17,
	AUD_ID_NUM_18,
	AUD_ID_NUM_19,
	AUD_ID_NUM_20,
	AUD_ID_NUM_30,
	AUD_ID_NUM_40,
	AUD_ID_NUM_50,
	AUD_ID_NUM_60,
	AUD_ID_NUM_70,
	AUD_ID_NUM_80,
	AUD_ID_NUM_90,
	AUD_ID_HUNDRED,
	AUD_ID_THOUSAND,
	AUD_ID_MILLION,
	AUD_ID_DOT,
	AUD_ID_UNIT,
	AUD_ID_PAYMENT,



	AUD_ID_WELCOME,
	AUD_ID_KEY_BEEP,
	AUD_ID_KEY_ERR_BEEP,
	AUD_ID_LOG_MODE_USB,
	AUD_ID_OR,
	AUD_ID_PLEASE_WAITING,
	AUD_ID_SIM_NOT_FOUND,
	AUD_ID_SIM_INSERT_REQ,
	AUD_ID_NET_FAULT,
	AUD_ID_NET_CONNECTING,
	AUD_ID_NET_CONNECT_FAIL,
	AUD_ID_NET_CONNECT_SUCESS,
	AUD_ID_NET_NOT_CONNECT,
	AUD_ID_NET_CONNECT,
	AUD_ID_NET_MODE_GPRS,
	AUD_ID_NET_MODE_WIFI,
	AUD_ID_NET_MODE_WIFI_AIRKISS,
	AUD_ID_NET_MODE_WIFI_AP,
	AUD_ID_MQTT_CONNECT_SUCESS,
	AUD_ID_MQTT_CONNECT_FAIL,
	AUD_ID_HOW_GPRS_TO_WIFI,
	AUD_ID_HOW_WIFI_TO_GPRS,
	AUD_ID_HOW_WIFI_TO_AP,
	AUD_ID_HOW_WIFI_TO_AIRKISS,
	AUD_ID_EXIT_WIFI_SET,
	AUD_ID_PWR_LOWBAT,
	AUD_ID_PWR_OFF,
	AUD_ID_PWR_LOWBATOFF,
	AUD_ID_PWR_PER,
	AUD_ID_VOL_MAX,
	AUD_ID_VOL_MIN,
	AUD_ID_EXIT_RECODE,
	AUD_ID_RECODE_NOT_FOUND,
	AUD_ID_RECODE_FIRST,
	AUD_ID_RECODE_LAST,
	AUD_ID_RECOVER_FACTORY,
	AUD_ID_PARAM_EMPTY,
	AUD_ID_ACTIVE_FAIL,
	AUD_ID_ACTIVE_SUCESS,
	AUD_ID_GET_SIGNAL_FAIL,
	AUD_ID_CHARGE_IN,
	AUD_ID_CHARGE_OUT,
	AUD_ID_CHARGE_FULL,
	AUD_ID_UPDATE_START,
	AUD_ID_UPDATE_SUCESS,
	AUD_ID_UPDATE_FAIL,
	AUD_ID_AND,

	AUD_ID_GET_GPS_SUCESS,
#if BT_DEMO
	AUD_ID_ON_MUSIC,
	AUD_ID_BT_CON,
	AUD_ID_BT_DISCON,
	AUD_ID_BT_OPEN,
	AUD_ID_BT_CLOSE,
	AUD_ID_BT_OTA,
#endif

	AUD_ID_LAOHUA_TEST,
	AUD_ID_LAOHUA_TEST_tuichu,
	AUD_ID_LAOHUA_TEST_wancheng,
	AUD_ID_MAX
}AudioPlayIdxDef;


/*************The comment at the beginning of STR is a Chinese interpretation and has no practical effect****************/
#define STR_ID_FAC_TEST_MODE							"??????"
#define STR_ID_FAC_TEST_COMPLETE					"???????"
#define STR_ID_FAC_TEST_AUDIO							"????????"
//#define STR_ID_FAC_TEST_TTS								"TTS????????"
#define STR_ID_FAC_TEST_PWROFF						"??????"
#define STR_ID_FAC_TEST_GET_VESION_FAIL		"????��???"
#define STR_ID_FAC_TEST_SIM_NOT_FOUND			"?????SIM??"
#define STR_ID_FAC_TEST_GET_LEV_FAIL			"????????????"
#define STR_ID_FAC_TEST_TTS_ACTIVEOK			"TTS???????"

#define STR_ID_CUR_SIGNAL					"???????"

#define STR_ID_SALE_CANCLE		"??????????"
#define STR_ID_NUM_0					"zero"
#define STR_ID_NUM_1					"one"	
#define STR_ID_NUM_2					"two"	
#define STR_ID_NUM_3					"three"
#define STR_ID_NUM_4					"four"
#define STR_ID_NUM_5					"five"
#define STR_ID_NUM_6					"six"
#define STR_ID_NUM_7					"seven"
#define STR_ID_NUM_8					"eight"
#define STR_ID_NUM_9					"nine"
#define STR_ID_NUM_10					"ten"
#define STR_ID_NUM_11					"eleven"
#define STR_ID_NUM_12					"twelve"
#define STR_ID_NUM_13					"thirteen"
#define STR_ID_NUM_14					"fourteen"
#define STR_ID_NUM_15					"fifteen"
#define STR_ID_NUM_16					"sixteen"
#define STR_ID_NUM_17					"seventeen"
#define STR_ID_NUM_18					"eighteen"
#define STR_ID_NUM_19					"nineteen"
#define STR_ID_NUM_20					"twenty"
#define STR_ID_NUM_30					"thirty"
#define STR_ID_NUM_40					"forty"
#define STR_ID_NUM_50					"fifty"
#define STR_ID_NUM_60					"sixty"
#define STR_ID_NUM_70					"seventy"
#define STR_ID_NUM_80					"eighty"
#define STR_ID_NUM_90					"ninety"
#define STR_ID_HUNDRED						"hundred"
#define STR_ID_THOUSAND						"thousand"
#define STR_ID_MILION					"milion"
//#define STR_ID_WAN						"??"
#define STR_ID_DOT						"dot"
#define STR_ID_UNIT						"?"

#define STR_PAY_MSG_SK			"????"

#define STR_ID_RECORD_DI			"??"
#define STR_ID_RECORD_BI			"??"

#define STR_ID_KEY_BEEP					"??"
#define STR_ID_KEY_ERR_BEEP			"?"
#define STR_ID_OR								"??"
#define STR_ID_LOG_MODE_USB			"USB??????????????"
#define STR_ID_WELCOME					"????????????"
#define STR_ID_WISH_BUSINESS		"??????????"
#define STR_ID_PLEASE_WAITING		"?????"

#define STR_ID_SIM_INSERT				"SIM???????"
#define STR_ID_SIM_REMOVED			"SIM???????"
#define STR_ID_SIM_NOT_FOUND		"��???SIM??"
#define STR_ID_SIM_INSERT_REQ		"?????SIM??"

#define STR_ID_NET_FAULT							"??????????????????"
#define STR_ID_NET_CONNECTING					"????????????"
#define STR_ID_NET_CONNECT_FAIL				"????????????????????"
#define STR_ID_NET_CONNECT_SUCESS			"??????????"
#define STR_ID_NET_NOT_CONNECT				"??????"/*????��????*/
#define STR_ID_NET_CONNECT						"????????"/*??????????*/
#define STR_ID_NET_MODE_GPRS					"??????GPRS??????"
#define STR_ID_NET_MODE_WIFI					"??????WIFI??????"
#define STR_ID_NET_MODE_WIFI_AIRKISS	"??????WIFI??????????"
#define STR_ID_NET_MODE_WIFI_AP				"??????WIFI AP??????"
#define STR_ID_MQTT_CONNECT_SUCESS		"??????????"
#define STR_ID_MQTT_CONNECT_FAIL			"???????????"

#define STR_ID_HOW_GPRS_TO_WIFI				"??????????��???WIFI??????"
#define STR_ID_HOW_WIFI_TO_GPRS				"??????????��???GPRS??????"
#define STR_ID_HOW_WIFI_TO_AP					"?????????��???WIFI AP??????"
#define STR_ID_HOW_WIFI_TO_AIRKISS		"?????????��???WIFI ??????????"
#define STR_ID_EXIT_WIFI_SET					"?????????"

#define STR_ID_PWR_LOWBAT				"???????????"
#define STR_ID_PWR_OFF					"??????"
#define STR_ID_PWR_LOWBATOFF				"????????????"
#define STR_ID_PWR_PER					"?????????????"

#define STR_ID_CHARGE_IN				"?????"
#define STR_ID_CHARGE_OUT				"????????"
#define STR_ID_CHARGE_FULL			"????????"

#define STR_ID_UPDATE_START			"????????????"
#define STR_ID_UPDATE_SUCESS		"???????"
#define STR_ID_UPDATE_FAIL			"???????"

#define STR_ID_ACTIVE_FAIL			"???��???????????????��???��"	
#define STR_ID_ACTIVE_SUCESS		"???��????"	

#define STR_ID_VOL_MAX					"???????"	
#define STR_ID_VOL_MIN					"??????��"	

#define STR_ID_EXIT_RECODE			"????????"	
#define STR_ID_RECODE_NOT_FOUND	"?????????"	
#define STR_ID_RECODE_FIRST			"??????"
#define STR_ID_RECODE_LAST			"??????"
#define STR_ID_RECOVER_FACTORY	"??????????????"
#define STR_ID_PARAM_EMPTY			"??��???��????"

#define STR_ID_GET_SIGNAL_FAIL	"?????????????"
//#define AUD_ID_UPDATE_NOT_NEED	"?????????"

#define STR_ID_GET_GPS_SUCESS	"?????��???"

#define STR_ID_LAOHUA_CESHI   "老化测试"
#define STR_ID_LAOHUA_CESHI_tuichu  "退出老化测试"
#define STR_ID_LAOHUA_CESHI_wancheng  "完成老化测试"

#define STR_ID_AND			"and"
#define STR_ID_RECEIVE		"receive"
#define STR_ID_BLING		"bling"
#define STR_ID_SERATUS		"SERATUS"
#define STR_ID_SERIBU		"SERIBU"
#if BT_DEMO
#define STR_ID_BT_CON "??????????"
#define STR_ID_BT_DISCON "????????"
#define STR_ID_BT_OPEN "?????????"
#define STR_ID_BT_CLOSE "????????"
#define STR_ID_BT_OTA "??????????"
#endif
/*************The comment at the beginning of STR is a Chinese interpretation and has no practical effect****************/

#endif


#ifndef _PRJ_COMMON_H_
#define _PRJ_COMMON_H_

#if 0
typedef enum {

    SPEAKER_PWREN_INDEX = 0,
    WIFI_MODULE_INDEX,
    WIFI_WAKEUP_INDEX,
//    CHARGER_INS_INDEX,
    CHARGER_STAT_INDEX,
    FUNCTION_BUTTON_INDEX,
    VOLUME_BUTTON_INDEX,
    RED_LED_INDEX,
    GREEN_LED_INDEX,
    BLUE_LED_INDEX,
    BATTERY_CONTROL_INDEX,
    CHECK_LEVEL_INDEX,
    
    VOLUME_BUTTON_INDEX_MAX,
}ePerIndex;
#endif

#define OS_OK 0

#define COMMON_OK        0
#define COMMON_ERROR     -1

////?��?ql_audio.h/AUDIOHAL_SPK_VOL_QTY ?????????11
#define SPEADKERS_VOLUME_SET_MAX           6	//???4??????,0??????????????????1
//#define CLOUD_SPEADKERS_VOLUME_MAX           4	//????10???????????????????????
#define CLOUD_SPEADKERS_VOLUME_MIN           1
//#define CLOUD_SPEADKERS_VOLUME_STEP_VALUE    1

#define PROFILE_IDX 1

#define PRJCONF_SYSPARAM_SAVE_TO_FLASH       1
#define PRJCONF_SYSPARAM_BACKUP              1

#define AUDIO_RESOURCE_USE_FILE_SYSTME     1

#define ONLY_PLAY_TTS_SUPPORT              0  

/* audio resource start address */
#define PRJCONF_DOWNLOAD_ADDR            (0)

#define IMAGE_INVALID_ADDR	(0xFFFFFFFF)

#define ALIYUN_THREE_PARAMETER_MQTT_SUPPORT

/* this function can play welcome audio faster than welcome audio in flash */
#define WELCOME_AUDIO_BUILD_IN_APP_SUPPORT

#define DEBUG_LEVEL      0

#define BT  1   //		1,add  BT ;			0,remove BT;
#if BT
#include "drv_bt_jl_6963.h"
#define BT_DEMO 0
#define BT_POW_OFF(...)  do{ql_gpio_init( GPIO_PIN_NO_25, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, PIN_LEVEL_LOW );}while(0)
#else
#define BT_POW_OFF(...)  do{ql_gpio_init( GPIO_PIN_NO_25, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, PIN_LEVEL_LOW );}while(0)
#define BT_DEMO 0
#endif

#define CFG_ENV_MODE    1           //1   prod mode   0  dev mode 

#define CFG_LARKTMS_SUPPORT 1
#define CFG_LARKTMS_APP_VERSION     "V1.0.0"

#endif


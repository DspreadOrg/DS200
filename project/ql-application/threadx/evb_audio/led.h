#ifndef __LED_H__
#define __LED_H__


//#define CHARG_SOFT_SET_REDLED

#define PRJCONF_LED_BLINK_PERIOD 500
#define PRJCONF_LED_FAST_BLINK_PERIOD 300
#define PRJCONF_LED_FLOW_BLINK_PERIOD 1000

typedef enum
{
	BLACK_COLOR=0,     // 黑
	RED_COLOR,       // 红
	GREEN_COLOR,     // 绿
	BLUE_COLOR,       // 蓝
	NET_COLOR,       // NET_LED
	WHITE_COLOR,
	NORMAL_COLOR,
	RGB_BLINK,        //红绿蓝交替闪烁
	RB_BLINK,		  //红蓝交替闪烁
	RGB_COLOR,
}led_color;

typedef enum
{
    TERM_NORMAL,//0
    TERM_ABNORMAL,
    TERM_ABNORMAL_EXT,
	TERM_INIT_START,
	TERM_INIT_END,
	TERM_POWEROFF,
	TERM_OTA_START,
	TERM_OTA_FAIL,
	TERM_OTA_OK,
//	TERM_WAIT,
//	TERM_BUSY,
	TERM_NET_START,
	TERM_NET_CON,//10
	TERM_NET_DIS,
	TERM_NET_FAILED,
	TERM_NET_APMODE,
	TERM_NET_ABNORMAL,
	TERM_LTE_NET_ABNORMAL,
	TERM_FAULT,
	TERM_REG_FAIL,
	TERM_PARAM_ERR,
	TERM_CHARGE_START,
	TERM_CHARGE_END,//20
	TERM_CHARGE_FULL,
	TERM_LOWBAT,
	TERM_LOWBAT2,
	TERM_NO_SIM,
	TERM_NET_SERVER_CON,
	TERM_NET_SERVER_FAILED,
}LedModeDef;


#ifdef CHARG_SOFT_SET_REDLED
extern void ChargeStatLedSet(int OnOff);
#endif
//extern void gpio_led_pin_init(void);
//extern void gpio_led_pin_deinit(void);
//extern void setRedLedOnOff(int on);
//extern void setBlueLedOnOff(int on);
//extern void setGreenLedOnOff(int on);
extern void color_led_func(led_color led);
extern void platform_led_timer_start(led_color led, uint32_t periodic, led_color led_type);
extern void platform_led_timer_stop(void);
extern void gpio_led_task_init(void);
extern void TermLedShow(LedModeDef mode);
extern void set_net_led_state(int state);
#define GREEN_LED_SET 	(1<<0)
#define BLUE_LED_SET 		(1<<1)
#define AMBER_LED_SET 		(1<<1)
#define RED_LED_SET 		(1<<2)

typedef enum
{
	FlashFast,
	FlashSlow,
	AlwaysOn,
	AlwaysOff,
	Blink,
	Breath,
}LedStatSet;

typedef enum  {
	GPIO_TYPE = 0,
	PWM_TYPE = 1,
}LedTypeSet;


typedef struct {
	unsigned char LedSet;
	unsigned int SetTimeMs;
} LedStatSeq;

//void led_onoff_test(void);

//extern void LedModeSet(int LedSel,LedStatSet Mode);
//extern void led_init(void);
//extern void setBreathLedMode(LedTypeSet type);
//extern void setBreathLedTime(int time);//单位ms

//extern void setBreathLedMode(LedTypeSet type);
//extern void setBreathLedTime(int time);//单位ms 
//extern void Breath_led_Deinit(void);

#endif

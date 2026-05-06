#include <stdio.h>
#include "ql_gpio.h"
#include "ql_rtos.h"
#include "ql_application.h"

#include "prj_common.h"
#include "public_api_interface.h"
#include "led.h"
#include "gpio.h"

#define LOG_DBG(fmt, arg...) //printf("[LED DBG]"fmt, ##arg)
#define LOG_INFO(fmt, arg...) printf("[LED INFO]"fmt, ##arg)

struct LedMode{
	 char Step;
	 LedStatSeq LedStat[5];
};
struct LedMode LedStatSetNext;

char LedReadyFlag=0;
static void Led_task(u32 argv);
static void NET_Led_task(u32 argv);

static void LedModeSet(int LedSel,LedStatSet Mode);

ql_sem_t LedModeChang = NULL;
ql_task_t Led_task_ctrl_thread = NULL;
ql_task_t NET_Led_task_ctrl_thread = NULL;


#define LED_TASK_CTRL_THREAD_STACK_SIZE	(1024)


void ChargeStatLedSet(int OnOff)
{
#ifdef CHARG_SOFT_SET_REDLED
	LOG_INFO("========== led onoff %d\r\n",OnOff);
	setRedLedOnOff((OnOff>0)?1:0);
#endif
}

void gpio_led_task_init(void)
{
	QlOSStatus ret = 0;
    dev_config_t *pdevconf = get_device_config();

#ifdef CHARG_SOFT_SET_REDLED
//	set_charge_status_cb(ChargeStatLedSet);
#endif	
	ret = ql_rtos_semaphore_create(&LedModeChang, 0);
	if(ret)
	{
		LOG_INFO("========== led semaphore create error\r\n");
		return ;
	}
//	LedModeSet(2,LedStartMode)
	if((pdevconf->id==ID_DS200)||(pdevconf->id==ID_DS200BT))
	LedModeSet(GREEN_LED_SET,AlwaysOn);
	else
	LedModeSet(GREEN_LED_SET|BLUE_LED_SET|RED_LED_SET,AlwaysOn);
//	LedModeSet(RED_LED_SET,AlwaysOn);	
	ret = ql_rtos_task_create(&Led_task_ctrl_thread, LED_TASK_CTRL_THREAD_STACK_SIZE, 100, "Led_task", Led_task, NULL);
	if(ret)
	{
		LOG_INFO("========== led thread create error\r\n");
		return ;
	}
	ret = ql_rtos_task_create(&NET_Led_task_ctrl_thread, LED_TASK_CTRL_THREAD_STACK_SIZE, 100, "NET_Led_task", NET_Led_task, NULL);
	if(ret)
	{
		LOG_INFO("========== NET led thread create error\r\n");
		return ;
	}

}

void gpio_led_task_deinit(void)
{
    if (Led_task_ctrl_thread != NULL)
    {
        Led_task_ctrl_thread = NULL;
        ql_rtos_task_delete(NULL);
    }
}


const int SlowTime[2]={600,800};
const int FastTime[2]={100,100};
const int BlinkTime=300;

//static void gpio_led_onoff(uint8_t port, uint8_t pin, uint8_t status)
//{
//	if(status)
//		HAL_GPIO_WritePin(port, pin, GPIO_PIN_HIGH);
//	else
//		HAL_GPIO_WritePin(port, pin, GPIO_PIN_LOW);
//}

static void Led_task(u32 argv)
{
	QlOSStatus ret = 0;
	unsigned int time;
	int step=0;
	struct LedMode LedModeCur;//=LedStatSetNext;
	LedStatSeq * ledOut;
	
	LedReadyFlag=1;
	while(1)
	{
		LedModeCur=LedStatSetNext;
		while(1)
		{
			if (step>=LedModeCur.Step) step=0;
			ledOut=&LedModeCur.LedStat[step];
            
#ifndef CHARG_SOFT_SET_REDLED
            setRedLedOnOff((ledOut->LedSet&RED_LED_SET)?1:0);
#endif
            setBlueLedOnOff((ledOut->LedSet&BLUE_LED_SET)?1:0);
            setGreenLedOnOff((ledOut->LedSet&GREEN_LED_SET)?1:0);
            
            if (ledOut->SetTimeMs==-1)
				time = 0xFFFFFFFF;
			else
				time = ledOut->SetTimeMs;

			ret = ql_rtos_semaphore_wait(LedModeChang, time);
			if (ret == 0) 
			{
				break;
			}
			step++;
		}
	}
}
#define NET_LED_Bink_time 350
int led_net_state=0;
static void NET_Led_task(u32 argv)
{
	static char onoff=0;
	setNetLedOnOff(0);

	while(1)
	{
		if(onoff)
		{
			setNetLedOnOff(1);
			onoff=0;
		}
		else if(!led_net_state)
		{
			setNetLedOnOff(0);
			onoff=1;
		}
		ql_rtos_task_sleep_ms(NET_LED_Bink_time);

	}
}
void set_net_led_state(int state)
{
	led_net_state=state;
}
static void LedModeSet(int LedSel,LedStatSet Mode)
{
	LedStatSeq * set;
	int ii;

	set=LedStatSetNext.LedStat;

    if (Mode == Breath)
    {
        Mode = FlashSlow;
    }

	setBreathLedMode(GPIO_TYPE);
	if(Mode==Breath)
	{
		set->LedSet=0;
		set->SetTimeMs=-1;
		LedStatSetNext.Step=1;
		setBreathLedMode(PWM_TYPE);
	}
	
	switch(Mode)
	{
		case Blink:
			for(ii=0;ii<8;ii++)
			{
				if (LedSel&(1<<ii))
				{
					set->LedSet=1<<ii;
					set->SetTimeMs=BlinkTime;
					set++;
				}
			}
			for(ii=0;ii<8;ii++)
				if (&LedStatSetNext.LedStat[ii]==set) break;
			if (ii==1)
			{
				set->LedSet=0;
				set->SetTimeMs=BlinkTime;
				ii++;
			}
			LedStatSetNext.Step=ii;
			break;
		case FlashFast:
			set->LedSet=LedSel;
			set->SetTimeMs=FastTime[0];
			set++;
			set->LedSet=0;
			set->SetTimeMs=FastTime[1];
			LedStatSetNext.Step=2;				
			break;
		case FlashSlow:
			set->LedSet=LedSel;
			set->SetTimeMs=SlowTime[0];
			set++;
			set->LedSet=0;
			set->SetTimeMs=SlowTime[1];
			LedStatSetNext.Step=2;
			break;
		case AlwaysOn:
			set->LedSet=LedSel;
			set->SetTimeMs=-1;
			LedStatSetNext.Step=1;
			break;
		case AlwaysOff:
			set->LedSet=0;
			set->SetTimeMs=-1;
			LedStatSetNext.Step=1;
			break;
		case Breath:
			setBreathLedTime(-1);//单位ms 
			break;
	}

	ql_rtos_semaphore_release(LedModeChang);
}

/******************************************************************************
  Function: TermLedShow
  Description: Terminal les show status
  INPUT: mode
  OUTPUT: 
  Return: 
 ******************************************************************************/
void TermLedShow(LedModeDef mode)
{
    int red_led_support = 0;
    dev_config_t *pdevconf = get_device_config();
#ifndef CHARG_SOFT_SET_REDLED
    if( pdevconf->pins->led_red.gpio_num < (GPIO_PIN_NO_MAX+1) )
    {
        red_led_support = 1;
    }
#endif
    usb_log_printf("%s: set mode %d red_led_support = %d\n",__func__,mode,red_led_support);
	if((pdevconf->id==ID_DS200)||(pdevconf->id==ID_DS200BT))
	{
		switch(mode)
		{
			case TERM_CHARGE_START:
	                LedModeSet( RED_LED_SET, AlwaysOn );
	            break;

			case TERM_CHARGE_FULL:
	            	LedModeSet(GREEN_LED_SET, AlwaysOn);
	            break;
			case TERM_LOWBAT:
	                LedModeSet( AMBER_LED_SET, AlwaysOn );
	            break;
			case TERM_LOWBAT2:
	                LedModeSet( AMBER_LED_SET, FlashFast );
	            break;

			case TERM_POWEROFF: 		LedModeSet(AMBER_LED_SET|RED_LED_SET|GREEN_LED_SET, AlwaysOff);	break;
			default:						            	//LedModeSet(GREEN_LED_SET, AlwaysOn); 
			break;
		}

	}
	else
	{
		switch(mode)
		{
	        case TERM_ABNORMAL:
				LedModeSet( GREEN_LED_SET, FlashSlow );
				break;
	        case TERM_NET_START:
	        case TERM_NO_SIM:
	            if ( red_led_support )
	            {
	                LedModeSet( RED_LED_SET, FlashFast );
	            }
	            else
	            {
	                LedModeSet( GREEN_LED_SET, FlashFast );
	            }
	            break;

	        case TERM_ABNORMAL_EXT:
	            if ( red_led_support )
	            {
	                LedModeSet( RED_LED_SET | GREEN_LED_SET | BLUE_LED_SET, Blink );
	            }
	            else
	            {
	                LedModeSet( GREEN_LED_SET | BLUE_LED_SET, Blink );
	            }
	            break;
	        
			case TERM_INIT_START:		LedModeSet(GREEN_LED_SET,AlwaysOn);			               		break;
			case TERM_INIT_END:			LedModeSet(BLUE_LED_SET|RED_LED_SET|GREEN_LED_SET,Blink);   	break;
			case TERM_OTA_START:	    LedModeSet(RED_LED_SET|GREEN_LED_SET|BLUE_LED_SET,Blink);     	break;
	        
			case TERM_OTA_FAIL:
			    if ( red_led_support )
	            {
	                LedModeSet( RED_LED_SET, Blink );
	            }
	            else
	            {
	                LedModeSet( GREEN_LED_SET, Blink );
	            }
	            break;
	            
			case TERM_OTA_OK:		    LedModeSet(GREEN_LED_SET,AlwaysOn);	               				break;

			case TERM_NET_DIS:
			    if ( red_led_support )
	            {
	                LedModeSet( BLUE_LED_SET | GREEN_LED_SET, Blink );
	            }
	            else
	            {
	                LedModeSet( BLUE_LED_SET | GREEN_LED_SET, Blink );
	            }
	            break;
	        
			case TERM_NET_CON:		    LedModeSet(BLUE_LED_SET,FlashSlow);	               				break;
			case TERM_NET_SERVER_CON:	LedModeSet(BLUE_LED_SET,AlwaysOn);	               				break;
			case TERM_NET_SERVER_FAILED:LedModeSet(BLUE_LED_SET,FlashSlow);				break;
	        case TERM_NET_ABNORMAL:		LedModeSet(BLUE_LED_SET,FlashFast);	               				break;
	        case TERM_LTE_NET_ABNORMAL:	LedModeSet(BLUE_LED_SET, Breath);	               				break;

	        case TERM_PARAM_ERR:
			case TERM_REG_FAIL:
			    if ( red_led_support )
	            {
	                LedModeSet( GREEN_LED_SET, FlashSlow );
	            }
	            else
	            {
	                LedModeSet( GREEN_LED_SET | BLUE_LED_SET, Blink );
	            }
	            break;
	            
			case TERM_CHARGE_START: //充电状态的led显示由硬件控制红灯，红灯亮充电，灭充满或未充电 软件不能控制
			    // if ( red_led_support )
	            // {
	            //     LedModeSet( RED_LED_SET, AlwaysOn );
	            // }
	            // else
	            // {
	            //     LedModeSet( RED_LED_SET | GREEN_LED_SET | BLUE_LED_SET, Blink );
	            // }
	            break;

			case TERM_CHARGE_FULL:
// #ifndef CHARG_SOFT_SET_REDLED
// 	            LedModeSet(GREEN_LED_SET, AlwaysOn);
// #else
// 	//            setRedLedOnOff(0);
// //            setRedLedOnOff(0);
// #endif        
	            break;
			case TERM_LOWBAT:
			case TERM_LOWBAT2:
			    if ( red_led_support )
	            {
	                LedModeSet( GREEN_LED_SET | RED_LED_SET, Blink );
	            }
	            else
	            {
	                LedModeSet( GREEN_LED_SET | BLUE_LED_SET, Blink );
	            }
	            break;
	            
			case TERM_POWEROFF:		    LedModeSet(BLUE_LED_SET|RED_LED_SET|GREEN_LED_SET, AlwaysOff);	break;
			default:			        break;
		}
	}
}

//application_init(quec_led_test, "quec_led_test", 2, 0);


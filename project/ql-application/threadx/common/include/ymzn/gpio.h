#ifndef __GPIO_H__
#define __GPIO_H__

enum {
    CS10=0,
    CS20,
    CS21,
    CS30,
    ID_CS50M,
    ID_CS30M,
    ID_CS30ML,
    ID_CS30MD,
    ID_CS21C,
    ID_CS50ML,
    ID_CS50MK,
    ID_CS56M,
	ID_CS50P,
	ID_DS10,
	ID_DS200,
	ID_DS200BT,
    DEVTYPE_NOT_SUPPORT,
};

//#define GPIO_BUTTON_CMD_LONG_PRESS      0
//#define GPIO_BUTTON_CMD_SHORT_PRESS     1
//#define GPIO_BUTTON_CMD_REPEAT          2
//#define GPIO_BUTTON_CMD_RELEASE         3
//#define GPIO_BUTTON_CMD_DOUBLE_CLICK    4
//#define GPIO_BUTTON_CMD_NULL            5
//#define GPIO_BUTTON_CMD_MAX             6  // do not edit

typedef enum {
    GPIO_BUTTON_CMD_LONG_PRESS=0,
    GPIO_BUTTON_CMD_SHORT_PRESS,
    GPIO_BUTTON_CMD_DOUBLE_CLICK,
    GPIO_BUTTON_CMD_REPEAT,
    GPIO_BUTTON_CMD_RELEASE,
    GPIO_BUTTON_CMD_MAX,
}GPIO_BUTTON_STAT;

#define GPIO_BUTTON_CMD_NULL     GPIO_BUTTON_CMD_MAX

typedef enum {
    GPIO_BUTTON_0,
    GPIO_BUTTON_1,
    GPIO_BUTTON_2,
    GPIO_BUTTON_3,
    GPIO_BUTTON_4,
    GPIO_BUTTON_5,
    GPIO_BUTTON_6,
    GPIO_BUTTON_7,
    GPIO_BUTTON_NUM,
}GPIO_BUTTON_ID;

typedef struct
{
    GPIO_BUTTON_STAT cmd;
    GPIO_BUTTON_ID id;
}GPIO_Button_Cmd_Info;

typedef struct
{
    uint32_t repeat_Press_Hold_Time_Ms;
    uint32_t repeat_Period_Ms;
}Gpio_Button_Repeat;


typedef struct
{
    GPIO_BUTTON_ID button_Id;
    uint32_t button_Value;
    uint32_t short_Press_Hold_Time_Ms;
    uint32_t long_Press_Hold_Time_Ms;
    uint32_t double_click_Time_Ms;
    Gpio_Button_Repeat *repeat_Mode;
//    uint8_t button_cmd_list[GPIO_BUTTON_CMD_MAX];
}Gpio_Button_Info;

typedef struct
{
	GPIO_BUTTON_ID Id;
	uint8_t button_cmd_val[GPIO_BUTTON_CMD_MAX];
}Gpio_Button_Cfg;

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
//    CHECK_LEVEL_INDEX,
    ADD_VOLUME_BUTTON_INDEX,

    VOLUME_BUTTON_INDEX_MAX,
}ePerIndex;

typedef struct {
    unsigned short gpio_num;
    unsigned char pin_pull;
    unsigned char pin_level;
}gpio_pin_t;

typedef struct
{
    uint8_t currentset;
    gpio_pin_t speaker;
    gpio_pin_t wifi_power;
    gpio_pin_t wifi_wakeup;
//    gpio_pin_t charger_ins;
    gpio_pin_t charger_stat;
    gpio_pin_t function;
    gpio_pin_t volume_down;
    gpio_pin_t volume_up;
    gpio_pin_t led_red;
    gpio_pin_t led_green;
    gpio_pin_t led_blue;
    gpio_pin_t battery_ctrl;
//    gpio_pin_t wifi_pwr_level;
    gpio_pin_t pwr_on;
    gpio_pin_t led_net;
}dev_pin_config_t;

typedef struct
{
    uint32_t has_lcd_fg7864 :1;
    uint32_t has_led_fg00ahk :1;
    uint32_t has_led_tm1604 :2;  // 1-CS30M接线方式，2-旧版CS30N接线方式
    uint32_t has_lcd_f240320 :1;
}dev_opt_def;

typedef union
{
    dev_opt_def bits;
    uint32_t value;
} dev_opt_t;

typedef struct{
    uint8_t id;
    const char *devname;
    const char *module;
    const char *board;
    const dev_pin_config_t *pins;
    const Gpio_Button_Info *btconf;
    const dev_opt_t *opt;
}dev_config_t;

//extern int ym_switch_peripheral_function(ePerIndex num, uint8_t OnFlag);
extern int ym_get_peripheral_function_state(ePerIndex num);
extern int ym_get_peripheral_function_gpio(ePerIndex num);
extern int ym_get_config_current_device_type(void);
extern const char *ym_get_current_device_name(void);

extern int getAddVolumeButtonGpio(void);
extern void gpio_speaker_onoff(uint8_t status);

extern void gpio_wifi_power_onoff(uint8_t OnFlag);
extern void gpio_wifi_wakeup_Set(uint8_t OnFlag);

extern void gpio_battery_control_pin_onoff(uint8_t OnFlag);

extern void setRedLedOnOff(uint8_t OnFlag);
extern void setBlueLedOnOff(uint8_t OnFlag);
extern void setGreenLedOnOff(uint8_t OnFlag);
extern void setNetLedOnOff(uint8_t OnFlag);

extern int getFunctionButtonGpio(void);
extern int getVolumeButtonGpio(void);

extern int get_device_type(void);
const char *get_device_name(void);
const dev_config_t *get_device_config(void);

//extern int get_charger_ins_gpio(void);
extern gpio_pin_t get_charger_stat_gpio(void);
extern int get_charger_control_gpio(void);

void load_device_config(const char *devname);

#endif

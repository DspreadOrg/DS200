#ifndef __BT_JL2209_H__
#define __BT_JL2209_H__
#include "ext_wifi.h"
//蓝牙使能开关



typedef enum
{
    TYPE_APP   = 0XFA,
    TYPE_BT_IC = 0XFB,
    
}BT_TYPE;
typedef enum
{
	TYPE_MSG_NULL = 0 ,
    TYPE_MSG_READ ,
    TYPE_MSG_SEND ,
    TYPE_MSG_POWER,
    
}bt_msg;

struct BTsend_buff_msg {
    
    char * buf;
    int len;
    bt_msg msg;
    BT_TYPE   type;

};

struct BT_Param_map {
char * BT_name;
int BT_name_len;
char * Set_BT_name;
int Set_BT_name_len;
char * BTVer;
int BTVer_len;
char * BTMac;
int BTMac_len;
}BT_Param;

struct  BT_CON_info{
char address[6];
char name[64];
};


typedef void(*BT_connect_state_cb_t)(int);
/**
 * @brief  set Bluetooth connection callback
 * @param  cb - callback(result),connect(result==1),disconnect(result==0)
 */
void set_BT_connect_state_cb(BT_connect_state_cb_t cb);


typedef void(*switch_Bluetooth_state_cb_t)(int);

/**
 * @brief  set switch_Bluetooth callback
 * @param  cb - callback(result),open(result==1),close(result==0)
 */
void set_switch_Bluetooth_state_cb(switch_Bluetooth_state_cb_t cb);



typedef void(*Bluetooth_OTA_cb_t)(void);
//Bluetooth update callback function
void set_Bluetooth_OTA_cb(Bluetooth_OTA_cb_t cb);


typedef void(*BT_config_wifi_ssid_same_cb_t)(void);

//Bluetooth configuration wifi ssid same call this function
void set_BT_config_wifi_ssid_same_cb(BT_config_wifi_ssid_same_cb_t cb);


typedef void(*BT_pair_request_cb_t)(char* ,char* );
//Bluetooth update callback function
void set_BT_pair_request_cb(BT_pair_request_cb_t cb);

typedef void(*Bluetooth_unpair_cb_t)(void);
//Bluetooth unpair callback function
void set_Bluetooth_unpair_cb(Bluetooth_unpair_cb_t cb);

char checksum(char * data,int  data_len);//校验和
void BT_OFF(void);
void BT_ON(void);
void BT_DISCONNECT(void);
void audio_BT_sem_release(void);
void audio_BT_sem_wait(void);
void set_bt_name(int SN_len);
int BT_power_state_check(void);
void BT_device_refactory(void);
int BT_music_state_check(void);
int BT_mute_state_check(void);
int BT_connect_state_check(void);
int BT_broadcast_mode_check(void);
void BT_param_mapping_func(char * BT_name,int BT_name_len,
									char * Set_BT_name,int Set_BT_name_len,
									char * BTVer ,int BTVer_len,
									char * BTMac ,int BTMac_len);
void BT_pair_Ack(char ack);
void BT_audio_TEST(void);//factory mode use
void BT_ver_get(void);//factory mode use
void BT_self_name_get(void);//factory mode use
void BT_next_song(void);
void BT_last_song(void);
void BT_paly_pause(void);
void BT_vol_XX(char volume_level);
void BT_close_time_to_network_fail(uint time);
void BT_mute(int mode);
void BT_switch(void);
void BT_broadcast_mode_switch(char onoff);
void set_BT_broadcast_mode(char mode);
struct BT_CON_info * BT_con_list_Read(int * num);
int BT_con_list_Delete(char* mac_adder);
void set_bt_pair_time(int time);
void BT_ble_switch(char onoff);
char * Get_phone_BT_mac(void);//CON: ret=macadd           DIS: ret=NULL
void set_bt_faraway_recon_time(int time);


#endif

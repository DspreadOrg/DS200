#ifndef _SYSTEMPARAM__H_
#define _SYSTEMPARAM__H_

//#include "ota/ota.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYSINFO_SSID_LEN_MAX		(32)
#define SYSINFO_PSK_LEN_MAX			(65)


#define SOFTVER "0.0.0"

/**
 * @brief Sysinfo station wlan parameters definition
 */
struct sysinfo_wlan_sta_param {
	uint8_t ssid[SYSINFO_SSID_LEN_MAX];
	uint8_t ssid_len;

	uint8_t psk[SYSINFO_PSK_LEN_MAX];
};


/**
 * @brief SysParam structure definition
 */
/*
|YMW | ��������00/01 | ���̴���00~03 | Ӳ������00~04 | �����ͺ�00~99 | ������ | �յ����汾��0~9 | 
 R/D��� 0:release 1��debug
*/
typedef struct sysparamold {
	int battery; // ���
	int flash;   // flash
	int microphone; // ¼�� ��˷�
	int led; // led��
	int speaker; // ����
	int register_state; // �豸ע��״̬
	int volume;       // ��������	
	char firmware_version[50]; // �̼��汾
	char device_secret[64]; // �豸��Կ
	char product_key[64]; // ��Ʒkey
	//char product_secret[64]; // ��Ʒ��Կ
	char device_name[64]; // �豸��
	char device_SN[64]; // �豸SN
	char server_url[64]; // ��������ַ
	char hwver[32]; //Ӳ���汾
	char server_mqtt[64];  // mqtt url���������°�ʵ������ҵ���������ַ
	int uart_log;       // ����log
	struct sysinfo_wlan_sta_param wlan_sta_param;
	char WifiVer[20]; // wifi�̼��汾
	char Topic[64];
	int backlight_level;  // �������ȣ�0-100
	int device_type; // 0:��4G�豸, 1:WIFI+4G�豸
	int NetChanlLTE;
	int airkiss;
	int button_level;
	int ota;
	int first_boot;
	int example[512];//Example parameter replacement
#if BT
	char BT_name[20];//Read the version from Bluetooth
	char Set_BT_name[20];//To write the name of the need to change the bluetooth
	char BTVer[10];//Bluetooth software version number
	char BTMac[7];//Bluetooth mac
	int pair_time;
	int BT_SN_len;
	int unuse[77];
#else
	int unuse[100];
#endif
#if CFG_LARKTMS_SUPPORT
	int ota_init;
	char larktms_url[64];
	char larktms_port[8];
	char larktms_mqtt_url[64];
	char larktms_mqtt_port[8];
	char larktms_secretkey[64];
	char larktms_productkey[64];
	char larktms_devname[32];
	char larktms_topic[64];
	char larktms_unuse[12];
#else
	char larktms_unuse[384];
#endif
	char Md5[16];
};

typedef struct sysparam {
	int battery; // ���
	int flash;   // flash
	int microphone; // ¼�� ��˷�
	int led; // led��
	int speaker; // ����
	int register_state; // �豸ע��״̬
	int volume;       // ��������	
	char firmware_version[50]; // �̼��汾
	char device_secret[64]; // �豸��Կ
	char product_key[64]; // ��Ʒkey
	//char product_secret[64]; // ��Ʒ��Կ
	char device_name[64]; // �豸��
	char device_SN[64]; // �豸SN
	char server_url[64]; // ��������ַ
	char hwver[32]; //Ӳ���汾
	char server_mqtt[64];  // mqtt url���������°�ʵ������ҵ���������ַ
	int uart_log;       // ����log
	struct sysinfo_wlan_sta_param wlan_sta_param;
	char WifiVer[20]; // wifi�̼��汾
	char Topic[64];
	int backlight_level;  // �������ȣ�0-100
	int device_type; // 0:��4G�豸, 1:WIFI+4G�豸
	int NetChanlLTE;
	int airkiss;
	int button_level;
	int ota;
	int first_boot;
	int example[512];//Example parameter replacement
#if BT
	char BT_name[20];//Read the version from Bluetooth
	char Set_BT_name[20];//To write the name of the need to change the bluetooth
	char BTVer[10];//Bluetooth software version number
	char BTMac[7];//Bluetooth mac
	int BT_SN_len;
	int unuse[81];
#else
	int unuse[100];
#endif

#if CFG_LARKTMS_SUPPORT
	int ota_init;
	char larktms_url[64];
	char larktms_port[8];  // ort server port
	char larktms_mqtt_url[64];
	char larktms_mqtt_port[8];
	char larktms_secretkey[64];
	char larktms_productkey[64];
	char larktms_devname[32];
	char larktms_topic[64];
	char larktms_unuse[12];
#else
	char larktms_unuse[384];
#endif
	char server_mode[3]; //'1'ssl SERVER_AUTH   '2' ssl client and server auth  '3' tcp  
	char server_port[8]; // mqtt server port
	char username[64];
	char password[64];
	char clientId[64];

	int  CertState;
	char RootCA[2048];
	char DevCert[2048];
	char DevKey[2048];
	
	char Md5[16];

	char aging_state[32];
	char aging_time[32];


}sysparam_t;

extern int SYSPARAM_SIZE;//	sizeof(struct sysparam)

extern int sysparam_init(void);
extern void sysparam_deinit(void);
extern int sysparam_default(void);
extern int sysparam_save(void);
extern int sysparam_load(void);

extern struct sysparam *sysparam_get(void);
extern struct sysinfo_wlan_sta_param sysparam_get_wlanparam(void);
extern int sysparam_get_uart_log(void);

extern int sysparam_get_device_type(void);
extern void sysparam_set_device_type(int value);
extern void sysparam_set_wifi_version(const char * WifiVer);

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_CHIP_SYSTEM_CHIP_H_ */



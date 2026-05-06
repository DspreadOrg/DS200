#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "ql_rtos.h"
#include "ql_fs.h"
#include "ql_audio.h"
//#include "ql_timer.h"

#include "prj_common.h"
#include "systemparam.h"
#include "devicePara.h"
#include "tl_md5.h"
#include "public_api_interface.h"
#include "terminfodef.h"


#if 0
#define SYSTEM_PARAM_DEBUG   usb_log_printf
#else
#define SYSTEM_PARAM_DEBUG(...)   do{}while(0);
#endif

#if 0
#define SYSTEM_PARAM_ERROR   usb_log_printf
#else
#define SYSTEM_PARAM_ERROR(...)   do{}while(0);
#endif

//#define FILE_NAME			"systemparam.txt"

//#define U_DISK				"U:"
//#define U_ROOT_PATH			""U_DISK"/"
//#define SYSTEM_PARAM_U_FILE_ROOT 	""U_DISK"/"FILE_NAME""   //save original systemparam

//#define B_DISK				"B:"
//#define B_ROOT_PATH			""B_DISK"/"
//#define SYSTEM_PARAM_B_FILE_ROOT 	""B_DISK"/"FILE_NAME""   //save backup systemparam
#define SYSTEM_PARAM_U_FILE_ROOT   "S:/systemparam.txt"
#define SYSTEM_PARAM_B_FILE_ROOT   "S:/systemparambak.txt"


static struct sysparam g_sysparam={
	.uart_log=1,
};
extern int sysparam_md5(void *buff, size_t buff_len, uint32_t md5[4]);
#define ERR_TAG "[sysinfo ERR]"
extern char firmware_version[50];
int _sysparam_loadold(const char *path,struct sysparamold * g_sysparam);
int _sysparam_create_file(const char *path);
int _sysparam_load(const char *path,struct sysparam * g_sysparam);
int _sysparam_save(const char *path,struct sysparam * g_sysparam);

int SYSPARAM_SIZE=sizeof(struct sysparam);

struct sysparam *xr_sysDeviceParam = NULL;
ql_mutex_t sysparam_file_mutex = NULL;

#define	OFFSET(x)	offsetof(struct sysparam, x)

const ym_setting_item_struct ym_setting_items[] =
{
	{ITEM_READ_WRITE, "Battery", 1,OFFSET(battery),IsIntNumber},			
	{ITEM_READ_WRITE, "Flash", 1,OFFSET(flash),IsIntNumber},
	{ITEM_READ_WRITE, "Microphone", 1,OFFSET(microphone),IsIntNumber}, 
	{ITEM_READ_WRITE, "Led", 1,OFFSET(led),IsIntNumber},
	{ITEM_READ_WRITE, "Speaker", 1,OFFSET(speaker),IsIntNumber},
	{ITEM_READ_WRITE, "Status", 1,OFFSET(register_state),IsIntNumber},
	{ITEM_READ_WRITE, "Volume", 1,OFFSET(volume),IsIntNumber},
	{ITEM_READ_WRITE, "LogStatus", 1,OFFSET(uart_log),IsIntNumber},
	{ITEM_READ_WRITE, "AppVer", sizeof(xr_sysDeviceParam->firmware_version),OFFSET(firmware_version),IsStr},
	{ITEM_READ_WRITE, "WifiVer", sizeof(xr_sysDeviceParam->WifiVer),OFFSET(WifiVer),IsStr},
	{ITEM_READ_WRITE, "SecretKey", sizeof(xr_sysDeviceParam->device_secret),OFFSET(device_secret),IsStr},
	{ITEM_READ_WRITE, "ProductKey", sizeof(xr_sysDeviceParam->product_key),OFFSET(product_key),IsStr},
	{ITEM_READ_WRITE, "DevName", sizeof(xr_sysDeviceParam->device_name),OFFSET(device_name),IsStr},
	{ITEM_READ_WRITE1, "Serial", sizeof(xr_sysDeviceParam->device_SN),OFFSET(device_SN),IsStr},
	{ITEM_READ_WRITE, "Url", sizeof(xr_sysDeviceParam->server_url),OFFSET(server_url),IsStr},
	{ITEM_READ_WRITE, "MqttUrl", sizeof(xr_sysDeviceParam->server_mqtt),OFFSET(server_mqtt),IsStr},
	{ITEM_READ_WRITE, "Topic", sizeof(xr_sysDeviceParam->Topic),OFFSET(Topic),IsStr},
	{ITEM_READ_WRITE, "HWVer", sizeof(xr_sysDeviceParam->hwver),OFFSET(hwver),IsStr},
    {ITEM_READ_WRITE, "WifiExist", 1,OFFSET(device_type),IsIntNumber},
#if BT
		{ITEM_READ, 	  "BtName",  sizeof(xr_sysDeviceParam->BT_name),OFFSET(BT_name),IsStr},  
		{ITEM_READ_WRITE, "SetBtName",	sizeof(xr_sysDeviceParam->Set_BT_name),OFFSET(Set_BT_name),IsStr}, 
		{ITEM_READ, 	  "BtVer",	sizeof(xr_sysDeviceParam->BTVer),OFFSET(BTVer),IsStr}, 
		{ITEM_READ, 	  "BtMac",	sizeof(xr_sysDeviceParam->BTMac),OFFSET(BTMac),IsStr}, 
		{ITEM_READ_WRITE, "BtNameSNlen",1,OFFSET(BT_SN_len),IsIntNumber}, 
#endif

#if CFG_LARKTMS_SUPPORT
		{ITEM_READ_WRITE, "OtaInit",  1,OFFSET(ota_init),IsIntNumber},  
		{ITEM_READ_WRITE, "OtaInitUrl",	sizeof(xr_sysDeviceParam->larktms_url),OFFSET(larktms_url),IsStr}, 
		{ITEM_READ_WRITE, "OtaInitPort",	sizeof(xr_sysDeviceParam->larktms_port),OFFSET(larktms_port),IsStr}, 
		{ITEM_READ_WRITE, "OtaMqttUrl",	sizeof(xr_sysDeviceParam->larktms_mqtt_url),OFFSET(larktms_mqtt_url),IsStr}, 
		{ITEM_READ_WRITE, "OtaMqttPort",	sizeof(xr_sysDeviceParam->larktms_mqtt_port),OFFSET(larktms_mqtt_port),IsStr},
		{ITEM_READ_WRITE, "OtaSecretKey",	sizeof(xr_sysDeviceParam->larktms_secretkey),OFFSET(larktms_secretkey),IsStr}, 
		{ITEM_READ_WRITE, "OtaProductKey",	sizeof(xr_sysDeviceParam->larktms_productkey),OFFSET(larktms_productkey),IsStr}, 
		{ITEM_READ_WRITE, "OtaDevName",	sizeof(xr_sysDeviceParam->larktms_devname),OFFSET(larktms_devname),IsStr}, 
		{ITEM_READ_WRITE, "OtaTopic",	sizeof(xr_sysDeviceParam->larktms_topic),OFFSET(larktms_topic),IsStr}, 
#endif
		{ITEM_READ_WRITE, "MqttMode",	sizeof(xr_sysDeviceParam->server_mode),OFFSET(server_mode),IsStr}, 
		{ITEM_READ_WRITE, "MqttPort",	sizeof(xr_sysDeviceParam->server_port),OFFSET(server_port),IsStr}, 
		{ITEM_READ_WRITE, "username",	sizeof(xr_sysDeviceParam->username),OFFSET(username),IsStr}, 
		{ITEM_READ_WRITE, "password",	sizeof(xr_sysDeviceParam->password),OFFSET(password),IsStr}, 
		{ITEM_READ_WRITE, "clientId",	sizeof(xr_sysDeviceParam->clientId),OFFSET(clientId),IsStr}, 

		{ITEM_READ_WRITE, "CertState",  1,OFFSET(CertState),IsIntNumber},  //工厂订单生产时，该标志设置为 0x01，表明已下载证书
		{ITEM_READ_WRITE, "RootCA",	sizeof(xr_sysDeviceParam->RootCA),OFFSET(RootCA),IsStr}, 
		{ITEM_READ_WRITE, "DevCert",	sizeof(xr_sysDeviceParam->DevCert),OFFSET(DevCert),IsStr}, 
		{ITEM_READ_WRITE, "DevKey",	sizeof(xr_sysDeviceParam->DevKey),OFFSET(DevKey),IsStr}, 

		{ITEM_READ, "AgingState", sizeof(xr_sysDeviceParam->aging_state),OFFSET(aging_state),IsStr},
		{ITEM_READ, "AgingTime", sizeof(xr_sysDeviceParam->aging_time),OFFSET(aging_time),IsStr},

    //Displays the newly added item on the tool
		{ITEM_READ_WRITE, "example", sizeof(xr_sysDeviceParam->example),OFFSET(example),IsStr },
	//****************************************************************
//	{ITEM_READ, "SdkVer", sizeof(xr_sysDeviceParam->SdkVer),OFFSET(SdkVer),IsStr},
//	{ITEM_READ, "ChipID", 16,-1,EFPG_FIELD_CHIPID},
//	{ITEM_READ, "Mac", 6,-1,EFPG_FIELD_MAC},
	{0, NULL, 0,0,0},
};

/**
 * @brief Initialize the sysparam module
 * @return 0 on success, -1 on failure
 */
void FirstRunInit(struct sysparam * g_sysparam)
{
	usb_log_printf("--------%s line %d-------- \n",__func__, __LINE__);
		
	memset(g_sysparam->firmware_version, 0, sizeof(g_sysparam->firmware_version));
	memcpy(g_sysparam->firmware_version, firmware_version, strlen(firmware_version));
//	return 1;
}

int sysparam_init(void)
{
	SYSTEM_PARAM_DEBUG("sysparam_init line %d \n", __LINE__);

    ql_rtos_mutex_create(&sysparam_file_mutex);
#if BT
	//The Bluetooth parameters in the system are mapped into lib
	BT_param_mapping_func(g_sysparam.BT_name,sizeof(g_sysparam.BT_name),   \
						g_sysparam.Set_BT_name,sizeof(g_sysparam.Set_BT_name),  \
						g_sysparam.BTVer,sizeof(g_sysparam.BTVer),
						g_sysparam.BTMac,sizeof(g_sysparam.BTMac));
#endif
#if PRJCONF_SYSPARAM_SAVE_TO_FLASH
#if PRJCONF_SYSPARAM_CHECK_OVERLAP
	uint32_t image_size = image_get_size();//+PRJCONF_SYSINFO_SIZE;
	if (image_size == 0) {
		SYSTEM_PARAM_ERROR(ERR_TAG "get image size failed\n");
		return -1;
	}
	if (image_size > PRJCONF_SYSPARAM_ADDR) {
		SYSTEM_PARAM_ERROR(ERR_TAG "image is too big: %#x, please make it smaller than %#x\n",
		            image_size, PRJCONF_SYSPARAM_ADDR);
		return -1;
	}
#endif
	int ret1=-1;
	ret1 = ql_access(SYSTEM_PARAM_U_FILE_ROOT, 0);
	if (ret1 != LFS_ERR_OK) // file not exist
	{
		_sysparam_create_file(SYSTEM_PARAM_U_FILE_ROOT);
	}
	else
	{
		ret1=_sysparam_load(SYSTEM_PARAM_U_FILE_ROOT,&g_sysparam);
		if (ret1!=0)//sysparam verification failure,enable old version verification
		{
			struct sysparamold * param=malloc(sizeof(struct sysparamold));
			ret1=_sysparam_loadold(SYSTEM_PARAM_U_FILE_ROOT,param);//old version verification
			if (ret1==0)
			{
				sysparamcpy(&g_sysparam,param);//copy
				_sysparam_save(SYSTEM_PARAM_U_FILE_ROOT,&g_sysparam);
			}
			free(param);
		}
	}
	
//#if PRJCONF_SYSPARAM_BACKUP
	struct sysparam *g_sysparambak;//=g_sysparam;
	g_sysparambak=malloc(sizeof(struct sysparam));
	if (g_sysparambak==NULL)
		return -1;
	int ret2=-1;
	ret2 = ql_access(SYSTEM_PARAM_B_FILE_ROOT, 0);
	if (ret2 != LFS_ERR_OK) // file not exist
	{
		_sysparam_create_file(SYSTEM_PARAM_B_FILE_ROOT);
	}
	else
	{
		ret2=_sysparam_load(SYSTEM_PARAM_B_FILE_ROOT,g_sysparambak);
		if (ret2!=0)//backup sysparam verification failure,enable old version verification
		{
			struct sysparamold * param=malloc(sizeof(struct sysparamold));
			ret2=_sysparam_loadold(SYSTEM_PARAM_B_FILE_ROOT,param);//old version verification
			if (ret2==0)
			{
				sysparamcpy(g_sysparambak,param);//copy
				_sysparam_save(SYSTEM_PARAM_B_FILE_ROOT,g_sysparambak);
			}
			free(param);
		}
	}
//#endif
		SYSTEM_PARAM_DEBUG("[----%d---] ret1 %d, ret2 %d\r\n",__LINE__,ret1,ret2);
	if (ret1==0)
	{
		SYSTEM_PARAM_DEBUG("[----%d---] ret1 %d, ret2 %d\r\n",__LINE__,ret1,ret2);
		if (strcmp(g_sysparam.firmware_version, firmware_version))
		{
			FirstRunInit(&g_sysparam);
			_sysparam_save(SYSTEM_PARAM_B_FILE_ROOT,&g_sysparam);
			_sysparam_save(SYSTEM_PARAM_U_FILE_ROOT,&g_sysparam);
			SYSTEM_PARAM_DEBUG("[----%d---] ret1 %d, ret2 %d\r\n",__LINE__,ret1,ret2);
		}
		else if (ret2||(memcmp(&g_sysparam,g_sysparambak,sizeof(g_sysparam)) ))
		{
			_sysparam_save(SYSTEM_PARAM_B_FILE_ROOT,&g_sysparam);
			SYSTEM_PARAM_DEBUG("[----%d---] ret1 %d, ret2 %d\r\n",__LINE__,ret1,ret2);
		}
	}
	else if (ret2==0)
	{
		SYSTEM_PARAM_DEBUG("[----%d---] ret1 %d, ret2 %d\r\n",__LINE__,ret1,ret2);
		if (strcmp(g_sysparambak->firmware_version, firmware_version))
		{
			SYSTEM_PARAM_DEBUG("[----%d---] ret1 %d, ret2 %d\r\n",__LINE__,ret1,ret2);
			g_sysparam=*g_sysparambak;
			FirstRunInit(&g_sysparam);
			_sysparam_save(SYSTEM_PARAM_U_FILE_ROOT,&g_sysparam);
			_sysparam_save(SYSTEM_PARAM_B_FILE_ROOT,&g_sysparam);
		}
		else if (ret1||(memcmp(&g_sysparam,g_sysparambak,sizeof(g_sysparam)) ))
		{
			SYSTEM_PARAM_DEBUG("[----%d---] ret1 %d, ret2 %d\r\n",__LINE__,ret1,ret2);
			g_sysparam=*g_sysparambak;
			_sysparam_save(SYSTEM_PARAM_U_FILE_ROOT,&g_sysparam);
		}
	}
	else if (ret1==-3)
	{
		FirstRunInit(&g_sysparam);
		SYSTEM_PARAM_DEBUG("[----%d---] ret1 %d, ret2 %d\r\n",__LINE__,ret1,ret2);
		_sysparam_save(SYSTEM_PARAM_B_FILE_ROOT,&g_sysparam);
		_sysparam_save(SYSTEM_PARAM_U_FILE_ROOT,&g_sysparam);
	}
	else if (ret2==-3)
	{
		SYSTEM_PARAM_DEBUG("[----%d---] ret1 %d, ret2 %d\r\n",__LINE__,ret1,ret2);
		g_sysparam=*g_sysparambak;
		FirstRunInit(&g_sysparam);
		_sysparam_save(SYSTEM_PARAM_U_FILE_ROOT,&g_sysparam);
		_sysparam_save(SYSTEM_PARAM_B_FILE_ROOT,&g_sysparam);
	}
	else
	{
		SYSTEM_PARAM_DEBUG("[----%d---] ret1 %d, ret2 %d\r\n",__LINE__,ret1,ret2);
		sysparam_default();
	}

	free(g_sysparambak);
#else /* PRJCONF_SYSPARAM_SAVE_TO_FLASH */
	sysparam_default();
#endif
	return 0;
}

/**
 * @brief DeInitialize the sysparam module
 * @return None
 */
void sysparam_deinit(void)
{
#if PRJCONF_SYSPARAM_SAVE_TO_FLASH
	
#endif
}

/**
 * @brief Set default value to sysparam
 * @return 0 on success, -1 on failure
 */
int sysparam_default(void)
{
#if PRJCONF_SYSPARAM_SAVE_TO_FLASH
	int ret=-1;
	ret = ql_access(SYSTEM_PARAM_U_FILE_ROOT, 0);
	if (ret != LFS_ERR_OK) {
		SYSTEM_PARAM_ERROR(ERR_TAG "file system uninitialized ret = %d\n", ret);
		return -1;
	}
#endif
	SYSTEM_PARAM_DEBUG("<<%s>> enter line %d\n", __func__, __LINE__);
	memset(&g_sysparam, 0, SYSPARAM_SIZE);
	g_sysparam.battery = 0x00;
	g_sysparam.flash = 0x01;
	g_sysparam.led = 0x01;
	g_sysparam.microphone = 0x00;
	g_sysparam.speaker = 0x01;
	g_sysparam.register_state = 0x00;
	g_sysparam.volume = SPEADKERS_VOLUME_SET_MAX-1;
	g_sysparam.uart_log = 0x00;
	g_sysparam.button_level = 0x00;
	g_sysparam.airkiss = 0x01;
	g_sysparam.first_boot = 0x01;
	g_sysparam.device_type = -1;

	g_sysparam.NetChanlLTE = 4; // default is 4G
	g_sysparam.backlight_level = 10;
	
	memcpy(g_sysparam.hwver, "1.2", strlen("1.2"));
	memset(g_sysparam.device_SN, 0, sizeof(g_sysparam.device_SN)/sizeof(g_sysparam.device_SN[0]));
	memcpy(g_sysparam.firmware_version, firmware_version, strlen(firmware_version));
    memcpy(g_sysparam.Topic, "user/message", strlen("user/message"));

#if BT
		memset(g_sysparam.Set_BT_name, 0, sizeof(g_sysparam.Set_BT_name)/sizeof(g_sysparam.Set_BT_name[0]));
		memset(g_sysparam.BT_name, 0, sizeof(g_sysparam.BT_name)/sizeof(g_sysparam.BT_name[0]));
		memset(g_sysparam.BTVer, 0, sizeof(g_sysparam.BTVer)/sizeof(g_sysparam.BTVer[0]));
		memset(g_sysparam.BTMac, 0, sizeof(g_sysparam.BTMac)/sizeof(g_sysparam.BTMac[0]));
		g_sysparam.BT_SN_len = 5;//last 5 digits of DSN
		memcpy(g_sysparam.Set_BT_name, "JioPay VB", strlen("JioPay VB"));//bluetooth name
#endif

#if PRJCONF_SYSPARAM_SAVE_TO_FLASH
	sysparam_save();
#endif
	return 0;
}

#if PRJCONF_SYSPARAM_SAVE_TO_FLASH
/**
 * @brief create new null file
 * @return 0 on success, -1 on failure
 */
int _sysparam_create_file(const char *path)
{
	QFILE * fp = NULL;

	if (path == NULL)
	{
		SYSTEM_PARAM_ERROR(ERR_TAG "%s_%d ===path is NULL\n", __func__, __LINE__);
		return -1;
	}
	
	fp = ql_fopen(path, "w+");
	if(fp == NULL)
	{
		SYSTEM_PARAM_ERROR(ERR_TAG "%s_%d === *** file create fail: %s ***\n", __func__, __LINE__, path);
		return -1;
	}

	if(fp)
	{
		ql_fclose(fp);
		SYSTEM_PARAM_DEBUG("%s_%d ===[FS] file closed: %s\r\n", __func__, __LINE__, path);
	}

	return 0;
}

/**
 * @brief Save sysparam to flash
 * @return 0 on success, -1 on failure
 */
int _sysparam_save(const char *path,struct sysparam * g_sysparam)
{
	QFILE * fp = NULL;
	uint32_t md5_main[4] = { 0 };
	char * ptr1;
	char * ptr2;
	int ret=-1;

	if (path == NULL)
	{
		SYSTEM_PARAM_ERROR(ERR_TAG "%s_%d ===path is NULL\n", __func__, __LINE__);
		return -1;
	}
	
	fp = ql_fopen(path, "w+");
	if (fp == NULL) {
		SYSTEM_PARAM_ERROR(ERR_TAG "%s_%d === open %s file failed!\n", __func__, __LINE__, path);
		return -1;
	}

	ptr1=g_sysparam->Md5;
	ptr2=(char * )g_sysparam;
	sysparam_md5(ptr2, ptr1-ptr2, md5_main);
	memcpy(g_sysparam->Md5,md5_main,sizeof(g_sysparam->Md5));
	
	if (ql_fwrite((void *)g_sysparam, SYSPARAM_SIZE, 1, fp) < 0) 
	{
		ret=-1;
	}
	else
	{
        ret = ql_fsync(fp);
        if (ret != LFS_ERR_OK)
        {
            ql_rtos_task_sleep_ms(1000);
            ret = ql_fsync(fp);
        }

        if (ret != LFS_ERR_OK)
        {
            ret = -1;
        }
        else
        {
		    ret=0;	
        }
	}

	if(fp != NULL)
	{
		ql_fclose(fp);
		SYSTEM_PARAM_DEBUG("%s_%d ===[FS] file closed: %s\r\n", __func__, __LINE__, path);
	}
	
	if (ret != 0)
	{
 		SYSTEM_PARAM_ERROR(ERR_TAG "fdcm write failed\n");
		return -1;
	}
 	SYSTEM_PARAM_DEBUG("save sysparam to flash\n");
	return 0;
}

int sysparam_save(void)
{
    int ret = 0;

    ql_rtos_mutex_lock(sysparam_file_mutex, 0xFFFFFFFF);

    ret = _sysparam_save(SYSTEM_PARAM_U_FILE_ROOT,&g_sysparam);

    _sysparam_save(SYSTEM_PARAM_B_FILE_ROOT,&g_sysparam);

    ql_rtos_mutex_unlock(sysparam_file_mutex);
	
	return ret;
}
int sysparamcpy(struct sysparam * pdsc,struct sysparamold * psrc)
{
	memcpy(pdsc,psrc,sizeof(struct sysparamold)<sizeof(struct sysparam)?sizeof(struct sysparamold):sizeof(struct sysparam));
	//Add default values for new items
	memcpy(pdsc->example, "An_example_of_parameter_substitution", strlen("An_example_of_parameter_substitution"));
	
	memset(pdsc->unuse,0,sizeof(pdsc->unuse));
}

int _sysparam_loadold(const char *path,struct sysparamold * g_sysparam)
{
	QFILE * fp = NULL;
	uint32_t md5_main[4] = { 0 };
	char * ptr1;
	char * ptr2;
	int ret = -1;
	SYSTEM_PARAM_ERROR(ERR_TAG "----------%s_%d ---------------\n", __func__, __LINE__);
	if (path == NULL)
	{
		SYSTEM_PARAM_ERROR(ERR_TAG "%s_%d ===path is NULL\n", __func__, __LINE__);
		return -1;
	}
	
	fp = ql_fopen(path, "r");
	if (fp == NULL) {
		SYSTEM_PARAM_ERROR(ERR_TAG "%s_%d === open %s file failed!\n", __func__, __LINE__, path);
		return -1;
	}

	ret = ql_fread((void *)g_sysparam, sizeof(struct sysparamold), 1, fp);
	
	if(fp != NULL)
	{
		ql_fclose(fp);
		SYSTEM_PARAM_DEBUG("%s_%d ===[FS] file closed: %s\r\n", __func__, __LINE__, path);
	}
	
	if (ret < 0) {
		SYSTEM_PARAM_ERROR(ERR_TAG "%s_%d ===ql_fread failed\n", __func__, __LINE__);
		return -1;
	}
	
	ptr1=g_sysparam->Md5;
	ptr2=(char * )g_sysparam;
	sysparam_md5(ptr2, ptr1-ptr2, md5_main);

	if (memcmp(md5_main, g_sysparam->Md5, 16) != 0)
	{
		SYSTEM_PARAM_ERROR(ERR_TAG "Verify:  MD5 %08x-%08x-%08x-%08x, actual %08x-%08x-%08x-%08x\n",
				g_sysparam->Md5[0], g_sysparam->Md5[1], g_sysparam->Md5[2], g_sysparam->Md5[3],
				md5_main[0], md5_main[1], md5_main[2], md5_main[3]);
		if (md5_main[0] == 0xFFFFFFFF && md5_main[1] == 0xFFFFFFFF && md5_main[2] == 0xFFFFFFFF && md5_main[3] == 0xFFFFFFFF)
		{
			//memcpy(Md5, md5_ptr, 16);
			return -3;
		}	
		else return -2;
	}

	SYSTEM_PARAM_DEBUG( "load sysparam from flash\n");
	return 0;
}

/**
 * @brief Load sysparam from flash
 * @return 0 on success, -1 on failure
 */
int _sysparam_load(const char *path,struct sysparam * g_sysparam)
{
	QFILE * fp = NULL;
	uint32_t md5_main[4] = { 0 };
	char * ptr1;
	char * ptr2;
	int ret = -1;
SYSTEM_PARAM_ERROR(ERR_TAG "----------%s_%d ---------------\n", __func__, __LINE__);
	if (path == NULL)
	{
		SYSTEM_PARAM_ERROR(ERR_TAG "%s_%d ===path is NULL\n", __func__, __LINE__);
		return -1;
	}
	
	fp = ql_fopen(path, "r");
	if (fp == NULL) {
		SYSTEM_PARAM_ERROR(ERR_TAG "%s_%d === open %s file failed!\n", __func__, __LINE__, path);
		return -1;
	}

	ret = ql_fread((void *)g_sysparam, SYSPARAM_SIZE, 1, fp);
	
	if(fp != NULL)
	{
		ql_fclose(fp);
		SYSTEM_PARAM_DEBUG("%s_%d ===[FS] file closed: %s\r\n", __func__, __LINE__, path);
	}
	
	if (ret < 0) {
		SYSTEM_PARAM_ERROR(ERR_TAG "%s_%d ===ql_fread failed\n", __func__, __LINE__);
		return -1;
	}
	
	ptr1=g_sysparam->Md5;
	ptr2=(char * )g_sysparam;
	sysparam_md5(ptr2, ptr1-ptr2, md5_main);

	if (memcmp(md5_main, g_sysparam->Md5, 16) != 0)
	{
		SYSTEM_PARAM_ERROR(ERR_TAG "Verify:  MD5 %08x-%08x-%08x-%08x, actual %08x-%08x-%08x-%08x\n",
				g_sysparam->Md5[0], g_sysparam->Md5[1], g_sysparam->Md5[2], g_sysparam->Md5[3],
				md5_main[0], md5_main[1], md5_main[2], md5_main[3]);
		if (md5_main[0] == 0xFFFFFFFF && md5_main[1] == 0xFFFFFFFF && md5_main[2] == 0xFFFFFFFF && md5_main[3] == 0xFFFFFFFF)
		{
			//memcpy(Md5, md5_ptr, 16);
			return -3;
		}	
		else return -2;
	}
	
	SYSTEM_PARAM_DEBUG( "load sysparam from flash\n");
	return 0;
}

int sysparam_load()
{
    int ret = -1;

    ql_rtos_mutex_lock(sysparam_file_mutex, 0xFFFFFFFF);
	ret = _sysparam_load(SYSTEM_PARAM_U_FILE_ROOT,&g_sysparam);
    ql_rtos_mutex_unlock(sysparam_file_mutex);

    return ret;
}


int sysparam_md5(void *buff, size_t buff_len, uint32_t md5_str[4])
{
	MD5_CTX md5;

	if ((buff == NULL) || (md5_str == NULL))
	{
		usb_log_printf("%s_%d ===parameter is error\n", __func__, __LINE__);
		return -1;
	}

	MD5Init(&md5);
	MD5Update(&md5,buff,buff_len);  
	MD5Final(&md5,md5_str);

	return 0;
}

int sysparam_get_device_type(void)
{
	return sysparam_get()->device_type;
}

void sysparam_set_device_type(int value)
{
	sysparam_get()->device_type = value;
}

void sysparam_set_wifi_version(const char * WifiVer)
{
	if (strcmp(sysparam_get()->WifiVer,WifiVer))
	{
		snprintf(sysparam_get()->WifiVer,sizeof(sysparam_get()->WifiVer),"%s",WifiVer);
		sysparam_save();
	}	
}

void sysparam_data_clear(void)
{
    memset(&g_sysparam, 0, SYSPARAM_SIZE);
}

#else
int sysparam_save(void)
{
	return 0;
}
int sysparam_load()
{
	return 0;//_sysparam_load(g_fdcm_param_hdl,&g_sysparam);
}

#endif /* PRJCONF_SYSPARAM_SAVE_TO_FLASH */

/**
 * @brief Get the pointer of the sysparam
 * @return Pointer to the sysparam, NULL on failure
 */
struct sysparam *sysparam_get(void)
{
	return &g_sysparam;
}

struct sysinfo_wlan_sta_param sysparam_get_wlanparam(void)
{
	return sysparam_get()->wlan_sta_param;
}

int sysparam_get_uart_log(void)
{
	return sysparam_get()->uart_log;
}

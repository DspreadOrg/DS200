#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_type.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_data_call.h"
#include "ql_sim.h"
#include "ql_nw.h"
#include "ql_dev.h"
#include "sockets.h"
#include "netdb.h"
#include "module_lte.h"
#include "prj_common.h"
#include "public_api_interface.h"
#include "systemparam.h"
#include "terminfodef.h"
#include "tts_yt_task.h"

#include "cJSON.h"
#include "apn.h"

#if 1
#define MODULE_GSM_ERROR(...)   usb_log_printf(__VA_ARGS__)
#else
#define MODULE_GSM_ERROR(...)   do{}while(0);
#endif

#if 0
#define MODULE_GSM_DEBUG(...)   usb_log_printf(__VA_ARGS__)
#else
#define MODULE_GSM_DEBUG(...)   do{}while(0);
#endif

#if 1
#define MODULE_GSM_INFO(...)   usb_log_printf(__VA_ARGS__)
#else
#define MODULE_GSM_INFO(...)   do{}while(0);
#endif

//
//char ModelInitOk = 0;
ql_task_t module_net_thread = NULL;
unsigned char g_network_register_firstend = 0;
char module_ctrl = 0;
static ql_flag_t SimHotPlus = NULL;
static char SimHotStat=0;
static char sim_care_check=0;
#define SIM_HOT_PLUS_CHG                 0x01
#define SIM_HOT_PLUS_MASK                (SIM_HOT_PLUS_CHG)

#define INVALID_SOCKET -1

#define DATA_CALL_PROFILE_IDX 1
#define IP_PROTOCOL 0 
#define SIM_INSERT_PIN_LEVEL 1

typedef enum 
{
	QL_SUCCESS,
	QL_GENERIC_FAILURE,
}QL_ERROR_CODE_E;

static int data_call_successed_flag=0;
int data_call_state=-1;

static struct in_addr module_ip4_addr = {0};


#define WAIT_CHECK_SIM_STATA_MAX_NUM          60 // 120  // 60
#define WAIT_CHECK_SIM_STATA_MAX_TIME         1000 // 500 // 1000

#define WAIT_NETWORK_REGISTER_MAX_NUM         1 // 2 // 1
#define WAIT_NETWORK_REGISTER_MAX_TIME        1000 // 500 // 1000

int check_sim_state(void)
{
	int ret=-1,card_status=-1;
	
	while(1)
	{
		ret = ql_sim_get_card_status(&card_status);
		MODULE_GSM_ERROR("ql_sim_get_card_status ret:%d, status:%d\n",ret,card_status);
		if (!ret)
			break;
		ql_rtos_task_sleep_ms(200);
	}

	return card_status;
}


QL_ERROR_CODE_E wait_network_register(int time)
{
	int ret=0,retry_num=0;
	QL_NW_REG_STATUS_INFO_T  reg_status;
RETRY_CHECK:
	ret=ql_nw_get_reg_status(&reg_status);
	MODULE_GSM_INFO("The nw state: %d\n", reg_status.data_reg.state);
	if((ret==0)&&(SimHotStat))
	{
		switch(reg_status.data_reg.state)
		{
			case QL_NW_REG_STATE_HOME_NETWORK:
			case QL_NW_REG_STATE_ROAMING:
			case QL_NW_REG_STATE_DENIED:
				return QL_SUCCESS;
			default:
				ql_rtos_task_sleep_ms(WAIT_NETWORK_REGISTER_MAX_TIME); //����
				if(retry_num<(time*WAIT_NETWORK_REGISTER_MAX_NUM))
				{
					retry_num++;
					goto RETRY_CHECK;
				}
				break;
		}
	}
	return QL_GENERIC_FAILURE;
}

static void datacall_status_callback(int profile_idx, int status)
{
    int ret = -1;
	struct ql_data_call_info info = {0};
	char ip_addr_str[64] = {0};
	if(status)
	{
		ret = ql_get_data_call_info(profile_idx, IP_PROTOCOL, &info);
		MODULE_GSM_DEBUG("info.profile_idx: %d\n", info.profile_idx);
		MODULE_GSM_DEBUG("info.ip_version: %d\n", info.ip_version);

        if (ret == 0)
        {
    		if(info.ip_version)
    		{
    			MODULE_GSM_DEBUG("info.v6.state: %d\n", info.v6.state);
    			MODULE_GSM_DEBUG("info.v6.reconnect: %d\n", info.v6.reconnect);

    			inet_ntop(AF_INET6, &info.v6.addr.ip, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.ip: %s\n", ip_addr_str);

    			inet_ntop(AF_INET6, &info.v6.addr.pri_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.pri_dns: %s\n", ip_addr_str);

    			inet_ntop(AF_INET6, &info.v6.addr.sec_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.sec_dns: %s\n", ip_addr_str);
    		}
    		else
    		{
    			MODULE_GSM_INFO("info.v4.state: %d\n", info.v4.state);
    			MODULE_GSM_INFO("info.v4.reconnect: %d\n", info.v4.reconnect);

    			inet_ntop(AF_INET, &info.v4.addr.ip, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.ip: %s\n", ip_addr_str);

    			inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.pri_dns: %s\n", ip_addr_str);

    			inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.sec_dns: %s\n", ip_addr_str);

    			module_ip4_addr = info.v4.addr.ip;
    		}
            data_call_successed_flag=1;
        }
		
	}
	else
	{
		data_call_successed_flag=0;
		MODULE_GSM_DEBUG("Data call failed! profile_idx:%d\n", info.profile_idx);
	}
}

static void ql_data_call_cb(int profile_idx, int nw_status)
{
    int ret = -1;
	struct ql_data_call_info info = {0};
	char ip_addr_str[64] = {0};
    
	MODULE_GSM_INFO("call_cb: profile%d status=%d\r\n", profile_idx, nw_status);
	data_call_state=nw_status;

    if(nw_status)
	{
		ret = ql_get_data_call_info(profile_idx, IP_PROTOCOL, &info);
		MODULE_GSM_DEBUG("info.profile_idx: %d\n", info.profile_idx);
		MODULE_GSM_DEBUG("info.ip_version: %d\n", info.ip_version);

        if (ret == 0)
        {
    		if(info.ip_version)
    		{
    			MODULE_GSM_DEBUG("info.v6.state: %d\n", info.v6.state);
    			MODULE_GSM_DEBUG("info.v6.reconnect: %d\n", info.v6.reconnect);

    			inet_ntop(AF_INET6, &info.v6.addr.ip, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.ip: %s\n", ip_addr_str);

    			inet_ntop(AF_INET6, &info.v6.addr.pri_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.pri_dns: %s\n", ip_addr_str);

    			inet_ntop(AF_INET6, &info.v6.addr.sec_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.sec_dns: %s\n", ip_addr_str);
    		}
    		else
    		{
    			MODULE_GSM_INFO("info.v4.state: %d\n", info.v4.state);
    			MODULE_GSM_INFO("info.v4.reconnect: %d\n", info.v4.reconnect);

    			inet_ntop(AF_INET, &info.v4.addr.ip, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.ip: %s\n", ip_addr_str);

    			inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.pri_dns: %s\n", ip_addr_str);

    			inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.sec_dns: %s\n", ip_addr_str);

    			module_ip4_addr = info.v4.addr.ip;
    		}
            data_call_successed_flag=1;
        }
		
	}
	else
	{
		data_call_successed_flag=0;
		MODULE_GSM_DEBUG("Data call failed! profile_idx:%d\n", info.profile_idx);
	}
}

void ql_nw_notify_cb(unsigned int ind_flag, void *ind_msg_buf, unsigned int ind_msg_len, void *contextPtr)
{
    switch(ind_flag)
    {
        case QL_NW_IND_VOICE_REG_EVENT_FLAG:
        {
             QL_NW_COMMON_REG_STATUS_INFO_T  *voice_reg_status=(QL_NW_COMMON_REG_STATUS_INFO_T  *)ind_msg_buf;
                MODULE_GSM_DEBUG("QL_NW_IND_VOICE_REG_EVENT:\n\
state:%d; lac:0x%x; cid:0x%x; rat:%d; rejectCause:%d; psc:0x%x;\n", 
                        voice_reg_status->state,
                        voice_reg_status->lac,
                        voice_reg_status->cid,
                        voice_reg_status->rat,
                        voice_reg_status->rejectCause,
                        voice_reg_status->psc);
        }
        break;
        case QL_NW_IND_DATA_REG_EVENT_FLAG:
        {
             QL_NW_COMMON_REG_STATUS_INFO_T  *data_reg_status=(QL_NW_COMMON_REG_STATUS_INFO_T  *)ind_msg_buf;
                MODULE_GSM_DEBUG("QL_NW_IND_DATA_REG_EVENT:\n\
state:%d; lac:0x%x; cid:0x%x; rat:%d; rejectCause:%d;\n", 
                        data_reg_status->state,
                        data_reg_status->lac,
                        data_reg_status->cid,
                        data_reg_status->rat,
                        data_reg_status->rejectCause);
	
                
				if (data_reg_status->state == 1 || data_reg_status->state == 5)
				{
					data_call_successed_flag = 1;
				}
                else
                {
                    data_call_successed_flag = 0;
                }
        }
        break;
        case QL_NW_IND_SIGNAL_STRENGTH_EVENT_FLAG:
        {
            QL_NW_SIGNAL_STRENGTH_INFO_T *signal=(QL_NW_SIGNAL_STRENGTH_INFO_T *)ind_msg_buf;
            MODULE_GSM_DEBUG("QL_NW_IND_SIGNAL_STRENGTH_EVENT:\n");
            MODULE_GSM_INFO("GW: rssi:%d; bitErrorRate:%d; rscp:%d; ecno:%d;\n",
            signal->GW_SignalStrength.rssi,
            signal->GW_SignalStrength.bitErrorRate,
            signal->GW_SignalStrength.rscp,
            signal->GW_SignalStrength.ecno);
            MODULE_GSM_INFO("LTE: rssi:%d; rsrp:%d; rsrq:%d; cqi:%d;\n",
            signal->LTE_SignalStrength.rssi,
            signal->LTE_SignalStrength.rsrp,
            signal->LTE_SignalStrength.rsrq,
            signal->LTE_SignalStrength.cqi);
        }
        break;
        case QL_NW_IND_NITZ_TIME_UPDATE_EVENT_FLAG:
        {
            QL_NW_NITZ_TIME_INFO_T  *time_info=(QL_NW_NITZ_TIME_INFO_T *)ind_msg_buf;
            MODULE_GSM_DEBUG("QL_NW_IND_NITZ_TIME_UPDATE_EVENT:\nnitz_time:%s; abs_time:%ld; leap_sec:%d\n",time_info->nitz_time,time_info->abs_time,time_info->leap_sec);
            MODULE_GSM_DEBUG("now time:%ld\n",time(NULL));
        }
        break;
    }
}
int get_PLMN_lock_state(const char    * mcc,const char * mnc) //PLMN  lock
{

	char strImsi[32] = {0};
	memset(strImsi, 0x0, sizeof(strImsi));
	ql_sim_get_imsi(strImsi, sizeof(strImsi));
	if(!strncmp(mcc,strImsi,3))
	{
		if(!strncmp(mnc,strImsi+3,2))
		{	
			printf("PLMN unlock\n");
			return 0;
		}
	}
	printf("PLMN lock\n");
	return 1;
}
void Apncmp(int profile_idx, int ip_version, char *apnname, char *username, char *password, int auth_type)
{
	int ret_at = 0;
	int i;
	char resp_buf[128] = {0};
	char atcmd[64];
	memset(resp_buf,0,128);
	sprintf(atcmd,"AT+CGDCONT?\r\n");
	ret_at = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
	usb_log_printf("=============%s , line %d,%s===========ret %d  \n%s\n", __func__, __LINE__,atcmd,ret_at,resp_buf);
	
//	usb_log_printf("=============%s , line %d,%s\n", __func__, __LINE__,apnname);
	if((strstr(resp_buf,apnname)==NULL)&&(data_call_successed_flag==0))
	{
		usb_log_printf("=============%s , line %d\n", __func__, __LINE__);
		{
			int ret = 0;
			char resp_buf[128] = {0};
			char atcmd[64];
			sprintf(atcmd,"AT+QICSGP=%d,%d,\"%s\",\"%s\",\"%s\",%d\r\n",profile_idx,ip_version,apnname,username,password,auth_type);
			for (i = 0; i < 3; i++)
			{
				ret_at = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
				if (ret_at	== 0)
				{
					break;
				}
				usb_log_printf("=============%s , line %d,%s===========ret %d  \n%s\n", __func__, __LINE__,atcmd,ret_at,resp_buf);
				ql_rtos_task_sleep_ms(1000);
			}
		}
		memset(resp_buf,0,128);
		sprintf(atcmd,"AT+CFUN=0\r\n");
		for (i = 0; i < 3; i++)
		{
			ret_at = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
			if (ret_at	== 0)
			{
				break;
			}
			usb_log_printf("=============%s , line %d,%s===========ret %d  \n%s\n", __func__, __LINE__,atcmd,ret_at,resp_buf);
			ql_rtos_task_sleep_ms(1000);
		}
		ql_rtos_task_sleep_ms(1000);
		memset(resp_buf,0,128);
		sprintf(atcmd,"AT+CFUN=1\r\n");
		for (i = 0; i < 3; i++)
		{
			ret_at = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
			if (ret_at	== 0)
			{
				break;
			}
			usb_log_printf("=============%s , line %d,%s===========ret %d  \n%s\n", __func__, __LINE__,atcmd,ret_at,resp_buf);
			ql_rtos_task_sleep_ms(1000);
		}
	}
	else
	usb_log_printf("=============%s , line %d\n", __func__, __LINE__);

}

QL_ERROR_CODE_E start_data_call(void)
{
	int ret = QL_SUCCESS;
    char IMSI[32] = {0};
    int i = 0;
    APN* apn;
	
	data_call_successed_flag=0;
	get_PLMN_lock_state("460","08");
	MODULE_GSM_INFO("start_data_call:%d\n", __LINE__);
	ret=ql_wan_start(datacall_status_callback);
	if(ret!=0)
	{
		ret = QL_GENERIC_FAILURE;
		goto exit;
	}
	ql_set_auto_connect(DATA_CALL_PROFILE_IDX, TRUE);
	//printf("start_data_call:%d\n", __LINE__);
	if(ret!=0)
	{
		ret = QL_GENERIC_FAILURE;
		goto exit;
	}

    for (i = 0; i < 5; i++)
    {
	    ret=ql_sim_get_imsi(IMSI, sizeof(IMSI));
        if (strlen(IMSI) != 0)
        {
            break;
        }
        ql_rtos_task_sleep_ms(1000);
    }

	MODULE_GSM_INFO("ql_sim_get_imsi:%d %s\n", __LINE__,IMSI);
    ql_set_data_call_asyn_mode(1, ql_data_call_cb);

	apn = find_apn(IMSI);


	
	if(apn){
		MODULE_GSM_INFO("%s_%d IMSI: %s, apn: %s\n", __func__, __LINE__, IMSI, apn->apn);
		
		Apncmp(DATA_CALL_PROFILE_IDX, IP_PROTOCOL, apn->apn, apn->user, apn->password, 0);
		ql_start_data_call(DATA_CALL_PROFILE_IDX, IP_PROTOCOL, apn->apn, apn->user, apn->password, 0);
	}
	else{
		MODULE_GSM_INFO("%s_%d apn is null\n", __func__, __LINE__);
	}
		
	//printf("start_data_call:%d\n", __LINE__);
	if(ret!=0)
		ret = QL_GENERIC_FAILURE;

exit:
	
	return ret;
}

int module_send_atcmd_sync(char *atcmd, char *resp, int resp_size)
{
    int ret = 0;
    int i = 0;
    char resp_buf[128] = {0};

    if (atcmd == NULL)
    {
        return -1;
    }

    for(i=0; i<3; i++)
	{
        memset(resp_buf, 0, sizeof(resp_buf));
        ret = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
		if(ret == 0)
		{
			break;
		}
	}

    if (resp != NULL)
    {
        strncpy(resp, resp_buf, resp_size);
    }
    
	return ret;
}

/*
����fastdormancy�����ó�15�봥���Զ��ͷ�rrc�������û���յ���վ�ͷ�RRC����Ϣ��δ�ͷ�RRC����
*/
int open_fastdormancy_mode(void)
{
	int ret = 0;
    int i = 0;
    char resp_buf[128] = {0};
    int tmp_param = 0;
    int rrc_release_timeout = 0;

    for(i=0; i<3; i++)
    {
        memset(resp_buf, 0, sizeof(resp_buf));
        ret = ql_atcmd_send_sync("AT+MEDCR=1,34,15\r\n", resp_buf, sizeof(resp_buf), NULL, 5);
        if (ret == 0)
        {
            sscanf(resp_buf, "+MEDCR:%d,%d\r\n", &tmp_param, &rrc_release_timeout);
            MODULE_GSM_DEBUG("%s_%d ===rrc_release_timeout =%d\n", __func__, __LINE__, rrc_release_timeout);
            break;
        }
    }

    if ((ret == 0) && (rrc_release_timeout != 15))
    {
        for(i=0; i<3; i++)
    	{
            memset(resp_buf, 0, sizeof(resp_buf));
            ret = ql_atcmd_send_sync("AT+MEDCR=0,34,15\r\n", resp_buf, sizeof(resp_buf), NULL, 5);
    		if(ret == 0)
    		{
    			break;
    		}
    	}
    }
    
	return ret;
}

int device_set_low_power_mode(void)
{
    module_send_atcmd_sync("at*pmicreg=w,1,25,81\r\n", NULL, 0);
    return 0;
}

int set_LTE_module_low_power(void)
{
    int ret ;
    //static int flag = 0; 

    //if (flag == 1) 
        //return 0;

    usb_log_printf("Enter ------ set_LTE_module_low_power func ------\n");
    
    ret = ql_atcmd_send_sync("at*pmicreg=w,1,25,81\r\n",NULL,0,NULL,5);
    if (ret != 0)
    {
        ret = ql_atcmd_send_sync("at*pmicreg=w,1,25,81\r\n",NULL,0,NULL,5);
        if (ret != 0) 
        {
            usb_log_printf("Failed : ql_atcmd_send_sync(at*pmicreg=w,1,25,81) \n");    
        }
    }
    
    ql_rtos_task_sleep_ms(100);
    
    ret = ql_atcmd_send_sync("at*pmicreg=w,1,24,88\r\n",NULL,0,NULL,5);
    if (ret != 0)
    {
        ret = ql_atcmd_send_sync("at*pmicreg=w,1,24,88\r\n",NULL,0,NULL,5);
        if (ret != 0) 
        {
            usb_log_printf("Failed : ql_atcmd_send_sync(at*pmicreg=w,1,24,88) \n");    
        }
    }

    //flag = 1;
    
    return ret;
}

int Module_Net_State(void)
{
	return data_call_successed_flag;
}

void Module_Net_State_Set(int state)
{
	data_call_successed_flag = state;
}

int LteNetConnect(void)
{
	int ret = 0;
//	uint16_t module_net_cnt = 280;
//    struct ql_data_call_info info = {0};
	
	//���SIM��״̬
//	ModelInitOk = 1;
		
	//�ȴ�ģ��ע���ƶ����� 180s
	MODULE_GSM_INFO("%s , line %d\n", __func__, __LINE__);
	ret=wait_network_register(180);
	MODULE_GSM_INFO("%s , line %d, ret %d\n", __func__, __LINE__,ret);
	if(ret>0)
	{
		MODULE_GSM_ERROR("*** Network register failed ! ***\n");
		goto exit;
//		return;
	}
//    g_network_register_firstend = 1;
//    MODULE_GSM_INFO("%s_%d === wait_for_read_sim_card ok\n", __func__, __LINE__);
    
#if 0
    ret = openAutoAnswer();

    if (ret == 0)
    {
        tts_play_set(AudioOpenAutoAnswer,AudioOpenAutoAnswerLen,FixAudioTypeDef);
    }
#endif

	ql_nw_event_register(0x0f);
	ql_nw_add_event_handler(ql_nw_notify_cb, NULL);

	//����ģ�����ݲ���
	ret=start_data_call();
	if(ret>0)
	{
		MODULE_GSM_ERROR("*** Start data failed ! ***\n");
		goto exit;
//		return;
	}
	MODULE_GSM_DEBUG("%s , line %d, module net init finished!\n", __func__, __LINE__);

	open_fastdormancy_mode();

    //device_set_low_power_mode();

	set_LTE_module_low_power();
    
	ret=0;
exit:    
	if (ret!=0)
		ret= -1;
		
	MODULE_GSM_INFO("%s , line %d, ret %d\n", __func__, __LINE__,ret);
	return ret;
}

int ModuleGetCellInfo(cJSON *root)
{
	char buf[64];
//	cJSON *root = NULL;
//	QL_NW_CELL_INFO_T  ql_nw_cell_infor;//=calloc(1,sizeof(QL_NW_CELL_INFO_T));
//	QL_NW_CELL_INFO_T * ql_nw_cell_info=&ql_nw_cell_infor;//=calloc(1,sizeof(QL_NW_CELL_INFO_T));
	QL_NW_CELL_INFO_T * ql_nw_cell_info=malloc(sizeof(QL_NW_CELL_INFO_T));
	int ret;
	int ii;
	int set;
	
	if (ql_nw_cell_info==NULL)
		return -1;

//	root = cJSON_CreateObject();
//	if (root==NULL) 
//		goto cJson_EXIT;

	ret=ql_nw_get_cell_info(ql_nw_cell_info);
	printf("\nql_nw_get_ql_nw_cell_info ret = %d\n", ret);

	set=0;
	if(ql_nw_cell_info->gsm_info_valid)
	{
		for(ii=0;ii<ql_nw_cell_info->gsm_info_num;ii++)
		{
			if (ql_nw_cell_info->gsm_info[ii].flag==0) 
			{
				set=ii;
				break;
			}
//			printf("Cell_%d [GSM] cid:%d, mcc:%d, mnc:%d, lac:%d, arfcn:%d, bsic:%d, rssi:%d\n",
//				i,
//				ql_nw_cell_info->gsm_info[i].cid,
//				ql_nw_cell_info->gsm_info[i].mcc,
//				ql_nw_cell_info->gsm_info[i].mnc,
//				ql_nw_cell_info->gsm_info[i].lac,
//				ql_nw_cell_info->gsm_info[i].arfcn,
//				ql_nw_cell_info->gsm_info[i].bsic,
//				ql_nw_cell_info->gsm_info[i].rssi);
		}
		sprintf(buf,"%x",ql_nw_cell_info->gsm_info[set].lac);
		cJSON_AddItemToObject(root, "lac", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->gsm_info[set].cid);
		cJSON_AddItemToObject(root, "cell", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->gsm_info[set].mcc);
		cJSON_AddItemToObject(root, "mcc", cJSON_CreateString(buf));
		sprintf(buf,"%d",ql_nw_cell_info->gsm_info[set].mnc);
		cJSON_AddItemToObject(root, "mnc", cJSON_CreateString(buf));
	}
	else if(ql_nw_cell_info->umts_info_valid)
	{
		for(ii=0;ii<ql_nw_cell_info->umts_info_num;ii++)
		{
			if (ql_nw_cell_info->gsm_info[ii].flag==0) 
			{
				set=ii;
				break;
			}
//			printf("Cell_%d [UMTS] cid:%d, lcid:%d, mcc:%d, mnc:%d, lac:%d, uarfcn:%d, psc:%d, rssi:%d\n",
//				i,
//				ql_nw_cell_info->umts_info[i].cid,
//				ql_nw_cell_info->umts_info[i].lcid,
//				ql_nw_cell_info->umts_info[i].mcc,
//				ql_nw_cell_info->umts_info[i].mnc,
//				ql_nw_cell_info->umts_info[i].lac,
//				ql_nw_cell_info->umts_info[i].uarfcn,
//				ql_nw_cell_info->umts_info[i].psc,
//				ql_nw_cell_info->umts_info[i].rssi);
		}
		sprintf(buf,"%x",ql_nw_cell_info->umts_info[set].lac);
		cJSON_AddItemToObject(root, "lac", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->umts_info[set].cid);
		cJSON_AddItemToObject(root, "cell", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->umts_info[set].mcc);
		cJSON_AddItemToObject(root, "mcc", cJSON_CreateString(buf));
		sprintf(buf,"%d",ql_nw_cell_info->umts_info[set].mnc);
		cJSON_AddItemToObject(root, "mnc", cJSON_CreateString(buf));
	}
	else if(ql_nw_cell_info->lte_info_valid)
	{
		for(ii=0;ii<ql_nw_cell_info->lte_info_num;ii++)
		{
			if (ql_nw_cell_info->gsm_info[ii].flag==0) 
			{
				set=ii;
				break;
			}
//			printf("Cell_%d [LTE] cid:%d, mcc:%d, mnc:%d, tac:%d, pci:%d, earfcn:%d, rssi:%d\n",
//				i,
//				ql_nw_cell_info->lte_info[i].cid,
//				ql_nw_cell_info->lte_info[i].mcc,
//				ql_nw_cell_info->lte_info[i].mnc,
//				ql_nw_cell_info->lte_info[i].tac,
//				ql_nw_cell_info->lte_info[i].pci,
//				ql_nw_cell_info->lte_info[i].earfcn,
//				ql_nw_cell_info->lte_info[i].rssi);
		}
		sprintf(buf,"%x",ql_nw_cell_info->lte_info[set].tac);
		cJSON_AddItemToObject(root, "lac", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->lte_info[set].cid);
		cJSON_AddItemToObject(root, "cell", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->lte_info[set].mcc);
		cJSON_AddItemToObject(root, "mcc", cJSON_CreateString(buf));
		sprintf(buf,"%d",ql_nw_cell_info->lte_info[set].mnc);
		cJSON_AddItemToObject(root, "mnc", cJSON_CreateString(buf));
	}
cJson_EXIT:
	free(ql_nw_cell_info);  
	
	return 0;
}



int GetGsmSignalLevel(void)
{
    QL_NW_SIGNAL_STRENGTH_INFO_T signal_info;

    ql_nw_get_signal_strength(&signal_info);
    return signal_info.LTE_SignalStrength.rsrp;
}

int GetLteIp(uint8_t *buff,uint8_t buffsize)
{
    int ret;
    struct ql_data_call_info info = {0};
    char ip_addr_str[64] = {0};
    ret = ql_get_data_call_info(DATA_CALL_PROFILE_IDX, IP_PROTOCOL, &info);
    if ( ret == 0 )
    {
        if ( info.ip_version )
        {
            inet_ntop( AF_INET6, &info.v6.addr.ip, ip_addr_str, sizeof(ip_addr_str) );
        }
        else
        {

            inet_ntop( AF_INET, &info.v4.addr.ip, ip_addr_str, sizeof(ip_addr_str) );
        }
        strncpy(buff,ip_addr_str,buffsize);
    }
    return ret;
}

//void LTE_CAT1_init(void)
//{
//	if (ql_rtos_task_create(&module_net_thread,
//						3*1024,
//						100,
//						"module_net_task",
//						module_net_task,
//						NULL) != OS_OK) {
//	}
//
//	MODULE_GSM_DEBUG("enter %s , line %d\n", __func__, __LINE__);
//}
//SIM����λص�����
//SIM card insertion and removal callback function
static void quec_sim_detect_callback(PIN_LEVEL_E pinlevel)
{
//	SIM_HOT_PLUG_MSG_T sim_hot_plug_msg = {0};
//	sim_hot_plug_msg.pinlevel = pinlevel;
	if (pinlevel) 
		SimHotStat=1;
	else
		SimHotStat=0;
////	if (ql_rtos_semaphore_get_cnt(SimHotPlus,&cnt)
//	ql_rtos_semaphore_release(SimHotPlus);
	ql_rtos_flag_release(SimHotPlus, SIM_HOT_PLUS_CHG, QL_FLAG_OR);
	MODULE_GSM_INFO("%s level (%d)\r\n",__func__,pinlevel);
}
char SIM_care_check_state(void)
{
	return sim_care_check;
}
void SimHotInsertTask(void * argv)
{
	int ret;
	int ii;
	
	module_ctrl=1;
	MODULE_GSM_INFO("%s , line %d start ...\n", __func__, __LINE__);
//	ql_rtos_semaphore_release(SimHotPlus);
	ql_rtos_flag_release(SimHotPlus, SIM_HOT_PLUS_CHG, QL_FLAG_OR);
	for(ii=0;ii<50*6;ii++)
	{
		ret=check_sim_state();
		MODULE_GSM_INFO("*sim chk %d ret %d***\n",ii,ret);
		if (ret!=QL_SIM_STATUS_UNKNOWN)
		{
//			if (ret==QL_SIM_STATUS_READY)
			if (ret!=QL_SIM_STATUS_NOT_INSERTED)
				SimHotStat=1;
			break;
		}
		ql_rtos_task_sleep_ms(200);
	}

	while(module_ctrl)
	{
		uint32_t flag;
//		ret = ql_rtos_flag_wait( SimHotPlus, SIM_HOT_PLUS_MASK, QL_FLAG_OR_CLEAR, &flag, QL_WAIT_FOREVER );
		ret = ql_rtos_flag_wait( SimHotPlus, SIM_HOT_PLUS_MASK, QL_FLAG_OR_CLEAR, &flag, 1000 );
//		ret=ql_rtos_semaphore_wait(SimHotPlus, QL_WAIT_FOREVER);
		if (ret != 0) 
		 continue;
		
		if (SimHotStat)
		{
			TermInfo.SIMState = 1;
			MODULE_GSM_INFO("TermInfo.SIMState = %d \n",TermInfo.SIMState);
			if (g_network_register_firstend)
//				tts_play_text("SIM���Ѳ���",GBK_TEXT);
				MODULE_GSM_INFO("The SIM card is inserted \n");

				LteNetConnect();
		}
		else
		{
			TermInfo.SIMState = 0;
			MODULE_GSM_INFO("TermInfo.SIMState = %d \n",TermInfo.SIMState);
			if (g_network_register_firstend)
//				tts_play_text("SIM�����Ƴ�",GBK_TEXT);
				MODULE_GSM_INFO("Pull out the SIM card \n");
			else
//				tts_play_set(AudioWeijcdsimk,AudioWeijcdsimkLen,FixAudioTypeDef);
				MODULE_GSM_INFO("No SIM card was detected \n");

				
			MODULE_GSM_INFO("%s , line %d SimHotStat %d\n", __func__, __LINE__,SimHotStat);
		}
		sim_care_check=1;
		g_network_register_firstend=1;
	}
	
	g_network_register_firstend=1;
	module_net_thread = NULL;
	ql_rtos_task_delete(NULL);
}
//Enable SIM card detection. If flag is 1, module networking is enabled. If flag is 0, module networking is disabled
//����SIM����⣬flagΪ1������ģ��������flagΪ0����ر�ģ������
void ModuleNetFunc(int flag)
{
	int ret;

	if (SimHotPlus==NULL)
		ql_rtos_flag_create(&SimHotPlus);

	if (flag)
	{
		MODULE_GSM_INFO("%s: SimHotInsertTask creat...\n",__func__);
		ret = ql_sim_config_hot_plug_detect(1,SIM_INSERT_PIN_LEVEL,quec_sim_detect_callback);
		if(ret != QL_SIM_SUCCESS)
			MODULE_GSM_INFO("ql_sim_config_hot_plug_detect() fail ret(%d)\r\n",ret);
		else
			MODULE_GSM_INFO("ql_sim_config_hot_plug_detect() success ret(%d)\r\n",ret);
		ql_dev_set_modem_fun(QL_DEV_MODEM_FULL_FUN,0);

		if (module_ctrl) 
			MODULE_GSM_INFO("%s: SimHotInsertTask is running,skip\n",__func__);
		else
		{
		//	else
		//		SimHotStat=1;
			g_network_register_firstend=0;
			if (ql_rtos_task_create(&module_net_thread,
									3*1024,
									100,
									"module_net_task",
									SimHotInsertTask,
									NULL) != OS_OK) {
			}
		}
	}
	else
	{
		ret = ql_sim_config_hot_plug_detect(0,SIM_INSERT_PIN_LEVEL,quec_sim_detect_callback);
		if(ret != QL_SIM_SUCCESS)
			MODULE_GSM_INFO("ql_sim_config_hot_plug_detect() fail ret(%d)\r\n",ret);
			
		ql_dev_set_modem_fun(QL_DEV_MODEM_MIN_FUN,0);
		module_ctrl=0;		
		MODULE_GSM_INFO("%s: SimHotInsertTask exit...\n",__func__);
		for(int ii=0;ii<100;ii++)
		{
			if (module_net_thread!=NULL )
				ql_rtos_task_sleep_ms(100);
		}
	}
}


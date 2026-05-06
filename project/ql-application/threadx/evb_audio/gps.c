
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_uart.h"
#include "ql_gpio.h"
#include "ql_type.h"
#include "ql_rtos.h"
#include "ql_fs.h"
#include "prj_common.h"
#include "public_api_interface.h"
#include "gpio.h"
#include "systemparam.h"
#include "terminfodef.h"
#include "ext_wifi.h"
#include "tts_yt_task.h"
#include "gps.h"

#if DEV_GPS_SUPPORT
//#define usb_log_printf(x,...)
#define LOG_DBG(fmt, arg...)		printf("[GPS DBG]"fmt, ##arg)
#define LOG_INFO(fmt, arg...) printf("[GPS INFO]"fmt, ##arg)

uint8_t en_gps_echo = 0;

typedef struct
{
	u8 UtcTime[11]; // UTC
	u8 Statue;   //
	u8  Latitude[10];	//
	u8 LatitudeNS; //
	u8 Longitude[11];	 //
	u8 LongitudeEW;//
	u8 Speed[6];	     //
	u8 Azimuth[6];	 //
	u8 UtcData[7]; //	  
		//u8 SatelliteNum;//
}RMC_INFO;

typedef struct
{
	u8 UtcTime[11]; // UTC
	u8  Latitude[10];	//
	u8 LatitudeNS; //
	u8 Longitude[11];	 //
	u8 LongitudeEW;//
	u8 Quality;	     //
	u8 NumSatUsed[3];	 //
	u8 Altitude[8];   //
		//u8 SatelliteNum;//
}GGA_INFO;

typedef struct
{
	RMC_INFO RMC;
	GGA_INFO GGA;
}GPS_INFO;

static GPS_INFO GpsInfo={0,};

void GPS_RMC_INFO(const char* inbuf, int len);
void GPS_GGA_INFO(const char* inbuf, int len);

//#define	OFFSET(x)	offsetof(struct sysparam, x)
//
//struct {
//	const int offset;
//	const int len;
//}GPS_SET[] =
//{
//	{OFFSET(battery),sizeof(xr_sysDeviceParam->device_secret)},
//}

/************************************************************************/
/*                    Customer ATC URC callback                         */
/************************************************************************/

#if 0
int GPS_Decode(const char* inbuf, int inlen)
{
    int i;
    int len;
    
    if ((NULL == inbuf)||(inlen<2))
        return -1;
		inbuf +=2;
		inlen -=2;
		
    // For system URCs
    for (i = 0; i < NUM_ELEMS(m_SysURCHdlEntry); i++)
    {
        len=strlen( m_SysURCHdlEntry[i].keyword);
        if ((inlen>=len)&&(!memcmp(inbuf, m_SysURCHdlEntry[i].keyword,len)))
         {
            if (m_SysURCHdlEntry[i].handler!=NULL)
            	m_SysURCHdlEntry[i].handler(inbuf, inlen);
            return 0;
        }
    }

	return -1;
}
#endif

void GPS_RMC_INFO(const char* inbuf, int inlen)
{
	char * buf;
	char* ptr;
	char *pst;
	int idx;
	RMC_INFO RmcInfo;
	
	LOG_DBG("%s_%d ===  RMC INFO len %d:%.*s\n", __func__, __LINE__,inlen,inlen,inbuf);
//	LOG_DBG("%s_%d ===  %d:%.*s\n", __func__, __LINE__,inlen,inlen,inbuf);
	buf=malloc(inlen+1);
	if (buf==NULL)
		goto exit;
	memset(buf,0,inlen+1);
	memcpy(buf,inbuf,inlen);
	
//	LOG_DBG("%s_%d ===  %d:%s\n", __func__, __LINE__,strlen(buf),buf);
	ptr=strchr(buf,'*');
	if (ptr==NULL) 
		goto exit;
	*ptr=0;
	
//	LOG_DBG("%s_%d ===  %d:%s\n", __func__, __LINE__,strlen(buf),buf);
	pst=strchr(buf,',');
	if(pst==NULL)
		goto exit;
	pst++;
	
//	LOG_DBG("%s_%d ===  %d:%s\n", __func__, __LINE__,strlen(pst),pst);
	
	idx=0;
	while(1)
	{
		ptr=strchr(pst,',');
		if (ptr==NULL) ptr=pst+strlen(pst);
//	LOG_DBG("%s_%d ===  %d:%p,%p\n", __func__, __LINE__,idx,pst,ptr);
//	LOG_DBG("%s_%d ===  %d:%d,%.*s\n", __func__, __LINE__,idx,ptr-pst,ptr-pst,pst);
		switch(idx)
		{
			case 0:snprintf(RmcInfo.UtcTime,sizeof(RmcInfo.UtcTime),"%.*s",ptr-pst,pst); break;
			case 1:RmcInfo.Statue=*pst; break;
			case 2:snprintf(RmcInfo.Latitude,sizeof(RmcInfo.Latitude),"%.*s",ptr-pst,pst); break;
			case 3:RmcInfo.LatitudeNS=*pst; break;
			case 4:snprintf(RmcInfo.Longitude,sizeof(RmcInfo.Longitude),"%.*s",ptr-pst,pst); break;
			case 5:RmcInfo.LongitudeEW=*pst; break;
			case 6:snprintf(RmcInfo.Speed,sizeof(RmcInfo.Speed),"%.*s",ptr-pst,pst); break;
			case 7:snprintf(RmcInfo.Azimuth,sizeof(RmcInfo.Azimuth),"%.*s",ptr-pst,pst); break;
			case 8:snprintf(RmcInfo.UtcData,sizeof(RmcInfo.UtcData),"%.*s",ptr-pst,pst); break;
			default:break;
		}
		if ((*ptr==0)||(idx>=8))
			break;
		idx++;
		pst=ptr+1;
	}
exit:
	if (buf)
		free(buf);
	if (RmcInfo.Statue=='A')
	{
		if ((GpsInfo.RMC.Statue!='A')&&(GpsInfo.GGA.Quality<'0'))
			tts_play_set_idx(AUD_ID_GET_GPS_SUCESS,0,0);
//			tts_play_text("卫星定位成功!",GBK_TEXT);

		GpsInfo.RMC=RmcInfo;
	}
		
//	usb_log_printf("%s: RMC INFO read end\n",__func__);
}

void GPS_GGA_INFO(const char* inbuf, int inlen)
{
	char * buf;
	char* ptr;
	char *pst;
	int idx;
	GGA_INFO GgaInfo;
	
	LOG_DBG("%s_%d ===  GGA INFO len %d:%.*s\n", __func__, __LINE__,inlen,inlen,inbuf);
//	LOG_DBG("%s_%d ===  %d:%.*s\n", __func__, __LINE__,inlen,inlen,inbuf);
	buf=malloc(inlen+1);
	if (buf==NULL)
		goto exit;
	memset(buf,0,inlen+1);
	memcpy(buf,inbuf,inlen);
	
//	LOG_DBG("%s_%d ===  %d:%s\n", __func__, __LINE__,strlen(buf),buf);
	ptr=strchr(buf,'*');
	if (ptr==NULL) 
		goto exit;
	*ptr=0;
	
//	LOG_DBG("%s_%d ===  %d:%s\n", __func__, __LINE__,strlen(buf),buf);
	pst=strchr(buf,',');
	if(pst==NULL)
		goto exit;
	pst++;
	
//	LOG_DBG("%s_%d ===  %d:%s\n", __func__, __LINE__,strlen(pst),pst);
	
	idx=0;
	while(1)
	{
		ptr=strchr(pst,',');
		if (ptr==NULL) ptr=pst+strlen(pst);
//	LOG_DBG("%s_%d ===  %d:%p,%p\n", __func__, __LINE__,idx,pst,ptr);
//	LOG_DBG("%s_%d ===  %d:%d,%.*s\n", __func__, __LINE__,idx,ptr-pst,ptr-pst,pst);
		switch(idx)
		{
			case 0:snprintf(GgaInfo.UtcTime,sizeof(GgaInfo.UtcTime),"%.*s",ptr-pst,pst); break;
			case 1:snprintf(GgaInfo.Latitude,sizeof(GgaInfo.Latitude),"%.*s",ptr-pst,pst); break;
			case 2:GgaInfo.LatitudeNS=*pst; break;
			case 3:snprintf(GgaInfo.Longitude,sizeof(GgaInfo.Longitude),"%.*s",ptr-pst,pst); break;
			case 4:GgaInfo.LongitudeEW=*pst; break;
			case 5:GgaInfo.Quality=*pst; break;
			case 6:snprintf(GgaInfo.NumSatUsed,sizeof(GgaInfo.NumSatUsed),"%.*s",ptr-pst,pst); break;
			case 7:break;
			case 8:snprintf(GgaInfo.Altitude,sizeof(GgaInfo.Altitude),"%.*s",ptr-pst,pst); break;
			default:break;
		}
		if ((*ptr==0)||(idx>=8))
			break;
		idx++;
		pst=ptr+1;
	}
exit:
	if (buf)
		free(buf);
	if (GgaInfo.Quality!='0')
	{
		if ((GpsInfo.RMC.Statue!='A')&&(GpsInfo.GGA.Quality<'0'))
			tts_play_set_idx(AUD_ID_GET_GPS_SUCESS,0,0);
//			tts_play_text("卫星定位成功!",GBK_TEXT);
		GpsInfo.GGA=GgaInfo;
	}
		
//	usb_log_printf("%s: GGA INFO read end\n",__func__);
}


void GpsInfoToStr(char * str)
{
	GPS_INFO * info;
	char * ptr;
	
	info=&GpsInfo;
	if ((info->RMC.Statue!='A')&&(info->GGA.Quality<='0'))
	{
		sprintf(str,"定位信息不可用");
		return;		
	}
		
	ptr=str;
	
	ptr +=sprintf(ptr,"当前位置:");
	if (info->RMC.Statue=='A')
	{
		if (info->RMC.LatitudeNS=='N')
			ptr +=sprintf(ptr,"北");
		else if (info->RMC.LatitudeNS=='S')
			ptr +=sprintf(ptr,"南");
		ptr +=sprintf(ptr,"纬%.2s度%s分,",info->RMC.Latitude,info->RMC.Latitude+2);
	
		if (info->RMC.LongitudeEW=='E')
			ptr +=sprintf(ptr,"东");
		else if (info->RMC.LongitudeEW=='W')
			ptr +=sprintf(ptr,"西");
		ptr +=sprintf(ptr,"经%.3s度%s分,",info->RMC.Longitude,info->RMC.Longitude+3);
	}
	else if (info->GGA.Quality!='0')
	{
		if (info->GGA.LatitudeNS=='N')
			ptr +=sprintf(ptr,"北");
		else if (info->GGA.LatitudeNS=='S')
			ptr +=sprintf(ptr,"南");
		ptr +=sprintf(ptr,"纬%.2s度%s分,",info->GGA.Latitude,info->GGA.Latitude+2);
	
		if (info->GGA.LongitudeEW=='E')
			ptr +=sprintf(ptr,"东");
		else if (info->GGA.LongitudeEW=='W')
			ptr +=sprintf(ptr,"西");
		ptr +=sprintf(ptr,"经%.3s度%s分,",info->GGA.Longitude,info->GGA.Longitude+3);
	}

	if (info->GGA.Quality>'0')
	{
		ptr +=sprintf(ptr,"卫星%s颗，高度%s米,",info->GGA.NumSatUsed,info->GGA.Altitude);
	}

	if (info->RMC.Statue=='A')
	{
		ptr +=sprintf(ptr,"速度%s节，航向%s度",info->RMC.Speed,info->RMC.Azimuth);
	}
}

int GpsCtrlInit(void)
{
//	int ret=-1;
	int ii;

	usb_log_printf("%s: start\n",__func__);
    
	//WifiSocketInit();//开机仅操作一次 
	gpio_gps_Pin_init();
	Gps_PwrOnOff(1);
//	gpio_Gps_WKUP(1);
	
	gps_serial_init();//开机仅操作一次
	ql_uart_close(QL_BT_UART_PORT);
	gps_serial_start(QL_BT_UART_PORT, QL_UART_BAUD_9600, QL_UART_DATABIT_8, QL_UART_PARITY_NONE, QL_UART_STOP_1, QL_FC_HW);

//	for(ii=0;ii<1;ii++)
//	{
//		gpio_Gps_Rst(0);
//		ql_rtos_task_sleep_ms(50);
//		gpio_Gps_Rst(1);
//	}

////	Ext_wifi_Off();
//	for(ii=0;ii<5;ii++)
//	{
////		gpio_wifi_power_onoff(0);
////		ql_rtos_task_sleep_ms(50);
////		gpio_wifi_power_onoff(1);
//
//			ret = gps_SendATCmd("AT\r\n", 4,AT_back_handler,NULL,MODULE_CMD_TIMEOUT_NORMAL);
//			usb_log_printf("%s: at ret %d\n",__func__,ret);
//			if (ret==RIL_AT_SUCCESS)
//			{
//				//ret=0;
////				usb_log_printf("%s: ex_wifi check ok\n",__func__);
//				break;
//		}
//	}
	
//	return ret;
}

#endif
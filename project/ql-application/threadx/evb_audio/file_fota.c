#include <stdio.h>
#include "ql_rtos.h"
#include "ql_power.h"
#include "ql_data_call.h"
#include "ql_application.h"
#include "ql_fs.h"
#include "ql_fota.h"
#include "public_api_interface.h"
#include "HTTPClient/HTTPCUsr_api.h"
#include "terminfodef.h"

#if 1
#define DFOTA_PKG_PATH "U:/dfota.bin"
#define FILE_READ_BUF_LEN 1024

#define fota_exam_log usb_log_printf
int FullFotaParaSet(char const * url);

extern bool audio_pause_play;
int Ext_Wifi_AppOta(char * url)
{
	fota_exam_log("========== fota start ==========\r\n");

    qlFotaImgProcCtxPtr ctx = NULL;
    //QFILE * fp = NULL;
    char *file_read_buf = NULL;
    int ret;
    int filesize = 0;
    int file_read_total_len = 0;
    int per=-1;

    uint8_t eof;
    unsigned int recSize;
    HTTPParameters *clientParams = NULL;

    ret = -1;
    file_read_buf = malloc( FILE_READ_BUF_LEN );
    if ( !file_read_buf )
    {
        fota_exam_log( "*** heap memory is not enough ***\r\n" );
        goto exit;
    }

    clientParams = malloc( sizeof( *clientParams ) );
    if ( !clientParams )
        goto exit;
    memset( clientParams, 0, sizeof(HTTPParameters) );
    strcpy( clientParams->Uri, url );

    if( LFS_ERR_OK == ql_access( "U:/FotaFile.bin", 0 ) )
    {
        if (LFS_ERR_OK!=ql_remove("U:/FotaFile.bin"))
        	fota_exam_log("%s line %d,FotaFile.bin removed fail!\r\n",__func__,__LINE__);
        else
        	fota_exam_log("%s line %d,FotaFile.bin removed!\r\n",__func__,__LINE__);
    }

    ctx = ql_fota_init( );
    if ( !ctx )
    {
        fota_exam_log( "*** fota init fail ***\r\n" );
        goto exit;
    }

	audio_pause_play = true;

    eof = 0;
    while ( 1 )
    {
        recSize=0;
        ret = HTTPC_get( clientParams, (CHAR*) file_read_buf, (INT32) FILE_READ_BUF_LEN, (INT32*) &recSize );
        if ( ret == HTTP_CLIENT_SUCCESS )
            eof = 0;
        else if ( ret == HTTP_CLIENT_EOS )
            eof = 1;
        else
        {
            ret = -1;
            break;
        }
        if ( recSize )
        {
            file_read_total_len += recSize;
            ret = ql_fota_image_write( ctx, (void*) file_read_buf, recSize );
            if ( ret )
            {
                fota_exam_log( "*** fota image write fail ***\r\n" );
				ret = -2;
                goto exit;
            }
			if (filesize==0)
			{
				HTTP_PARAM      HTTPParam;                                  // A generic pointer\length parameter for parsing
				if (HTTPIntrnHeadersFind((P_HTTP_SESSION)clientParams->pHTTP,"content-length",&HTTPParam,TRUE,0)==HTTP_CLIENT_SUCCESS)
				{
					//char buf[30];
					//snprintf(buf,sizeof(buf),"%.*s",(int)HTTPParam.nLength,HTTPParam.pParam);
					fota_exam_log("found headin:%.*s\r\n",(int)HTTPParam.nLength,HTTPParam.pParam);
					sscanf(HTTPParam.pParam,"%*[^0-9]%d",&filesize);
				}
				else
				{
					printf("content-length not found!\r\n");
					filesize=700*1024;
				}
				fota_exam_log("filesize:%d\r\n",filesize);
			}

			if (per!=file_read_total_len * 100/filesize)
			{
				per=file_read_total_len * 100/filesize;
				if (per>100) per=100;
				disp_set_ota_state(per);
				ql_rtos_task_sleep_ms(20);//����ʾ
			}
        }

        if ( eof )
        {
            fota_exam_log( "%s_%d =========eof ok\n", __func__, __LINE__ );
            break;
        }
    }

    fota_exam_log( "%s_%d =========%d\n", __func__, __LINE__, file_read_total_len );
    if ( ( eof ) && ( file_read_total_len ) )
    {
        ret = ql_fota_image_flush( ctx );
        if ( ret )
        {
            fota_exam_log( "*** fota image flush fail ***\r\n" );
			ret = -3;
            goto exit;
        }

        disp_set_ota_state(100);
        ql_rtos_task_sleep_ms(100);
        fota_exam_log( "fota image write done, verifing ...\r\n" );

        ret = ql_fota_image_verify( ctx );
        if ( ret )
        {
            fota_exam_log( "*** fota image verify fail ***\r\n" );
			ret = -3;
            goto exit;
        }

        fota_exam_log( "fota image verify done, will restart to update ...\r\n" );

//		ql_power_reset();
    }

    if ( ret < 0 )
    {
        fota_exam_log( "*** dfota pkg file read fail ***\r\n" );
        goto exit;
    }
	
	
exit:
	audio_pause_play = false;
//		fota_exam_log("%s_%d =========\n", __func__, __LINE__);
    HTTPC_close( clientParams );
	if(ctx) ql_fota_deinit(ctx);
	if(file_read_buf) free(file_read_buf);
	if(clientParams) free(clientParams);
	fota_exam_log("========== fota end ==========\r\n");
	return ret;
}

#else

#define DFOTA_PKG_PATH "U:/dfota.bin"
#define FILE_READ_BUF_LEN 1024

#define fota_exam_log usb_log_printf
//#define FOTAHTTPSCUT

#define FotaInfoFileName	"U:/filefotainfo.txt"
//#define FOTA_SERVER_ADDRESS_STEP1 "http://112.31.84.164:8300/august/old_new.bin_1"
//#define FOTA_SERVER_ADDRESS_STEP2 "http://112.31.84.164:8300/august/old_new.bin_2"
//#define FOTA_FTP_SERVER_USERNAME "quec"
//#define FOTA_FTP_SERVER_PASSWD   "quec"
#define PROFILE_IDX 1

#define LOG_INFO(fmt, arg...) printf("[FOTA INFO]"fmt, ##arg)

enum{
	FotaSetLinIdx=0,
	FotaBakUrlLinIdx,
}FotaLinDef;

char * otainfo=NULL;

char const * FotaGetLine(char const * sbuf,int idx)
{
	char const * ptr;
	while(idx)
	{
		ptr=strstr(sbuf,"\r\n");
		if (ptr==NULL)
			return NULL;
		sbuf=ptr+2;
		idx--;
	}
	return sbuf;
}


int FileFotaInfoLoad(void)
{
	char const * const path = FotaInfoFileName;
	QFILE * fp = NULL;
	int filesize;
	void * * set;
	char * ptr;
	int rlen = -1;
	int ret=-1;

	LOG_INFO("%s_%d =========\n", __func__, __LINE__);
	if( LFS_ERR_OK != ql_access( path, 0 ) )
		goto err;

	fp = ql_fopen(path, "r");
	if (fp == NULL) 
		goto err;

	filesize = ql_fsize(fp);
//	otainfo=calloc(filesize,1);
	otainfo=malloc(filesize+1);
	if (otainfo==NULL)
		goto err;

	memset(otainfo,0,filesize+sizeof(*otainfo));
	rlen = ql_fread((void *)otainfo, filesize, 1, fp);
//	LOG_INFO( "%s ,line %d:%s\n", __func__, __LINE__,sbuf);
	ql_fclose(fp);
	if ((rlen < 0) ||(rlen>filesize))
	{
		free(otainfo);
		otainfo=NULL;
		goto err;	
	}

	ret=0;
err:
	LOG_INFO("%s_%d =========\n", __func__, __LINE__);
//	fota_exam_log("%s line %d\r\n",__func__,__LINE__);
	return ret;
}

int FileFotaParamChk(void)
{
	char const * const path = FotaInfoFileName;
	QFILE * fp = NULL;
	char sbuf[128];
	char * ptr;
	int rlen = -1;
	int ret=-1;
	int OtaResult;
	int type;

	if( LFS_ERR_OK != ql_access( path, 0 ) )
		goto err;

	fp = ql_fopen(path, "r");
	if (fp == NULL) 
		goto err;

	memset(sbuf,0,sizeof(sbuf));
	rlen = ql_fread((void *)sbuf, sizeof(sbuf), 1, fp);
//	LOG_INFO( "%s ,line %d:%s\n", __func__, __LINE__,sbuf);
	ql_fclose(fp);
	
	ql_remove(path);
	
	if (rlen < 0) 
		goto err;	
		
	ptr=sbuf;
	ptr=strchr(ptr,',');
	if (ptr==NULL)
		goto err;
	ptr=strchr(ptr+1,',');
	if (ptr==NULL)
		goto err;

	OtaResult=-1;
	sscanf(sbuf, "%d,", &type);
	sscanf(ptr+1, "%d", &OtaResult);

	LOG_INFO("%s_%d =========%s,%d,%d\n", __func__, __LINE__,sbuf,type,OtaResult);
	if (type==0)
	{
		if (TermInfo.NewVerFirstRun)
			ret=0;
		else
			ret=1;
	}
	else
	{
		if (OtaResult==0)
			ret=0;
		else 
			ret=1;
	}

err:
	
	return ret;
}

//??????????????
//const char * FileFotaPlaySetStr="http://yunmazhineng.com/firmware/test/mid/4GW_V91_107_R07A02_OTA.bin";
char const * FileFotaPlaySetStr="\r\n"; 	//???????URL
//					"\r\n"; 		//????????,??????

//OTA????????OTA???????APP???????OTA??
//??????????????????????APP????
//????OTA????????-1????,??URL?????OTA??????APP???????????????
//??????URL,????????
int FileFotaParamSet(int type)
{
	char const * const path = FotaInfoFileName;
	QFILE * fp = NULL;
	char sbuf[128] = "";
	char * ptr;
//	int ret = -1;

//	LOG_INFO("%s_%d =========\n", __func__, __LINE__);
	fp = ql_fopen(path, "w");
	if (fp == NULL) 
		goto err;

	sprintf(sbuf,"%d,%d,%d\r\n",type,3,3);//OTA???OTA???????(-1???),??URL????
	ql_fwrite(sbuf, strlen(sbuf), 1, fp);
//	ql_fwrite(FileFotaPlaySetStr, strlen(FileFotaPlaySetStr), 1, fp);
//	if (ql_fwrite(FileFotaPlaySetStr, strlen(FileFotaPlaySetStr), 1, fp) >0 ) 
//		ret=0;

//	LOG_INFO("%s_%d =========\n", __func__, __LINE__);
	ql_fclose(fp);
err:
	LOG_INFO("%s_%d =========\n", __func__, __LINE__);
//	fota_exam_log("%s line %d,ret=%d\r\n",__func__,__LINE__,ret);
	return 0;
}

//MiniFota,???4G??
int MiniFotaParaSet(char const * Step1,char const * Step2)
{
	int ret = -1;
#ifdef FOTAHTTPSCUT
	char * url1;
	char * url2;
	url1=malloc(strlen(Step1)+1);
	url2=malloc(strlen(Step2)+1);
	strcpy(url1,Step1);
	if ( (url1[4]=='s')||(url1[4]=='S'))
		memmove(url1+4,url1+5,strlen(url1+5)+1);
	strcpy(url2,Step2);
	if ( (url2[4]=='s')||(url2[4]=='S'))
		memmove(url2+4,url2+5,strlen(url2+5)+1);
	Step1=url1;
	Step2=url2;

	fota_exam_log("\turl1: %s\n", url1);
	fota_exam_log("\turl2: %s\n", url2);
#endif
//	if((TermInfo.NetMode == WIFI_MODE)||(TermInfo.NetMode == WIFI_BAKE_MODE))
//			return -2;
	if((TermInfo.NetMode != GPRS_MODE)&&(TermInfo.NetMode != GPRS_BAKE_MODE)) 
	{
		return; // ???GPRS???????????
	}

	fota_exam_log("========== MiniFota para set ==========\r\n");
//	fota_exam_log("url1:%s\r\n",Step1);
//	fota_exam_log("url2:%s\r\n",Step2);
//	if (0!= ql_fota_mini_system(PROFILE_IDX, QL_FOTA_DWNLD_MOD_HTTP, FOTA_SERVER_ADDRESS_STEP1 ,FOTA_SERVER_ADDRESS_STEP2, NULL,NULL))

	FileFotaParamSet(0);
	if (0!= ql_fota_mini_system(PROFILE_IDX, QL_FOTA_DWNLD_MOD_HTTP, Step1 ,Step2, NULL,NULL))
	{
		fota_exam_log("========== MiniFota para set fail!==========");
//		return -1;
	}
	else
		fota_exam_log("========== MiniFota para set ok!==========");

	ret =0;
exit:	
#ifdef FOTAHTTPSCUT
	free(url1);
	free(url2);
#endif
	return ret;
}

//#define TestUrl "https://asset.echase.cn/tms-app-test/CS28/CS684GW_23_jxyc_2023030702.bin"
//MiniFota,???4G??
int FullFotaParaSet(char const * url)
{
//	url=TestUrl;
#ifdef FOTAHTTPSCUT
	char * url1;
	url1=malloc(strlen(url)+1);
	strcpy(url1,url);
	if ( (url1[4]=='s')||(url1[4]=='S'))
		memmove(url1+4,url1+5,strlen(url1+5)+1);
	url=url1;

	fota_exam_log("\turl: %s\n", url);
#endif
	if((TermInfo.NetMode != GPRS_MODE)&&(TermInfo.NetMode != GPRS_BAKE_MODE)) 
	{
		return; // ???GPRS???????????
	}

	FileFotaParamSet(1);
	if (0!= ql_fullfota_app_start(url))
	{
		fota_exam_log("========== Full Fota para set fail!==========");
#ifdef FOTAHTTPSCUT
	free(url);
#endif
		return -1;
	}
	else
		fota_exam_log("========== Full Fota para set ok!==========");
	
#ifdef FOTAHTTPSCUT
	free(url);
#endif
		
	return 0;
}

#endif



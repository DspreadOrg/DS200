#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_type.h"
#include "ql_rtos.h"
#include "prj_common.h"
#include "httplayerjson.h"
#include "base64.h"
#include "cjson.h"
#include "systemparam.h"
#include "public_api_interface.h"
#include "tts_yt_task.h"
#include "mqtt_aliyun.h"
#include "play_receipt.h"
#include "terminfodef.h"
//#include "platform.h"
//#include "md_internal.h"
#include "HTTPClient/HTTPCUsr_api.h"
#include "security.h"


#define HTTPLAYERJSON_DEBUG(fmt, arg...) //usb_log_printf("[[DBG HTTP]"fmt, ##arg)
#define HTTPLAYERJSON_INFOR(fmt, arg...) usb_log_printf("[INFO HTTP]"fmt, ##arg)


#define cJSON_free free
#define PRODUCTTYPE "CS10C"
#define ACKOK "000000"

extern char firmware_version[50];
extern char g_kernel_version[20];

uint32_t msg_nonce = 100001;
char SendBuf[HTTPTEXTMAXSIZE] = {0};
//static int http_receive_len = 0;

void BcdToAsc(unsigned char const *bcd, char *asc, int asc_len )
{
   	uint8_t is_first;
   	uint8_t by;
   	uint16_t ii,jj;
	
	
   	is_first =(asc_len % 2); 
   	
	for (ii=0,jj=0;ii<asc_len;ii++) 
	{
		by=bcd[jj];
		if (is_first) 
		{
			jj++;
			by &=0x0f;
		}
		else 
		{
			by>>=4;
		}
		
		if (by>9) 
		{
		    by +='A'-0x0a-'0';
		}
		asc[ii]=by+'0';		
		
		is_first = !is_first;
		
	}	
}

void AscToBcd(const char *asc, unsigned char *bcd,int asc_len )	
{
	char by;
	uint8_t is_high,Set;

	Set = asc_len % 2;
	is_high=1;
	
	while(asc_len-->0)
	{
		by = *asc++;
		if ( !(by&0x10)&&(by>0x30)) 
		{
		    by += 9;
		}
		if (is_high)  
		{
		    *bcd = by << 4;
		}
		else
		{
			by&=0x0f;
			*bcd++ |= by;
		}
		is_high = !is_high;
	}
	if (Set) 
	{
	    *bcd|=0xf;
	}    
}

// 16����ת�ַ�
uint8_t HexToChar(uint8_t temp)
{    
	uint8_t dst;    
	if (temp < 10)
	{        
		dst = temp + '0';    
	}
	else
	{        
		dst = temp -10 +'A';   
	}    
	return dst;
}

// 16����ת�ַ���
uint8_t HexToStr(uint8_t * src, uint8_t len,char * des)
{
	uint8_t i=0;
	char *str = calloc(1,len*2);
	if( str == NULL )
	{
	    return 0;
	}
	for(i=0; i<len; i++)
	{
		str[2*i] = src[i]>>4;
        str[2*i+1] = src[i]&0xf;
	}
	for(i = 0; i<len*2; i++)    
	{        
		des[i] = HexToChar(str[i]);    
	}         
	free(str);
	return len*2;
}

// �Խӽӿ�������Ϣ����
char * XR871_Enctry(char * buf)
{
	int len;
	uint8_t MD5_buf[17] = {0};
	char MD5_str_buf[48] = {0};
	unsigned char *out_encrypt = NULL;
	unsigned char * base64_buf = NULL;
	int base64_len = 0;

	md5_encrypt(buf, strlen(buf), MD5_buf);
	HexToStr(MD5_buf, 16, MD5_str_buf);
    #if 0
		xr871ƽ̨block_size = 16
	������������ݳ���Ϊx����ô�����ں���padding���ֽ���ĿΪblock_size-(x%block_size)��ÿ��padding���ֽ�ֵ��block_size-(x%block_size)��
	�ر�أ������������ݳ���xǡ����block_size����������Ҳ��Ҫ�ں��������block_size���ֽڣ�ÿ���ֽ���0x(block_size)��
	ԭ�ģ�https://blog.csdn.net/test1280/article/details/75268255 
	#endif

	out_encrypt = aes_private_encrypt(MD5_str_buf, strlen(MD5_str_buf), &len);
	base64_buf = base64_encode((unsigned char *)out_encrypt, len, &base64_len);
	base64_buf[base64_len-1] = 0;

	if (out_encrypt != NULL)
	{
		free(out_encrypt);
		out_encrypt = NULL;
	}

	return (char *)base64_buf;
}

void num_str_func(int n,char s[])
{
	int i = 0;
	int temp = 0;
	int m;
	while(n>0)
	{
		s[i++] = n%10 + '0';//��Ҫ����������ַ�֮���ת����Ҫ֪��һ��֪ʶ�� ����+��0�� = �����֡����ַ������������֡� - ��0�� = ����   ��
		n = n / 10;
	}
	s[i] = '\0';

    for(m=0;m<i/2;m++)
    {
         temp=s[m];
         s[m]=s[(i-1)-m];
         s[(i-1)-m]=temp;
    }	
}

// �������ݽ���
static int DeviveRegisterDecrypt(char * srcbuf, char * desbuf)
{
	unsigned char * aes_desbuf = NULL;
	unsigned int out_len = 0;
    unsigned char *out_decrypted = NULL;
	int ret = 0;

	if ((srcbuf == NULL) || (desbuf == NULL))
	{
		return -1;
	}

	//HTTPLAYERJSON_INFOR("enter <<%s>> ,line %d\n", __func__, __LINE__, srcbuf);
	aes_desbuf = base64_decode((unsigned char *)srcbuf, strlen(srcbuf), &ret);
	out_decrypted = aes_private_decrypt(aes_desbuf, ret, &out_len);
	if (out_decrypted == NULL)
	{
		return -1;
	}

	memcpy(desbuf, out_decrypted, out_len);
	
	//HTTPLAYERJSON_INFOR("desbuf str: %s \n", desbuf);
	if (aes_desbuf != NULL)
	{
		free(aes_desbuf);
	}
	
	if (out_decrypted != NULL)
	{
		free(out_decrypted);
	}
	HTTPLAYERJSON_INFOR( "exit <<%s>> ,line %d\n", __func__, __LINE__);
	return ret;
}
//char YSW1_messageId[50] = {0};
//#if 1
//char YSW1_voiceMsg[200] = {0};
//#else
//char * YSW1_voiceMsg = NULL;
//#endif
//char YSW1_notifyUrl[256] = {0};
//char lcd_money_number[20] = {0};

uint32_t msg_back_addr_analy(const char * resbuf, char * desbuf)
{
	char buf[80];
	char * ptr;
	int port=80;
	
	memset(buf,0,sizeof(buf));
	memcpy(buf,resbuf,79);
	strupr(buf);
	ptr=strstr(buf,"HTTPS://");
	if (ptr) 
	{
		*desbuf++=0x1;
		ptr +=8;
		port=443;
	}
	else
	{
		*desbuf++=0x0;
		ptr=strstr(buf,"HTTP://");
		if (ptr) ptr+=7;
		else 
		{
			ptr=strstr(buf,"//");
			if (ptr) ptr +=2;
			else ptr=buf;
		}
	}
	resbuf +=ptr-buf;
	memcpy(buf,resbuf,79);
//	strupr(buf);
	usb_log_printf("%s,line %d,buf %s\n",__func__,__LINE__,buf);
	ptr=strstr(buf,"/");
	if (ptr) *ptr=0;
	usb_log_printf("%s,line %d,buf %s\n",__func__,__LINE__,buf);

	ptr=strstr(buf,":");
	if (!ptr) strcpy(desbuf,buf);
	else 
	{
		*ptr=0;
		strcpy(desbuf,buf);
		while(*++ptr<=0x20);
		port=0;
		while(1)
		{
			if ((*ptr<'0')||(*ptr>'9')) break;
			port *=10;
			port +=*ptr++&0xf;
		}
		if (port==0) port=80;
	}
//	TRACE_printf("port %d\r\n",port);
	usb_log_printf("%s,line %d,buf %s,port %d\n",__func__,__LINE__,buf,port);
	return port;
}

static VOID HTTPDebug(const CHAR* FunctionName,const CHAR *DebugDump,UINT32 iLength,CHAR *DebugDescription,...) // Requested operation
{

    va_list            pArgp;
    char               szBuffer[2048];

    memset(szBuffer,0,2048);
    va_start(pArgp, DebugDescription);
    vsprintf((char*)szBuffer, DebugDescription, pArgp); //Copy Data To The Buffer
    va_end(pArgp);

    printf("%s %s %s\n", FunctionName,DebugDump,szBuffer);
}

int HTTPC_PostB(HTTPParameters *ClientParams, const char * const * HeadAdd,CHAR *Buffer,INT32 bufSize, unsigned int *recvSize)
{
#if 1
	int ret = 0;
	UINT32 ReadLength ;//= * recvSize;
	HTTP_CLIENT httpClient;
	char const * HeadName;
	char const * HeadData;
	//char sendbufprn[500];
	
	memset((void *)&httpClient, 0, sizeof(httpClient));

	*recvSize=0;

	ClientParams->HttpVerb = VerbPost;

request:
	if ((ret = HTTPC_open(ClientParams)) != 0) {
		printf("HTTPC_open err\n");
		goto relese;
	}
#ifdef _HTTP_DEBUGGING_
					HTTPClientSetDebugHook(ClientParams->pHTTP,&HTTPDebug);
#endif
	if (HeadAdd!=NULL)
		while(1)
	{
		HeadName=*HeadAdd++;
		HeadData=*HeadAdd++;
		if ((HeadName==NULL)||(HeadData==NULL)) break;
		if((ret = HTTPIntrnHeadersAdd((P_HTTP_SESSION)(ClientParams->pHTTP),(char *)HeadName,strlen(HeadName),
			(char *)HeadData,strlen(HeadData)))!= HTTP_CLIENT_SUCCESS)
		{
			goto relese;
		}
	}
/*	
	if((ret = HTTPIntrnHeadersAdd((P_HTTP_SESSION)(ClientParams->pHTTP),"X-Login-Name",12,sysparam_get()->device_SN,strlen(sysparam_get()->device_SN)))!= HTTP_CLIENT_SUCCESS)
	{
		goto relese;
	}
	if((ret = HTTPIntrnHeadersAdd((P_HTTP_SESSION)(ClientParams->pHTTP),"X-Client-PV",11,PRODUCTTYPE,strlen(PRODUCTTYPE)))!= HTTP_CLIENT_SUCCESS)
	{
		goto relese;
	}
	if((ret = HTTPIntrnHeadersAdd((P_HTTP_SESSION)(ClientParams->pHTTP),"X-Client-Bus-Ver",16,firmware_version,strlen(firmware_version)))!= HTTP_CLIENT_SUCCESS)
	{
		goto relese;
	}
	*/
	//printf("ClientParams->pHTTP:%s\n",(char*)ClientParams->pHTTP);

	if ((ret = HTTPC_request(ClientParams, NULL)) != 0) {
//		CMD_ERR("http request err..\n");
		printf("HTTPC_request err=%d\n\n",ret);
		goto relese;
	}

	printf("send total:%ld\n",httpClient.RequestBodyLengthSent);
	if ((ret = HTTPC_get_request_info(ClientParams, &httpClient)) != 0) {
//		CMD_ERR("http get request info err..\n");
		printf("HTTPC_get_request_info err\n");
		goto relese;
	}

	printf("recv total:%ld\n",httpClient.HTTPStatusCode);
	if (httpClient.HTTPStatusCode != HTTP_STATUS_OK) {
		if((httpClient.HTTPStatusCode == HTTP_STATUS_OBJECT_MOVED) ||
				(httpClient.HTTPStatusCode == HTTP_STATUS_OBJECT_MOVED_PERMANENTLY)) {
//			CMD_DBG("Redirect url..\n");
			HTTPC_close(ClientParams);
			memset(ClientParams, 0, sizeof(*ClientParams));
			ClientParams->HttpVerb = VerbGet;
			if (httpClient.RedirectUrl->nLength < sizeof(ClientParams->Uri))
				strncpy(ClientParams->Uri, httpClient.RedirectUrl->pParam,
						httpClient.RedirectUrl->nLength);
			else
				goto relese;
//			CMD_DBG("go to request.\n");
			goto request;

		} else {
			ret = -1;
			printf("get result not correct.\n");
//			CMD_DBG("get result not correct.\n");
			goto relese;
		}
	}
//	if (httpClient.TotalResponseBodyLength != 0 || (httpClient.HttpFlags & HTTP_CLIENT_FLAG_CHUNKED )) {
		do {
//			memset(buf, 0, 4096);
			if ((ret = HTTPC_read(ClientParams, Buffer, bufSize, &ReadLength)) != 0) {
				//CMD_DBG("get data,Received:%d\n",Received);
				if (ret == 1000) {
					*recvSize +=ReadLength;
					ret = 0;
				}
				break;
			}
			Buffer +=ReadLength;
			bufSize -=ReadLength;
			*recvSize +=ReadLength;
		} while (1);
//	}
relese:
{
		P_HTTP_SESSION pHTTP=(P_HTTP_SESSION)ClientParams->pHTTP;
		if (pHTTP)
		{
			printf("\r\n---HeadersOut:%.*s\r\n",(int)pHTTP->HttpHeaders.HeadersOut.nLength,pHTTP->HttpHeaders.HeadersOut.pParam);
			printf("\r\n---httpheadin:%.*s\r\n",(int)pHTTP->HttpHeaders.HeadersIn.nLength,pHTTP->HttpHeaders.HeadersIn.pParam);
		}
#if 0
		HTTP_PARAM      HTTPParam;                                  // A generic pointer\length parameter for parsing
		//HTTPClientFindCloseHeader
		//HTTPClientFindFirstHeader
		//HTTPClientGetNextHeader
		if (HTTPIntrnHeadersFind((P_HTTP_SESSION)ClientParams->pHTTP,"resp_code",&HTTPParam,TRUE,0)==HTTP_CLIENT_SUCCESS)
			printf("\r\n----httpinfo:%.*s\r\n",(int)HTTPParam.nLength,HTTPParam.pParam);
		else
			printf("\r\n---notfound\r\n");
		if (HTTPIntrnHeadersFind((P_HTTP_SESSION)ClientParams->pHTTP,"content-length",&HTTPParam,TRUE,0)==HTTP_CLIENT_SUCCESS)
			printf("\r\n----httpinfo:%.*s\r\n",(int)HTTPParam.nLength,HTTPParam.pParam);
		else
			printf("\r\n---notfound\r\n");
#endif
}
//	free(buf);
	HTTPC_close(ClientParams);
	return ret;
#else
	return 0;
#endif
}
int Gprs_HTTPC_PostB(HTTPParam *httpinfo, const char * const * HeadAdd,CHAR *Buffer,INT32 bufSize, unsigned int *recvSize)
{
	int ret=-1;
		HTTPParameters *clientParams=NULL;
		clientParams = malloc(sizeof(*clientParams));
		if (!clientParams) return -1;
		memset(clientParams,0,sizeof(HTTPParameters));
		strcpy(clientParams->Uri,httpinfo->url);
	//	clientParams->HttpVerb = VerbPost;
		clientParams->pData = httpinfo->out;
		clientParams->pLength = httpinfo->outlen;
		ret = HTTPC_PostB(clientParams,HeadAdd,Buffer, bufSize, recvSize);
		if (clientParams)
			free(clientParams);
		if ((ret != HTTP_CLIENT_SUCCESS) && (ret != HTTP_CLIENT_EOS))
			return -2;
		else return 0;
}

//static int  device_sign_data_parse();
int device_sign(uint8_t net_type)
{
	cJSON *root = NULL;
	cJSON * psub;
	cJSON * item;
	char * out = NULL;
	
	int ret = -1;
	char buf[200];
	unsigned int len = 0;
		
	struct sysparam * ysw1_sysparam;
	ysw1_sysparam = sysparam_get();

	root = cJSON_CreateObject();
	if(!root) return -1;
  	
	char raw_buf[64] = {0};
	char * base64_buf;
	char time_ts_buf[20] = {0};
	char msg_nonce_buf[10] = {0};
	uint32_t time_ts = read_rtc_time();

	sprintf(time_ts_buf,"%d",time_ts);
	sprintf(msg_nonce_buf,"%d",msg_nonce++);
	sprintf(raw_buf, "%s%s%s", ysw1_sysparam->device_SN, time_ts_buf, msg_nonce_buf);
	base64_buf = XR871_Enctry(raw_buf);
	
	cJSON_AddItemToObject(root, "deviceSn", cJSON_CreateString(ysw1_sysparam->device_SN));
	cJSON_AddItemToObject(root, "ts", cJSON_CreateString(time_ts_buf));
	cJSON_AddItemToObject(root, "nonce", cJSON_CreateString(msg_nonce_buf));
	cJSON_AddItemToObject(root, "sign", cJSON_CreateString((const char *)base64_buf));
	free(base64_buf);

	out=cJSON_Print(root);
	//out=cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	root=NULL;
	if(!out) return -1;

	HTTPLAYERJSON_DEBUG("--------%s,line %d,url %s,send len %d:%s\r\n",__func__,__LINE__, \
						ysw1_sysparam->server_url,strlen(out),out);

	memset(SendBuf,0,sizeof(SendBuf));
		const char * const HeadAdd[]=
		{
			"Content-Type","application/json",
			"X-Login-Name",sysparam_get()->device_SN,
			"X-Client-PV",PRODUCTTYPE,
			"X-Client-Bus-Ver",firmware_version,
			"Connection","keep-alive",
			NULL,
		};
		HTTPParam httpinfo;
		httpinfo.url=malloc(strlen(sysparam_get()->server_url)+1);
		if (httpinfo.url==NULL)
			return -1;
		strcpy(httpinfo.url,sysparam_get()->server_url);
		httpinfo.out = out;
		httpinfo.outlen = strlen(out);
		
		ret = Gprs_HTTPC_PostB(&httpinfo,HeadAdd,(CHAR *)SendBuf, (INT32)sizeof(SendBuf), &len);

		free(out);
		free(httpinfo.url);
		if (ret) return -1;
		SendBuf[len]=0;	
	usb_log_printf("%s_%d === %d,%d\n", __func__, __LINE__,ret,len);
{
	int len2;
	char * ptr1=SendBuf;
	char * ptr2=SendBuf+len;
	HTTPLAYERJSON_INFOR("--------%s,line %d\r\n",__func__,__LINE__);
	while(ptr1<ptr2)
	{
		len2=ptr2-ptr1;
		if (len2>500) len2=500;
		HTTPLAYERJSON_INFOR("%.*s\r\n",len2,ptr1);
		ptr1 +=len2;
	}
	HTTPLAYERJSON_INFOR("--------%s,line %d\r\n",__func__,__LINE__);
}
	root=cJSON_Parse(SendBuf);
	if (root==NULL)
		return -3;
	HTTPLAYERJSON_DEBUG( "line %d  NULL!=root\n", __LINE__);
	psub = cJSON_GetObjectItem(root,"retCode");
	if ((NULL==psub)||(psub->type!=cJSON_String)) 
	{
		cJSON_Delete(root);
		return -3;
	}	

	HTTPLAYERJSON_DEBUG( "%s_%d ===retCode = %s\n", __func__, __LINE__, psub->valuestring);
	if (strcmp(psub->valuestring, ACKOK))
	{
		// ��Ӧ�벻��
		sprintf(buf,"������%s",psub->valuestring);
		//tts_play_text(buf, GBK_TEXT);
		psub = cJSON_GetObjectItem(root, "retMsg"); 		
		if ((NULL==psub)||(psub->type!=cJSON_String)) 
		;//	sprintf(buf,"ע��ʧ��");
		else
		{
			sprintf(buf,",%s",psub->valuestring);
			//tts_play_text(buf, UTF8_TEXT);
		}	
	
		cJSON_Delete(root);
		return 1;
	}
	
	psub = cJSON_GetObjectItem(root, "retMsg");			
	if ((NULL==psub)||(psub->type!=cJSON_String)) 
	{
		cJSON_Delete(root);
		return -3;
	}	
	
	item = cJSON_GetObjectItem(root, "retData");
	if (NULL==item) 
	{
		cJSON_Delete(root);
		return -3;
	}	

	psub=cJSON_GetObjectItem(item,"deviceInfo");	//if (NULL==psub) return -DealConnOver;
	if ((NULL==psub)||(psub->type!=cJSON_String)) 
	{
		cJSON_Delete(root);
		return -3;
	}	

	DeviveRegisterDecrypt(psub->valuestring, SendBuf);
	cJSON_Delete(root);
	
	root=cJSON_Parse(SendBuf);
	if(!root) return -5;
	HTTPLAYERJSON_DEBUG("--------%s,line %d,len %d,%s\r\n",__func__,__LINE__,len,SendBuf);
	root=cJSON_Parse(SendBuf);
	psub = cJSON_GetObjectItem(root, "productKey");			
	if ((NULL==psub)||(psub->type!=cJSON_String)) 
	{
		cJSON_Delete(root);
		return -5;
	}	
	memcpy(ysw1_sysparam->product_key, psub->valuestring, strlen(psub->valuestring)+1);
	HTTPLAYERJSON_DEBUG("--%s, Key: %s\r\n",__func__,psub->valuestring);

	psub = cJSON_GetObjectItem(root, "deviceName");			
	if ((NULL==psub)||(psub->type!=cJSON_String)) 
	{
		cJSON_Delete(root);
		return -5;
	}	
	memcpy(ysw1_sysparam->device_name, psub->valuestring, strlen(psub->valuestring)+1);
	HTTPLAYERJSON_DEBUG("--%s, device_name: %s\r\n",__func__,psub->valuestring);

	psub = cJSON_GetObjectItem(root, "deviceSecret");			
	if ((NULL==psub)||(psub->type!=cJSON_String)) 
	{
		cJSON_Delete(root);
		return -5;
	}	
	memcpy(ysw1_sysparam->device_secret, psub->valuestring, strlen(psub->valuestring)+1);	
	HTTPLAYERJSON_DEBUG("--%s, secret: %s\r\n",__func__,psub->valuestring);

	cJSON_Delete(root);

    return 0;
}

//  ������ִ
int DevicePlayResultSend(char PlayflagOk, uint8_t net_type, struct play_receipt_msg recvMsg)
{
	cJSON *root=NULL;
	cJSON * psub;
	char * out=NULL;
	int ret = -1;
	char buf[200];
	char time_ts_buf[20] = {0};
	char msg_nonce_buf[10] = {0};	
	unsigned int len;
	char * base64_buf;
		
	uint32_t time_ts = read_rtc_time();
	num_str_func((int)time_ts, time_ts_buf);
	num_str_func(msg_nonce, msg_nonce_buf);
	msg_nonce++;
	
	memset(SendBuf, 0,sizeof(SendBuf)/sizeof(SendBuf[0]));
	sprintf(buf, "%s%d%d%s", recvMsg.msgid_buf, PlayflagOk, time_ts, msg_nonce_buf);
	base64_buf = XR871_Enctry(buf);
	
	root = cJSON_CreateObject();
	if(!root) return -1;
	
	cJSON_AddItemToObject(root, "messageId", cJSON_CreateString(recvMsg.msgid_buf));
	cJSON_AddItemToObject(root, "pushStatus", cJSON_CreateNumber(PlayflagOk?1:0));
	cJSON_AddItemToObject(root, "deviceVer", cJSON_CreateString(firmware_version));
	cJSON_AddItemToObject(root, "ts", cJSON_CreateString(time_ts_buf));
	cJSON_AddItemToObject(root, "nonce", cJSON_CreateString(msg_nonce_buf));
	cJSON_AddItemToObject(root, "sign", cJSON_CreateString((const char *)base64_buf));
	free(base64_buf);
	out=cJSON_Print(root);
	cJSON_Delete(root);
	if(!out) return -1;

    //HTTPLAYERJSON_INFOR("--------%s,line %d,url %s,send len %d:%s\r\n",__func__,__LINE__,ysw1_sysparam->server_url,strlen(out),out);

    const char * const HeadAdd[]=
    {
        "Content-Type","application/json",
        "X-Login-Name",sysparam_get()->device_SN,
        "X-Client-PV",PRODUCTTYPE,
        "X-Client-Bus-Ver",firmware_version,
        "Connection","keep-alive",
        NULL,
    };
    HTTPParam httpinfo;
    httpinfo.url=malloc(strlen(recvMsg.url_buf)+1);
    if (httpinfo.url==NULL)
        return -1;
    strcpy(httpinfo.url,recvMsg.url_buf);
    httpinfo.out = out;
    httpinfo.outlen = strlen(out);

    ret = Gprs_HTTPC_PostB(&httpinfo,HeadAdd,(CHAR *)SendBuf, (INT32)sizeof(SendBuf), &len);

    free(out);
    free(httpinfo.url);
    if (ret) return -1;
    SendBuf[len]=0;

#if 0
{
	int len2;
	char * ptr1=SendBuf;
	char * ptr2=SendBuf+len;
    //HTTPLAYERJSON_INFOR("SendBuf:%s,line %d\r\n",SendBuf,__LINE__);
	while(ptr1<ptr2)
	{
		len2=ptr2-ptr1;
		if (len2>500) len2=500;
		HTTPLAYERJSON_INFOR("%.*s\r\n",len2,ptr1);
		ptr1 +=len2;
	}
    HTTPLAYERJSON_INFOR("--------%s,line %d\r\n",__func__,__LINE__);
}
#endif
	root=cJSON_Parse(SendBuf);
	if (root==NULL)
	{
	    HTTPLAYERJSON_DEBUG( "%s: invalid rsp data\n", __func__);
		return -3;
	}

	psub = cJSON_GetObjectItem(root,"retCode");
	if ((NULL==psub)||(psub->type!=cJSON_String)) 
	{
        HTTPLAYERJSON_INFOR("--------%s,line %d\r\n",__func__,__LINE__);
		cJSON_Delete(root);
		return -3;
	}	
	if (strcmp(psub->valuestring, ACKOK))
	{
        HTTPLAYERJSON_INFOR("--------%s,line %d\r\n",__func__,__LINE__);
		// ��Ӧ�벻��
		cJSON_Delete(root);
		return 1;
	}
    HTTPLAYERJSON_DEBUG("--------%s success\n",__func__);
	cJSON_Delete(root);

	return 0;
}

//  ��Ϣ��ִ
int DeviveMessageAnaly(unsigned char * inbuf,int len)
{
	cJSON*root = NULL;
	cJSON*psub = NULL;
	char const * YSW1_messageId=NULL;
	char const * YSW1_voiceMsg;
	char const * YSW1_notifyUrl;
	int64_t lcd_money_value = 0;
	int ret = -1;
	char voiceMsg[64] = {0};
	
	HTTPLAYERJSON_INFOR( "enter %s ,line %d\n", __func__, __LINE__);
	
	root=cJSON_Parse((void *)inbuf);

	if (NULL!=root)
	{
		psub=cJSON_GetObjectItem(root,"status");	
		if ((NULL==psub)||(psub->type!=cJSON_String)) 
			goto cJson_EXIT;

		if(memcmp(psub->valuestring,"VOICE_TEST",10) == 0 || memcmp(psub->valuestring,"voice_test",10) == 0)
		{
			HTTPLAYERJSON_INFOR("%s: start voice test\n",__func__);
			AllVoiceTest();
			goto cJson_EXIT;
		}
		else if(memcmp(psub->valuestring,"SUCCESS",7) != 0 && memcmp(psub->valuestring,"success",7) != 0)
		{
			HTTPLAYERJSON_INFOR("%s: Not a successful transaction\n",__func__);
			goto cJson_EXIT;
		}

		psub = cJSON_GetObjectItem(root,"orderId");
		if ((NULL==psub)||(psub->type!=cJSON_String)) 
			goto cJson_EXIT;

		HTTPLAYERJSON_INFOR( "orderId = %s\n", psub->valuestring);
		if(Record_Compare_MSGID(0, psub->valuestring) != 0) //msgid�ظ�
		{
			HTTPLAYERJSON_INFOR("%s: duplicate msg,ignore\n",__func__);
			goto cJson_EXIT;
		}
		YSW1_messageId=psub->valuestring;

		psub = cJSON_GetObjectItem(root, "amount");			
		if ((NULL==psub)||(psub->type!=cJSON_Number)) 
			goto cJson_EXIT;
		HTTPLAYERJSON_INFOR( "amount = %f\n", psub->valuedouble);
		lcd_money_value = (psub->valuedouble + 0.005)*100;
		HTTPLAYERJSON_INFOR( "amount = %lld\n", lcd_money_value);

		memset(voiceMsg,0,sizeof(voiceMsg));
		sprintf(voiceMsg,"Collection%ldUSD",lcd_money_value);
		YSW1_voiceMsg = voiceMsg;
		HTTPLAYERJSON_INFOR( "%s: msgid=%s, msg=%s\n", __func__, YSW1_messageId, YSW1_voiceMsg );
		Record_Manage_Write(YSW1_messageId, YSW1_voiceMsg, lcd_money_value);//�ɹ����¼��ǰ������Ϣ
		if (tts_play_set_sale_str(YSW1_messageId,YSW1_notifyUrl,YSW1_voiceMsg,lcd_money_value,read_rtc_time())!=0)//�������ٷ���ִ
		{
			HTTPLAYERJSON_INFOR("%s: tts play fail!\n",__func__);
			goto cJson_EXIT;
		}

		ret = 0;
	}
	else 
		HTTPLAYERJSON_DEBUG(HTTPLAYERJSON "exit %s, ret = -1 \n", __func__);

cJson_EXIT:
	if (root)
	{
		cJSON_Delete(root);
	}
	return ret;
}

int HTTPC_Process_G(HTTPParameters *ClientParams, const char * const * HeadAdd,CHAR *Buffer,INT32 bufSize, unsigned int *recvSize)
{
#if 1
	int ret = 0;
	UINT32 ReadLength ;//= * recvSize;
	HTTP_CLIENT httpClient;
	char const * HeadName;
	char const * HeadData;
	//char sendbufprn[500];
	
	memset((void *)&httpClient, 0, sizeof(httpClient));

	*recvSize=0;

	ClientParams->HttpVerb = VerbGet;

request:
	if ((ret = HTTPC_open(ClientParams)) != 0) {
		printf("HTTPC_open err\n");
		goto relese;
	}
#ifdef _HTTP_DEBUGGING_
					HTTPClientSetDebugHook(ClientParams->pHTTP,&HTTPDebug);
#endif
	if (HeadAdd!=NULL)
		while(1)
	{
		HeadName=*HeadAdd++;
		HeadData=*HeadAdd++;
		if ((HeadName==NULL)||(HeadData==NULL)) break;
		if((ret = HTTPIntrnHeadersAdd((P_HTTP_SESSION)(ClientParams->pHTTP),(char *)HeadName,strlen(HeadName),
			(char *)HeadData,strlen(HeadData)))!= HTTP_CLIENT_SUCCESS)
		{
			goto relese;
		}
	}
/*	
	if((ret = HTTPIntrnHeadersAdd((P_HTTP_SESSION)(ClientParams->pHTTP),"X-Login-Name",12,sysparam_get()->device_SN,strlen(sysparam_get()->device_SN)))!= HTTP_CLIENT_SUCCESS)
	{
		goto relese;
	}
	if((ret = HTTPIntrnHeadersAdd((P_HTTP_SESSION)(ClientParams->pHTTP),"X-Client-PV",11,PRODUCTTYPE,strlen(PRODUCTTYPE)))!= HTTP_CLIENT_SUCCESS)
	{
		goto relese;
	}
	if((ret = HTTPIntrnHeadersAdd((P_HTTP_SESSION)(ClientParams->pHTTP),"X-Client-Bus-Ver",16,firmware_version,strlen(firmware_version)))!= HTTP_CLIENT_SUCCESS)
	{
		goto relese;
	}
	*/
	//printf("ClientParams->pHTTP:%s\n",(char*)ClientParams->pHTTP);

	if ((ret = HTTPC_request(ClientParams, NULL)) != 0) {
//		CMD_ERR("http request err..\n");
		printf("HTTPC_request err=%d\n\n",ret);
		goto relese;
	}

	printf("send total:%ld\n",httpClient.RequestBodyLengthSent);
	if ((ret = HTTPC_get_request_info(ClientParams, &httpClient)) != 0) {
//		CMD_ERR("http get request info err..\n");
		printf("HTTPC_get_request_info err\n");
		goto relese;
	}

	printf("recv total:%ld\n",httpClient.HTTPStatusCode);
	if (httpClient.HTTPStatusCode != HTTP_STATUS_OK) {
		if((httpClient.HTTPStatusCode == HTTP_STATUS_OBJECT_MOVED) ||
				(httpClient.HTTPStatusCode == HTTP_STATUS_OBJECT_MOVED_PERMANENTLY)) {
//			CMD_DBG("Redirect url..\n");
			HTTPC_close(ClientParams);
			memset(ClientParams, 0, sizeof(*ClientParams));
			ClientParams->HttpVerb = VerbGet;
			if (httpClient.RedirectUrl->nLength < sizeof(ClientParams->Uri))
				strncpy(ClientParams->Uri, httpClient.RedirectUrl->pParam,
						httpClient.RedirectUrl->nLength);
			else
				goto relese;
//			CMD_DBG("go to request.\n");
			goto request;

		} else {
			ret = -1;
			printf("get result not correct.\n");
//			CMD_DBG("get result not correct.\n");
			goto relese;
		}
	}
//	if (httpClient.TotalResponseBodyLength != 0 || (httpClient.HttpFlags & HTTP_CLIENT_FLAG_CHUNKED )) {
		do {
//			memset(buf, 0, 4096);
			if ((ret = HTTPC_read(ClientParams, Buffer, bufSize, &ReadLength)) != 0) {
				//CMD_DBG("get data,Received:%d\n",Received);
				if (ret == 1000) {
					*recvSize +=ReadLength;
					ret = 0;
				}
				break;
			}
			Buffer +=ReadLength;
			bufSize -=ReadLength;
			*recvSize +=ReadLength;
		} while (1);
//	}
relese:
{
		P_HTTP_SESSION pHTTP=(P_HTTP_SESSION)ClientParams->pHTTP;
		if (pHTTP)
		{
			printf("\r\n---HeadersOut:%.*s\r\n",(int)pHTTP->HttpHeaders.HeadersOut.nLength,pHTTP->HttpHeaders.HeadersOut.pParam);
			printf("\r\n---httpheadin:%.*s\r\n",(int)pHTTP->HttpHeaders.HeadersIn.nLength,pHTTP->HttpHeaders.HeadersIn.pParam);
		}
#if 0
		HTTP_PARAM      HTTPParam;                                  // A generic pointer\length parameter for parsing
		//HTTPClientFindCloseHeader
		//HTTPClientFindFirstHeader
		//HTTPClientGetNextHeader
		if (HTTPIntrnHeadersFind((P_HTTP_SESSION)ClientParams->pHTTP,"resp_code",&HTTPParam,TRUE,0)==HTTP_CLIENT_SUCCESS)
			printf("\r\n----httpinfo:%.*s\r\n",(int)HTTPParam.nLength,HTTPParam.pParam);
		else
			printf("\r\n---notfound\r\n");
		if (HTTPIntrnHeadersFind((P_HTTP_SESSION)ClientParams->pHTTP,"content-length",&HTTPParam,TRUE,0)==HTTP_CLIENT_SUCCESS)
			printf("\r\n----httpinfo:%.*s\r\n",(int)HTTPParam.nLength,HTTPParam.pParam);
		else
			printf("\r\n---notfound\r\n");
#endif
}
//	free(buf);
	HTTPC_close(ClientParams);
	return ret;
#else
	return 0;
#endif
}



int Gprs_HTTPC_GetB(HTTPParam *httpinfo, const char * const * HeadAdd,CHAR *Buffer,INT32 bufSize, unsigned int *recvSize)
{
	int ret=-1;
		HTTPParameters *clientParams=NULL;
		clientParams = malloc(sizeof(*clientParams));
		if (!clientParams) return -1;
		memset(clientParams,0,sizeof(HTTPParameters));
		strcpy(clientParams->Uri,httpinfo->url);
	//	clientParams->HttpVerb = VerbPost;
		clientParams->pData = httpinfo->out;
		clientParams->pLength = httpinfo->outlen;
		ret = HTTPC_Process_G(clientParams,HeadAdd,Buffer, bufSize, recvSize);
		if (clientParams)
			free(clientParams);
		if ((ret != HTTP_CLIENT_SUCCESS) && (ret != HTTP_CLIENT_EOS))
			return -2;
		else return 0;
}
void http_get_test()
{

	int ret;
	char *out=NULL;
	const char * const HeadAdd[]=
	{
		"Accept","*/*",
		"Connection","close",
        "User-Agent","lwip/2.0.3 (embedded)",
		NULL,
	};					
	//uint8_t getBuf[29510]={0};
	uint8_t *getBuf=malloc(29510);
	//http://[2402:4e00:1013:e500:0:9671:f018:4947]/
	//http://test-ipv6.com/
	//http://ipv6.baidu.com/
	HTTPParam httpinfo;
	httpinfo.url=malloc(512);
	if (httpinfo.url==NULL)
		return -1;
	//strcpy(httpinfo.url,"https://www.baidu.com/");
	strcpy(httpinfo.url,"https://jiosit-nxptms.jio.com/chatak-tms-tpos-services/secure-v3/oauth/authorize?response_type=code&client_id=d5u%2BzDVHd1mHHFDYt4C1CA==&scope=read%20write%20trust&code_challenge=eeNUP-GCsRnUu1him8Bm8xHJ5Hn34WGCO9ovwRKjvn0&code_challenge_method=S256");
	httpinfo.out = out;
	httpinfo.outlen = strlen(out);
	int len;
	ret = Gprs_HTTPC_GetB(&httpinfo,HeadAdd,(CHAR *)getBuf, 29510, &len);
	free(out);
	free(httpinfo.url);
	if (ret) return -1;
	getBuf[29509]=0;		
	usb_log_printf("HTTP GET DATA len =%d\r\n",len);
	usb_log_printf("HTTP GET DATA:\r\n%s\r\n",getBuf);
	free(getBuf);
}
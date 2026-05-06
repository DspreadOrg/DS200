#include <stdio.h>
#include <math.h>
#include "ql_rtos.h"
#include "ql_power.h"
#include "ql_fs.h"
#include "prj_common.h"
#include "HTTPClient/HTTPCUsr_api.h"
#include "mbedtls/mbedtls.h"
#include "mbedtls/ssl.h"


#define LOG_DBG(fmt, arg...) //usb_log_printf("[DBG HTTPSTEST]"fmt, ##arg)
#define LOG_INFO(fmt, arg...) usb_log_printf("[INFO HTTPSTEST]"fmt, ##arg)

#define FILE_READ_BUF_LEN 1024


char httpstest_ctrl = 0;
ql_task_t httpstest_task_ctrl_thread = NULL;


const char TEST_SERVER_PEM[]=
"-----BEGIN CERTIFICATE-----\r\n"
"MIIFVTCCAz2gAwIBAgIJALE+9MSvKIILMA0GCSqGSIb3DQEBCwUAMFUxCzAJBgNV\r\n"
"BAYTAmJqMQswCQYDVQQIDAJiajELMAkGA1UEBwwCYmoxCzAJBgNVBAoMAmRoMQsw\r\n"
"CQYDVQQLDAJkaDESMBAGA1UEAwwJbG9jYWxob3N0MB4XDTIxMDUyODAzMDgxMVoX\r\n"
"DTMxMDUyNjAzMDgxMVowVTELMAkGA1UEBhMCYmoxCzAJBgNVBAgMAmJqMQswCQYD\r\n"
"VQQHDAJiajELMAkGA1UECgwCZGgxCzAJBgNVBAsMAmRoMRIwEAYDVQQDDAlsb2Nh\r\n"
"bGhvc3QwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQC8xdVsZxxGDHud\r\n"
"SfHpjSqyXHC/JprCFFZylCIuQpNg8t6ZmNYfaJaUcHmXO4QfHQwt1jZJ04oDNx79\r\n"
"dZ4cO2hj2Jw/h8d6GwsfZiv0N7AFVCcxndv0JHHwUypR3Ryq/ZcvUdZXDA/0VjAa\r\n"
"90uT2g6tFSO52TgeiPL9PNnALnNsABc8qF4+iz8yDuAfsqpd6fSzoUNlA86Bs20+\r\n"
"9Nmd3JiH9P07cJiDvs6Ov5+qSmLBsOoBmkHaXOT0HMjHaVL7IdmlZItG5aQnRfbg\r\n"
"jwgjlA4AS/tYieJnGycIqctPyixXPcfsH8Q9VrFXW6cQpynYkSxu1IYf9pcMZuGP\r\n"
"R4WOeZuHEKAzKkJjJfIBpHQunzxAhjeCYWkGc+xGXQ9BG+DlJkpdIGzlAIS3lXIY\r\n"
"XBTvhnaONR0pK2oQA0rny5+El2cO2v6NkuyaYwEJixkl6z1a0I1921qR0sB9v7Pb\r\n"
"5KkySbWjgmBV9qt0ZvHwnX7t0SoB4vpBDjs/qcpGEWYokpruDK91vUQ700YriS7u\r\n"
"bozk+v2kJxlEF6hkA/JgsJQP5X1hvsJzL5dAs0OYyqkEGoZVTS/3BfDIfDME+MO+\r\n"
"uBjB1VxtU6QNbYv9+/MsqWy3qyq0nCusUpmHBlkP5nwydztXiw85zjDp97ygnkEx\r\n"
"AIEyh0lCqwzbQGVGg5QIb7AKleammwIDAQABoygwJjAkBgNVHREEHTAbgglsb2Nh\r\n"
"bGhvc3SCDmNzLmRzcHJlYWQubmV0MA0GCSqGSIb3DQEBCwUAA4ICAQCS8V8VvNYk\r\n"
"zm3BUS3JjaNuzx4JI2WCZxzrlHRL+gtASklM4VF2H2/8yv5aKd9aGew0HcKu+YhY\r\n"
"DWvZGFGkMTEqHWePaESV+141aaBHP3Y0qLeTSU5P1m/ydPWKEEkR2kBzSP8Wfw7k\r\n"
"L698s30KbXsEsVLQgov0e/UdCyYjqiX9VL1piLpvlHRTvD1hVV8LTdvPXuP4RV3t\r\n"
"kki3iyulBmFWReeE+1+EcRUbdujDEiFzeOcaEhb8VxHoxn57/eCTazsJULVfKfgz\r\n"
"s2meBaP/u2ux6YzO4SaXhEOLCIo8ThPNt3xpGELBEk9Kwm3rZSuli9N5+IibBFBn\r\n"
"fighUCkegPAY2MczVh3bJNqRqcSZIfCTT3UNd12bmBWAn+4i3ySCG+2RgAYS9Ekc\r\n"
"EGmUKCu2ujQlFn7qO8TXRfBTalAKBeukZ6wbqov7hrQP0d5UAckC+IWfYHldH4Ez\r\n"
"xidJr7qIZmlwpf9IYL77bt8l+QgHvn8TvwLvMSyc6CLSd8CgIPjFZxBxdCTugBGD\r\n"
"TAlCYnwrhEjD4lOSDCgrQJgZ5VlS9BU1qFm4Ql4/z/AqiZ7wKN74n95HOoVDjyrF\r\n"
"XoVDXVSIBS5HO2iLbrYV8S/RqPQbAKcTuGdvGGD77Jx2kOZoWJh88OCbPsQpwan/\r\n"
"2EUvASWeP0TPn4S4BLybf0seCeRqhY5+Qw==\r\n"
"-----END CERTIFICATE-----\r\n";

const char TEST_CLIENT_CERT_PEM[]=
"-----BEGIN CERTIFICATE-----\r\n"
"MIIFVTCCAz2gAwIBAgIJAIFMucxEEhbZMA0GCSqGSIb3DQEBCwUAMFUxCzAJBgNV\r\n"
"BAYTAmJqMQswCQYDVQQIDAJiajELMAkGA1UEBwwCYmoxCzAJBgNVBAoMAmJqMQsw\r\n"
"CQYDVQQLDAJiajESMBAGA1UEAwwJbG9jYWxob3N0MB4XDTIxMDUyODAzMTA1OFoX\r\n"
"DTMxMDUyNjAzMTA1OFowVTELMAkGA1UEBhMCYmoxCzAJBgNVBAgMAmJqMQswCQYD\r\n"
"VQQHDAJiajELMAkGA1UECgwCYmoxCzAJBgNVBAsMAmJqMRIwEAYDVQQDDAlsb2Nh\r\n"
"bGhvc3QwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQC4oT3pGCSDo4aL\r\n"
"Nw/EgUo9tFr0FBiEGVzyJ8APfYK+1zimJX1gIEQ+YKfhfDQ/78g2E5162dq7Hvb2\r\n"
"5JVbHg+Wfew1MCsIxSRtII8mHNj3upo2dp6WDfdvZ/2epQ859LwZ8qeTkTv6/9MB\r\n"
"PHx3CTkXVR2LExLILsSHHFO+fMuTFS4PPNeGPJ3CbzcQKvQYai2YO2g1ISHhKDAw\r\n"
"nBLY4dWwfgUwTilRCYio5UDhxDkhTX86X5Ogvs9nBQ9XsF3kGvvINPhUFQc3GMnC\r\n"
"d5mSMzHQVw78vzhYOusrOODf6NaqFIIjGnYJnKR2e2Deg14Js0eOoHEpdBrLlqIX\r\n"
"2qUTEUEzih9YvqBBLGBajUu4PGeYCG+21KlT1gj/2d+7LmbzvczFu1XhSmuY0Aus\r\n"
"njUsWJBG5E3pODPfV/vlvAAMgKQ2iTbqRy+b79qdioT6S3UUQE2camwSnEa8pNpA\r\n"
"rUlYe7HzdqMm7SWnbweXnRlsAvCSPYoQgQQCqzflrW5rrrtbAAsJShsdru9tAPN4\r\n"
"JQf/xhJqa8vKTC0sok3at3QjxowPVvQjp56rqHm6Xvh/I26CAKQZGeRTft3o3T/I\r\n"
"B6/52/4faVLseV/28GnoY6rJp16PwqwrpaCbReauwkwPTFvQVq/eLIDzHqz8LVJW\r\n"
"PsumGu+C/og6UC66hmakjYUA2d7otQIDAQABoygwJjAkBgNVHREEHTAbgglsb2Nh\r\n"
"bGhvc3SCDmNzLmRzcHJlYWQubmV0MA0GCSqGSIb3DQEBCwUAA4ICAQAtWUAOzjib\r\n"
"4Ilv38FcXAyLN+SkxfhO3J4crlHZ7st1XTPlExhR76wln11WFjPFWHh8KVj5uIHC\r\n"
"LluZLjufNri3S0huR4KamF6ywQbDyX338Xi7+lQV/NN/XVHapCZ7q9otGV3AoSz/\r\n"
"IQJI25FTMGlEWKvLtqjy8CAolwBPEhtCcymprfLSjZ1RQeDmtN1cwp5xXxSRAzUP\r\n"
"wwi7nKBUjkeJkAR5D1GXuB9wW7bc5h7tkEoYLM6JL2Sn4mIEKDYviXSS3F8s97Q8\r\n"
"BXAuGtSSM0t/OEiLUhoU6VIJT2DmGMLrQ3yt4Qx/sOkjDuMR+ZY9bDQsywBvOqAK\r\n"
"B7jxEFmyxz9wbdk3vAMIUo2+J8CMzyhpvwBi39lDJsd48zB99VqUvl9cBMJNCNln\r\n"
"GRvnpc1aLXnyRvhqUNm6pa+iwgHmd1jY7PlL69Nrak+l3mw+wqaNxqJJ36JkxNDh\r\n"
"2YWDZixSQ83YtofwPGr+lwhi/5c49KPJaeGAtZxXeqgIZRUVMCIN6c6fadR3Cdg1\r\n"
"6Jnrr25ifH3qb3HnOWoV4sNswc0Dif9BTAQCzB+Bd7K7QqEAJotXkundgV1HIjwB\r\n"
"KOdAMrMcGVq68LtQyg81N7keBAJiQCuSvdUkxgulMAyNA2cvnSjFCoXC8U49xhtu\r\n"
"pF1zxkU0dTvyQhGJ++i1+a/8UHVHRTqUlA==\r\n"
"-----END CERTIFICATE-----\r\n";

const char TEST_CLIENT_KEY_PEM[]=
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"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\r\n"
"-----END RSA PRIVATE KEY-----\r\n";

//#define HTTPC_CUSTOM_CA_PEM           TEST_CA_PEM
//#define HTTPC_CUSTOM_CA_PEM_LEN       sizeof(TEST_CA_PEM)
#define HTTPC_CUSTOM_CA_PEM           TEST_SERVER_PEM
#define HTTPC_CUSTOM_CA_PEM_LEN       sizeof(TEST_SERVER_PEM)
#define HTTPC_CUSTOM_CRT_PEM          TEST_CLIENT_CERT_PEM
#define HTTPC_CUSTOM_CRT_PEM_LEN      sizeof(TEST_CLIENT_CERT_PEM)
#define HTTPC_CUSTOM_KEY              TEST_CLIENT_KEY_PEM
#define HTTPC_CUSTOM_KEY_LEN          sizeof(TEST_CLIENT_KEY_PEM)

static security_client HttpCert;
//static security_client HttpCert=
//{
//	.pCa=HTTPC_CUSTOM_CA_PEM,
//	.nCa=HTTPC_CUSTOM_CA_PEM_LEN,
//	.certs={
//		.pCa=HTTPC_CUSTOM_CA_PEM,
//		.nCa=HTTPC_CUSTOM_CA_PEM_LEN,
//		.pCert=HTTPC_CUSTOM_CRT_PEM,
//		.nCert=HTTPC_CUSTOM_CRT_PEM_LEN,
//		.pKey=HTTPC_CUSTOM_KEY,
//		.nKey=HTTPC_CUSTOM_KEY_LEN,
//	},
//};

security_client * HttpCertSet(void)
{
	security_client tmp=
	{
		.pCa=HTTPC_CUSTOM_CA_PEM,
		.nCa=HTTPC_CUSTOM_CA_PEM_LEN,
		.certs={
			.pCa=HTTPC_CUSTOM_CA_PEM,
			.nCa=HTTPC_CUSTOM_CA_PEM_LEN,
			.pCert=HTTPC_CUSTOM_CRT_PEM,
			.nCert=HTTPC_CUSTOM_CRT_PEM_LEN,
			.pKey=HTTPC_CUSTOM_KEY,
			.nKey=HTTPC_CUSTOM_KEY_LEN,
		},
	};
	HttpCert=tmp;
	return &HttpCert;
}


void https_auth_test(void)
{
//	const char * url="https://192.168.5.19:1300/";
	const char * url="https://cs.dspread.net:9044/test/1";
	unsigned int recSize;
	HTTPParameters *clientParams = NULL;
	char file_read_buf[FILE_READ_BUF_LEN];
	int ret;
	uint8_t eof;

	LOG_INFO("%s line %d========test start ==========\r\n", __func__, __LINE__);
	clientParams = malloc( sizeof( *clientParams ) );
	if ( !clientParams )
		return;
	memset( clientParams, 0, sizeof(HTTPParameters) );
	strcpy( clientParams->Uri, url );

	HTTPC_Register_user_certs(HttpCertSet);
//		HTTPC_set_ssl_verify_mode(MBEDTLS_SSL_VERIFY_NONE);
//		HTTPC_set_ssl_verify_mode(MBEDTLS_SSL_VERIFY_OPTIONAL);
	HTTPC_set_ssl_verify_mode(MBEDTLS_SSL_VERIFY_REQUIRED);
//		HTTPC_set_ssl_verify_mode(MBEDTLS_SSL_VERIFY_UNSET);

	recSize=0;
	ret = HTTPC_get( clientParams, (CHAR*) file_read_buf, (INT32) FILE_READ_BUF_LEN, (INT32*) &recSize );
	LOG_INFO("%s line %d========http ret %d, rec body %d bytes ==========\r\n", __func__, __LINE__,ret,recSize);
	if (( ret == HTTP_CLIENT_SUCCESS )|| ( ret == HTTP_CLIENT_EOS )) 
	{
		if (recSize)
			print_hex("recpack:",file_read_buf,recSize);
	}
//	if ( ret == HTTP_CLIENT_SUCCESS )
//		LOG_INFO("%s line %d========http not end ==========\r\n", __func__, __LINE__);
//	else if ( ret == HTTP_CLIENT_EOS )
//		LOG_INFO("%s line %d========http end ==========\r\n", __func__, __LINE__);
//	else
//		LOG_INFO("%s line %d========http err ==========\r\n", __func__, __LINE__);

	HTTPC_close( clientParams );
	if(clientParams) free(clientParams);
	LOG_INFO("%s line %d========test end ==========\r\n", __func__, __LINE__);
}

//extern void mqtt_test(void);
void https_test_set(void * argv)
{
	httpstest_ctrl = 1;

	while(httpstest_ctrl)
	{
    while ( 1 )
    {
       https_auth_test();
       ql_rtos_task_sleep_s( 10 );
//       mqtt_test();
       ql_rtos_task_sleep_s( 20 );
     }
	}
	LOG_INFO("httpstest_exit line %d\n", __LINE__);

	httpstest_task_ctrl_thread = NULL;
	ql_rtos_task_delete(NULL);
}

//int MqttExit(void)
//{
//	int ii;
//	httpstest_ctrl=0;
//
//	return 0;
//}

//void start_Mqtt_task(void)
//{
//	if (httpstest_task_ctrl_thread != NULL) {
//		LOG_INFO("httpstest task is running\n");
//		return;
//	}
//	if (ql_rtos_task_create(&httpstest_task_ctrl_thread,
//						10*1024,
//						98,
//						"httpstest",
//						https_test_set,
//						NULL) != OS_OK)
//	{
//		usb_log_printf("thread create error\n");
//	}
//}


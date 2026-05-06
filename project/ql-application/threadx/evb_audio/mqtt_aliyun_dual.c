#include <stdio.h>
#include <math.h>
#include "ql_rtos.h"
#include "ql_power.h"
#include "ql_fs.h"
#include "MQTTClient.h"
#include "ql_spi.h"
#include "public_api_interface.h"
#include "prj_common.h"
#include "cJSON.h"
#include "mqtt_aliyun.h"
#include "systemparam.h"
#include "tts_yt_task.h"
#include "led.h"
#include "drv_bat.h"
#include "mqtt_client.h"
#include "terminfodef.h"
#include "record_store.h"
#include "ota.h"
#include "module_lte.h"
#include "httplayerjson.h"
#include "play_receipt.h"
#include "lowpower_mgmt.h"
#include "disp_port.h"
#include "utf8strdef.h"

#ifdef DUAL_MQTT_SUPPORT

#if  CFG_ENV_MODE

static const char *aws_client_crt =  {
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIC/DCCAeSgAwIBAgIUZfblisoRtOieCk2fTOYAnzavKhcwDQYJKoZIhvcNAQEL\r\n"
	"BQAwETEPMA0GA1UEAwwGUm9vdENBMB4XDTI1MDMwMzA4MzYzOFoXDTM1MDMwMTA4\r\n"
	"MzYzOFowGzEZMBcGA1UEAwwQY29tbW9uLXByb2QtY2VydDCCASIwDQYJKoZIhvcN\r\n"
	"AQEBBQADggEPADCCAQoCggEBAIGcL1M1DRwLi+Bnh5sbAXpOJK6LqCMuwJM/SKpH\r\n"
	"/xNOUcIiJcO2bIOKv9Q6UWWQH2Qmflh5Xvtsgy7GfpfcYa/s/pzrdBpREraJfNch\r\n"
	"407tucTGRmQV1Q0hjAOVzH+0ebuRsS7c8QaYuMG9Rwwb49t1HVAGhZ+TRkkx6LcR\r\n"
	"hdvLvtMT6+LHAiLGxi6VBI5b17ajQqMsI2bjVD7+j2FY4sZBfsyoCB1w5PJC2dRI\r\n"
	"YwhhC1uviY75D+VVf6F03tNmlXYun4omMhval61trNop06bw42+Yd0jEotah5RD5\r\n"
	"DsZHNcV6hEtWsGQOStxzVDjcfErd7sqmCYeOVT5YyZL1iW0CAwEAAaNCMEAwHQYD\r\n"
	"VR0OBBYEFJzclwuqs1e9M5So2mlJGpeqMBxLMB8GA1UdIwQYMBaAFIv1Qh5QmWfO\r\n"
	"rh6/V9Ma4rMTa/drMA0GCSqGSIb3DQEBCwUAA4IBAQAtJzbc+TfI1vlDV7DlPgQY\r\n"
	"S3WrMZM5XseYqvnP9CIVSjjo+eavNa0GmzfuERCOPftNP/neN9nibSRXAdOG4WUJ\r\n"
	"vgdJOhataKtpOp3/FDDimnjI+IHVJb2gmWrOUNuIXMTWAGHYF8zvmLmgDFI70yz1\r\n"
	"FhE6xpOHlOFY9p1buq5/GJ/cT6lS52OgrHitE/L/AzBuP5MBcypMiSogpuhQbTSo\r\n"
	"p0jzy7VxpG5JieQ4sphloOpJkYZtj1iCZeDLWGa6MTmcBj4yWtXAxd8PPwfL1Rsh\r\n"
	"d1HeXNy+PTiQtQZ2tdyHuuDHUwNvP2UM5dk4RwU0p9SeCvwluEGmk0XfQbAUJfqG\r\n"
	"-----END CERTIFICATE-----"};
static const char *aws_client_key_crt =  {
	"-----BEGIN PRIVATE KEY-----\r\n"
	"MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCBnC9TNQ0cC4vg\r\n"
	"Z4ebGwF6TiSui6gjLsCTP0iqR/8TTlHCIiXDtmyDir/UOlFlkB9kJn5YeV77bIMu\r\n"
	"xn6X3GGv7P6c63QaURK2iXzXIeNO7bnExkZkFdUNIYwDlcx/tHm7kbEu3PEGmLjB\r\n"
	"vUcMG+PbdR1QBoWfk0ZJMei3EYXby77TE+vixwIixsYulQSOW9e2o0KjLCNm41Q+\r\n"
	"/o9hWOLGQX7MqAgdcOTyQtnUSGMIYQtbr4mO+Q/lVX+hdN7TZpV2Lp+KJjIb2pet\r\n"
	"bazaKdOm8ONvmHdIxKLWoeUQ+Q7GRzXFeoRLVrBkDkrcc1Q43HxK3e7KpgmHjlU+\r\n"
	"WMmS9YltAgMBAAECggEAF03FRpEx17qNvl3vCGjdJaf6OXX718OWtYL55kAGsWpy\r\n"
	"EdVA9AUPUsF2R0rrCrdUpZW0PuUnuk4i8hyXc2eZGuAiUYWWPQsZsb9btnSt4aml\r\n"
	"rqc1zdYNFgzTl2tuNwH0p1CFSpdIMPbxtUf6h8dV+0X97L8XdH8u8ocqQAxDBKdG\r\n"
	"md+GPQEYp09quxt1nUtVRuFqNySM9ooa8fd70emtUlyUNFtQ/TkCpQp/WiULe1o0\r\n"
	"jj5ua3MmGRXpwqD7OT+cq7m3TqknIu3i2HhevzHvlu8GATzThRt73LhDj93bKx5t\r\n"
	"HGXIO/93y74s62jJnXomQNsMdqx4LO19OI8bW8RUYwKBgQC2PmpEklGzAXE1Xnbt\r\n"
	"aSqDbNokQVmHVcTYyqz+ZYtyzAyEIyBxsX7YlKiA+LkbgOeWGT6LavRVtwYNPqgX\r\n"
	"y3nLiSCZAsDLhi9xWpaO2HfvClajK0F34E73MJDJWHLtv7Zjw2ogIS+efmxOTWj7\r\n"
	"Rnu867M0tMQM2ltqt2TTk196SwKBgQC2EJeIQeJYTOGNPRZlFEqjPoGK6Nm47U2u\r\n"
	"R3UaSipqpJnAH/QpwDqR+Z66ISt1tWMEj4irdPGWFEh/gdGBhBEdfvFNz/AOUuf/\r\n"
	"DCFyEjwXoCQs529u3DkJLrof/KrHYHO5paAOyf34WrYDhRXyvskUCPypjv3rDHi6\r\n"
	"JNWczAy4JwKBgETOBcMD1xKVPc+fbZkutl1OMIG1pn6A9EPBKVUWs3JQ9XzB4lLq\r\n"
	"Z3CPl5qxzPlETqsNtt5eMAdYhVFk0YPCT6Z3jDYYov90Ft9eCy+9LHK0XYdXIm2G\r\n"
	"42lSDafm2+VFFzBNK/kbbmYRZwSdF0CfGMJ33n3MraQIoLEGsmtq3YZHAoGBALF/\r\n"
	"aYxQ1y/I/FirRqsS5lVSeg8IRkNRL1uKlPMlfUbs5Gx3ep49p5xrJUFHV0xxo3Bt\r\n"
	"0wt9fo4XhjqxQKSpMdp0ySJYaCw+zalMlUrV1joeSUZx96S4FO8L3C+VMJYiRZCn\r\n"
	"lWz1pqG7Jho2ZJh9QiJDRLi2DH1cclLf2FX+ZSQfAoGBAIt3LGfDqu/zFxJMKeH0\r\n"
	"f6WP0SlrKec/Gqn0IhTVri0SnKBfq7rHF8e+v3mvT+z0uR7ma5QW+QmAcb8v3/Lf\r\n"
	"WeBAPfetFhkG1noi4OiCF5UjA03I2Vyt3qkXZtXZGHN3Cfwn/4rRvzJIjy7b639c\r\n"
	"2R8IbESmWdWDrfqX8Q4pPyyP\r\n"
	"-----END PRIVATE KEY-----"};
	
static const char *aws_ca_crt =  {
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"
	"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"
	"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"
	"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
	"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"
	"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"
	"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"
	"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"
	"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"
	"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"
	"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"
	"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"
	"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"
	"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"
	"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"
	"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"
	"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"
	"rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"
	"-----END CERTIFICATE-----"};	
#else

static const char *aws_client_crt =  {
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDBjCCAe6gAwIBAgIUE7C7XbXlh1cDgPvpeJKzQjb9PN8wDQYJKoZIhvcNAQEL\r\n"
	"BQAwFDESMBAGA1UEAwwJUm9vdENBRGV2MB4XDTI1MDMwMzA4MTYyMVoXDTM1MDMw\r\n"
	"MTA4MTYyMVowIjEgMB4GA1UEAwwXY29tbW9uLWRldi1zdGFnaW5nLWNlcnQwggEi\r\n"
	"MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDKf9rYT/5RZHaZrYSwBVUBjkQQ\r\n"
	"UA3rO3BC4rcTaAT2Z+EJwAk8jJUxpGMOAKZNUJ6oHp/QRlSUHRAEO1VNcSxV1Jg/\r\n"
	"Xc6EFOeTZp9pYaLf6bx759yMGtL4w9eRgtWi73ShkvC2JvyP7zIScJCdatYz3XYp\r\n"
	"8NUn3ykD4HHB4qAlUTc5QjQDQ9QwMJXDn5vCGnSO+vqPfLN5k4pUCri3uVF2xqsT\r\n"
	"AGdvYITByGCAVOUK1QJp7WMv7ehA5NyvZxYQCbmtvS3usnA7fRxfjGrxaFwfRaNl\r\n"
	"L0Psm1HZhiEtcd7QzBtxM/qMLa2H56sfOfRyqjRzgOjoIOViwoUQVurSvIwJAgMB\r\n"
	"AAGjQjBAMB0GA1UdDgQWBBQSyadqXb47JTRt5AJwOR2JAokzmDAfBgNVHSMEGDAW\r\n"
	"gBRkRphUtz3STrDX/oMRnOMJB25fDzANBgkqhkiG9w0BAQsFAAOCAQEAbYQR3L2u\r\n"
	"/+9TGRux50A0gh/xo55deSnoHNqb6RJl5tA2Dul5Qmi+ydrKlePDQ1cF+nlDH2DP\r\n"
	"iLtvz1iwkTCnh39jLOkx8C6A2hhPJ935d8Lzldj4TM+2GMQ8DHTzLWUd89dXoWFn\r\n"
	"ZtbZkkSfgb39KZBMj7KRKO3nCDsKOfgFn8tj/XsKLcWXsjqQYvw22mzLoUZMSXYu\r\n"
	"nGpkRt/3r1E06r9ly0MchUqdG8jJtz3kOQgWSB2KZzbVHy7qSVwtwRvw4g+Qnixq\r\n"
	"HgPsR3vblWY6Sq92tor/lxbqE+8AqvlkV7uvzqY8RmSoPYOisvwMy6QG49FAkv7M\r\n"
	"+IVI5dcuODkCKA==\r\n"
	"-----END CERTIFICATE-----"};
static const char *aws_client_key_crt =  {
	"-----BEGIN PRIVATE KEY-----\r\n"
	"MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDKf9rYT/5RZHaZ\r\n"
	"rYSwBVUBjkQQUA3rO3BC4rcTaAT2Z+EJwAk8jJUxpGMOAKZNUJ6oHp/QRlSUHRAE\r\n"
	"O1VNcSxV1Jg/Xc6EFOeTZp9pYaLf6bx759yMGtL4w9eRgtWi73ShkvC2JvyP7zIS\r\n"
	"cJCdatYz3XYp8NUn3ykD4HHB4qAlUTc5QjQDQ9QwMJXDn5vCGnSO+vqPfLN5k4pU\r\n"
	"Cri3uVF2xqsTAGdvYITByGCAVOUK1QJp7WMv7ehA5NyvZxYQCbmtvS3usnA7fRxf\r\n"
	"jGrxaFwfRaNlL0Psm1HZhiEtcd7QzBtxM/qMLa2H56sfOfRyqjRzgOjoIOViwoUQ\r\n"
	"VurSvIwJAgMBAAECggEAAUBpvvpPc5wjC6vweIqhpBMz2kcM1IBPBaIVuEorOqD9\r\n"
	"acznydrpBw1uWbRn8P2hryipTGVd8YR92bTF2bYySo5R0Vj+TWXtyeD91HKuK6zS\r\n"
	"QqNTKtURbhnnWKqNdt6WUNCLma6DUQLVRZQWY6XlBQFNSilUDqRlZRpSM9xZQnh3\r\n"
	"xfIczBvN0mL2hYmyBukHCibzEY3I25ybg4QOp5lSjQAtN4cq8joZNTeR/563ya4o\r\n"
	"4hSZ1mDTagEMVFQh3QIY2XycmbUiQt1IicWcJK4W1byz8EnFuTkxRwRIrVd1zrrU\r\n"
	"rcZCrunaWcElCCtWbrR5Oh2HNJdqclF1rtmj2xewBQKBgQD7qS+xhqX4eNtKZzO8\r\n"
	"UowTr9/RoaCF1p7J9ZoT1BLz2/ayslyqJd/5jF2yB/bPMYc0awHCCmUkeJWvMSF7\r\n"
	"dHfLCSYH27bCo6QkiA6z9xrnZV5265MKUJ0rInDcMC9mQyzANWYq8/0YTaHBmkNr\r\n"
	"JKeEhOoiFALdtyRPbpqYngjsKwKBgQDN/axe9LRa3bXFNoalLLQiQqgIli0E3rws\r\n"
	"Mxe5o+z8BG8sZgECDOsQINnMM93+hiIj7o/k5anrjj4BuoNHf8DVwL/ubhaHS8TB\r\n"
	"JVanejr2vNaFmiOZsl9GfhFoIm/hK5soJ4bgj0R0qJGqxwTdNJhcgWYVXEeLavZC\r\n"
	"8u5gVgOqmwKBgQCygWFZFN8pDeDON8HUtOCzHi8uLO0FwiRDsLgnlGgVHVMxkZ50\r\n"
	"+gSSp+PV4hxI/yKeJcDLeEAhzH6wE7NiNEy8cbLOJCTQju5hEzkKoh4tQBPvWk2k\r\n"
	"JWY2MGnJZUxZxxA5Did4FDTURkyUXO2xVACW2QiSP4uA5OUWiqeR1ZcSnQKBgQCA\r\n"
	"8yYuoz0sDNK3TDTpvZxePh/2kBxKiix99tYJTkcSVKxLvOcwo2mbRRVSsb1skgdZ\r\n"
	"OwQ1JFhEr1y9tEZuQdkzdmAwIBZWR+Y5iug+G1RCFWCtbhqPIoxmEpFPAQ/Mg4Gz\r\n"
	"l1rlxIn0BXp6z++Jkl02u4JuKMBY1HKo4m67O907tQKBgEd45nOW0FCCnRjUuGG7\r\n"
	"sRNqOiE5ooWbY2/hjarMAXeAQcHMAsOYT85jXsNTjqmWqOZWxfliEVwiZORIEAqu\r\n"
	"vLW8FKaS4BWhgzfBPH/woieNh395Kis00d7HIi2lVfPA6nsOajAb0smvfEw5P/8l\r\n"
	"U2YraCrdYfmBgrflHuY0sn5Z\r\n"
	"-----END PRIVATE KEY-----"};
	
static const char *aws_ca_crt =  {
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"
	"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"
	"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"
	"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
	"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"
	"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"
	"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"
	"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"
	"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"
	"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"
	"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"
	"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"
	"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"
	"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"
	"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"
	"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"
	"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"
	"rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"
	"-----END CERTIFICATE-----"};
#endif

extern int ssl_mqtt_client_more_config(Client* client, const char *ca_cert, uint32_t ca_length,
	const char *client_crt, uint32_t client_crt_len, const char *client_key, uint32_t client_key_len,
	const char *client_pwd, uint32_t client_pwd_len,xr_mqtt_param_t	*xr_mqtt_para);

#define aliot_debug_on      1
#define aliot_mes_call_on   1
#define aliot_infor_on      1

#define ALIOT_LOG(flags, fmt, arg...)   \
    do {                                \
        if (flags)                      \
            usb_log_printf(fmt, ##arg);         \
    } while (0)

#define ALIOT_DEBUG(fmt, arg...)   \
    ALIOT_LOG(aliot_debug_on, "[AliotDebug] "fmt, ##arg)

#define ALIOT_PRINT(fmt, arg...)   \
    ALIOT_LOG(aliot_mes_call_on, "[ALIOT_Print] "fmt, ##arg)

#define ALIOT_INFOR(fmt, arg...)   \
    ALIOT_LOG(aliot_infor_on, "[ALIOT_Infor] "fmt, ##arg)


//message for aliyun,define product key, device name, devcie secret
#define DEV_CLIENT_ID			"00001"

//define secure mode ;  2 : tls connect mode, 3: tcp stright connect mode
#define MQTT_TCP_MODE			"3"
#define MQTT_TLS_MODE			"2"

#define SECURE_MODE_A			MQTT_TLS_MODE
#define SECURE_MODE_B			MQTT_TCP_MODE

typedef struct {
	uint32_t		size;
	char 			*version;
	char 			*url;
	ota_verify_t	verify;
	char			*verify_value;
}ota_params;

char mqtt_ctrlA = 0;
char mqtt_ctrlB = 0;

typedef struct {
	mqt_cal cal_set;
	uint8_t mqtt_con_nums;
	uint8_t mqtt_con_rsp_times;
	Client ysw1_mqtt_client;
	aliot_devc_info_t xr_devc_info;
	xr_mqtt_param_t	xr_mqtt_para;
	char * ExitFlag;
} reconnectpara;

unsigned char mqtt_first_connet = 0;

#define OTA_THREAD_STACK_SIZE		(5 * 1024)
ql_task_t g_ota_thread = NULL;

extern char firmware_version[50];
extern char YSW1_messageId[50];
extern char YSW1_notifyUrl[200];

//#if 1
//extern char YSW1_voiceMsg[200];
//#else
//extern char * YSW1_voiceMsg;
//#endif
//extern int64_t lcd_money_value;


void ota_end( int OtaOK )
{
//    if (ota_download_progress == 100)
    if ( OtaOK == 1 )
    {
        //tts_play_set(AudioShenjcg,AudioShenjcgLen,FixAudioTypeDef);
        tts_play_set_idx(AUD_ID_UPDATE_SUCESS,0,0);
        sysparam_get()->ota = 1;
        ALIOT_INFOR( "%s_%d ===\n", __func__, __LINE__ );
    }
    else
    {
        ALIOT_INFOR( "%s_%d ===ota failed\n", __func__, __LINE__ );

        //tts_play_set(AudioShengjsb,AudioShengjsbLen,FixAudioTypeDef);
        tts_play_set_idx(AUD_ID_UPDATE_FAIL,0,0);
        sysparam_get()->ota = 2;
    }
    ql_rtos_task_sleep_ms( 2000 );
    sysparam_save( );
    ql_rtos_task_sleep_ms( 800 );

    WaitAudioOver( );
//    if (OtaDesc)
//  	  ql_rtos_task_sleep_ms( 4000 );

	AudioPlayHalt();
    ql_power_reset( );
	AudioPlayContinue();
}

void ota_run(void *arg)
{
	ota_params *params = arg;
	uint8_t mqtt_exit_cnt = 0;

	ALIOT_PRINT("OTA begin, url %s\n", params->url);
	
	TermInfo.OTAMode = 1;
    TermLedShow(TERM_OTA_START);
    disp_set_ota_state(0);;
//	tts_play_set(AudioZhenzsjrj,AudioZhenzsjrjLen,FixAudioTypeDef);
	tts_play_set_idx(AUD_ID_UPDATE_START,0,0);
	ql_rtos_task_sleep_ms(800);
	WaitAudioPlayOver();

	MqttExit( );

	ALIOT_PRINT("mqtt_exit_cnt = %d\n", mqtt_exit_cnt);
    if ( Ext_Wifi_AppOta( params->url) == 0 )
        ota_end( 1 );
    else
        ota_end( 0 );

//    AudioPlayContinue( );
	
	if (params->version)
	{
		free(params->version);
	}
	if (params->url)
	{
		free(params->url);
	}
	if (params)
	{
		free(params);
	}
	
//	MqttExit();
	TermInfo.OTAMode=0;
	ALIOT_PRINT("OTA exit\n");
    g_ota_thread = NULL;
    ql_rtos_task_delete(NULL);
}

void _otaUpdateArrived(MessageData* data)
{
	cJSON *root=NULL, *item_data=NULL,
		*item_size, *item_version, *item_url, *item_signMethod, *item_sign;
	uint32_t size = 0;
	char *version_buff = 0, *url_buff = 0, *sign_buff = 0;
	ota_params *params = 0;
	
	ALIOT_PRINT("OTA Message arrived: %.*s: %.*s\n",
				data->topicName->lenstring.len,
				data->topicName->lenstring.data,
				data->message->payloadlen,
				(char *)data->message->payload);

	if (g_ota_thread != NULL) {
		ALIOT_PRINT("OTA task is running\n");
		return;
	}

	if ((TermInfo.LowBat)&&(TermInfo.Charge==0))
	{
		return; // ?????��???????????
	}
	
	root = cJSON_Parse(data->message->payload);
	if (!root)
	{
		ALIOT_PRINT("OTA parse message failed\n");
		goto OTA_ERR;
	}
	item_data = cJSON_GetObjectItem(root, "data");
	if (!item_data)
	{
		ALIOT_PRINT("OTA not found data item\n");
		goto OTA_ERR;
	}
	//size
	item_size = cJSON_GetObjectItem(item_data, "size");
	if (!item_size)
	{
		ALIOT_PRINT("OTA not found size item\n");
		goto OTA_ERR;
	}
	size = item_size->valueint;
//	if (0)//size > PRJCONF_IMG_MAX_SIZE)
//	{
//		ALIOT_PRINT("OTA bin is too large, curr %d, max %d\n", size, PRJCONF_IMG_MAX_SIZE);
//		goto OTA_ERR;
//	}

	//version
	item_version = cJSON_GetObjectItem(item_data, "version");
	if (!item_version)
	{
		ALIOT_PRINT("OTA not found version item\n");
		goto OTA_ERR;
	}
	version_buff = malloc(strlen(item_version->valuestring) + 1);
	if (!version_buff)
	{
		ALIOT_PRINT("OTA allocate %d bytes for version failed\n", strlen(item_version->valuestring));
		goto OTA_ERR;
	}
	strcpy(version_buff, item_version->valuestring);
	//url
	item_url = cJSON_GetObjectItem(item_data, "url");
	if (!item_url)
	{
		ALIOT_PRINT("OTA not found url item\n");
		goto OTA_ERR;
	}
	url_buff = malloc(strlen(item_url->valuestring) + 1);
	if (!url_buff)
	{
		ALIOT_PRINT("OTA allocate %d bytes for url failed\n", strlen(item_url->valuestring));
		goto OTA_ERR;
	}
	strcpy(url_buff, item_url->valuestring);

	//signMethod
	item_signMethod = cJSON_GetObjectItem(item_data, "signMethod");
	if (!item_signMethod)
	{
		ALIOT_PRINT("OTA not found signMethod item\n");
		goto OTA_ERR;
	}

	//sign
	item_sign = cJSON_GetObjectItem(item_data, "sign");
	if (!item_sign)
	{
		ALIOT_PRINT("OTA not found sign item\n");
		goto OTA_ERR;
	}
	sign_buff = malloc(strlen(item_sign->valuestring) + 1);
	if (!sign_buff)
	{
		ALIOT_PRINT("OTA allocate %d bytes for sign failed\n", strlen(item_sign->valuestring));
		goto OTA_ERR;
	}
	strcpy(sign_buff, item_sign->valuestring);
	
	ALIOT_PRINT("OTA new firmware\n");
	ALIOT_PRINT("------------------------------\n");
	ALIOT_PRINT("\tsize: %d\n", size);
	ALIOT_PRINT("\tversion: %s\n", version_buff);
	ALIOT_PRINT("\turl: %s\n", url_buff);
	ALIOT_PRINT("\tsignMethod: %s\n", item_signMethod->valuestring);
	ALIOT_PRINT("\tsign: %s\n", sign_buff);
	ALIOT_PRINT("------------------------------\n");

	if (strcmp(version_buff, firmware_version) == 0)
	{
		ALIOT_PRINT("OTA version is same with current version, skip...\n");
		goto OTA_ERR;
	}

	params = malloc(sizeof(ota_params));
	memset(params, 0, sizeof(ota_params));
	params->size = size;
	params->version = version_buff;
	params->url = url_buff;
	if (strcmp("Md5", item_signMethod->valuestring) == 0)
	{
		params->verify = OTA_VERIFY_MD5;
		params->verify_value = sign_buff;
	}
	else if (strcmp("SHA256", item_signMethod->valuestring) == 0)
	{
		params->verify = OTA_VERIFY_SHA256;
		params->verify_value = sign_buff;
	}
	else
	{
		params->verify = OTA_VERIFY_NONE;
		params->verify_value = NULL;
		if (sign_buff)
		{
			free(sign_buff);
			sign_buff = NULL;
		}
	}
//	params->net_mode = OTA_PROTOCOL_HTTP;

	if (ql_rtos_task_create(&g_ota_thread,
						OTA_THREAD_STACK_SIZE,
						100,
						"ota_run",
						ota_run,
						params) != OS_OK) {
		ALIOT_PRINT("OTA task create failed\n");
		if (url_buff)
		{
			free(url_buff);
			url_buff = NULL;
		}
		goto OTA_ERR;
	}
	ALIOT_PRINT("OTA start, wait...\n");
	goto OTA_EXIT;

OTA_ERR:
	if (version_buff)
	{
		free(version_buff);
	}
	if (url_buff)
	{
		free(url_buff);
	}
	if (sign_buff)
	{
		free(sign_buff);
	}
	if (params)
	{
		free(params);
	}
OTA_EXIT:
	if (root)
	{
		cJSON_Delete(root);
	}
}

void otaUpdateArrivedA(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	_otaUpdateArrived(data);
}

void otaUpdateArrivedB(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	#if CFG_LARKTMS_SUPPORT
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	larktmsUpdateArrived(data);
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	#else
	_otaUpdateArrived(data);
	#endif
}

void _messageArrived(MessageData* data)
{
#if 1
	ALIOT_PRINT("Message arrived on topic : %.*s: %.*s\n",
				data->topicName->lenstring.len,
				data->topicName->lenstring.data,
				data->message->payloadlen,
				(char *)data->message->payload);
#else
	ALIOT_PRINT("Message arrived on topic : %.*s: %d\n",
				data->topicName->lenstring.len,
				data->topicName->lenstring.data,
				data->message->payloadlen);
#endif

	if(DeviveMessageAnaly(data->message->payload,data->message->payloadlen) == 0)
	{	
		;
	}
}

void messageArrivedA(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	_messageArrived(data);
}

void messageArrivedB(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	_messageArrived(data);
}

#if 0
const char * messrec[]=
{
"{\"messageId\":\"F037EE67A71C463386BBADE18CF16B01\",\"money\":1.00,\"voiceMsg\":\"" PAY_MSG_LKLSK "1.00" PAY_MSG_YUAN "\",\"notifyUrl\":\"http://180.167.225.202:29090/shanghutong/api/qr/voice/notify\"}",
"{\"messageId\":\"F037EE67A71C463386BBADE18CF16B02\",\"money\":1.01,\"voiceMsg\":\"" PAY_MSG_LKLDZ  "1.01" PAY_MSG_YUAN "\",\"notifyUrl\":\"\"}",
"{\"messageId\":\"F037EE67A71C463386BBADE18CF16B03\",\"money\":15.00,\"voiceMsg\":\"" PAY_MSG_ZFBDZ "15.00" PAY_MSG_YUAN "\",\"notifyUrl\":\"\"}",
"{\"messageId\":\"F037EE67A71C463386BBADE18CF16B04\",\"money\":123.1,\"voiceMsg\":\"" PAY_MSG_WXDZ "123.1" PAY_MSG_YUAN "\",\"notifyUrl\":\"http://180.167.225.202:29090/shanghutong/api/qr/voice/notify\"}",
"{\"messageId\":\"F037EE67A71C463386BBADE18CF16B05\",\"money\":215.13,\"voiceMsg\":\"" PAY_MSG_YSFDZ "215.13" PAY_MSG_YUAN "\",\"notifyUrl\":\"http://180.167.225.202:29090/shanghutong/api/qr/voice/notify\"}",
"{\"messageId\":\"F037EE67A71C463386BBADE18CF16B06\",\"money\":10006.87,\"voiceMsg\":\"" PAY_MSG_LKLDZ  "10006.87" PAY_MSG_YUAN "\",\"notifyUrl\":\"\"}",
"{\"messageId\":\"F037EE67A71C463386BBADE18CF16B07\",\"money\":138,\"voiceMsg\":\"" PAY_MSG_ZFBDZ "138" PAY_MSG_YUAN "\",\"notifyUrl\":\"\"}",
"{\"messageId\":\"F037EE67A71C463386BBADE18CF16B08\",\"money\":15,\"voiceMsg\":\"" PAY_MSG_WXDZ "15" PAY_MSG_YUAN "\",\"notifyUrl\":\"http://180.167.225.202:29090/shanghutong/api/qr/voice/notify\"}",
"{\"messageId\":\"F037EE67A71C463386BBADE18CF16B09\",\"money\":10000,\"voiceMsg\":\"" PAY_MSG_YSFDZ "10000" PAY_MSG_YUAN "\",\"notifyUrl\":\"http://180.167.225.202:29090/shanghutong/api/qr/voice/notify\"}",
};
char * topic="/user/down/zbkby1dy42CDQM6d/8108200700100045";

void testmess(void)
{
	static int set=0;
//	const char * messag="";
	MessageData aa;
	MQTTString str;
	MQTTMessage mess;
	
	usb_log_printf("------------\n%s,line %d,mess %p\n",__func__,__LINE__,messrec[set%3]);
	str.cstring=topic;
	str.lenstring.data=topic;
	str.lenstring.len=strlen(topic);

	mess.payload=(char *)messrec[set%(sizeof(messrec)/sizeof(messrec[0]))];
	mess.payloadlen=strlen(mess.payload);
	
	aa.message=&mess;
	aa.topicName=&str;
	set++;
	if ((set/1)<=(sizeof(messrec)/sizeof(messrec[0])))
//	if (set%20)
		{
			messageArrivedA(&aa);
		}
}

void testota(void)
{
#if 1
	ota_params *params = 0;
	char *url_buff = 0;
//	const char * url="http://server.natappfree.cc:40862/EC100Y_CS10WG_Yir_JH_GW2.99.bin";
//	const char * url="http://yunmazhineng.com/firmware/test/mid/EC800NCNLE_CS56N_7.3_1.1.1_20220914.bin";
//	const char * url="http://yunmazhineng.com/firmware/test/mid/EC600MCNLA_CS50ML_8.2_1.0.5_LZMA_20220920.bin";
//	const char * url="http://yunmazhineng.com/firmware/test/mid/EC600MCNLA_CS50ML_8.2_1.0.5_NOLZMA_20220920.bin";
//	const char * url="http://yunmazhineng.com/firmware/test/mid/EC600MCNLA_CS50ML_8.3_8.1_1.0.0_diff_20220920.bin";
//	const char * url="http://yunmazhineng.com/firmware/test/mid/EC600MCNLA_CS50ML_8.4_8.3_1.0.0_diff_20221010.bin";
	const char * url="http://yunmazhineng.com/firmware/test/mid/4GW_V83_105_R06A04_OTA.bin";
//	const char * url="http://yunmazhineng.com/firmware/test/mid/4GW_V84_100_R06A05_OTA.sign.bin";
	char flag=0;
	
	if (g_ota_thread!=NULL)
		return;
		
	url_buff = malloc(strlen(url) + 1);
	strcpy(url_buff,url);
	
	params = malloc(sizeof(ota_params));
	memset(params, 0, sizeof(ota_params));
	params->url = url_buff;
	
	if (ql_rtos_task_create(&g_ota_thread,
						OTA_THREAD_STACK_SIZE,
						100,
						"ota_run",
						ota_run,
						params) != OS_OK) {
		ALIOT_PRINT("OTA task create failed\n");
		if (url_buff)
		{
			free(url_buff);
		}
		if (params)
		{
			free(params);
		}
	}
	ALIOT_PRINT("OTA start, wait...\n");
#else
	static int set=0;
//	const char * messag="";
	MessageData aa;
	MQTTString str;
	MQTTMessage mess;
	const char * otatopic="/ota/device/upgrade/48b1NeMrLpRTcDpp/YR9992011L4M000021";
	const char * otamess=
												/ota/device/upgrade/a1QyLNqji87/LDLTE0000010: 
												{"code":"1000","data":{"size":4064984,"sign":"6024c1aab77794880407abd6dbce5e85","version":"1.0.1","url":"https://iotx-ota.oss-cn-shanghai.aliyuncs.com/ota/377754e42556b94217f4b5ed0182738d/ckljbc70k0001268jjf7z3icu.bin?Expires=1614849068&OSSAccessKeyId=LTAI4G1TuWwSirnbAzUHfL3e&Signature=2BHwoYrFfG61QIRH7iFd1aKG6U4%3D","signMethod":"Md5","md5":"6024c1aab77794880407abd6dbce5e85"},"id":1614762668146,"message":"success"}
		
//	"{\"dn\":\"YR9992011L4M000021\",\"id\":\"164118048\",\"method\":\"upgrade\",\"msg\":\"{\\\"softName\\\":\\\"����4G+wifi����\\\",\\\"fileMd5\\\":\\\"C076E3C3580D223438218122ED6FFA3A\\\",\\\"type\\\":\\\"firmware\\\",\\\"version\\\":\\\"1.2_GW2.01_A\\\",\\\"url\\\":\\\"http://192.168.5.236/EC100Y_GW2.99_JH_20201214.bin\\\"}\",\"ts\":1607400656532}";
	usb_log_printf("------------\n%s,line %d,mess %p\n",__func__,__LINE__,otamess);
	str.cstring=otatopic;
	str.lenstring.data=otatopic;
	str.lenstring.len=strlen(otatopic);

	mess.payload=(char *)otamess;
	mess.payloadlen=strlen(mess.payload);
	
	aa.message=&mess;
	aa.topicName=&str;
	set++;
	if (set<5)
		{
			otaUpdateArrived(&aa);
		}
#endif
}

#endif


#define MQTT_THREAD_STACK_SIZE           (1024 * 5)
#define MQTT_DISCONNECT_MAX_TIMES        40 //20
#define MQTT_YIELD_FAILED_MAX_TIMES      50
ql_task_t mqtt_task_ctrl_threadA = NULL;
ql_task_t mqtt_task_ctrl_threadB = NULL;
ql_task_t larktms_task_ctrl_threadC = NULL;

//uint8_t mqtt_con_nums = 0;
//uint8_t para->mqtt_con_rsp_times = 0;

/* 0: disconnet   1: connected   -1 : error*/
int check_mqtt_server(reconnectpara * para)
{
	int ret = -1;
	char out_stand = 0;

	if (para == NULL)
	{
		return -1;
	}
    
	out_stand = para->ysw1_mqtt_client.ping_outstanding;
	usb_log_printf("%s mqtt_con_rsp_times: %d,out_stand %d\n",__func__ ,para->mqtt_con_rsp_times,out_stand);
	if(out_stand == 1) {
		para->mqtt_con_rsp_times ++;
	}
	else {
		para->mqtt_con_rsp_times = 0;
	}

    
	//if ((mqtt_con_rsp_times > 20) || (mqtt_disconnect_times > 1000*MQTT_DISCONNECT_MAX_TIMES)) // test code
	if (para->mqtt_con_rsp_times > 20) // test code
	{
		usb_log_printf("%s mqtt_con_rsp_times = %d,  mqtt disconnect\n",__func__, para->mqtt_con_rsp_times);
		para->mqtt_con_rsp_times = 0;
		//c->mqtt_con_ping_cnt = 0; // test code
		para->cal_set.mqtt_con = MQTT_CACK;
		para->cal_set.mqtt_sub = MQTT_CACK;
		para->cal_set.mqtt_pub = MQTT_CACK;
		ret = 0;
	}
	else
	{
		ret = 1;
	}

	return ret;
}

void mqtt_reconnect(reconnectpara * para )
{
	int iresult=1;
	int ii;
		
	para->mqtt_con_nums ++;
    
	if(para->mqtt_con_nums == 1) {
		ii = para->mqtt_con_nums*10;
		while((ii--)&&(*para->ExitFlag))
			ql_rtos_task_sleep_ms(100);
		ALIOT_PRINT("mqtt_reconnect--mqtt_con_nums==1[%d]\n", para->mqtt_con_nums);
	} else if(para->mqtt_con_nums < 7) {
		iresult = pow(2,para->mqtt_con_nums);
		ii=iresult*10;
		while((ii--)&&(*para->ExitFlag))
			ql_rtos_task_sleep_ms(100);
		ALIOT_PRINT("mqtt_reconnect--mqtt_con_nums<2-6>[%d|%d]\n", para->mqtt_con_nums, iresult);
	} else if(para->mqtt_con_nums == 7) {
		para->mqtt_con_nums = 0;
		ALIOT_PRINT("mqtt_reconnect--mqtt_con_nums==7[%d]\n", para->mqtt_con_nums);
	}
}

#define MAX_MQTT_BUFF_SIZE		1024

unsigned char sendbufA[MAX_MQTT_BUFF_SIZE];
unsigned char readbufA[MAX_MQTT_BUFF_SIZE];
unsigned char sendbufB[MAX_MQTT_BUFF_SIZE];
unsigned char readbufB[MAX_MQTT_BUFF_SIZE];

	char topic_sub_messageA[100] = {0};
//	char topic_pub_informA[100] = {0};
	char topic_sub_upgradeA[100] = {0};
	char topic_sub_messageB[100] = {0};
//	char topic_pub_informB[100] = {0};
	char topic_sub_upgradeB[100] = {0};

void default_msgArriveA(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	if(strstr(data->topicName->lenstring.data, topic_sub_upgradeA))
		otaUpdateArrivedA(data);
	else if(strstr(data->topicName->lenstring.data, topic_sub_messageA))
		messageArrivedA(data);
}

void default_msgArriveB(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	if(strstr(data->topicName->lenstring.data, topic_sub_upgradeB))
		otaUpdateArrivedB(data);
	else if(strstr(data->topicName->lenstring.data, topic_sub_messageB))
		messageArrivedB(data);
}


char * PubTermInfo(void)
{
	cJSON *root = NULL;
	cJSON *params = NULL;
	char * pout;

	//update version for OTA start
	root = cJSON_CreateObject();
	if (root==NULL) return NULL;
	cJSON_AddItemToObject(root, "devName", cJSON_CreateString(sysparam_get()->device_name));
	cJSON_AddItemToObject(root, "version", cJSON_CreateString(CFG_LARKTMS_APP_VERSION));
	cJSON_AddItemToObject(root, "sn", cJSON_CreateString(sysparam_get()->device_SN));
	pout = cJSON_Print(root);
	cJSON_Delete(root);

	return pout;
}

void mqtt_work_taskA(void * argv)
{
	int rc = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	reconnectpara para;
	struct sysparam * psysconfig = sysparam_get();
	// char clientId[32] = {0};
	// char *username = "22222222";
	// char *passd="22222222";
	// strcpy(clientId,"22222222");
	
	memset(&para,0,sizeof(para));
	para.ExitFlag=&mqtt_ctrlA;
//	para.c=&ysw1_mqtt_clientA;

	TermInfo.MqttIsRuning |= MqttMaskA;
	TermInfo.ServiceOnline &= ~MqttMaskA;
	
	usb_log_printf("func %s line %d\n", __func__, __LINE__);
	mqtt_client_close(&para.ysw1_mqtt_client);
//	memset(&ysw1_mqtt_clientA,0,sizeof(ysw1_mqtt_clientA));
	sprintf(topic_sub_messageA, "%s/%s", sysparam_get()->Topic, sysparam_get()->device_SN);
	ALIOT_PRINT(" topic_sub_messageA=%s\r\n",topic_sub_messageA);

	mqtt_ctrlA = 1;
	para.cal_set.mqtt_con = MQTT_CACK;
	para.cal_set.mqtt_sub = MQTT_CACK;
	para.cal_set.mqtt_pub = MQTT_CACK;

//	alink_devc_init();
//	mqtt_para_init();

	//aliot_device_info_mulx(psysconfig->product_key, psysconfig->device_name, psysconfig->device_secret, DEV_CLIENT_ID, SECURE_MODE_A,&para.xr_devc_info);
	
	// sprintf(clientId,"%s",sysparam_get()->device_SN);

	/* connect para */
	connectData.MQTTVersion 		= 4;
	connectData.keepAliveInterval 	= 60;
	connectData.cleansession		= 0;
	connectData.clientID.cstring 	= sysparam_get()->device_SN;
	connectData.username.cstring 	= sysparam_get()->username;
	connectData.password.cstring 	= sysparam_get()->password;
	ALIOT_PRINT("connectData.clientID.cstring=%s\r\n",connectData.clientID.cstring);
	ALIOT_PRINT("connectData.username.cstring=%s\r\n",connectData.username.cstring);
	ALIOT_PRINT("connectData.password.cstring=%s\r\n",connectData.password.cstring);
	/* mqtt para */
	para.xr_mqtt_para.command_timeout_ms = 3000; //mqtt 
	para.xr_mqtt_para.read_buf = readbufA;
	para.xr_mqtt_para.send_buf = sendbufA;
	para.xr_mqtt_para.read_buf_size = MAX_MQTT_BUFF_SIZE;
	para.xr_mqtt_para.send_buf_size = MAX_MQTT_BUFF_SIZE;
	para.xr_mqtt_para.port	= atoi(sysparam_get()->server_port);
	
	strncpy(para.xr_mqtt_para.host_name,sysparam_get()->server_mqtt, strlen(sysparam_get()->server_mqtt));
	// strcpy(para.xr_mqtt_para.host_name,"c0013137.ala.dedicated.aws-cn.emqxcloud.cn");

	usb_log_printf("func %s line %d ----------> transaction Mqtt para <------------\n", __func__, __LINE__);
	ALIOT_PRINT("para.xr_mqtt_para.host_name111=%s\r\n",para.xr_mqtt_para.host_name);
	usb_log_printf("func %s line %d ----------> transaction Mqtt para <------------\n", __func__, __LINE__);
	ql_rtos_task_sleep_s(3);


	
	while(mqtt_ctrlA)
	{
		//ALIOT_PRINT("while(mqtt_ctrl)\n");
		if(para.cal_set.mqtt_con == MQTT_CACK) {
			TermInfo.ServiceOnline &= ~MqttMaskA;
			lpm_set(LPM_LOCK_MQTT,1);
			if(0 == memcmp(sysparam_get()->server_mode, MQTT_TCP_MODE, 1))
				rc = tcp_mqtt_client_mulx(&para.ysw1_mqtt_client,&para.xr_mqtt_para);
			else if(0 == memcmp(sysparam_get()->server_mode, MQTT_TLS_MODE, 1))
			{
				ALIOT_PRINT("before ssl_mqtt_client_more_config, line %d\n", __LINE__);
				rc = ssl_mqtt_client_more_config(&para.ysw1_mqtt_client, sysparam_get()->RootCA, strlen(sysparam_get()->RootCA)+1, 
					sysparam_get()->DevCert, strlen(sysparam_get()->DevCert) + 1, sysparam_get()->DevKey, strlen(sysparam_get()->DevKey) + 1, NULL, 0,&para.xr_mqtt_para);
				ALIOT_PRINT("after ssl_mqtt_client_more_config, rc=%d\n", rc);
			}
			else
			{
				ALIOT_PRINT("before ssl_mqtt_client, line %d\n", __LINE__);
				rc = ssl_mqtt_client_mulx(&para.ysw1_mqtt_client, &para.xr_mqtt_para,alink_ca_get(), strlen(alink_ca_get())+1);
				ALIOT_PRINT("after ssl_mqtt_client, rc=%d\n", rc);
			}
			para.ysw1_mqtt_client.defaultMessageHandler=default_msgArriveA;
	
			if (rc  != 0) {
				mqtt_reconnect(&para);
				TermLedShow(TERM_NET_SERVER_FAILED);
				continue;
			}

			rc = MQTTConnect(&para.ysw1_mqtt_client, &connectData);
			if (rc != 0) {
				ALIOT_PRINT("%s:MQTT client connect error! Return error code is %d\n",__func__, rc);
				mqtt_reconnect(&para);
				TermLedShow(TERM_NET_SERVER_FAILED);
				continue;
			}
			ALIOT_PRINT("%s:MQTT Connected\n",__func__);
			para.cal_set.mqtt_con = MQTT_DICACK;
		}

		if(para.cal_set.mqtt_sub == MQTT_CACK) {

			rc = MQTTSubscribe(&para.ysw1_mqtt_client, topic_sub_messageA, XR_MQTT_QOS1, messageArrivedA);
			
			if (rc != 0) {
				ALIOT_PRINT("%s:Return code from MQTT subscribe is %d\n",__func__, rc);
				para.cal_set.mqtt_con = MQTT_CACK;
				mqtt_reconnect(&para);
				continue;
			}

			ALIOT_PRINT("%s:MQTT Subscrible is success\n",__func__);
			para.cal_set.mqtt_sub = MQTT_DICACK;
			TermLedShow(TERM_NET_SERVER_CON);
			if (mqtt_first_connet == 0)
			{
				tts_play_set_idx(AUD_ID_MQTT_CONNECT_SUCESS,0,0);
				mqtt_first_connet = 1;
			}
			lpm_set(LPM_LOCK_MQTT,0);

			TermInfo.ServiceOnline |= MqttMaskA;
			if ((TermInfo.ServiceOnline&(MqttMaskA|MqttMaskB))==(MqttMaskA|MqttMaskB))
			{
				ql_rtos_task_sleep_ms( 50 );
			}
		}

		// if(para.cal_set.mqtt_pub == MQTT_CACK) {
		// 	char topic_pub_inform[100] = {0};
		// 	//update version for OTA start
		// 	MQTTMessage pub_inform;
		// 	char *buff;

		// 	buff=PubTermInfo();
		// 	if (buff==NULL) continue;
		// 	pub_inform.qos = XR_MQTT_QOS1;
		// 	pub_inform.retained = 0;
		// 	pub_inform.payload = buff;
		// 	pub_inform.payloadlen = strlen(buff);
			
		// 	ALIOT_PRINT("%s:OTA infrom:\n---------------------\n%s\n---------------------\n", __func__,buff);
		// 	sprintf(topic_pub_inform, "/ota/device/inform/%s/%s", psysconfig->device_name, psysconfig->device_SN);
		// 	rc = MQTTPublish(&para.ysw1_mqtt_client, topic_pub_inform, &pub_inform);
		// 	if (rc != 0) {
		// 		ALIOT_PRINT("%s:Return code from MQTT publish is %d\n",__func__, rc);
		// 		para.cal_set.mqtt_con = MQTT_CACK;
		// 		para.cal_set.mqtt_sub = MQTT_CACK;
		// 		free(buff);
		// 		mqtt_reconnect(&para);
		// 		continue;
		// 	}
		// 	free(buff);
		// 	ALIOT_PRINT("%s:Inform version for OTA success\n",__func__);
		// 	//update version for OTA end
			

		// 	ALIOT_PRINT("%s:MQTT publish is success\n",__func__);
		// 	para.cal_set.mqtt_pub = MQTT_DICACK;
		// }
		
		rc = MQTTYield(&para.ysw1_mqtt_client, 3000);
		if (rc != 0)
		{
			//mqtt_first_connet  = 0;
			ALIOT_PRINT("%s:Return code from yield is %d\n",__func__, rc);
			rc = MQTTDisconnect(&para.ysw1_mqtt_client);
			if (rc != 0)
				ALIOT_INFOR("%s:Return code from MQTT disconnect is %d\n",__func__,rc);
			else
				ALIOT_INFOR("%s:MQTT disconnect is success\n",__func__);


			para.cal_set.mqtt_con = MQTT_CACK;
			para.cal_set.mqtt_sub = MQTT_CACK;
			para.cal_set.mqtt_pub = MQTT_CACK;
			mqtt_reconnect(&para);
			continue;
		}
//		testmess();
//		testota();
//        ql_rtos_task_sleep_ms(200);
		para.mqtt_con_nums = 0;
		check_mqtt_server(&para);
	}

	ALIOT_INFOR("%s:mqtt_exit line %d\n", __func__,__LINE__);
	
	//MQTTUnsubscribe(&ysw1_mqtt_client, topic_sub_upgrade);
	//MQTTUnsubscribe(&ysw1_mqtt_client, topic_sub_message);

	rc = MQTTDisconnect(&para.ysw1_mqtt_client);
	if (rc != 0)
		ALIOT_PRINT("%s:Return code from MQTT disconnect is %d\n",__func__, rc);
	else
		ALIOT_PRINT("%s:MQTT disconnect is success\n",__func__);

	mqtt_client_close(&para.ysw1_mqtt_client);
	ALIOT_PRINT("%s:mqtt_work_set end\n",__func__);
	lpm_set(LPM_LOCK_MQTT,1);
	disp_sleep_enable(0);
	TermInfo.MqttIsRuning &= ~MqttMaskA;
	TermInfo.ServiceOnline &= ~MqttMaskA;
	mqtt_task_ctrl_threadA = NULL;
	ql_rtos_task_delete(NULL);
}


#if CFG_LARKTMS_SUPPORT


void mqtt_work_taskB(void * argv)
{
	int rc = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	reconnectpara para;

	char clientId[32] = {0};
	char name[32] = {0};
	memset(&para,0,sizeof(para));
	para.ExitFlag=&mqtt_ctrlA;
//	para.c=&ysw1_mqtt_clientA;
	int connect_repeat = 5;

	TermInfo.MqttIsRuning |= MqttMaskB;
	TermInfo.ServiceOnline &= ~MqttMaskB;

	// memset(sysparam_get()->RootCA,0,sizeof(sysparam_get()->RootCA));

	// strcpy(sysparam_get()->device_SN,"BP110000000000000008");
	// sysparam_save();
	
	usb_log_printf("func %s line %d\n", __func__, __LINE__);

	usb_log_printf("rootca: %s\n", sysparam_get()->RootCA);
	usb_log_printf("DevCert: %s\n", sysparam_get()->DevCert);
	usb_log_printf("DevKey: %s\n", sysparam_get()->DevKey);
	
	mqtt_client_close(&para.ysw1_mqtt_client);
//	memset(&ysw1_mqtt_clientB,0,sizeof(ysw1_mqtt_clientB));

	int count = 3;
	int ret = 0;
	while(1)
	{
		ql_rtos_task_sleep_ms( 1000*2);
		if(count <= 0)
			break ;
		ret = larktms_init();

		if(ret == 0)
			break;
		
		count --;  

	}
	
	strcpy(topic_sub_upgradeB, sysparam_get()->larktms_topic);

	mqtt_ctrlB = 1;
	para.cal_set.mqtt_con = MQTT_CACK;
	para.cal_set.mqtt_sub = MQTT_CACK;
	para.cal_set.mqtt_pub = MQTT_CACK;

//	aliot_device_info_mulx(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, DEV_CLIENT_ID, SECURE_MODE_B,&para.xr_devc_info);

	/* connect para */
	connectData.MQTTVersion 		= 4;
	connectData.keepAliveInterval 	= 300;
	connectData.cleansession		= 0;
	//connectData.clientID.cstring 	= IOT_CLIENT_ID;
	sprintf(clientId,"%s&%s",sysparam_get()->larktms_productkey,sysparam_get()->larktms_devname);
	connectData.clientID.cstring = clientId;
	//connectData.username.cstring 	= IOT_USER_NAME;
	sprintf(name,"%s&%s",sysparam_get()->larktms_productkey,sysparam_get()->larktms_devname);
	connectData.username.cstring = name;
	//connectData.password.cstring 	= IOT_PASSWORD;
	connectData.password.cstring = sysparam_get()->larktms_secretkey;
	/* mqtt para */
	para.xr_mqtt_para.command_timeout_ms = 3000; //mqtt��ʱʱ�� 
	para.xr_mqtt_para.read_buf = readbufB;
	para.xr_mqtt_para.send_buf = sendbufB;
	para.xr_mqtt_para.read_buf_size = MAX_MQTT_BUFF_SIZE;
	para.xr_mqtt_para.send_buf_size = MAX_MQTT_BUFF_SIZE;
	para.xr_mqtt_para.port	= atoi(sysparam_get()->larktms_mqtt_port);
	strncpy(para.xr_mqtt_para.host_name, sysparam_get()->larktms_mqtt_url, strlen(sysparam_get()->larktms_mqtt_url));
	ql_rtos_task_sleep_s(3);
//	ysw1_mqtt_client.defaultMessageHandler=otaUpdateArrived;
#if 0
	if(sysparam_get()->ota_init)
	{
		usb_log_printf("func %s line %d ----------> LarmTms Mqtt para <------------\n", __func__, __LINE__);
		usb_log_printf("clientId : %s \n", connectData.clientID.cstring);
		usb_log_printf("username : %s \n", connectData.username.cstring);
		usb_log_printf("password : %s \n", connectData.password.cstring);
		usb_log_printf("host_name : %s \n", para.xr_mqtt_para.host_name);
		usb_log_printf("port : %d \n", para.xr_mqtt_para.port);
		usb_log_printf("topic : %s \n", topic_sub_upgradeB);
		usb_log_printf("func %s line %d ----------> LarmTms Mqtt para <------------\n", __func__, __LINE__);	
	}
#endif
	while(mqtt_ctrlB)
	{
		if(sysparam_get()->ota_init == 0)
		{
			usb_log_printf("func %s line %d ----------> LarmTms stop <------------\n", __func__, __LINE__);
			TermInfo.ServiceOnline |= MqttMaskB;
			ql_rtos_task_sleep_ms( 1000 *30);
			continue;	
		}

		if(connect_repeat <= 0)
		{
			usb_log_printf("func %s line %d ----------> connect_repeat  %d<------------\n", __func__, __LINE__,connect_repeat);
			sysparam_get()->ota_init = 0;
			sysparam_save();
			continue;
		}
		//ALIOT_PRINT("while(mqtt_ctrl)\n");
		if(para.cal_set.mqtt_con == MQTT_CACK) {
		    TermInfo.ServiceOnline &= ~MqttMaskB;
		 //lpm_set(LPM_LOCK_MQTT,1);
			if(0 == memcmp(SECURE_MODE_B, MQTT_TCP_MODE, 1))
				rc = tcp_mqtt_client_mulx(&para.ysw1_mqtt_client,&para.xr_mqtt_para);
			if(0 == memcmp(SECURE_MODE_B, MQTT_TLS_MODE, 1))
			{
				ALIOT_PRINT("%s:before ssl_mqtt_client, line %d\n", __func__,__LINE__);
//				rc = ssl_mqtt_client_mulx(&para.ysw1_mqtt_client, &para.xr_mqtt_para,alink_ca_get(), strlen(alink_ca_get())+1);
				rc = ssl_mqtt_client_mulx(&para.ysw1_mqtt_client, &para.xr_mqtt_para,NULL, 0);
				ALIOT_PRINT("%s:after ssl_mqtt_client,line %d\n", __func__,__LINE__);
				ALIOT_PRINT("%s:ssl_mqtt_client : rc = %d \n",__func__, rc);
			}
			para.ysw1_mqtt_client.defaultMessageHandler=default_msgArriveB;
	
			if (rc  != 0) {
				mqtt_reconnect(&para);
				connect_repeat --;
				continue;
			}

			rc = MQTTConnect(&para.ysw1_mqtt_client, &connectData);
			if (rc != 0) {
				ALIOT_PRINT("%s:MQTT client connect error! Return error code is %d\n",__func__, rc);
				mqtt_reconnect(&para);
				connect_repeat --;
				continue;
			}
			ALIOT_PRINT("%s:MQTT Connected\n",__func__);
			para.cal_set.mqtt_con = MQTT_DICACK;
		}


		if(para.cal_set.mqtt_sub == MQTT_CACK) {
			//subscribe OTA notify start
			rc = MQTTSubscribe(&para.ysw1_mqtt_client, topic_sub_upgradeB, XR_MQTT_QOS1, otaUpdateArrivedB);
			if (rc != 0) 
			{
				ALIOT_PRINT("%s:Return code from MQTT subscribe(OTA) is %d\n",__func__, rc);
				para.cal_set.mqtt_con = MQTT_CACK;
				mqtt_reconnect(&para);
				connect_repeat --;
				continue;
			}

			ALIOT_PRINT("%s:MQTT Subscrible is success\n",__func__);
			connect_repeat = 5;
			para.cal_set.mqtt_sub = MQTT_DICACK;
		}
		TermInfo.ServiceOnline |= MqttMaskB;
		if ((TermInfo.ServiceOnline&(MqttMaskA|MqttMaskB))==(MqttMaskA|MqttMaskB))
		{
			ql_rtos_task_sleep_ms( 50 );
		}

		rc = MQTTYield(&para.ysw1_mqtt_client, 3000);
		if (rc != 0)
		{
			ALIOT_PRINT("%s:Return code from yield is %d\n",__func__, rc);
			rc = MQTTDisconnect(&para.ysw1_mqtt_client);
			if (rc != 0)
				ALIOT_INFOR("%s:Return code from MQTT disconnect is %d\n",__func__,rc);
			else
				ALIOT_INFOR("%s:MQTT disconnect is success\n",__func__);


			para.cal_set.mqtt_con = MQTT_CACK;
			para.cal_set.mqtt_sub = MQTT_CACK;
			para.cal_set.mqtt_pub = MQTT_CACK;
			mqtt_reconnect(&para);
			connect_repeat --;
			continue;
		}
//        ql_rtos_task_sleep_ms(200);
		para.mqtt_con_nums = 0;
		check_mqtt_server(&para);
	}

	ALIOT_INFOR("%s:mqtt_exit line %d\n", __func__,__LINE__);
	
	//MQTTUnsubscribe(&ysw1_mqtt_client, topic_sub_upgrade);
	//MQTTUnsubscribe(&ysw1_mqtt_client, topic_sub_message);

	rc = MQTTDisconnect(&para.ysw1_mqtt_client);
	if (rc != 0)
		ALIOT_PRINT("%s:Return code from MQTT disconnect is %d\n",__func__, rc);
	else
		ALIOT_PRINT("%s:MQTT disconnect is success\n",__func__);

	mqtt_client_close(&para.ysw1_mqtt_client);
	ALIOT_PRINT("%s:mqtt_work_set end\n",__func__);
	lpm_set(LPM_LOCK_MQTT,1);
	disp_sleep_enable(0);
	TermInfo.ServiceOnline &= ~MqttMaskB;
	TermInfo.MqttIsRuning &= ~MqttMaskB;
	mqtt_task_ctrl_threadB = NULL;
	ql_rtos_task_delete(NULL);
}
#else


#define IOT_CLIENT_ID			"CTYXY00000000001"  
#define IOT_USER_NAME			"test0001"				
#define IOT_PASSWORD			"test0001"
#define IOT_HOST_NAME			"218.66.91.181"
#define IOT_PORT			7075
#define PRODUCT_KEY				"0123456789ABCDEF"
#define DEVICE_NAME				"ds"
#define DEVICE_SECRET			"0123456789ABCDEF"

void mqtt_work_taskB(void * argv)
{
	int rc = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	reconnectpara para;

	memset(&para,0,sizeof(para));
	para.ExitFlag=&mqtt_ctrlA;
//	para.c=&ysw1_mqtt_clientA;

	TermInfo.MqttIsRuning |= MqttMaskB;
	
	usb_log_printf("func %s line %d\n", __func__, __LINE__);
	mqtt_client_close(&para.ysw1_mqtt_client);
//	memset(&ysw1_mqtt_clientB,0,sizeof(ysw1_mqtt_clientB));
	
	sprintf(topic_sub_messageB, "/%s/%s/user/message", PRODUCT_KEY, DEVICE_NAME);
	sprintf(topic_sub_upgradeB, "/ota/device/upgrade/%s/%s", PRODUCT_KEY, DEVICE_NAME);
	
	mqtt_ctrlB = 1;
	para.cal_set.mqtt_con = MQTT_CACK;
	para.cal_set.mqtt_sub = MQTT_CACK;
	para.cal_set.mqtt_pub = MQTT_CACK;

	aliot_device_info_mulx(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, DEV_CLIENT_ID, SECURE_MODE_B,&para.xr_devc_info);

	/* connect para */
	connectData.MQTTVersion 		= 4;
	connectData.keepAliveInterval 	= 60;
	connectData.cleansession		= 0;
	connectData.clientID.cstring 	= para.xr_devc_info.client_id;
	connectData.username.cstring 	= para.xr_devc_info.user_name;
	connectData.password.cstring 	= para.xr_devc_info.password;

	/* mqtt para */
	para.xr_mqtt_para.command_timeout_ms = 3000; //mqtt��ʱʱ�� 
	para.xr_mqtt_para.read_buf = readbufB;
	para.xr_mqtt_para.send_buf = sendbufB;
	para.xr_mqtt_para.read_buf_size = MAX_MQTT_BUFF_SIZE;
	para.xr_mqtt_para.send_buf_size = MAX_MQTT_BUFF_SIZE;
	para.xr_mqtt_para.port	= para.xr_devc_info.port;
	strncpy(para.xr_mqtt_para.host_name, para.xr_devc_info.host_name, strlen(para.xr_devc_info.host_name));
	ql_rtos_task_sleep_s(3);
//	ysw1_mqtt_client.defaultMessageHandler=otaUpdateArrived;
	
	while(mqtt_ctrlB)
	{
		//ALIOT_PRINT("while(mqtt_ctrl)\n");
		if(para.cal_set.mqtt_con == MQTT_CACK) {
			if(0 == memcmp(SECURE_MODE_B, MQTT_TCP_MODE, 1))
				rc = tcp_mqtt_client_mulx(&para.ysw1_mqtt_client,&para.xr_mqtt_para);
			if(0 == memcmp(SECURE_MODE_B, MQTT_TLS_MODE, 1))
			{
				ALIOT_PRINT("before ssl_mqtt_client, line %d\n", __LINE__);
				rc = ssl_mqtt_client_mulx(&para.ysw1_mqtt_client, &para.xr_mqtt_para,alink_ca_get(), strlen(alink_ca_get())+1);
				ALIOT_PRINT("after ssl_mqtt_client, line %d\n", __LINE__);
			}
			para.ysw1_mqtt_client.defaultMessageHandler=default_msgArriveB;
	
			if (rc  != 0) {
				mqtt_reconnect(&para);
				continue;
			}

			rc = MQTTConnect(&para.ysw1_mqtt_client, &connectData);
			if (rc != 0) {
				ALIOT_PRINT("MQTT client connect error! Return error code is %d\n", rc);
				continue;
			}
			ALIOT_PRINT("MQTT Connected\n");
			para.cal_set.mqtt_con = MQTT_DICACK;
		}

		if(para.cal_set.mqtt_sub == MQTT_CACK) {
			//subscribe OTA notify start
			rc = MQTTSubscribe(&para.ysw1_mqtt_client, topic_sub_upgradeB, XR_MQTT_QOS1, otaUpdateArrivedB);
			if (rc != 0) 
			{
				ALIOT_PRINT("Return code from MQTT subscribe(OTA) is %d\n", rc);
				para.cal_set.mqtt_con = MQTT_CACK;
				continue;
			}
			//subscribe OTA notify end
			rc = MQTTSubscribe(&para.ysw1_mqtt_client, topic_sub_messageB, XR_MQTT_QOS1, messageArrivedB);
			
			if (rc != 0) {
				ALIOT_PRINT("Return code from MQTT subscribe is %d\n", rc);
				para.cal_set.mqtt_con = MQTT_CACK;
				continue;
			}

			ALIOT_PRINT("MQTT Subscrible is success\n");
			para.cal_set.mqtt_sub = MQTT_DICACK;
		}

		if(para.cal_set.mqtt_pub == MQTT_CACK) {
			char topic_pub_inform[100] = {0};
			//update version for OTA start
			MQTTMessage pub_inform;
			char *buff;

			buff=PubTermInfo();
			if (buff==NULL) continue;
			pub_inform.qos = XR_MQTT_QOS1;
			pub_inform.retained = 0;
			pub_inform.payload = buff;
			pub_inform.payloadlen = strlen(buff);
			
			ALIOT_PRINT("OTA infrom:\n---------------------\n%s\n---------------------\n", buff);
			sprintf(topic_pub_inform, "/ota/device/inform/%s/%s", PRODUCT_KEY, DEVICE_NAME);
			rc = MQTTPublish(&para.ysw1_mqtt_client, topic_pub_inform, &pub_inform);
			if (rc != 0) {
				ALIOT_PRINT("Return code from MQTT publish is %d\n", rc);
				para.cal_set.mqtt_con = MQTT_CACK;
				para.cal_set.mqtt_sub = MQTT_CACK;
				free(buff);
				continue;
			}
			free(buff);
			ALIOT_PRINT("Inform version for OTA success\n");
			//update version for OTA end
			

			ALIOT_PRINT("MQTT publish is success\n");
			para.cal_set.mqtt_pub = MQTT_DICACK;
			
			{
				//tts_play_set(AudioFuwljcg,AudioFuwljcgLen,FixAudioTypeDef);
                tts_play_set_idx(AUD_ID_MQTT_CONNECT_SUCESS,0,0);
			}
				
		}
		
		rc = MQTTYield(&para.ysw1_mqtt_client, 3000);
		if (rc != 0)
		{
			ALIOT_PRINT("Return code from yield is %d\n", rc);
			rc = MQTTDisconnect(&para.ysw1_mqtt_client);
			if (rc != 0)
				ALIOT_INFOR("Return code from MQTT disconnect is %d\n", rc);
			else
				ALIOT_INFOR("MQTT disconnect is success\n");


			para.cal_set.mqtt_con = MQTT_CACK;
			para.cal_set.mqtt_sub = MQTT_CACK;
			para.cal_set.mqtt_pub = MQTT_CACK;
			continue;
		}
//        ql_rtos_task_sleep_ms(200);
		para.mqtt_con_nums = 0;
		check_mqtt_server(&para);
	}

	ALIOT_INFOR("mqtt_exit line %d\n", __LINE__);
	
	//MQTTUnsubscribe(&ysw1_mqtt_client, topic_sub_upgrade);
	//MQTTUnsubscribe(&ysw1_mqtt_client, topic_sub_message);

	rc = MQTTDisconnect(&para.ysw1_mqtt_client);
	if (rc != 0)
		ALIOT_PRINT("Return code from MQTT disconnect is %d\n", rc);
	else
		ALIOT_PRINT("MQTT disconnect is success\n");

	mqtt_client_close(&para.ysw1_mqtt_client);
	ALIOT_PRINT("mqtt_work_set end\n");
	TermInfo.MqttIsRuning &= ~MqttMaskB;
    mqtt_task_ctrl_threadB = NULL;
	ql_rtos_task_delete(NULL);
}
#endif
extern char larktms_heartbeat_ctrlC;
int MqttExit(void)
{
	int ii;
	mqtt_ctrlA=0;
	mqtt_ctrlB=0;
	ALIOT_PRINT("<-- MqttExit -->\n");
	for(ii=0;ii<100;ii++)
	{
		if (!TermInfo.MqttIsRuning ) return 0;
		ql_rtos_task_sleep_ms(100);
	}
	return -1;
}

void start_Mqtt_task(void)
{
	usb_log_printf("TermInfo.MqttIsRuning = %d\n",TermInfo.MqttIsRuning);
	if (!(TermInfo.MqttIsRuning&MqttMaskA))
	{
		if (ql_rtos_task_create(&mqtt_task_ctrl_threadA,
						5*1024,
						98,
						"mqtt_work_taskA",
						mqtt_work_taskA,
						NULL) != OS_OK)
		{
			usb_log_printf("thread create error\n");
		}
		
	}
	if (!(TermInfo.MqttIsRuning&MqttMaskB))
	{
		if (ql_rtos_task_create(&mqtt_task_ctrl_threadB,
							5*1024,
							98,
							"mqtt_work_taskB",
							mqtt_work_taskB,
							NULL) != OS_OK)
		{
			usb_log_printf("thread create error\n");
		}
	}
#if CFG_LARKTMS_SUPPORT
	if (!(TermInfo.MqttIsRuning&LarkTmsMaskC))
	{
		if (ql_rtos_task_create(&larktms_task_ctrl_threadC,
							5*1024,
							98,
							"larktms_heartbeat_work_taskC",
							larktms_heartbeat_work_taskC,
							NULL) != OS_OK)
		{
			usb_log_printf("thread create error\n");
		}
	}
#endif	
}


#endif



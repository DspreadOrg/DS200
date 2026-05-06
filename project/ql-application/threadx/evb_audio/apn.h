#ifndef __APN__
#define __APN__
typedef struct _APN{
	const char* plmn;
	const char* apn;
	const char* user;
	const char* password;
}APN;

APN * find_apn(const char*imsi);
#endif


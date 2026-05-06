#ifndef __GPS_H__
#define __GPS_H__

#ifdef __cplusplus
	 extern "C" {
#endif

#define DEV_GPS_SUPPORT     0

#if DEV_GPS_SUPPORT
extern int GpsCtrlInit(void);
#else
#define GpsCtrlInit()
#endif

#ifdef __cplusplus
	 }
#endif

#endif


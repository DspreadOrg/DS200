#ifndef _EFUSE_H
#define _EFUSE_H

#ifdef __cplusplus
extern "C" {
#endif

//define fuse value for psram devie
#define AP_16M               0
#define WB_16M               1
#define AP_8M8M              2
#define WB_8M8M              4
#define AP_8M                5
#define WB_8M                6
#define WB_250MHZ_8M         7
#define AP_UHS_8M8M          8
#define WB_250MHZ_8M8M       9
#define AP_250MHZ_8M8M       10
#define WB_XCCELA_8M         11
#define WB_XCCELA_8M8M       12
#define WB_4M                13
#define AP_4M                14
#define WB_XCCELA_4M         15
#define AP_250MHZ_8M         16
#define WB_32M               17

unsigned efuse_psram(void);
unsigned efuse_cranew_mcu_psram(void);
int fuse_read_embed_flash(void);
void fuse_read_lotid(void);
int fuse_trust_boot_enabled(void);

#ifdef __cplusplus
}
#endif

#endif /* _EFUSE_H */
/** @} */

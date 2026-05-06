#ifndef _CPU_H_
#define _CPU_H_

#include "syscall-arom.h"

#define CRANE_MFPR_BASE                      0xD401E000
#define M_ICU_BASE                           0xD4282000
#define PMU_BASE                             0xD4282800
#define PMU_MAIN_BASE                        0xD4050000
#define QSPI_IPB_BASE                        0xD420B000
#define DMA_BASE                             0xD4000000
#define CIU_BASE                             0xD4282C00
#define QSPI_AMBA_BASE                       0x80000000

/* apb clock */
#define APBC_BASE                            0xD4015000
#define APBC_TWSI0_CLK_RST                   0x2C /* CI2C 0 */
#define APBC_TWSI1_CLK_RST                   0x60 /* CI2C 1 */
#define APBC_TWSI2_CLK_RST                   0x78 /* PI2C */
#define APBC_TWSI3_CLK_RST                   0x7C /* CI2C 2 */

/* timer */
#define CRANE_APB_TIMER0_BASE                0xD4014000
#define TIMER_BASE                           CRANE_APB_TIMER0_BASE
#define TIMER_MAX_DEV_NUM                    3   /* counter */
#define TIMER_MAX_CHAN_NUM                   3   /* match */
#define TIMER_DEV0_IRQ                       13
#define TIMER_DEV1_IRQ                       14
#define TIMER_DEV2_IRQ                       14

/* uart */
#define UART0_BASE                           0xD4017000
#define UART1_BASE                           0xD4018000
#define UART0_IRQ                            27
#define UART1_IRQ                            59
#define HAS_UART_1                           1
#define APBC_CLK_FPGA                        13000000
#define APBC_CLK_VDK                         24000000

/* usb */
#define CRANE_USB_BASE                       0xD4208000
#define USB_IRQ_NUM                          44

/* TWSI */
#define TWSI0_BASE 0xD4011000
#define TWSI1_BASE 0xD4010800
#define TWSI2_BASE 0xD4037000
#define TWSI3_BASE 0xD403E000

/* GEU */
#if AROM_VERSION == AROM_VER_2022_08_08_CRANEW_CR5
#define GEU_BASE                             0x40002000 /* cranew CR5, share use the geu of MCU */
#else
#define GEU_BASE                             0xD4201000 /* crane, craneG, craneM, craneL */
#endif

/* chip id */
#define CHIP_ID(x)                           ((x) & 0xFFFF)
#define REV_ID(x)                            (((x) >> 16) & 0xFF)

#define CHIP_ID_CRANE                        (0x6731)
#define CHIP_ID_CRANEG                       (0x3603) /*craneG A0*/
#define CHIP_ID_CRANEM                       (0x1603) /*craneM A0*/
#define CHIP_ID_FALCON                       (0x1803)
#define CHIP_ID_CRANEL                       (0x1606)
#define CHIP_ID_CRANELS                      (0x1609)
#define CHIP_ID_CRANEW                       (0x3606)

#define HW_PLATFORM_TYPE_SILICON             0
#define HW_PLATFORM_TYPE_FPGA                1
#define HW_PLATFORM_TYPE_ZEBU_Z1             2
#define HW_PLATFORM_TYPE_VDK                 3

/* CIU */
/* sw use it to store user data */
#define SW_SCRATCH                           0xE8
#define FLAGS_NORMAL_MODE                    0x0
#define FLAGS_FORCE_DOWNLOAD                 0x1
#define FLAGS_PRODUCTION_MODE                0x2

unsigned int hw_platform_type(void);
unsigned int hw_chip_id(void);
unsigned int hw_rev_id(void);

#endif /* _CPU_H_ */

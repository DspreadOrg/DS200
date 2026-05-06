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
#define UTMI_BASE                            0xD4207000

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
/* usb clk/reset reg 0x5C */
#define PMU_USB_CLK_RES_CTRL                 (PMU_BASE + 0x5c)
#define USB_AXI_CLK_EN                       (1 << 3)
#define USB_DONGLE_CLK_GATE_MASK             (1 << 1)
#define USB_AXI_RESET                        (1 << 0)
/* USB phy */
#define UTMI_USB2_REG0A                      0x28
#define UTMI_USB2_REG0B                      0x2C
#define UTMI_USB2_REG0D                      0x34
#define UTMI_USB2_REG25                      0x94
#define UTMI_USB2_REG26                      0x98
#define UTMI_USB2_REG28                      0xA0
#define UTMI_USB2_REG29                      0xA4

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

#if AROM_VERSION == AROM_VER_2022_08_08_CRANEW_CR5
/* CIU */
/*
   sw use it to store user data
   bit[3:0]: booting mode
   bit[7:4]: running stage
 */
#define SW_SCRATCH                           0xE8
/* booting mode */
#define FLAGS_NORMAL_MODE                    0x0
#define FLAGS_FORCE_DOWNLOAD                 0x1
#define FLAGS_PRODUCTION_MODE                0x2
#define FLAGS_BOOTING_MODE_MASK              0xF
#define FLAGS_BOOTING_MODE(x)                ((x) & 0xF)
/* running stage */
#define FLAGS_RUNNING_STAGE_PREBOOT_COMPLETE (0x1 << 4)
#define FLAGS_RUNNING_STAGE_MASK             (0xF << 4)
#define FLAGS_RUNNING_STAGE(x)               ((x) & (0xF << 4))
#endif

/* sd host controller 1 base address */
#define EMMC_BASE_ADDR                       0xD4280800
#define CP_TOP_SDH0_CLK_CTRL                 (PMU_BASE + 0x54)  /*sd or sdio */
#define CP_TOP_SDH1_CLK_CTRL                 (PMU_BASE + 0x58)  /*emmc */

/* chip id */
#define CHIP_ID(x)                           ((x) & 0xFFFF)
#define REV_ID(x)                            (((x) >> 16) & 0xFF)

#define CHIP_ID_CRANE                        (0x6731)
#define CHIP_ID_CRANEG                       (0x3603) /*craneG A0*/
#define CHIP_ID_CRANEM                       (0x1603) /*craneM A0*/
#define CHIP_ID_CRANEL                       (0x1606)
#define CHIP_ID_CRANELS                      (0x1609)
#define CHIP_ID_CRANEW                       (0x3606)
#define REV_ID_CRANEW_Z1                     (0x1)
#define REV_ID_CRANEW_Z2                     (0xC0)
#define CHIP_ID_CRANELR                      (0x1602)
#define CHIP_ID_CRANEGT                      (0x3605)

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

/* MFP */
#define MFP_GPIO_4                           (0xEC)
#define MFP_GPIO_5                           (0xF0)
#define MFP_GPIO_6                           (0xF4)
#define MFP_GPIO_7                           (0xF8)

#define MFP_GPIO_12                          (0x10C)
#define MFP_GPIO_13                          (0x110)
#define MFP_GPIO_14                          (0x114)
#define MFP_GPIO_15                          (0x118)

#define MFP_GPIO_16                          (0x11C)
#define MFP_GPIO_17                          (0x120)
#define MFP_GPIO_18                          (0x124)
#define MFP_GPIO_19                          (0x128)

#define MFP_GPIO_33                          (0x160)
#define MFP_GPIO_34                          (0x164)
#define MFP_GPIO_35                          (0x168)
#define MFP_GPIO_36                          (0x16C)

#define MFP_GPIO_61                          (0x308)
#define MFP_GPIO_62                          (0x30C)
#define MFP_GPIO_63                          (0x310)
#define MFP_GPIO_64                          (0x314)

/* CraneG has different offset with other chips */
#define MFP_GPIO_71                          (0x1BC)
#define MFP_GPIO_72                          (0x1C0)
#define MFP_GPIO_73                          (0x1C4)
#define MFP_GPIO_74                          (0x1C8)

#define QSPI_DAT3_MPFI                       (0x2C4)
#define QSPI_DAT2_MPFI                       (0x2C8)
#define QSPI_DAT1_MPFI                       (0x2CC)
#define QSPI_DAT0_MPFI                       (0x2D0)
#define QSPI_CLK_MPFI                        (0x2D4)
#define QSPI_CS1_MPFI                        (0x2D8)
#define QSPI_CS2_MPFI                        (0x2DC)
#define QSPI_DQM_MPFI                        (0x2E0)

unsigned int hw_platform_type(void);
unsigned int hw_chip_id(void);
unsigned int hw_rev_id(void);

#endif /* _CPU_H_ */

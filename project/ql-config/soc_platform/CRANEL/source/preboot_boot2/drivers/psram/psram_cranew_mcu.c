#include "psram.h"
#include "cpu.h"
#include "property.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "efuse.h"

#include "log.h"
#define LOG_MODULE "PsramMcu"
#define LOG_LEVEL LOG_LEVEL_MAIN

/*#define DDR_FPGA_PHY */
/*#define FIX_LATENCY */

#define NEW_VERSION_MCU_PSRAM

#define PSRAM_BASE_NEW  0x43000000
#define PSRAM_TYPE_NEW  0x1
#define WINBOND
#define PSPAD_SWAP

/* if need update mcu psram freq, instead of default 120MHz, define MCU_PSC_DFC */
/* #define MCU_PSC_DFC */

#define libEFUSE

#define REG32(addr) (*(volatile unsigned int *)(addr))

/*#define PS_TABLE_DEXC_IDX 21 / 0 disable */
/*#define PS_TABLE_EEXC_IDX 22  / 1 disable */

enum {
    WBD_DRIVE_34ohm = 0,
    WBD_DRIVE_115ohm,
    WBD_DRIVE_67ohm,
    WBD_DRIVEE_46ohm,
    WBD_DRIVEE_34ohm,
    WBD_DRIVEE_27ohm,
    WBD_DRIVEE_22ohm,
    WBD_DRIVEE_19ohm,
};

static const char *wbd_drive_list[] = { "34ohm", "115ohm", "67ohm", "46ohm", "34ohm", "27ohm", "22ohm", "19ohm" };

#define WBD_DRIVE   WBD_DRIVE_115ohm

#define PSRAM_BASE 0x43000000
static int psram_write_mr(unsigned mr_addr, unsigned cs, unsigned num)
{
    unsigned read_data = 0;

    //read cur_fp_wr
    read_data = REG32(PSRAM_BASE + 0x8000);

    //modify cur_fp_wr(bit12_13)  - - select mr_byte0
    read_data &= 0xffffcfff;
    REG32(PSRAM_BASE + 0x8000) = read_data;


    /*REG_C, Config PSC*/ /*
       fp_mr_data_0[0]*/REG32(PSRAM_BASE + 0x8024) = num;


    // write MR register
    REG32(PSRAM_BASE + 0x8034) = mr_addr + (cs << 23);
    REG32(PSRAM_BASE + 0x8030) = 0x10a;


    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    //LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE + 0x8034), REG32(PSRAM_BASE + 0x8034));
    //LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE + 0x8038), read_data);

    return read_data;
}

static unsigned version;

#ifdef libEFUSE
static unsigned psram_fuse(void)
{
    /*craneW: Bit 177~173 */
    unsigned version_info = efuse_cranew_mcu_psram();

    char psram_str[PROP_VALUE_MAX] = "";

#if 0
    /* fake for sv test without fuse burn. */
    if (version_info == 0) {
        version_info = WB_32M;
        LOG_PRINT("No fuse bit , fake it to WB_32M instead of AP_16M\n");
    }
#endif
    LOG_INFO("########################\n");
    LOG_INFO("Version ID : %d\n", version_info);
    switch (version_info) {
        case AP_16M:
            memcpy(psram_str, "AP_16M", PROP_VALUE_MAX);
            break;
        case WB_16M:
            memcpy(psram_str, "WB_16M", PROP_VALUE_MAX);
            break;
        case AP_8M8M:
            memcpy(psram_str, "AP_8M8M", PROP_VALUE_MAX);
            break;
        case WB_8M8M:
            memcpy(psram_str, "WB_8M8M", PROP_VALUE_MAX);
            break;
        case AP_8M:
            memcpy(psram_str, "AP_8M", PROP_VALUE_MAX);
            break;
        case WB_8M:
            memcpy(psram_str, "WB_8M", PROP_VALUE_MAX);
            break;
        case WB_250MHZ_8M:
            memcpy(psram_str, "WB_250MHZ_8M", PROP_VALUE_MAX);
            break;
        case AP_UHS_8M8M:
            memcpy(psram_str, "AP_UHS_8M8M", PROP_VALUE_MAX);
            break;
        case WB_250MHZ_8M8M:
            memcpy(psram_str, "WB_250MHZ_8M8M", PROP_VALUE_MAX);
            break;
        case AP_250MHZ_8M8M:
            memcpy(psram_str, "AP_250MHZ_8M8M", PROP_VALUE_MAX);
            break;
        case WB_XCCELA_8M:
            memcpy(psram_str, "WB_XCCELA_8M", PROP_VALUE_MAX);
            break;
        case WB_XCCELA_8M8M:
            memcpy(psram_str, "WB_XCCELA_8M8M", PROP_VALUE_MAX);
            break;
        case WB_4M:
            memcpy(psram_str, "WB_4M", PROP_VALUE_MAX);
            break;
        case AP_4M:
            memcpy(psram_str, "AP_4M", PROP_VALUE_MAX);
            break;
        case WB_XCCELA_4M:
            memcpy(psram_str, "WB_XCCELA_4M", PROP_VALUE_MAX);
            break;
        case AP_250MHZ_8M:
            memcpy(psram_str, "AP_250MHZ_8M", PROP_VALUE_MAX);
            break;
        case WB_32M:
            memcpy(psram_str, "WB_32M", PROP_VALUE_MAX);
            break;
        default:
            LOG_PRINT("Winbond Device\n");
            break;
    }

    LOG_PRINT("%s\n", psram_str);
    asr_property_set("fuse.psram.type", psram_str);

    fuse_read_embed_flash();
    LOG_INFO("########################\n");
    return version_info;
}

#endif

static int psram_read_mr(unsigned mr_addr, unsigned cs)
{
    unsigned read_data = 0;

    /* read MR register */
    REG32(PSRAM_BASE_NEW + 0x8034) = mr_addr + (cs << 23);
    REG32(PSRAM_BASE_NEW + 0x8030) = 0x109;
    read_data = REG32(PSRAM_BASE_NEW + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE_NEW + 0x8030);
    }
    read_data = REG32(PSRAM_BASE_NEW + 0x8038);                                                             /* this is the mr read data value */
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE_NEW + 0x8034), REG32(PSRAM_BASE_NEW + 0x8034));
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE_NEW + 0x8038), read_data);   /* bit 6~4 is drive_strength */

    return read_data;
}

/* type=0 top psc */
/* type=1 mcu psc initial */
/* type=2 mcu psc wakeup */
void psram_init_cranew_mcu(void)
{
    unsigned read_data;
    int i;
    unsigned mcu_psc;
    unsigned psram_base, type;

    unsigned reg = 0;
    unsigned drive_strength = 0;

    psram_base = PSRAM_BASE_NEW;
    type = PSRAM_TYPE_NEW;

    drive_strength = WBD_DRIVE;
    /*get version information */
#ifdef libEFUSE
    version = psram_fuse();
#endif

    /* LOG_PRINT("[%s] psram_base:[0x%x]   type:[0x%x] !!!  \n", __func__, psram_base, type); */

#if defined(NEW_VERSION_MCU_PSRAM)
    LOG_PRINT("cranew new psram init for STAR(MCU) -- silicon !!! drive_strength = WBD_DRIVE_%s \n", wbd_drive_list[drive_strength]);
#elif defined(NEW_VERSION_CR5_PSRAM)
    LOG_INFO("psram_init  - -  cranew new psram init for CR5 --silicon!!!  \n");
#endif

    mcu_psc = (type == 1) || (type == 2);

    if (type == 0) {
        /*Set GPIO_16 to func1(psram_rstn) and enable gpio pad for reset */
        REG32(/*APBC_AIB_CLK_RST*/ 0xD4015000 + 0x3c) = 0x3; /* 0xFFFFFFFB; */
        for (i = 0; i < 10; i++) {}
        REG32(/*MPFI_REG_BASE*/ 0xD401E000 + 71 * 4) = (1 << 15) | (1 << 14) | 0x1;

        /*enable psram hclk */
        REG32(0xd42828f4) = 0x3;
        /* REG32(0xd4282800+0x15c) |= 0x50800;    //enable pu for dll and phy,and phy_clk rdy */

#if 0          /*remove by zhenbin */
        /*enable pll2 */
#ifdef SDF_ON
        switch (SET_PP_TEST) {
            /*only change tc */
            case PP_SDF_TC:
                REG32(MSG_PORT) = 0xabcd000a;
                REG32(0xd4090000 + 0x120) = (0x80 << 24) | (0x50 << 16) | (0xDD << 8) | 0x67;
                REG32(0xd4090000 + 0x128) = (0x3e << 24) | 0xE27627;
            case PP_SDF_WC:
                REG32(MSG_PORT) = 0xabcd000b;
            default:
                REG32(MSG_PORT) = 0xabcd000f;
        }
#endif

        REG32(0xd4090000 + 0x124) |= 0x3f;
        REG32(0xd4090000 + 0x128) |= 0x80000000;
#endif

        /*REG32(0xd42828b0) |= (1 << 23) | (0x5 << 18);     / *config for clk * / */
    }
    else if (type == 1) {
        /*Set MGPIO_52 to func1(psram_rstn) and enable gpio pad for reset */
        /* update to alwayson for resetn and csn pads */
        REG32(0x41200010) = 0x3;
        for (i = 0; i < 10; i++) {}
        REG32(0x41207000 + 97 * 4) |= (1 << 15) | (1 << 14) | 0x1;

#if defined(MCU_PSC_DFC)
        read_data = REG32(0x410000b0);
        read_data &= ~(0x3 << 16);
        REG32(0x410000b0) = read_data;
        /* eg: freq = 240/(n+1)/2 MHz: 0: 120MHz; 1: 60MHz */
        unsigned mcu_psc_clk_div = 0;
        REG32(0x410000b0) |= (mcu_psc_clk_div << 16) | (1 << 23);  //17:16 div, 23 trig fc
        do {
            read_data = REG32(0x410000b0);
        } while ((read_data & (1 << 23)));
        LOG_PRINT("update freq, mcu_psc_clk_div, reg[0x410000b0] = 0x%x\n", REG32(0x410000b0));
#endif
    }

    if (mcu_psc) {
        /* enable clock gating for PSC,MCU_AP reg */
        REG32(0x43300040) = 0xff;
    }

    /* reload initial value */
#ifdef WINBOND
    REG32(psram_base + 0x4024) = (mcu_psc << 16) | (mcu_psc << 12) | (1 << 8) | 0x7;
    REG32(psram_base + 0x8008) |= 0x10;
#else
    REG32(psram_base + 0x4024) = (mcu_psc << 16) | (mcu_psc << 12) | (0 << 8) | 0x7;
#endif

#if 0 /*remove by zhenbin */
#ifndef NOT_SIM_SPEED_UP
    /* NOTE, only for simulation, mclk is 3M+ instead of 32K, also update model to 1/10 */
    /* 7:4	wait_mp_cycle		RW	0	wait_2 instr count for this+1 mclk as 1 step */
    /* 3:0	wait_up_cycle		RW	0	wait_1 instr count for this+1 uclk as 1 step */
    REG32(psram_base + 0x802c) = 0x70;
#endif
#endif

#ifdef DDR_FPGA_PHY
#ifdef FIX_LATENCY
#ifdef WINBOND
#ifdef NEG_SAMPLE
    REG32(psram_base + 0x18008) = 0xc7ff;                   /*neg, fix */
    REG32(psram_base + 0x19008) = 0xc7ff;                   /*neg, fix */
#else
    REG32(psram_base + 0x18008) = 0x47ff;                   /*pos, fix */
    REG32(psram_base + 0x19008) = 0x47ff;                   /*pos, fix */
#endif
#else
#ifdef NEG_SAMPLE
    REG32(psram_base + 0x18008) = 0xf000;                   /*neg, fix */
    REG32(psram_base + 0x19008) = 0xf000;                   /*neg, fix */
#else
#if defined(NEW_VERSION_CR5_PSRAM)
    REG32(psram_base + 0x18008) = 0x7000;                   /*pos, fix */
    REG32(psram_base + 0x19008) = 0x7000;                   /*pos, fix */
#elif defined(NEW_VERSION_MCU_PSRAM)
    REG32(psram_base + 0x18008) = 0xf1ff;                   /*pos, viable */
    REG32(psram_base + 0x19008) = 0xf1ff;                   /*pos, viable */
    REG32(psram_base + 0x1a008) = 0xf1ff;                   /*pos, viable */
    REG32(psram_base + 0x1b008) = 0xf1ff;                   /*pos, viable */
#endif
#endif
#endif
#else
#ifdef NEG_SAMPLE
    REG32(psram_base + 0x18008) = 0xb000;               /*neg, viable */
    REG32(psram_base + 0x19008) = 0xb000;               /*neg, viable */
#else
    REG32(psram_base + 0x18008) = 0x3000;               /*pos, viable */
    REG32(psram_base + 0x19008) = 0x3000;               /*pos, viable */
#endif
#endif
#endif

    if (type != 2) {
        /* reset device */
        /* tRP RESET# low pulse width  1us */
        /* tRST Reset to CMD valid     2us */
        REG32(psram_base + 0x18000) = 0x1;
        for (i = 0; i < 20; i++) {
            read_data = REG32(psram_base + 0x18000);
        }
        REG32(psram_base + 0x18000) = 0x5;

        if ((type == 1)) {
            /* get delayline code from top psram phy */
            /* 1. aclk reset release */
            read_data = REG32(0x41000000 + 0x10);
            while ((read_data & (1 << 29)) == 0) {
                read_data = REG32(0x41000000 + 0x10);
            }

            /* lijin: 2021_08_19-16:01 TODO, NOTE, enable top psram hclk here for require dll code, may should sync with cr5 initial sequence!! */
            /*enable psram hclk */
            REG32(0xd42828f4) = 0x3;

#if defined(SDF_ON)
            /* add delay for read dll after it lock or dll code may cause X to cpu */
            for (i = 0; i < 50; i++) {
                read_data = REG32(0xc0100000 + 0x18000);
            }
#endif

#ifdef PSC_BYPS_DLLC
            read_data = 0x7f;
#else
            /* 2. wait for top psc phy dll status */
            read_data = REG32(0xc0100000 + 0x18010);
            while ((read_data & 0x100) != 0x100) {
                read_data = REG32(0xc0100000 + 0x18010);
            }
#endif

            /* 3. read dll code from top and write to mcu */
            /* dll is freeze if no access to psram, so read code directly */
            /* #if defined(SDF_ON)&&defined(DIS_PSC_CACHE) */
            /* read_data = 0x56;// lijin: 2021_09_30-15:55 TODO, hack for dll_code=0 in read path */
            /* #else */
            read_data &= 0xff;
            /* #endif */
            read_data = read_data * 153 / 240 / 2;                    /* ui/2, top psram_2x is 153M, mcu psram_2x is 240M */
            read_data |= read_data << 16;
            /* dll code */
            REG32(psram_base + 0x18008) = read_data;                /* psc dphy reg */
            REG32(0x41000000 + 0xac) = read_data;                   /* aon reg */

            /* dll range */
            read_data = REG32(0xc0100000 + 0x18008);
            read_data = read_data >> 2;
            read_data &= 0x3 << 8;

            /* range and bypass enable */
            REG32(psram_base + 0x18008) |= read_data | (0x1 << (8 + 2));  /* psc dphy reg */

            /* enable pmu control */
            REG32(0x41000000 + 0xac) |= read_data | (0x1 << (8 + 2)) | (0x1 << 25); /* aon reg */

#ifdef PSPAD_SWAP
            REG32(0x41000000 + 0xac) |= (0x1 << 28);
#endif

            for (i = 0; i < 20; i++) {
                read_data = REG32(psram_base + 0x18000);                    /* for wdt reinit not enough time */
            }
        }
        else {
            for (i = 0; i < 40; i++) {
                read_data = REG32(psram_base + 0x18000);
            }
        }
    }

#ifdef FIX_LATENCY
#ifdef WINBOND
    /* low freq */
    /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80dc) = 0x840bc49f;
    /* high freq */
    /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80f8) = 0x2402842b;
    REG32(psram_base + 0x8024) = 0x0b9f;
    REG32(psram_base + 0x9024) = 0x2b9f;
#else
    /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80d8) = 0x24054433;
    REG32(psram_base + 0x8024) = 0x402b;
    REG32(psram_base + 0x9024) = 0x2033;
#endif
#endif

#ifdef DIS_PSC_CACHE
    /*disable cache */
    read_data = REG32(psram_base + 0x4000);
    read_data = read_data & 0xff0f;
    REG32(psram_base + 0x4000) = read_data;
#endif

    if (type == 0) {
        /*wait for dll status */
        read_data = REG32(psram_base + 0x18010);
        while ((read_data & 0x100) != 0x100) {
            read_data = REG32(psram_base + 0x18010);
        }
#ifdef  PSC_CKB_AS_CK
        /* phy_reg4[5]  enable clkb */
        /* phy_reg3[4]  clkb as clk, disable clk */
        /* phy_reg4[3]  apm32 en, if 1 override above */
        /*FP0 */
        read_data = REG32(psram_base + 0x18004);
        read_data |= (1 << (24 + 5)) | (1 << (16 + 4));
        REG32(psram_base + 0x18004) = read_data;
        /* FP1 */
        read_data = REG32(psram_base + 0x19004);
        read_data |= (1 << (16 + 4));
        REG32(psram_base + 0x19004) = read_data;
#endif
    }

#ifdef WINBOND
    /* enable phy for winbond */
    read_data = REG32(psram_base + 0x18004);
    read_data |= 1 << (24 + 4);
    REG32(psram_base + 0x18004) = read_data;
#endif

    if (type == 2) {
        /* exit from half sleep */
        REG32(psram_base + 0x8030) = 0x108;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
    }
    else {
#ifdef WINBOND
        /* // lijin: 2021_08_23-11:53 TODO cs1, temp read mr for if wdt reset initial, ?? */
        /* REG32(PSRAM_BASE + 0x8034) = 0x0; */
        /* REG32(PSRAM_BASE + 0x8030) = 0x109; */
        /* read_data = REG32(PSRAM_BASE + 0x8030) ; */
        /* while ((read_data & 0x100)!=0x0) { */
        /* read_data = REG32(PSRAM_BASE + 0x8030) ; */
        /* } */
#else
        /* global reset for if initial for wdt reset */
        REG32(psram_base + 0x8030) = 0x105;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
#endif

#ifdef WINBOND
        /* program MR for */
        /* 1. hybird read */
        /* 2. fix latency or variable latency */
        REG32(psram_base + 0x8030) = 0x104;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
#else
#ifdef FIX_LATENCY
        /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80d8) = 0x2405442b;
        REG32(psram_base + 0x8030) = 0x104;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
        /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80d8) = 0x24054433;
#endif
#endif
    }

    /*mmap */
#if defined(NEW_VERSION_MCU_PSRAM)
    /* address remap */
    REG32(psram_base + 0x8078) = 0x87868584;
    REG32(psram_base + 0x807c) = 0x8b8a8988;
    REG32(psram_base + 0x8080) = 0x8f8e8d8c;
    REG32(psram_base + 0x8084) = 0x93929190;
    REG32(psram_base + 0x8088) = 0x97969594;
    REG32(psram_base + 0x808c) = 0x9b9a0098;
    REG32(psram_base + 0x8060) = 0x4838281;

    REG32(psram_base + 0x0004) = 0x3009000D;        /*32M byte */
#elif defined(NEW_VERSION_CR5_PSRAM)
    REG32(psram_base + 0x0004) = 0x7E08000D;        /*16M byte */
#endif

    LOG_INFO("MCU PSRAM [0x%x]:0x%x  !!!  \n", psram_base + 0x0004, REG32(psram_base + 0x0004));

    /*enable DFC */
    /* / *SEQ_TABLE* / REG32(PSRAM_BASE + 0x8000) |= ( (0x1<<24) | 0x1 ); */
#if defined(SDF_ON) || defined(PSRAM_HIGH)
    if (type == 0) {
#ifdef SDF_ON
        /* 175M */
        read_data = REG32(0xd42828b0);      /*dclk fc request */
        read_data = read_data & 0xffe3ff87;
        REG32(0xd42828b0) = 0x01000000 | (0x1 << 26) | (0x1 << 18) | (1 << 6) | (1 << 4) | read_data;
#else // ~defined(SDF_ON)
        /* 156M */
        read_data = REG32(0xd42828b0);      /*dclk fc request */
        read_data = read_data & 0xffe3ff87;
        REG32(0xd42828b0) = 0x01000000 | (0x1 << 18) | (1 << 6) | (1 << 4) | read_data;
#endif // PSRAM_HIGH end

        read_data = REG32(0xd42828b0);
        while (read_data & (1 << 24) != 0) {
            read_data = REG32(0xd42828b0);
        }
    }
#endif
    /* initial psc done */

    reg = REG32(psram_base + 0x8024);
    reg &= ~(0x7 << 4);
    reg |= drive_strength << 4;

    psram_write_mr(0x1000, 0, reg);
    psram_read_mr(0x1000, 0);

#ifdef WINBOND
    psram_read_mr(0x0, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x1000, 0);
    psram_read_mr(0x1002, 0);
#else
    psram_read_mr(0x0, 0);
    psram_read_mr(0x1, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x3, 0);
    psram_read_mr(0x4, 0);
#endif
} /* init_psram end */

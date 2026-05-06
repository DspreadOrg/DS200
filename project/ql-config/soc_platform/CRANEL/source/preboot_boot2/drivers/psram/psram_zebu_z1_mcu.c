#include <stdio.h>
#include <stdbool.h>

#include "log.h"
#define LOG_MODULE "PsramPZ1"
#define LOG_LEVEL LOG_LEVEL_MAIN

#define REG32(addr) (*(volatile unsigned int *)(addr))
#define PSRAM_BASE 0x43000000

//#define DDR_FPGA_PHY
#define FIX_LATENCY

#define NEW_VERSION_MCU_PSRAM

#define PSRAM_BASE_NEW  0x43000000
#define PSRAM_TYPE_NEW  0x1
//#define WINBOND
//#define PSPAD_SWAP       //emulator don't need this, only silicon need this!!!

//#define	PS_TABLE_DEXC_IDX 21	// 0 disable
//#define	PS_TABLE_EEXC_IDX 22	// 1 disable

#if 0
static int psram_read_mr(unsigned mr_addr, unsigned cs)
{
    unsigned read_data = 0;

    // read MR register
    REG32(PSRAM_BASE_NEW + 0x8034) = mr_addr + (cs << 23);
    REG32(PSRAM_BASE_NEW + 0x8030) = 0x109;
    read_data = REG32(PSRAM_BASE_NEW + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE_NEW + 0x8030);
    }
    read_data = REG32(PSRAM_BASE_NEW + 0x8038);     // this is the mr read data value
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE_NEW + 0x8034), REG32(PSRAM_BASE_NEW + 0x8034));
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE_NEW + 0x8038), read_data);

    return read_data;
}
#endif

// type=0 top psc
// type=1 mcu psc initial
// type=2 mcu psc wakeup
//void psram_init(void)
void psram_init_zebu_z1_mcu(void)
{
    unsigned read_data;
    int i;
    unsigned mcu_psc;

    unsigned psram_base, type;

    psram_base = PSRAM_BASE_NEW;
    type = PSRAM_TYPE_NEW;

    LOG_INFO("psram_init_zebu_z1_mcu\n");

    #if defined(NEW_VERSION_MCU_PSRAM)
    LOG_INFO("psram_init  - -  cranew new psram init for STAR !!! --EMULATOR \n");
    #elif defined(NEW_VERSION_CR5_PSRAM)
    LOG_INFO("psram_init  - -  cranew new psram init for CR5 !!! --EMULATOR \n");
    #endif

    mcu_psc = (type == 1) || (type == 2);

    if (type == 0) {
        //Set GPIO_16 to func1(psram_rstn) and enable gpio pad for reset
        REG32(/*APBC_AIB_CLK_RST*/ 0xD4015000 + 0x3c) = 0x3; // 0xFFFFFFFB;
        for (i = 0; i < 10; i++) {}
        REG32(/*MPFI_REG_BASE*/ 0xD401E000 + 71 * 4) = (1 << 15) | (1 << 14) | 0x1;

        //enable psram hclk
        REG32(0xd42828f4) = 0x3;
        // REG32(0xd4282800+0x15c) |= 0x50800;    //enable pu for dll and phy,and phy_clk rdy

        #if 0  //remove by zhenbin
        //enable pll2
        #ifdef SDF_ON
        switch (SET_PP_TEST) { //only change tc
            case  PP_SDF_TC:
                REG32(MSG_PORT) = 0xabcd000a;
                REG32(0xd4090000 + 0x120) = (0x80 << 24) | (0x50 << 16) | (0xDD << 8) | 0x67;
                REG32(0xd4090000 + 0x128) = (0x3e << 24) | 0xE27627;
            case  PP_SDF_WC:
                REG32(MSG_PORT) = 0xabcd000b;
            default:
                REG32(MSG_PORT) = 0xabcd000f;
        }
        #endif

        REG32(0xd4090000 + 0x124) |= 0x3f;
        REG32(0xd4090000 + 0x128) |= 0x80000000;
        #endif

        #ifndef CRANEW_Z1_ZEBU_ENVIROMENT
        REG32(0xd42828b0) |= (1 << 23) | (0x5 << 18);     //config for clk
        #endif

    }
    else if (type == 1) {
        //Set MGPIO_52 to func1(psram_rstn) and enable gpio pad for reset
        // update to alwayson for resetn and csn pads
        REG32(0x41200010) = 0x3;
        for (i = 0; i < 10; i++) {}
        REG32(0x41207000 + 97 * 4) |= (1 << 15) | (1 << 14) | 0x1;

        #if defined(MCU_PSC_HFSPD)
        read_data = REG32(0x410000b0);
            #if defined(MCU_PSC_HFSPD)
        // 122M
        REG32(0x410000b0) |= (3 << 16) | (1 << 23) | (1 << 19);      //19:sel,  17:16 div, 23 trig fc
            #else
        // 120M
        REG32(0x410000b0) |= (1 << 16) | (1 << 23);                 //19:sel,  17:16 div, 23 trig fc
            #endif

        read_data = REG32(0x410000b0);
        while ((read_data & (1 << 23)) == (1 << 23)) {
            read_data = REG32(0x410000b0);
        }
        #endif
    }


    if (mcu_psc) {
        // enable clock gating for PSC,MCU_AP reg
        REG32(0x43300040) = 0xff;
    }


    // reload initial value
    #ifdef WINBOND
    REG32(psram_base + 0x4024) = (mcu_psc << 16) | (mcu_psc << 12) | (1 << 8) | 0x7;
    REG32(psram_base + 0x8008) |= 0x10;
    #else
    REG32(psram_base + 0x4024) = (mcu_psc << 16) | (mcu_psc << 12) | (0 << 8) | 0x7;
    #endif


#if 0 //remove by zhenbin
    #ifndef NOT_SIM_SPEED_UP
    // NOTE, only for simulation, mclk is 3M+ instead of 32K, also update model to 1/10
    // 7:4	wait_mp_cycle		RW	0	wait_2 instr count for this+1 mclk as 1 step
    // 3:0	wait_up_cycle		RW	0	wait_1 instr count for this+1 uclk as 1 step
    REG32(psram_base + 0x802c) = 0x70;
    #endif
#endif


    #ifdef DDR_FPGA_PHY
        #ifdef FIX_LATENCY
            #ifdef WINBOND
                #ifdef NEG_SAMPLE
    REG32(psram_base + 0x18008) = 0xc7ff;                   //neg, fix
    REG32(psram_base + 0x19008) = 0xc7ff;                   //neg, fix
                #else
    REG32(psram_base + 0x18008) = 0x47ff;                   //pos, fix
    REG32(psram_base + 0x19008) = 0x47ff;                   //pos, fix
                #endif
            #else
                #ifdef NEG_SAMPLE
    REG32(psram_base + 0x18008) = 0xf000;                   //neg, fix
    REG32(psram_base + 0x19008) = 0xf000;                   //neg, fix
                #else
                    #if defined(NEW_VERSION_CR5_PSRAM)
    REG32(psram_base + 0x18008) = 0x7000;                   //pos, fix
    REG32(psram_base + 0x19008) = 0x7000;                   //pos, fix
                    #elif defined(NEW_VERSION_MCU_PSRAM)
    REG32(psram_base + 0x18008) = 0xf1ff;                   //pos, viable
    REG32(psram_base + 0x19008) = 0xf1ff;                   //pos, viable
    REG32(psram_base + 0x1a008) = 0xf1ff;                   //pos, viable
    REG32(psram_base + 0x1b008) = 0xf1ff;                   //pos, viable
                    #endif
                #endif
            #endif
        #else
            #ifdef NEG_SAMPLE
    REG32(psram_base + 0x18008) = 0xb000;               //neg, viable
    REG32(psram_base + 0x19008) = 0xb000;               //neg, viable
            #else
    REG32(psram_base + 0x18008) = 0x3000;               //pos, viable
    REG32(psram_base + 0x19008) = 0x3000;               //pos, viable
            #endif
        #endif
    #endif

    if (type != 2) {
        // reset device
        // tRP RESET# low pulse width  1us
        // tRST Reset to CMD valid     2us
        REG32(psram_base + 0x18000 ) = 0x1;
        for (i = 0; i < 20; i++) {
            read_data = REG32(psram_base + 0x18000);
        }
        REG32(psram_base + 0x18000 ) = 0x5;

        if ((type == 1)) {
            // get delayline code from top psram phy
            // 1. aclk reset release
            read_data = REG32(0x41000000 + 0x10);
            while ((read_data & (1 << 29)) == 0) {
                read_data = REG32(0x41000000 + 0x10);
            }

            // lijin: 2021_08_19-16:01 TODO, NOTE, enable top psram hclk here for require dll code, may should sync with cr5 initial sequence!!
            //enable psram hclk
            REG32(0xd42828f4) = 0x3;

            #if defined(SDF_ON)
            // add delay for read dll after it lock or dll code may cause X to cpu
            for (i = 0; i < 50; i++) {
                read_data = REG32(0xc0100000 + 0x18000);
            }
            #endif

            #ifdef PSC_BYPS_DLLC
            read_data = 0x7f;
            #else
            // 2. wait for top psc phy dll status
            read_data = REG32(0xc0100000 + 0x18010);
            while ((read_data & 0x100) != 0x100) {
                read_data = REG32(0xc0100000 + 0x18010);
            }
            #endif

            // 3. read dll code from top and write to mcu
            // dll is freeze if no access to psram, so read code directly
            // #if defined(SDF_ON)&&defined(DIS_PSC_CACHE)
            // read_data = 0x56;// lijin: 2021_09_30-15:55 TODO, hack for dll_code=0 in read path
            // #else
            read_data &= 0xff;
            // #endif
            read_data = read_data * 153 / 240 / 2;                    // ui/2, top psram_2x is 153M, mcu psram_2x is 240M
            read_data |= read_data << 16;
            // dll code
            REG32(psram_base + 0x18008) = read_data;                // psc dphy reg
            REG32(0x41000000 + 0xac) = read_data;                   // aon reg

            // dll range
            read_data = REG32(0xc0100000 + 0x18008);
            read_data = read_data >> 2;
            read_data &= 0x3 << 8;

            // range and bypass enable
            REG32(psram_base + 0x18008) |= read_data | (0x1 << (8 + 2)); // psc dphy reg

            // enable pmu control
            REG32(0x41000000 + 0xac) |= read_data | (0x1 << (8 + 2)) | (0x1 << 25); // aon reg

            #ifdef PSPAD_SWAP
            REG32(0x41000000 + 0xac) |= (0x1 << 28);
            #endif

            for (i = 0; i < 20; i++) {
                read_data = REG32(psram_base + 0x18000);                    // for wdt reinit not enough time
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
    // low freq
    /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80dc) = 0x840bc49f;
    // high freq
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
    //disable cache
    read_data = REG32(psram_base + 0x4000);
    read_data = read_data & 0xff0f;
    REG32(psram_base + 0x4000) = read_data;
    #endif


    if (type == 0) {
        //wait for dll status
        read_data = REG32(psram_base + 0x18010);
        while ((read_data & 0x100) != 0x100) {
            read_data = REG32(psram_base + 0x18010);
        }
        #ifdef  PSC_CKB_AS_CK
        // phy_reg4[5]  enable clkb
        // phy_reg3[4]  clkb as clk, disable clk
        // phy_reg4[3]  apm32 en, if 1 override above
        //FP0
        read_data = REG32(psram_base + 0x18004);
        read_data |= (1 << (24 + 5)) | (1 << (16 + 4));
        REG32(psram_base + 0x18004) = read_data;
        // FP1
        read_data = REG32(psram_base + 0x19004);
        read_data |= (1 << (16 + 4));
        REG32(psram_base + 0x19004) = read_data;
        #endif
    }

    #ifdef WINBOND
    // enable phy for winbond
    read_data = REG32(psram_base + 0x18004);
    read_data |= 1 << (24 + 4);
    REG32(psram_base + 0x18004) = read_data;
    #endif

    if (type == 2) {
        // exit from half sleep
        REG32(psram_base + 0x8030) = 0x108;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
    }
    else {
        #ifdef WINBOND
        // // lijin: 2021_08_23-11:53 TODO cs1, temp read mr for if wdt reset initial, ??
        // REG32(PSRAM_BASE + 0x8034) = 0x0;
        // REG32(PSRAM_BASE + 0x8030) = 0x109;
        // read_data = REG32(PSRAM_BASE + 0x8030) ;
        // while ((read_data & 0x100)!=0x0) {
        // read_data = REG32(PSRAM_BASE + 0x8030) ;
        // }
        #else
        // global reset for if initial for wdt reset
        REG32(psram_base + 0x8030) = 0x105;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
        #endif

        #ifdef WINBOND
        // program MR for
        // 1. hybird read
        // 2. fix latency or variable latency
        REG32(psram_base + 0x8030) = 0x104;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
        #else
            #ifdef FIX_LATENCY
        ///*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80d8) = 0x2405442b;
        /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80d8) = 0x24054431;
        REG32(psram_base + 0x8030) = 0x104;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
        /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80d8) = 0x24054433;
            #endif
        #endif
    }

    //enable DFC
    // /*SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8000) |= ( (0x1<<24) | 0x1 );
    #if defined(SDF_ON) || defined(PSRAM_HIGH)
    if (type == 0) {
            #ifdef SDF_ON
        // 175M
        read_data = REG32(0xd42828b0);      //dclk fc request
        read_data = read_data & 0xffe3ff87;
        REG32(0xd42828b0) = 0x01000000 | (0x1 << 26) | (0x1 << 18) | (1 << 6) | (1 << 4) | read_data;
            #else                       // ~defined(SDF_ON)
        // 156M
        read_data = REG32(0xd42828b0);  //dclk fc request
        read_data = read_data & 0xffe3ff87;
        REG32(0xd42828b0) = 0x01000000 | (0x1 << 18) | (1 << 6) | (1 << 4) | read_data;
            #endif // PSRAM_HIGH end

        read_data = REG32(0xd42828b0);
        while (read_data & (1 << 24) != 0) {
            read_data = REG32(0xd42828b0);
        }
    }
    #endif
    // initial psc done
#if 0
    psram_read_mr(0x0, 0);
    psram_read_mr(0x1, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x3, 0);
    psram_read_mr(0x4, 0);

    psram_read_mr(0x0, 1);
    psram_read_mr(0x1, 1);
    psram_read_mr(0x2, 1);
    psram_read_mr(0x3, 1);
    psram_read_mr(0x4, 1);
#endif
} // init_psram end

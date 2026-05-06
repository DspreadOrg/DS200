#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "psram.h"
#include "pmic.h"
#include "cpu.h"
#include "secureboot.h"
#include "aboot.h"
#include "reg.h"
#include "smux.h"
#include "property.h"
#include "efuse.h"
#include "syscall-arom.h"
#if defined(CPU_MODEL_CRANE)
#include "freq.h"
#endif

#include "log.h"
#define LOG_MODULE "Preboot"
#define LOG_LEVEL LOG_LEVEL_MAIN

/*---------------------------------------------------------------------------*/
#if defined(CPU_MODEL_CRANE)
extern char __lzma_decode_start, __lzma_decode_size;
#endif
/*---------------------------------------------------------------------------*/
#define UMTI_BASE                 0xD4207000
#define USB_PHY_REG_04            0x10
#define USB_PHY_REG_25            0x94
#define USB_PHY_REG_28            0xA0
#define USB_PHY_REG_29            0xA4
#define HS_ISEL_MASK              (0x3 << 12)
#define HS_ISEL                   (0x3 << 12)
/*---------------------------------------------------------------------------*/

#define	QUECTEL_PREBOOT_OPT
#ifdef QUECTEL_PREBOOT_OPT
#include "i2c.h"
static int Quec_PMIC_GET_PWEKY_STATUS(void)
{
	uint8_t reg_val;
	pi2c_read_reg(0x01, &reg_val);
	if(reg_val & (0x01<<1))
	{
		return 1;
	}
	else
	{
		return 0;
	}
	return 0;
}
static int Quec_PowerOnCheck(void)
{
    unsigned int i=0;

    //uart_printf("[Quec][PWK]OnkeyPowerOnCheck\r\n");
    /* Set 15s for power-down-by-long-ONKEY-pressing period, this register is 
    restricted to change only one bit each time. */
    pi2c_init(PMIC_BASE_PAGE_ADDR);

    for(i=0;i<5;i++) // 50ms+270ms(run here need 270ms )= 320ms for powerkey check.
    {
        if (!Quec_PMIC_GET_PWEKY_STATUS())
        {
            LOG_PRINT("short press, do poweroff!!\r\n");
	    	clock_wait_ms(10); /* 100 ms */
            return 1; //Power down if ONKEY is not pressed long enough.
        }
        clock_wait_ms(5); /* 5 ms */
    }
	LOG_PRINT("long press, do poweron!!\r\n");
    return 0;
}
static void quec_powerkey_check(void)
{
	unsigned char power_up_flag = 0xff, i = 0;
	unsigned char val;
	int pmic_reset_retry = 3;
	unsigned char reg[10] = {0};

	while(pmic_reset_retry--) {
		pi2c_bus_reset();
		pi2c_reset();
		pi2c_init(PMIC_BASE_PAGE_ADDR);
		if(pi2c_read_reg(PMIC_ID, &val) == 0) {
			break;
		}
	}
	pi2c_read_reg(0x01, &reg[0]);
	pi2c_read_reg(0x10, &reg[1]);
	pi2c_read_reg(0xe5, &reg[2]);
	pi2c_read_reg(0xe6, &reg[3]);
	pi2c_read_reg(0xe2, &reg[4]);
	pi2c_read_reg(0xeb, &reg[5]);
	LOG_PRINT("PMIC REG:0x01,0x10,0xe5,0xe6,0xe2,0xeb\r\n"); 
	LOG_PRINT("PMIC VAL:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\r\n",reg[0],reg[1],reg[2],reg[3],reg[4],reg[5]);

	if(smux_running_mode_get() == SMUX_MODE_NORMAL)
	{
		power_up_flag = 0x01; // hot  power on
		LOG_PRINT("download mode, do power on\n"); 
	}
	else
	{
		if (PMIC_PID(val) == PM802)
		{
			pi2c_read_reg(0x10, &reg[0]); //read power up log
			pi2c_read_reg(0xf0, &reg[1]); //read xo reg
			pi2c_read_reg(0x01, &reg[2]); //read exton1n status

			if((reg[1] & 0xf0) == 0x0) //cold power up
			{
				if((reg[0] == 0x0) && ((reg[2] & 0x02) == 0)) // unkown wakeup source && pwrkey is not pressed
				{
					LOG_PRINT("unkown wakeup:%d, do power off!!!\n",reg[0]);

					pi2c_write_reg(0xe2, 0xf0); //set discharger timer to 0S //default 2S
					pi2c_read_reg(0xe7, &reg[0]);
					pi2c_write_reg(0xe7, (reg[0] & 0xfb)); //disalbe fault wakeup
					pi2c_read_reg(0x0d, &reg[0]);
					pi2c_write_reg(0x0d, (reg[0])|0x20) ;
					return;
				}
			}
			pi2c_write_reg(0xe2, 0xf1); //set discharger timer  back to 1S
		}
		
		if( 0 == (reg[5]&0x80))
		{
			power_up_flag = 0x00; // cold power on
			LOG_PRINT("cold power on\n"); 
		}
		else
		{
			power_up_flag = 0x01; // hot  power on
			LOG_PRINT("hot power on\n"); 
		}	
		if(0x00 == power_up_flag) // cold power on
		{
			if(Quec_PowerOnCheck())
			{
				LOG_PRINT("warning: powkey too short !!!!!\r\n");
				//quec_pmic_sw_pdown();
				pmic_sw_pdown();
				return;
			}
			//save hot flag
			pi2c_write_reg(0xeb, (reg[5] | 0x80));
		}
	}
	
	for(i = 0;i < 10; i++)
	{
		if(reg[i] == 0x0)
		{
			reg[i] = 0xFF;
		}
	}
	reg[9] = 0x0;
	asr_property_set("power_down_reg", (const char*)(reg));
}
#endif

void
usb_phy_config(void)
{
  unsigned val;
  unsigned chip_id;

  chip_id = hw_chip_id();
  if(chip_id == CHIP_ID_CRANE) {
    /* driver strength */
    val = (unsigned)readl(UMTI_BASE + USB_PHY_REG_28);
    writel((val & ~HS_ISEL_MASK) | HS_ISEL, UMTI_BASE + USB_PHY_REG_28);
    LOG_PRINT("Usb phy reg 28 0x%x -> 0x%lx\n", val, readl(UMTI_BASE + USB_PHY_REG_28));
  } else if((CHIP_ID_CRANEG == chip_id) || (CHIP_ID_CRANEM == chip_id)) {
    val = (unsigned)readl(UMTI_BASE + USB_PHY_REG_29);
    writel((val & ~0x1F) | 0x1B, UMTI_BASE + USB_PHY_REG_29);
    LOG_PRINT("Usb phy reg 29 0x%x -> 0x%lx\n", val, readl(UMTI_BASE + USB_PHY_REG_29));
    val = (unsigned)readl(UMTI_BASE + USB_PHY_REG_25);
    writel(val | (BIT7), UMTI_BASE + USB_PHY_REG_25);
    val = (unsigned)readl(UMTI_BASE + USB_PHY_REG_04);
    writel(val | (BIT4 | BIT5 | BIT6), UMTI_BASE + USB_PHY_REG_04);
  }
}
/*---------------------------------------------------------------------------*/
int
main(void)
{
#if AROM_VERSION == AROM_VER_2021_10_10_CRANEW_Z1
#define SCRATCH_START   0x140
#define SCRATCH_END     0x20000
  int (*sb_bl2_image_setup_arom)(void *base, size_t size) = 0x1000AA1D;
  sb_bl2_image_setup_arom(SCRATCH_START, SCRATCH_END - SCRATCH_START);
#endif

#if defined(CPU_MODEL_CRANE)
  cr5_axi_set_max_freq();
#endif
  /*
   * if preboot when booting(SMUX_MODE_BYPASS), forbid to LOG_INFO;
   * if preboot when downloading(SMUX_MODE_NORMAL), permit LOG_INFO.
   */
  if(smux_running_mode_get() == SMUX_MODE_BYPASS) {
    /* forbid LOG_INFO */
    log_set_level("main", LOG_LEVEL_WARN);
  }

  LOG_PRINT("Executing preboot application...\n");
  LOG_PRINT("Preboot version: %s\n", BOOTLOADER_VERSION);

  /* force init prop 1K space: 0XB0020C00~ 0xB0021000 */
  asr_property_area_init(1);
  asr_property_set("bl2.preboot.ver", BOOTLOADER_VERSION);

#if defined(CPU_MODEL_CRANE) || defined(CPU_MODEL_CRANEW_MCU)
#if AROM_VERSION != AROM_VER_2021_09_29_CRANEW_CR5
  /* arom-crane alread print log, this is only for set prop */
  if(!fuse_trust_boot_enabled()) {
    /*LOG_PRINT("### Non-trusted boot mode. ###\n"); */
    asr_property_set("bl1.secureboot", "0");
  } else {
    /*LOG_PRINT("### Trusted boot mode. ###\n"); */
    asr_property_set("bl1.secureboot", "1");
  }
#else
  /*LOG_PRINT("### Non-trusted boot mode. ###\n"); */
  asr_property_set("bl1.secureboot", "0");
#endif

#if AROM_VERSION != AROM_VER_2021_09_29_CRANEW_CR5
  /*
   * "halt": When download completed, if usb pull out, board powerdown. defaultly it is "halt",
   * "reboot": some customer want to reboot instead of powerdown, so customer set it to "reboot" manually.
   */
  asr_property_set("bl2.usb_disconnect", "halt");

  usb_phy_config();

  fuse_read_lotid();

  if(hw_platform_type() == HW_PLATFORM_TYPE_SILICON) {
#ifdef QUECTEL_PREBOOT_OPT
    quec_powerkey_check();
#endif
    pmic_setup();
#if 0	//quectel:Self-starting problems are judged by ourselves and do not use the original ASR strategy Quality Platform: IS0001963
    uint8_t powerup_reason = pmic_powerup_get_reason();
    LOG_PRINT("Power_up_reason=0x%x.\n", powerup_reason);

    pmic_fault_wu_en_disable();

    /* except bit1 and bit2 */
    if(!(powerup_reason & 0x79)) {
      LOG_WARN("Warning: power_up_reason unknown.\n");
      /* pmic_sw_pdown(); */
      /* return 0; */
    }

    /* workaround for 1601+PM802, if power up log = 0, pmic power down. PM813 no this hardware bug. */
    if(0 == powerup_reason) {
      LOG_WARN("Warning: power_up_reason = 0. need pmic power down\n");
      pmic_sw_pdown();
      return 0;
    }
#endif 
  }
#endif
#endif

  psram_init();

#if defined(CPU_MODEL_CRANE)
  /*
   * Atcm 64K is not enough to parse bootloder volume, so update to psram.
   */
  LOG_INFO("Set fip image load address spaces to 0x%x\n",
           (unsigned)&__lzma_decode_start);
  sb_fip_image_setup((void *)&__lzma_decode_start, (unsigned)&__lzma_decode_size);
#endif

  return 0;
}
/*---------------------------------------------------------------------------*/

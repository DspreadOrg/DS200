/* -*- C -*- */
/*
 * Copyright (c) 2020, ASR microelectronics
 * All rights reserved.
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "pmu.h"
#include "reg.h"
#include "cpu.h"

/* Log configuration */
#include "log.h"
#define LOG_MODULE "PMU"
#define LOG_LEVEL  LOG_LEVEL_MAIN

/*---------------------------------------------------------------------------*/
#define PMU_BASE                               0xD4282800
/* usb detect*/
#define PMU_SD_ROT_WAKE_CLR                    (PMU_BASE + 0x7C)
#define VBUS_DETECT                            (1 << 15)
/*---------------------------------------------------------------------------*/
/*
 * if USB_VBUS is connected(R1101(12K) and R1104(6.8K) is connected,
 *   USB_DETECT is connected to USB_VBUS),
 * if USB_VBUS is unconnected, bit VBUS_DETECT always 0.
 */
int
usb_connect_with_usb_detect(void)
{
  unsigned val = readl(PMU_SD_ROT_WAKE_CLR);

  LOG_DBG("PMU_SD_ROT_WAKE_CLR(0x%x) = 0x%x\n", PMU_SD_ROT_WAKE_CLR, val);
  return (val & VBUS_DETECT) ? 1 : 0;
}
/*---------------------------------------------------------------------------*/
#define USB_BASE                                0xD4208000
#define USB_PORT_SC                             (USB_BASE + 0x184)
#define SUSP_DETECT                             (1 << 7)
/*---------------------------------------------------------------------------*/
/*
 * if USB_VBUS is unconnected(R1101(12K) and R1104(6.8K) is unconnected,
 *  USB_DETECT is unconnected to USB_VBUS),
 * USB_DETECT always be 0 no matter usb connect or unconnect.
 * PMU_SD_ROT_WAKE_CLR can not be use,
 * need read USB_PORT_SC, if SUSP_DETECT[bit7] = 1 means usb is unconnected.
 *
 * if downloading, bit7 = 0. if usb pull out when download over, bit7 = 1
 * Note:
 * before try download, USB_PORT_SC always is 0x0, no matter USB_VBUS connected or not.
 * so usb_connect_without_usb_detect cannot be used before received uuuu(preamble),
 * it only can be used when downloading, to check whether usb is pulled out.
 */
int
usb_connect_without_usb_detect(void)
{
  unsigned val = readl(USB_PORT_SC);

  LOG_DBG("USB_PORT_SC(0x%x) = 0x%x\n", USB_PORT_SC, val);
  return (val & SUSP_DETECT) ? 0 : 1;
}
/*---------------------------------------------------------------------------*/
/*
 * if usb-vbus not connected, customer use at+qdownload = 1 to enter force download mode,
 * not support wait 1s to try download mode.
 *
 * cr5 should not invoke usb_connect(). it is for mcu.
 */
int
usb_connect(void)
{
  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    /* HW_PLATFORM_TYPE_FPGA can not detect usb, return connected */
    return 1;
  }

  return usb_connect_with_usb_detect();
}
/*---------------------------------------------------------------------------*/

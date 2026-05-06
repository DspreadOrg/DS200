#ifndef _CLK_DRV_H_
#define _CLK_DRV_H_

#include "syscall-arom.h"

/*
 * QSPI clock freqency definitions
 */
#if AROM_VERSION == AROM_VER_2022_08_08_CRANEW_CR5
#define QSPI_CLOCK_13MHZ    0
#define QSPI_CLOCK_26MHZ    1
#define QSPI_CLOCK_48MHZ    2
#define QSPI_CLOCK_52MHZ    3
#define QSPI_CLOCK_78MHZ    4
#define QSPI_CLOCK_104MHZ   5
#else
#define QSPI_CLOCK_13MHZ    1
#define QSPI_CLOCK_26MHZ    3
#define QSPI_CLOCK_39MHZ    4
#define QSPI_CLOCK_52MHZ    5
#define QSPI_CLOCK_78MHZ    6
#define QSPI_CLOCK_104MHZ   7
#endif

#endif

# name of your application
APPLICATION = preboot

# If no BOARD is found in the environment, use this default:
BOARD ?= crane

# If no AROM_VERSION is found in the environment, use this default:
AROM_VERSION ?= 20200730

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(realpath $(CURDIR)/../../)

BOARD_WHITELIST += crane cranew-mcu

# Include all submodules depending on
#DIRS +=
USEMODULE += libarom
ifeq (,$(filter x86-arom-native,$(BOARD)))
USEMODULE += psram
endif
USEMODULE += property
USEMODULE += log

CFLAGS += -Wno-int-conversion
CFLAGS += -DAROM_VERSION=$(AROM_VERSION)

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

LINKER_SOURCE_SCRIPT := $(APPLICATION).lds

include $(RIOTBASE)/Makefile.include

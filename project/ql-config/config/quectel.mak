# ---------------------------------------------------------------------
# (C) Copyright [2021-2028] Quectel Wireless Solutions Co., Ltd. 
# All Rights Reserved
# ---------------------------------------------------------------------
# 使用注意事项
# QUEC_TARGET_DFLAGS 用于增加大功能宏定义
# TARGET_ARMLINK_OPT 用于定义链接阶段的宏
# VARIANT_LIST       用于控制某些功能的C源文件是否参与编译
#######################################################################

$(warning ${QUECTEL_PROJECT_DEF})

QUEC_VARIANT_LIST := $(strip ${oem_ver})

ifneq (,$(findstring CRANEM,${ASR_PLATFORM}))
	#1603(1603S/1603E/1603C)
	QUEC_TARGET_DFLAGS  += -DASR_PLATFORM_CRANEM
else
	ifneq (,$(findstring CRANELR,${ASR_PLATFORM}))
		#1602
		QUEC_TARGET_DFLAGS  += -DASR_PLATFORM_CRANELR
	else
		#1606(1606C/1606S/1606L)
		QUEC_TARGET_DFLAGS  += -DASR_PLATFORM_CRANEL
	endif
endif
QUEC_TARGET_DFLAGS := -DQUEC_UART_DMA_TX_ENABLE -DQUEC_UART_DMA_RX_ENABLE -DQUEC_USB_ENABLE -DEXT_AT_MODEM_SUPPORT -DQUEC_USB_CDC_ENABLE -DQUEC_USE_NEWLIB_REENTRANT
		
ifneq (,$(findstring TRUE,${QUEC_OPEN_DEF}))
########################################################################
##
##OPEN项目功能控制
########################################################################
QUEC_TARGET_DFLAGS  += -DQUECTEL_OPEN_CPU -DEXTERNAL_SPI_NORFLASH_FS -DQUEC_LCD_SUPPORT
#同步asrlwip相关宏定义
QUEC_TARGET_DFLAGS += -DLWIP_PLAT_SOCKET -DLWIP_PLAT_NOIMS -DLWIP_PLAT_LTEONLY_THIN -DLWIP_PLAT_LTEONLY_THIN_SINGLE_SIM

  
VARIANT_LIST += NOPMIC802 NOPMIC813 NOLWM2M NOPAHO SD_NOT_SUPPORT NOATNET QUECTEL_OPEN_CPU LZMA_SUPPORT  NOMEP
VARIANT_LIST += NOUSBNET NO_ENVSIM NOBIP NOPPP NOIMS NODIALER REMOVE_PB LFS_SUPPORT_V2
##AES_SUPPORT

TARGET_ARMLINK_OPT  += --predefine="-DQUECTEL_OPEN_CPU"

##OEM空 场景1:默认场景,无gps,有audio,无sms,有lcd
ifeq (,$(oem_ver)) 

	VARIANT_LIST        += REMOVE_SMS MP3_DECODE QUEC_EXT_NVM_SUPPORT 
	QUEC_TARGET_DFLAGS  += -DQUEC_NVM_IN_EXT_FLASH  -DQUEC_CAMERA_SUPPORT -DQUECTEL_MINIFOTA_SUPPORT -DQUEC_APP_FULL_FOTA 
ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
	PACKAGE_LIST +=  \
			quectel/lcd \
			quectel/camera 		
endif
endif

##OEM:NOMINI 场景1:默认场景,无gps,有audio,无sms,有lcd,有Camera
ifneq (,$(findstring NOMINI,${oem_ver}))
	VARIANT_LIST        += REMOVE_SMS MP3_DECODE QUEC_EXT_NVM_SUPPORT
	QUEC_TARGET_DFLAGS  += -DQUEC_NVM_IN_EXT_FLASH  -DQUEC_CAMERA_SUPPORT 
	##此宏裁剪更多功能,不常使用的AT
	QUEC_TARGET_DFLAGS  += -DQUEC_LTEONLY_THIN -DNO_EXTEND_MY_Q_AT
ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
	PACKAGE_LIST +=  \
			quectel/lcd \
			quectel/camera 	
endif			
endif

##OEM=TRACK 场景二:TRACK行业版本内核,与公版的区别是支持SGS短信
ifneq (,$(findstring TRACK,${oem_ver}))
	VARIANT_LIST        +=  MP3_DECODE
	QUEC_TARGET_DFLAGS  += -DQUEC_HX_GNSS_SUPPORT -DENABLE_WIFI_SCAN -DQUECTEL_MINIFOTA_SUPPORT \
	-DQUEC_APP_FULL_FOTA

	ifeq (,$(findstring MIN_SYS,${VARIANT_LIST}))
		PACKAGE_LIST +=  \
			quectel/gnss
	endif
	VARIANT_LIST  += HX_GPS
endif

else
########################################################################
##
##STD项目功能控制
########################################################################





endif


TARGET_DFLAGS += ${QUEC_TARGET_DFLAGS}


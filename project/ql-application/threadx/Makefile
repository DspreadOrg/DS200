-include config.mk
#-------------------------------------------------------------------------------
# Configure variable
#-------------------------------------------------------------------------------
ifneq (${TOP_DIR},)
  TOP_DIR:=${TOP_DIR}
else
  TOP_DIR:=${CURDIR}
endif


#-------------------------------------------------------------------------------
# Configure compile directories
#-------------------------------------------------------------------------------
COMMPILE_DIRS:= \
  evb_audio \
#  src/common \
#  src/cjson \
#  src/serialdev \
#  src/net/cloud/aliyun \
#  src/net/HTTPClient \
#  src/net/mbedtls \
#  src/net/mqtt \
#  interface/lvgl \

#-------------------------------------------------------------------------------
# Configure default libarays
#-------------------------------------------------------------------------------
DEFAULT_LIBS+=  \
  common/lib/ql_common_api.lib \
  common/lib/HTTPClient.lib \
  common/lib/mbedtls.lib \
  common/lib/mqtt.lib \
  common/lib/aliyun.lib \
  common/lib/serialdev.lib \
  common/lib/ym_common.lib \
  common/lib/cjson.lib \
  common/lib/com_zip.lib \
#  common/lib/com_cell_locator.lib \
#  common/lib/com_ftp.lib \
#  common/lib/com_http.lib \
#  common/lib/com_mbedtls.lib \
#  common/lib/com_paho_mqtt.lib \
#  common/lib/com_qthsdk.lib \
#  common/lib/com_ssl.lib \

ifeq ($(CONF_DEVTYPE_NAME),CS50MK)  
  DEFAULT_LIBS+=common/lib/ym_lvgl_color.lib
#else ifeq ($(CONF_DEVTYPE_NAME),CS50KG)  
#  DEFAULT_LIBS+=common/lib/ym_lvgl_color.lib
else ifeq ($(CONF_DEVTYPE_NAME),CS50P)  
  DEFAULT_LIBS+=common/lib/ym_lvgl_color.lib
else ifeq ($(CONF_DEVTYPE_NAME),DS10)  
  DEFAULT_LIBS+=common/lib/ym_lvgl_color.lib
else ifeq ($(CONF_DEVTYPE_NAME),DS200)  
  DEFAULT_LIBS+=common/lib/ym_lvgl_color.lib
else
  DEFAULT_LIBS+=common/lib/ym_lvgl_mono.lib
endif

#-------------------------------------------------------------------------------
# include mk file
#-------------------------------------------------------------------------------
include ${TOP_DIR}/config/common/makefile.mk

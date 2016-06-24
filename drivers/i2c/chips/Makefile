#
# Makefile for miscellaneous I2C chip drivers.
#
# Do not add new drivers to this directory! It is DEPRECATED.
#
# Device drivers are better grouped according to the functionality they
# implement rather than to the bus they are connected to. In particular:
# * Hardware monitoring chip drivers go to drivers/hwmon
# * RTC chip drivers go to drivers/rtc
# * I/O expander drivers go to drivers/gpio
#

include $(srctree)/drivers/misc/mediatek/Makefile.custom

obj-$(CONFIG_AMP_TFA9895)       += tfa9895.o
obj-$(CONFIG_AMP_TFA9895L)      += tfa9895l.o
obj-$(CONFIG_INPUT_CAPELLA_CM36686)     += cm36686.o
obj-$(CONFIG_BOSCH_BMA253)      += bma253_bosch.o
obj-$(CONFIG_SENSORS_BMM050)    += bs_log.o bmm050_driver.o bmm050.o
EXTRA_CFLAGS += -DBMM_USE_BASIC_I2C_FUNC -DCONFIG_BMM_USE_PLATFORM_DATA

ifeq ($(CONFIG_I2C_DEBUG_CHIP),y)
EXTRA_CFLAGS += -DDEBUG
endif
#NFC ++
obj-$(CONFIG_SENSORS_NFC_PN544) += pn544.o
obj-$(CONFIG_SENSORS_NFC_PN544) += pn544_htc.o
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"a50cml")
EXTRA_CFLAGS += -DNFC_PVDD_LOAD_SWITCH=1
EXTRA_CFLAGS += -DNFC_PVDD_GPIO_DT=1
EXTRA_CFLAGS += -DNFC_READ_RFSKUID_MTK6753=1
#EXTRA_CFLAGS += -DNFC_POWER_OFF_SEQUENCE=1
#EXTRA_CFLAGS += -DNFC_OFF_MODE_CHARGING_ENABLE=1
EXTRA_CFLAGS += -DNFC_GET_BOOTMODE=1
EXTRA_CFLAGS += -DNFC_PLL_CLK_ONOFF=1
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"a51cml")
EXTRA_CFLAGS += -DNFC_PVDD_LOAD_SWITCH=1
EXTRA_CFLAGS += -DNFC_READ_RFSKUID_MTK6753=1
#EXTRA_CFLAGS += -DNFC_PVDD_GPIO_DT=1
#EXTRA_CFLAGS += -DNFC_POWER_OFF_SEQUENCE=1
#EXTRA_CFLAGS += -DNFC_OFF_MODE_CHARGING_ENABLE=1
EXTRA_CFLAGS += -DNFC_GET_BOOTMODE=1
#EXTRA_CFLAGS += -DNFC_PLL_CLK_ONOFF=1
endif

ifeq ($(CONFIG_ARCH_MTK_PROJECT),"a51bml")
EXTRA_CFLAGS += -DNFC_PVDD_LOAD_SWITCH=1
EXTRA_CFLAGS += -DNFC_READ_RFSKUID_MTK6753=1
#EXTRA_CFLAGS += -DNFC_PVDD_GPIO_DT=1
#EXTRA_CFLAGS += -DNFC_POWER_OFF_SEQUENCE=1
#EXTRA_CFLAGS += -DNFC_OFF_MODE_CHARGING_ENABLE=1
EXTRA_CFLAGS += -DNFC_GET_BOOTMODE=1
#EXTRA_CFLAGS += -DNFC_PLL_CLK_ONOFF=1
endif
ifeq ($(CONFIG_ARCH_MTK_PROJECT),"v36bml")
EXTRA_CFLAGS += -DNO_GPIO_NFC_ENABLE
endif



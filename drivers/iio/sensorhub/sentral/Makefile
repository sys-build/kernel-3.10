#
# Makefile for SENtral Sensor Hub
#
-include $(srctree)/drivers/misc/mediatek/Makefile.custom

EXTRA_CFLAGS += -fno-pic
obj-$(CONFIG_SENTRAL_IIO) += sentral-iio.o

sentral-iio-objs += sentral-core.o
sentral-iio-objs += sentral-warmstart.o

#obj-m += sentral-iio.o



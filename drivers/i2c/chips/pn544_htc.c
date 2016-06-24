/******************************************************************************
 *
 *  This is the implementation file for the PN547 NFC customization Functions
 *
 ******************************************************************************/

#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <mach/mt_gpio.h>
#include <linux/htc_devices_dtb.h>
#include "pn544_htc.h"
#include <mach/mt_clkbuf_ctl.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

#define D(x...)	\
	if (nfc_is_debug) \
		printk(KERN_DEBUG "[NFC] " x)
#define I(x...) printk(KERN_INFO "[NFC] " x)
#define E(x...) printk(KERN_ERR "[NFC] [Err] " x)


// for PVDD controlled load switch ++
#if NFC_PVDD_LOAD_SWITCH
static unsigned int   pvdd_gpio = 0;
#endif //NFC_PVDD_LOAD_SWITCH
// for PVDD controlled load switch --


/******************************************************************************
 *
 *  Function pn544_htc_check_rfskuid:
 *  Return With(1)/Without(0) NFC chip if this SKU can get RFSKUID in kernal
 *  Return is_alive(original value) by default.
 *
 ******************************************************************************/
#if NFC_READ_RFSKUID_MTK6753
#define HAS_NFC_CHIP 0x7000000
#endif //NFC_READ_RFSKUID_MTK6753

int pn544_htc_check_rfskuid(int in_is_alive){
#if NFC_READ_RFSKUID_MTK6753
	int i;
#if 0
	if(of_machine_hwid() < 2) {
		I("%s: of_machine_hwid() = %d\n",__func__,of_machine_hwid());
		return 0;
	}
#endif
	for ( i = 2; i <= 9; i++) {
		I("%s: get_sku_data(%d) = 0x%x\n",__func__,i,get_sku_data(i));
		if (get_sku_data(i) == HAS_NFC_CHIP) {
			I("%s: Check get_sku_data done device has NFC chip\n",__func__);
			return 1;
		}
	}
	I("%s: Check get_sku_data done remove NFC\n",__func__);
	return 0;
#else //NFC_READ_RFSKUID_MTK6753
	return in_is_alive;
#endif //NFC_READ_RFSKUID_MTK6753
}


/******************************************************************************
 *
 *  Function pn544_htc_get_bootmode:
 *  Return  NFC_BOOT_MODE_NORMAL            0
 *          NFC_BOOT_MODE_FTM               1
 *          NFC_BOOT_MODE_DOWNLOAD          2
 *          NFC_BOOT_MODE_OFF_MODE_CHARGING 5
 *  Return 	NFC_BOOT_MODE_NORMAL by default
 *          if there's no bootmode infomation available
 *
 ******************************************************************************/
int pn544_htc_get_bootmode(void) {
	char sbootmode[30] = "others";
#if NFC_GET_BOOTMODE
	strcpy(sbootmode,htc_get_bootmode());
#endif  //NFC_GET_BOOTMODE
	if (strcmp(sbootmode, "offmode_charging") == 0) {
		I("%s: Check bootmode done NFC_BOOT_MODE_OFF_MODE_CHARGING\n",__func__);
		return NFC_BOOT_MODE_OFF_MODE_CHARGING;
	} else if (strcmp(sbootmode, "charger") == 0) {
		I("%s: Check bootmode done NFC_BOOT_MODE_OFF_MODE_CHARGING\n",__func__);
		return NFC_BOOT_MODE_OFF_MODE_CHARGING;
	} else if (strcmp(sbootmode, "ftm") == 0 || strcmp(sbootmode, "factory") == 0) {
		I("%s: Check bootmode done NFC_BOOT_MODE_FTM\n",__func__);
		return NFC_BOOT_MODE_FTM;
	} else if (strcmp(sbootmode, "download") == 0) {
		I("%s: Check bootmode done NFC_BOOT_MODE_DOWNLOAD\n",__func__);
		return NFC_BOOT_MODE_DOWNLOAD;
	} else {
		I("%s: Check bootmode done NFC_BOOT_MODE_NORMAL mode = %s\n",__func__,sbootmode);
		return NFC_BOOT_MODE_NORMAL;
	}
}

/******************************************************************************
 *
 *  Function pn544_htc_get_bootmode:
 *  Get platform required GPIO number from device tree
 *  For Power off sequence and OFF_MODE_CHARGING
 *
 ******************************************************************************/
void pn544_htc_parse_dt(struct device *dev) {
	struct device_node *dt = dev->of_node;
#if NFC_PVDD_LOAD_SWITCH
#if NFC_PVDD_GPIO_DT

	if( of_find_property(dt, "nxp,pvdd-gpio", NULL) ) {
		of_property_read_u32(dt, "nxp,pvdd-gpio", &pvdd_gpio);
		I("%s: pvdd_gpio: pvdd_gpio %d\n", __func__, pvdd_gpio);
	} else {
		E("%s: pvdd_gpio: find FAIL\n", __func__);
	}
		pvdd_gpio = GPIO_NFC_VENB_PIN;
#else //NFC_PVDD_GPIO_DT
	I("%s: pvdd_gpio:%d\n", __func__, pvdd_gpio);
#endif
#endif
}

/******************************************************************************
 *
 *  Function pn544_htc_off_mode_charging
 *  Turn of NFC_PVDD when bootmode = NFC_BOOT_MODE_OFF_MODE_CHARGING
 *
 ******************************************************************************/
void pn547_work (struct work_struct *work) {

	int ret;
	msleep(3000);
	I("%s: NFC PN547 OFF_MODE_CHARGING sequence\n", __func__);
	ret = gpio_request(GPIO137, "nfc_i2c_scl");
	if (ret) {
		E("%s : request CPU2HUB_I2C_SCL GPIO137 %d fail\n",
			__func__, GPIO137);
		return;
	}
	ret = gpio_request(GPIO136, "nfc_i2c_sda");
	if (ret) {
		E("%s : request CPU2HUB_I2C_SDA GPIO136 %d fail\n",
			__func__, GPIO136);
		return;
	}
	mt_set_gpio_mode(GPIO137, GPIO_MODE_00);
	mt_set_gpio_mode(GPIO136, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO137, GPIO_DIR_OUT);
	mt_set_gpio_dir(GPIO136, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO137, GPIO_OUT_ONE);
	mt_set_gpio_out(GPIO136, GPIO_OUT_ONE);
	I("%s : SCL_SDA_HIGH",__func__);
	mdelay(2);
	mt_set_gpio_out(GPIO137, GPIO_OUT_ZERO);
	I("%s : SCL_LOW",__func__);
	mdelay(1);
	mt_set_gpio_out(GPIO136, GPIO_OUT_ZERO);
	I("%s : SDA_LOW",__func__);
	mdelay(50);
#if NFC_PVDD_LOAD_SWITCH
	mt_set_gpio_out(pvdd_gpio, GPIO_OUT_ZERO);
	I("%s : pvdd_low OFF_MODE_CHARGING",__func__);
#endif

}

#if NFC_OFF_MODE_CHARGING_ENABLE
static struct workqueue_struct *nfc_wq;
static struct delayed_work nfc_work;
#endif //NFC_OFF_MODE_CHARGING_ENABLE

void pn544_htc_off_mode_charging (void) {
#if NFC_OFF_MODE_CHARGING_ENABLE
	INIT_DELAYED_WORK(&nfc_work, pn547_work);
	nfc_wq = create_singlethread_workqueue("htc_nfc");
	I("NFC Turn off PVDD delay 3 secs \n");
	queue_delayed_work(nfc_wq, &nfc_work, 0);
#endif //NFC_OFF_MODE_CHARGING_ENABLE
}


/******************************************************************************
 *
 *  Function pn544_htc_pvdd_on
 *  Turn on NFC_PVDD
 *
 ******************************************************************************/
int pn544_htc_pvdd_on (void) {

#if NFC_PVDD_LOAD_SWITCH
	int ret = -1;
#if NFC_PVDD_GPIO_DT
	if(pvdd_gpio <= 0) return 0;

	I("%s: config NFC_1V8_EN pin\n", __func__);
	mt_set_gpio_mode(pvdd_gpio, GPIO_MODE_00);
	mt_set_gpio_dir(pvdd_gpio, GPIO_DIR_OUT);
	mt_set_gpio_out(pvdd_gpio, GPIO_OUT_ONE);
	I("%s: Set HIGH - NFC_1V8_EN pin  %d chk value %d\n", __func__, pvdd_gpio, mt_get_gpio_out(pvdd_gpio));
#else
	ret = hwPowerOn(MT6328_POWER_LDO_VGP1, VOL_1800, "NFC");
	if (ret)
		I("%s: Setting power on success!\n", __func__);
	else
		E("%s: Setting power on fail!",__func__);
#endif
#endif //NFC_PVDD_LOAD_SWITCH
	return 1;
}

/******************************************************************************
 *
 *  Function pn544_htc_pvdd_off
 *  Turn off NFC_PVDD
 *
 ******************************************************************************/
int pn544_htc_pvdd_off(void) {

#if NFC_PVDD_LOAD_SWITCH
	int ret = -1;
#if NFC_PVDD_GPIO_DT
	if(pvdd_gpio <= 0) return 0;

	I("%s: config NFC_1V8_EN pin\n", __func__);
	mt_set_gpio_mode(pvdd_gpio, GPIO_MODE_00);
	mt_set_gpio_dir(pvdd_gpio, GPIO_DIR_OUT);
	mt_set_gpio_out(pvdd_gpio, GPIO_OUT_ONE);
	I("%s: Set LOW - NFC_1V8_EN pin  %d chk value %d\n", __func__, pvdd_gpio, mt_get_gpio_out(pvdd_gpio));
#else
	ret = hwPowerDown(MT6328_POWER_LDO_VGP1, "NFC");
	if (ret)
		I("%s: Setting power off success!\n", __func__);
	else
		E("%s: Setting power off fail!",__func__);
#endif
#endif //NFC_PVDD_LOAD_SWITCH
	return 1;
}


/******************************************************************************
 *
 *  Function pn547_power_off_seq
 *  Turn off NFC chip step by step
 *
 ******************************************************************************/

void pn547_power_off_seq(void) {
#if NFC_POWER_OFF_SEQUENCE
	int ret;
	I("%s: NFC PN547 Power off sequence\n", __func__);
	ret = gpio_request(GPIO137, "nfc_i2c_scl");
	if (ret) {
		E("%s : request CPU2HUB_I2C_SCL GPIO137 %d fail\n",
			__func__, GPIO137);
		return;
	}
	ret = gpio_request(GPIO136, "nfc_i2c_sda");
	if (ret) {
		E("%s : request CPU2HUB_I2C_SDA GPIO136 %d fail\n",
			__func__, GPIO136);
		return;
	}
	mt_set_gpio_mode(GPIO137, GPIO_MODE_00);
	mt_set_gpio_mode(GPIO136, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO137, GPIO_DIR_OUT);
	mt_set_gpio_dir(GPIO136, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO137, GPIO_OUT_ONE);
	mt_set_gpio_out(GPIO136, GPIO_OUT_ONE);
	I("%s : SCL_SDA_HIGH",__func__);
	mdelay(2);
	mt_set_gpio_out(GPIO137, GPIO_OUT_ZERO);
	I("%s : SCL_LOW",__func__);
	mdelay(1);
	mt_set_gpio_out(GPIO136, GPIO_OUT_ZERO);
	I("%s : SDA_LOW",__func__);
	mdelay(50);
#if NFC_PVDD_LOAD_SWITCH
	mt_set_gpio_out(pvdd_gpio, GPIO_OUT_ZERO);
	I("%s : pvdd_low SYSTEM POWER OFF",__func__);
#endif
	mdelay(10);
#endif //NFC_POWER_OFF_SEQUENCE
}


/******************************************************************************
 *
 *  Function pn547_pll_clk_ctrl
 *  Enable or Disable PLL clock
 *
 ******************************************************************************/
int pn544_pll_clk_ctrl(int onoff) {
	int ret = 1;

#if NFC_PLL_CLK_ONOFF
	//enable or disable nfc clock buffer
	ret = clk_buf_ctrl(CLK_BUF_NFC, onoff);
	if (!ret){
		E("%s : Setting PLL clock fail! onoff:%d, ret:%d", __func__, onoff, ret);
	}else {
		I("%s : Setting PLL clock success! onoff:%d, ret:%d", __func__, onoff, ret);
	}

#endif

	return ret;
}


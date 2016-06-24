/* tpa6130a2 HEADSET PA
 *
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/tlv.h>
//#include <cust_gpio_usage.h>
//#include <mach/mt_gpio.h>

#include "tpa6130a2.h"


#define GSE_TAG                  "[Tpa6130a2] "
#define GSE_FUN(f)               printk(KERN_INFO GSE_TAG"%s\n", __FUNCTION__)
#define GSE_ERR(fmt, args...)    printk(KERN_ERR GSE_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define GSE_LOG(fmt, args...)    printk(KERN_INFO GSE_TAG fmt, ##args)
/*----------------------------------------------------------------------------*/

#define TPA6130A2_DEV_NAME        "tpa6130a2"
static const struct i2c_device_id tpa6130a2_i2c_id[] = {{TPA6130A2_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_tpa6130a2={ I2C_BOARD_INFO(TPA6130A2_DEV_NAME, 0x60)};

/*----------------------------------------------------------------------------*/
static int tpa6130a2_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int tpa6130a2_i2c_remove(struct i2c_client *client);

/*----------------------------------------------------------------------------*/

/* This struct is used to save the context */
struct tpa6130a2_data {
	struct mutex mutex;
	unsigned char regs[TPA6130A2_CACHEREGNUM];
	//struct regulator *supply;
	//int power_gpio;
	u8 power_state:1;
	//enum tpa_model id;
};

static struct i2c_client *tpa6130a2_client;
//struct tpa6130a2_data data;

static int tpa6130a2_i2c_read(int reg)
{
	struct tpa6130a2_data *data;
	int val;

	BUG_ON(tpa6130a2_client == NULL);
	data = i2c_get_clientdata(tpa6130a2_client);

	/* If powered off, return the cached value */
	if (data->power_state) {
		val = i2c_smbus_read_byte_data(tpa6130a2_client, reg);
		if (val < 0)
			dev_err(&tpa6130a2_client->dev, "Read failed\n");
		else
			data->regs[reg] = val;
	} else {
		val = data->regs[reg];
	}

	return val;
}

static int tpa6130a2_i2c_write(int reg, u8 value)
{
	struct tpa6130a2_data *data;
	int val = 0;

	BUG_ON(tpa6130a2_client == NULL);
	data = i2c_get_clientdata(tpa6130a2_client);

	printk("%s ON!\n",__func__);
	if (data->power_state) {
		printk("%s OK!\n",__func__);
		val = i2c_smbus_write_byte_data(tpa6130a2_client, reg, value);
		printk("%s OK!val = 0x%x\n",__func__,value);
		if (val < 0) {
			dev_err(&tpa6130a2_client->dev, "Write failed\n");
			return val;
		}
	}

	/* Either powered on or off, we save the context */
	data->regs[reg] = value;

	return val;
}

static u8 tpa6130a2_read(int reg)
{
	struct tpa6130a2_data *data;

	BUG_ON(tpa6130a2_client == NULL);
	data = i2c_get_clientdata(tpa6130a2_client);

	return data->regs[reg];
}

static int tpa6130a2_initialize(void)
{
	struct tpa6130a2_data *data;
	int i, ret = 0;

	BUG_ON(tpa6130a2_client == NULL);
	data = i2c_get_clientdata(tpa6130a2_client);

	for (i = 1; i < TPA6130A2_REG_VERSION; i++) {
		ret = tpa6130a2_i2c_write(i, data->regs[i]);
		if (ret < 0)
			break;
	}

	return ret;
}

static int tpa6130a2_power(u8 power)
{
	struct	tpa6130a2_data *data;
	u8	val;
	int	ret = 0;

	BUG_ON(tpa6130a2_client == NULL);
	data = i2c_get_clientdata(tpa6130a2_client);

	//mutex_lock(&data->mutex);
	
	if (power == data->power_state)
		goto exit;
	/*
	if (power) {
		ret = regulator_enable(data->supply);
		if (ret != 0) {
			dev_err(&tpa6130a2_client->dev,
				"Failed to enable supply: %d\n", ret);
			goto exit;
		}
		// Power on 
		if (data->power_gpio >= 0)
			gpio_set_value(data->power_gpio, 1);

		data->power_state = 1;
		ret = tpa6130a2_initialize();
		if (ret < 0) {
			dev_err(&tpa6130a2_client->dev,
				"Failed to initialize chip\n");
			if (data->power_gpio >= 0)
				gpio_set_value(data->power_gpio, 0);
			regulator_disable(data->supply);
			data->power_state = 0;
			goto exit;
		}
	} else {
		// set SWS 
		val = tpa6130a2_read(TPA6130A2_REG_CONTROL);
		val |= TPA6130A2_SWS;
		tpa6130a2_i2c_write(TPA6130A2_REG_CONTROL, val);

		// Power off 
		if (data->power_gpio >= 0)
			gpio_set_value(data->power_gpio, 0);

		ret = regulator_disable(data->supply);
		if (ret != 0) {
			dev_err(&tpa6130a2_client->dev,
				"Failed to disable supply: %d\n", ret);
			goto exit;
		}

		data->power_state = 0;
	}*/

	if (power) {
            tpa6130a2_initialize();
       
            data->power_state = 1;
            printk("%s set done\n", __func__);
	} else {
            // set SWS 
            val = tpa6130a2_read(TPA6130A2_REG_CONTROL);
            val |= TPA6130A2_SWS;
            tpa6130a2_i2c_write(TPA6130A2_REG_CONTROL, val);

            data->power_state = 0;
	}

exit:
	//mutex_unlock(&data->mutex);
	return ret;
}

static int tpa6130a2_get_volsw(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct tpa6130a2_data *data;
	unsigned int reg = mc->reg;
	unsigned int shift = mc->shift;
	int max = mc->max;
	unsigned int mask = (1 << fls(max)) - 1;
	unsigned int invert = mc->invert;

	BUG_ON(tpa6130a2_client == NULL);
	data = i2c_get_clientdata(tpa6130a2_client);

	mutex_lock(&data->mutex);

	ucontrol->value.integer.value[0] =
		(tpa6130a2_read(reg) >> shift) & mask;

	if (invert)
		ucontrol->value.integer.value[0] =
			max - ucontrol->value.integer.value[0];

	mutex_unlock(&data->mutex);
	return 0;
}

static int tpa6130a2_put_volsw(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct tpa6130a2_data *data;
	unsigned int reg = mc->reg;
	unsigned int shift = mc->shift;
	int max = mc->max;
	unsigned int mask = (1 << fls(max)) - 1;
	unsigned int invert = mc->invert;
	unsigned int val = (ucontrol->value.integer.value[0] & mask);
	unsigned int val_reg;

	printk("%s ON!\n",__func__);
	BUG_ON(tpa6130a2_client == NULL);
	data = i2c_get_clientdata(tpa6130a2_client);

	if (invert)
		val = max - val;

	mutex_lock(&data->mutex);

	val_reg = tpa6130a2_read(reg);
	if (((val_reg >> shift) & mask) == val) {
		mutex_unlock(&data->mutex);
		return 0;
	}

	val_reg &= ~(mask << shift);
	val_reg |= val << shift;
	tpa6130a2_i2c_write(reg, val_reg);

	mutex_unlock(&data->mutex);

	return 1;
}

/*
 * TPA6130 volume. From -59.5 to 4 dB with increasing step size when going
 * down in gain.
 */
static const unsigned int tpa6130_tlv[] = {
	TLV_DB_RANGE_HEAD(10),
	0, 1, TLV_DB_SCALE_ITEM(-5950, 600, 0),
	2, 3, TLV_DB_SCALE_ITEM(-5000, 250, 0),
	4, 5, TLV_DB_SCALE_ITEM(-4550, 160, 0),
	6, 7, TLV_DB_SCALE_ITEM(-4140, 190, 0),
	8, 9, TLV_DB_SCALE_ITEM(-3650, 120, 0),
	10, 11, TLV_DB_SCALE_ITEM(-3330, 160, 0),
	12, 13, TLV_DB_SCALE_ITEM(-3040, 180, 0),
	14, 20, TLV_DB_SCALE_ITEM(-2710, 110, 0),
	21, 37, TLV_DB_SCALE_ITEM(-1960, 74, 0),
	38, 63, TLV_DB_SCALE_ITEM(-720, 45, 0),
};

static const struct snd_kcontrol_new tpa6130a2_controls[] = {
	SOC_SINGLE_EXT_TLV("TPA6130A2 Headphone Playback Volume",
		       TPA6130A2_REG_VOL_MUTE, 0, 0x3f, 0,
		       tpa6130a2_get_volsw, tpa6130a2_put_volsw,
		       tpa6130_tlv),
};

/*
 * Enable or disable channel (left or right)
 * The bit number for mute and amplifier are the same per channel:
 * bit 6: Right channel
 * bit 7: Left channel
 * in both registers.
 */
static void tpa6130a2_channel_enable(u8 channel, int enable)
{
	u8	val;

	if (enable) {
		/* Enable channel */
		/* Enable amplifier */
		val = tpa6130a2_read(TPA6130A2_REG_CONTROL);
		val |= channel;
		val &= ~TPA6130A2_SWS;
		tpa6130a2_i2c_write(TPA6130A2_REG_CONTROL, val);

		/* Unmute channel */
		val = tpa6130a2_read(TPA6130A2_REG_VOL_MUTE);
		val &= ~channel;
		tpa6130a2_i2c_write(TPA6130A2_REG_VOL_MUTE, 0x3c);
	} else {
		/* Disable channel */
		/* Mute channel */
		val = tpa6130a2_read(TPA6130A2_REG_VOL_MUTE);
		val |= channel;
		tpa6130a2_i2c_write(TPA6130A2_REG_VOL_MUTE, val);

		/* Disable amplifier */
		val = tpa6130a2_read(TPA6130A2_REG_CONTROL);
		val &= ~channel;
		tpa6130a2_i2c_write(TPA6130A2_REG_CONTROL, val);
	}
}

int tpa6130a2_stereoL_enable(struct snd_soc_codec *codec, int enable)
{
	int ret = 0;
	if (enable) {
		ret = tpa6130a2_power(1);
		if (ret < 0)
			return ret;
		tpa6130a2_channel_enable(TPA6130A2_HP_EN_L,
					 1);
	} else {
		tpa6130a2_channel_enable(TPA6130A2_HP_EN_L,
					 0);
		ret = tpa6130a2_power(0);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(tpa6130a2_stereoL_enable);

int tpa6130a2_stereoR_enable(struct snd_soc_codec *codec, int enable)
{
	int ret = 0;
	if (enable) {
		ret = tpa6130a2_power(1);
		if (ret < 0)
			return ret;
		tpa6130a2_channel_enable(TPA6130A2_HP_EN_R,
					 1);
	} else {
		tpa6130a2_channel_enable(TPA6130A2_HP_EN_R,
					 0);
		ret = tpa6130a2_power(0);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(tpa6130a2_stereoR_enable);

int tpa6130a2_add_controls(struct snd_soc_codec *codec)
{
	return snd_soc_add_codec_controls(codec, tpa6130a2_controls,
						ARRAY_SIZE(tpa6130a2_controls));
}
EXPORT_SYMBOL_GPL(tpa6130a2_add_controls);

static int tpa6130a2_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device *dev;
	struct tpa6130a2_data *data;
	//struct tpa6130a2_platform_data *pdata;
	//const char *regulator;
	int ret;
	printk("tpa6130a2_probe on !\n");
	dev = &client->dev;
	client->addr = 0x60;
	/*if (client->dev.platform_data == NULL) {
		dev_err(dev, "Platform data not set\n");
		dump_stack();
		return -ENODEV;
	}*/

	data = devm_kzalloc(&client->dev, sizeof(*data), GFP_KERNEL);
	if (data == NULL) {
		dev_err(dev, "Can not allocate memory\n");
		return -ENOMEM;
	}

	tpa6130a2_client = client;

	i2c_set_clientdata(tpa6130a2_client, data);

	//pdata = client->dev.platform_data;
	//data->power_gpio = pdata->power_gpio;
	//data->id = id->driver_data;

	mutex_init(&data->mutex);

	/* Set default register values */
	data->regs[TPA6130A2_REG_CONTROL] =	TPA6130A2_SWS;
	data->regs[TPA6130A2_REG_VOL_MUTE] =	TPA6130A2_MUTE_R |
						TPA6130A2_MUTE_L;

	/*if (data->power_gpio >= 0) {
		ret = devm_gpio_request(dev, data->power_gpio,
					"tpa6130a2 enable");
		if (ret < 0) {
			dev_err(dev, "Failed to request power GPIO (%d)\n",
				data->power_gpio);
			goto err_gpio;
		}
		gpio_direction_output(data->power_gpio, 0);
	}*/

	/*switch (data->id) {
	default:
		dev_warn(dev, "Unknown TPA model (%d). Assuming 6130A2\n",
			 data->id);
	case TPA6130A2:
		regulator = "Vdd";
		break;
	case TPA6140A2:
		regulator = "AVdd";
		break;
	}

	data->supply = devm_regulator_get(dev, regulator);
	if (IS_ERR(data->supply)) {
		ret = PTR_ERR(data->supply);
		dev_err(dev, "Failed to request supply: %d\n", ret);
		goto err_gpio;
	}*/

	ret = tpa6130a2_power(1);
	if (ret != 0)
		goto err_gpio;


	/* Read version */
	ret = tpa6130a2_i2c_read(TPA6130A2_REG_VERSION) &
				 TPA6130A2_VERSION_MASK;
	if ((ret != 1) && (ret != 2))
		dev_warn(dev, "UNTESTED version detected (%d)\n", ret);

	/* Disable the chip */
	ret = tpa6130a2_power(0);
	if (ret != 0)
		goto err_gpio;

	printk("tpa6130a2_probe OK !\n");

	return 0;

err_gpio:
	tpa6130a2_client = NULL;

	return ret;
}

static int tpa6130a2_i2c_remove(struct i2c_client *client)
{
	tpa6130a2_power(0);
	tpa6130a2_client = NULL;

	return 0;
}

static struct of_device_id tpa6130_match_table[] = {
    { .compatible = "tpa,tpa6130-hs-amp",},
    { },
};


static struct i2c_driver tpa6130a2_i2c_driver = {
    .driver = {
        .owner          = THIS_MODULE,
        .name           = TPA6130A2_DEV_NAME,
        .of_match_table = tpa6130_match_table,
    },
	.probe      		= tpa6130a2_i2c_probe,
	.remove    			= tpa6130a2_i2c_remove,
//	.detect				= tpa6130a2_i2c_detect,
	.id_table = tpa6130a2_i2c_id,
//	.address_data = &tpa6130a2_addr_data,

};

static int __init tpa6130a2_init(void)
{
	int ret = 0;

	GSE_FUN();
	printk("tpa6130a2_init ok!\n");

	//i2c_register_board_info(2, &i2c_tpa6130a2, 1);

	return i2c_add_driver(&tpa6130a2_i2c_driver);    
}
/*----------------------------------------------------------------------------*/
static void __exit tpa6130a2_exit(void)
{
	GSE_FUN();
	i2c_del_driver(&tpa6130a2_i2c_driver);
}
/*----------------------------------------------------------------------------*/
module_init(tpa6130a2_init);
module_exit(tpa6130a2_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TPA6130A2 Headphone amplifier driver");
MODULE_AUTHOR("zhangwenyuan@huaqin.com");

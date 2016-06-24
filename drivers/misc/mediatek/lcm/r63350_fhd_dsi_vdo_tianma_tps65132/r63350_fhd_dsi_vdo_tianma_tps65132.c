/* BEGIN PN:DTS2013053103858 , Added by d00238048, 2013.05.31*/
#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/disp_drv_platform.h>
#include <platform/mt_i2c.h>
#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
#else
    //#include <linux/delay.h>
#include <mach/mt_gpio.h>
#include <linux/i2c.h>
#endif

#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL,fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif

#define LCD_HW_ID_STATUS_LOW      0
#define LCD_HW_ID_STATUS_HIGH     1
#define LCD_HW_ID_STATUS_FLOAT 0x02
#define LCD_HW_ID_STATUS_ERROR  0x03


const static unsigned char LCD_MODULE_ID = 0x02;
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define FRAME_WIDTH										(1080)
#define FRAME_HEIGHT										(1920)


#define REGFLAG_DELAY										0xFC
#define REGFLAG_END_OF_TABLE								0xFD   // END OF REGISTERS MARKER

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)										lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
//#include <linux/jiffies.h>
#include <linux/uaccess.h>
//#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
/*****************************************************************************
 * Define
 *****************************************************************************/
#define TPS_I2C_BUSNUM  1  //I2C_I2C_LCD_BIAS_CHANNEL//for I2C channel 0
#define I2C_ID_NAME "tps65132"
#define TPS_ADDR 0x3E
/*****************************************************************************
 * GLobal Variable
 *****************************************************************************/
static struct i2c_board_info __initdata tps65132_board_info = {I2C_BOARD_INFO(I2C_ID_NAME, TPS_ADDR)};
static struct i2c_client *tps65132_i2c_client = NULL;


/*****************************************************************************
 * Function Prototype
 *****************************************************************************/
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tps65132_remove(struct i2c_client *client);
/*****************************************************************************
 * Data Structure
 *****************************************************************************/

 struct tps65132_dev	{
	struct i2c_client	*client;

};

static const struct i2c_device_id tps65132_id[] = {
	{ I2C_ID_NAME, 0 },
	{ }
};

//#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
//static struct i2c_client_address_data addr_data = { .forces = forces,};
//#endif
static struct i2c_driver tps65132_iic_driver = {
	.id_table	= tps65132_id,
	.probe		= tps65132_probe,
	.remove		= tps65132_remove,
	//.detect		= mt6605_detect,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "tps65132",
	},

};
/*****************************************************************************
 * Extern Area
 *****************************************************************************/

/*****************************************************************************
 * Function
 *****************************************************************************/
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk( "*********hx8394d tps65132_iic_probe\n");
	printk("*********hx8394d TPS: info==>name=%s addr=0x%x\n",client->name,client->addr);
	tps65132_i2c_client  = client;
	return 0;
}


static int tps65132_remove(struct i2c_client *client)
{
  printk( "*********hx8394d tps65132_remove\n");
  tps65132_i2c_client = NULL;
   i2c_unregister_device(client);
  return 0;
}


 static int tps65132_write_bytes(unsigned char addr, unsigned char value)
{
	int ret = 0;
	struct i2c_client *client = tps65132_i2c_client;
	char write_data[2]={0};
	write_data[0]= addr;
	write_data[1] = value;
    ret=i2c_master_send(client, write_data, 2);
	if(ret<0)
	printk("*********hx8394d tps65132 write data fail !!\n");
	return ret ;
}



/*
 * module load/unload record keeping
 */

static int __init tps65132_iic_init(void)
{

   printk( "*********hx8394d tps65132_iic_init\n");
   i2c_register_board_info(TPS_I2C_BUSNUM, &tps65132_board_info, 1);
   printk( "*********hx8394d tps65132_iic_init2\n");
   i2c_add_driver(&tps65132_iic_driver);
   printk( "*********hx8394d tps65132_iic_init success\n");
   return 0;
}

static void __exit tps65132_iic_exit(void)
{
  printk( "*********hx8394d tps65132_iic_exit\n");
  i2c_del_driver(&tps65132_iic_driver);
}


module_init(tps65132_iic_init);
module_exit(tps65132_iic_exit);

MODULE_AUTHOR("Xiaokuan Shi");
MODULE_DESCRIPTION("MTK TPS65132 I2C Driver");
MODULE_LICENSE("GPL");
#endif


struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[128];
};


//update initial param for IC r63350
static struct LCM_setting_table lcm_initialization_setting[] = {

        /* r63350 start */
	{0xB0,  1, {0x00}},
	{0xD6,  1, {0x01}},
	{0xB3,  6, {0x14, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{0xB4,  2, {0x0C, 0x00}},
	{0xB6,  3, {0x4A, 0xDB,0x16}},
	{0xB7,  1, {0x00}},
	{0xC0,  1, {0x00}},
	{0xC1, 35, {0x84, 0x60, 0x00, 0xFF, 0x8F,
		    0xF2, 0xD1, 0x31, 0xE1, 0x47,
		    0xF8, 0x5C, 0x63, 0xAC, 0xB9,
		    0x07, 0xE3, 0x07, 0xE6, 0xA0,
		    0x4F, 0xC4, 0xFF, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x6A, 0x01,
		    0x02, 0x20, 0x00, 0x01,0x11}},
	{0xC2,  8, {0x31, 0xF7, 0x80, 0x06, 0x0E, 0x00, 0x00,0x08}},
	{0xC3,  3, {0x00, 0x00, 0x00}},
	{0xC4, 11, {0x70, 0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00, 0x01,
		    0x03}},
	{0xC6, 21, {0xC8, 0x01, 0x75, 0x05, 0x6A,
		    0x00, 0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x0A, 0x1B, 0x07,
		    0xC8}},

	{0xC7, 30, {0x00,0x0D,0x14,0x1D,0x2C,
		0x3B,0x46,0x57,0x3C,0x45,
		0x51,0x5E,0x66,0x6C,0x6F,
		0x00,0x0D,0x14,0x1D,0x2C,
		0x3B,0x46,0x57,0x3C,0x45,
		0x51,0x5E,0x66,0x6C,0x6F}},

	{0xC8, 19, {0x00, 0x00, 0x06, 0xFC, 0x02,
		    0xFC, 0x00, 0x00, 0x07, 0xFC,
		    0x04, 0xE4, 0x00, 0x03, 0x00,
		    0xFF,0x08,0xAE,0x00}},

	{0xCA, 43, {0x1C, 0xFC, 0xFC, 0xFC, 0x00,
		    0x00, 0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00, 0x00}},

	{0xCB,  15, {0xEC, 0xFD, 0xBF, 0x37, 0x20,
		    0x00, 0x00, 0x04, 0x00,0x00,
		    0x00, 0x00, 0xE0, 0x00,0x00,}},
	{0xCC,  1, {0x0E}},
	{0xCE, 25, {0x55, 0x40, 0x49, 0x53, 0x59,
		    0x5E, 0x63, 0x68, 0x6E, 0x74,
		    0x7E, 0x8A, 0x98, 0xA8, 0xBB,
		    0xD0, 0xFF, 0x04, 0x00, 0x04,
		    0x04, 0x42, 0x00,0x69,0x5A}},
	{0xD0, 10, {0x11, 0x00, 0x00, 0x54, 0xCF,
		    0x40, 0x19, 0x19, 0x09, 0x00}},
	{0xD1, 4, {0x00, 0x48, 0x16, 0x0F}},

	{0xD3, 26, {0x1B, 0x33, 0xBB, 0xBB, 0xB3,
		    0x33, 0x33, 0x33, 0X33,0x00,
		    0x01, 0x00, 0x00, 0xD8, 0xA0, 0x05,
		    0x2F, 0x2F, 0x33, 0x33, 0x72,
		    0x12, 0x8A, 0x57, 0x3D, 0xBC}},
	{0xD5,  7, {0x06,0x00,0x00,0x01,0x1F,0x01,0x1F}},
	{0xD7, 24, {0xBF, 0xF8, 0x7F, 0xA8, 0xCE,
		    0x3E, 0xFC, 0xC1, 0xE1, 0xEF,
		    0x83, 0x07, 0x3F, 0x10, 0x7F,
		    0xC0, 0x01, 0xE7, 0x40, 0x1C,
		    0x00, 0xC0,0x00,0x00}},
	{0xDE,  4, {0x00, 0x33, 0xF8, 0x10}},
	{0x35,  1, {0x00}},
	{0x51,  1, {0xFF}},
	{0x53,  1, {0x2C}},
	{0x55,  1, {0x00}},
	{0x29,  0, {}},
	{REGFLAG_DELAY, 20, {}},
	{0x11,  0, {}},
	{REGFLAG_DELAY, 120, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    //Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},

    {0xD3, 25, {0x13, 0x33, 0xBB, 0xB3, 0xB3,
                0x33, 0x33, 0x33, 0x00, 0x01,
                0x00, 0x00, 0xD8, 0xA0, 0x05,
                0x2F, 0x2F, 0x33, 0x33, 0x72,
                0x12, 0x8A, 0x57, 0x3D, 0xBC}},
    {REGFLAG_DELAY, 50, {}},
    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++)
    {
        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

            case REGFLAG_DELAY :
                if(table[i].count <= 10)
                    MDELAY(table[i].count);
                else
                    MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));
    params->type   = LCM_TYPE_DSI;
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;
    params->physical_width = 68;
    params->physical_height = 121;

    params->dsi.mode   = BURST_VDO_MODE;//SYNC_PULSE_VDO_MODE;

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_FOUR_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

   // Highly depends on LCD driver capability.
   //video mode timing

    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

    params->dsi.vertical_sync_active				= 2;
    params->dsi.vertical_backporch				= 4;
    params->dsi.vertical_frontporch				= 4;
    params->dsi.vertical_active_line				= FRAME_HEIGHT;

    params->dsi.horizontal_sync_active				= 20;
    params->dsi.horizontal_backporch				= 40;
    params->dsi.horizontal_frontporch				= 100;
    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

    //improve clk quality
    params->dsi.PLL_CLOCK = 450; //this value must be in MTK suggested table
    params->dsi.compatibility_for_nvk = 1;
    params->dsi.ssc_disable = 1;

    params->pwm_min = 6;
    params->pwm_default = 81;
    params->pwm_max = 255;
    params->camera_blk = 194;
    params->camera_dua_blk = 194;
    params->camera_rec_blk = 194;

    params->dsi.esd_check_enable = 1;
    params->dsi.customization_esd_check_enable = 1;
    params->dsi.lcm_esd_check_table[0].cmd			= 0x0A;
    params->dsi.lcm_esd_check_table[0].dcs_cmd_type =  0x6;
    params->dsi.lcm_esd_check_table[0].count		= 1;
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x1C;
}
#if 0
/*to prevent electric leakage*/
static void lcm_id_pin_handle(void)
{
    mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
    mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,GPIO_PULL_DOWN);
}
#endif


#ifdef BUILD_LK
static struct mt_i2c_t tps65132_i2c;
#define TPS65132_SLAVE_ADDR 0x3e
/**********************************************************
  *
  *   [I2C Function For Read/Write hx8394d]
  *
  *********************************************************/
static kal_uint32 TPS65132_write_byte(kal_uint8 addr, kal_uint8 value)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;

    write_data[0]= addr;
    write_data[1] = value;

    tps65132_i2c.id = 1;
    /* Since i2c will left shift 1 bit, we need to set hx8394d I2C address to >>1 */
    tps65132_i2c.addr = (TPS65132_SLAVE_ADDR);
    tps65132_i2c.mode = ST_MODE;
    tps65132_i2c.speed = 100;
    len = 2;

    ret_code = i2c_write(&tps65132_i2c, write_data, len);
    printf("%s: i2c_write: ret_code: %d\n", __func__, ret_code);

    return ret_code;
}
#endif


static void TPS65132_enable(char en)
{

	mt_set_gpio_mode(GPIO_LCD_BIAS_ENP_PIN, GPIO_LCD_BIAS_ENP_PIN_M_GPIO);
	mt_set_gpio_mode(GPIO_LCD_BIAS_ENN_PIN, GPIO_LCD_BIAS_ENN_PIN_M_GPIO);
	mt_set_gpio_dir(GPIO_LCD_BIAS_ENP_PIN, GPIO_DIR_OUT);
	mt_set_gpio_dir(GPIO_LCD_BIAS_ENN_PIN, GPIO_DIR_OUT);

	if (en)
	{

		mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
		MDELAY(5);
		mt_set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
		MDELAY(20);
		#ifdef BUILD_LK
		TPS65132_write_byte(0x00, 0x0E);
		MDELAY(5);
		TPS65132_write_byte(0x01, 0x0E);
		MDELAY(5);
		#else
		//tps65132_write_bytes(0x00, 0x0E);
		//MDELAY(5);
		//tps65132_write_bytes(0x01, 0x0E);
		//MDELAY(5);
		#endif
	}
	else
	{

		mt_set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ZERO);
		MDELAY(10);
		mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
		MDELAY(10);
	}

}


static void lcm_init(void)
{
  unsigned int data_array[35];
#ifdef BUILD_LK
	printf("%s,uboot:r63350 init enter .\n",__func__);
#else
	printk("%s,kernel:r63350 init enter.\n", __func__);
#endif
	 //enable VSP & VSN  ,here to set Offset voltage
    //lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
    //lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ONE);
    //mdelay(50);


	TPS65132_enable(1);
	MDELAY(1);

	//reset high to low to high
    mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(20);

   push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);


    //lcm_util.set_gpio_out(GPIO_LCD_DRV_EN_PIN, GPIO_OUT_ONE);


#ifdef BUILD_LK
	printf("%s,uboot:r63350 lcm_init exit.\n", __func__);
#else
	printk("%s,kernel:r63350 lcm_ini exit.\n", __func__);
#endif

}


static void lcm_suspend(void)
{
    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

/*
	unsigned int data_array[16];
	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(20);

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(40);
*/
    //reset low
    TPS65132_enable(0);

    mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);

    //lcm_util.set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    MDELAY(20);

#if 0
    //disable VSP & VSN
	lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
	lcm_util.set_gpio_out(GPIO_LCD_BIAS_ENN_PIN, GPIO_OUT_ZERO);
    mdelay(5);
#endif
#ifdef BUILD_LK
	printf("%s,uboot:r63350 suspend exit.\n", __func__);
#else
	printk("%s,kernel:r63350 suspend exit.\n", __func__);
#endif

}
static void lcm_resume(void)
{
    lcm_init();
#ifdef BUILD_LK
	printf("%s,uboot:r63350  resume exit.\n", __func__);
#else
	printk("%s,kernel:r63350 resume exit.\n", __func__);
#endif

}

static unsigned int lcm_compare_id(void)
{
     unsigned char low_read0 = 0;
     unsigned char  high_read0 = 0;
     unsigned int ret = 0;
     unsigned char  lcd_id0 = 0;
     unsigned char buffer[3];
     unsigned int id = 0xFF;
     unsigned int data_array[16];

     TPS65132_enable(1);
     MDELAY(1);

    //reset high to low to high
    mt_set_gpio_mode(GPIO_LCM_RST, GPIO_MODE_00);
    mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
    MDELAY(10);

    mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);
    MDELAY(20);

    ret = mt_set_gpio_mode(GPIO_DISP_ID0_PIN, GPIO_MODE_00);
    if(0 != ret)
    {
    #ifdef BUILD_LK
        printf("ID0 mt_set_gpio_mode fail\n");
	#else
	    printk("ID0 mt_set_gpio_mode fail\n");
	#endif
    }

    ret = mt_set_gpio_dir(GPIO_DISP_ID0_PIN, GPIO_DIR_IN);
    if(0 != ret)
    {
    #ifdef BUILD_LK
        printf("ID0 mt_set_gpio_dir fail\n");
	#else
		printk("ID0 mt_set_gpio_dir fail\n");
	#endif
    }

    ret = mt_set_gpio_pull_enable(GPIO_DISP_ID0_PIN, GPIO_PULL_ENABLE);
    if(0 != ret)
    {
    #ifdef BUILD_LK
        printf("ID0 mt_set_gpio_pull_enable fail\n");
	#else
	    printk("ID0 mt_set_gpio_pull_enable fail\n");
	#endif
    }

	//pull down ID0 PIN
    ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
    if(0 != ret)
    {
    #ifdef BUILD_LK
        printf("ID0 mt_set_gpio_pull_select->Down fail\n");
	#else
		printk("ID0 mt_set_gpio_pull_select->Down fail\n");
	#endif
    }

	MDELAY(100);
    //get ID0  status
    low_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);

    //pull up ID0 PIN
    ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_UP);
    if(0 != ret)
    {
	#ifdef BUILD_LK
        printf("ID0 mt_set_gpio_pull_select->Up fail\n");
	#else
		printk("ID0 mt_set_gpio_pull_select->Up fail\n");
	#endif
    }

	//delay 100ms , for charging capacitance
    MDELAY(100);
    //get ID0 ID1 status
    high_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
#ifdef BUILD_LK
	printf("low_read0 =%d,high_read0=%d\n",low_read0,high_read0);
#else
    printk("low_read0 =%d,high_read0=%d\n",low_read0,high_read0);
#endif

	if( low_read0 != high_read0 )
    {
        /*float status , pull down ID0 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
        if(0 != ret)
        {
        #ifdef BUILD_LK
            printf("ID0 mt_set_gpio_pull_select->Down fail\n");
		#else
			printk("ID0 mt_set_gpio_pull_select->Down fail\n");
		#endif
        }
        lcd_id0 = LCD_HW_ID_STATUS_FLOAT;
    }
    else if((LCD_HW_ID_STATUS_LOW == low_read0) && (LCD_HW_ID_STATUS_LOW == high_read0))
    {
        /*low status , pull down ID0 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DOWN);
        if(0 != ret)
        {
            #ifdef BUILD_LK
            printf("ID0 mt_set_gpio_pull_select->Down fail\n");
		#else
			printk("ID0 mt_set_gpio_pull_select->Down fail\n");
		#endif
        }
        lcd_id0 = LCD_HW_ID_STATUS_LOW;
    }
    else if((LCD_HW_ID_STATUS_HIGH == low_read0) && (LCD_HW_ID_STATUS_HIGH == high_read0))
    {
        /*high status , pull up ID0 ,to prevent electric leakage*/
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_UP);
        if(0 != ret)
        {
        #ifdef BUILD_LK
            printf("ID0 mt_set_gpio_pull_select->Up fail\n");
		#else
			printk("ID0 mt_set_gpio_pull_select->Up fail\n");
		#endif
        }
        lcd_id0 = LCD_HW_ID_STATUS_HIGH;
    }
    else
    {
     #ifdef BUILD_LK
        printf(" Read LCD_id0 error\n");
	 #else
		printk(" Read LCD_id0 error\n");
	 #endif
        ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,GPIO_PULL_DISABLE);
        if(0 != ret)
        {
        #ifdef BUILD_LK
            printf("ID0 mt_set_gpio_pull_select->Disbale fail\n");
		#else
			printk("ID0 mt_set_gpio_pull_select->Disbale fail\n");
		#endif
        }
        lcd_id0 = LCD_HW_ID_STATUS_ERROR;
    }
#ifdef BUILD_LK
	printf("which_lcd_module_triple,lcd_id0:%d\n",lcd_id0);
#else
    printk("which_lcd_module_triple,lcd_id0:%d\n",lcd_id0);
#endif


if(lcd_id0 == 0)
{
   data_array[0] = 0x00033700;
   dsi_set_cmdq(data_array, 1, 1);

   MDELAY(5);
   read_reg_v2(0xBF, buffer, 1);
   id = buffer[0];
}

#ifdef BUILD_LK
	printf("r63350 0xbf register value =%x\n",id);
#endif

    // r63350 0xbf register value is 0x2
    if((lcd_id0 == 0)&&(id == 2))
		return 1;
	else
		return 0;
}

static void lcm_setbacklight(unsigned int level)
{
#ifdef BUILD_LK
	printf("%s, r63350 backlight: level = %d\n",__func__, level);
#else
	printk("%s, r63350 backlight: level = %d\n", __func__, level);
#endif
	// Refresh value of backlight level.

	unsigned int cmd = 0x51;
	unsigned int count = 1;
	unsigned int value = level;
	dsi_set_cmdq_V2(cmd,count,&value,1);
}

static void lcm_set_lcm_cmd(void* handle,unsigned int *lcm_cmd,unsigned char *lcm_count,unsigned char *lcm_value)
{
#ifdef BUILD_LK
	printf("%s, lcm\n", __func__);
#else
     printk("%s, lcm\n", __func__);
#endif

	unsigned int cmd = lcm_cmd[0];
	unsigned char count = lcm_count[0];
	unsigned char *ppara =  lcm_value;

	dsi_set_cmdq_V2(cmd, count, ppara, 1);
}


LCM_DRIVER r63350_fhd_dsi_vdo_tianma_tps65132_drv =
{
    .name           = "r63350_fhd_dsi_vdo_tianma_tps65132",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,/*shunyu init fun.*/
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id     = lcm_compare_id,
    .set_backlight  = lcm_setbacklight,
	.set_lcm_cmd    = lcm_set_lcm_cmd,
};


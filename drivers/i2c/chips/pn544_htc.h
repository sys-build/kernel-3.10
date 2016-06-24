/******************************************************************************
 *
 *  This is the interface file for the PN547 NFC HTC customization Functions
 *
 ******************************************************************************/

/*for htc platform specified functions*/






/*default turn off for htc platform specified functions*/
#ifndef NFC_READ_RFSKUID
#define NFC_READ_RFSKUID 0
#endif

#ifndef NFC_GET_BOOTMODE
#define NFC_GET_BOOTMODE 0
#endif

#ifndef NFC_OFF_MODE_CHARGING_ENABLE
#define NFC_OFF_MODE_CHARGING_ENABLE 0
#endif

#ifndef NFC_POWER_OFF_SEQUENCE
#define NFC_POWER_OFF_SEQUENCE 0
#endif

#ifndef NFC_PVDD_GPIO_DT
#define NFC_PVDD_GPIO_DT 0
#endif

#ifndef NFC_PLL_CLK_ONOFF
#define NFC_PLL_CLK_ONOFF 0
#endif

#ifndef NFC_PVDD_LOAD_SWITCH
#define NFC_PVDD_LOAD_SWITCH 0
#endif

#ifndef NFC_READ_RFSKUID_MTK6795
#define NFC_READ_RFSKUID_MTK6795 0
#endif //NFC_READ_RFSKUID_MTK6795

#ifndef NFC_READ_RFSKUID_MTK6753
#define NFC_READ_RFSKUID_MTK6753 0
#endif //NFC_READ_RFSKUID_MTK6753

#ifndef NFC_READ_RFSKUID_A50CML
#define NFC_READ_RFSKUID_A50CML 0
#define NFC_I2C_SDA GPIO51
#define NFC_I2C_SCL GPIO52
#endif //NFC_READ_RFSKUID_A50CML

/* Define boot mode for NFC*/
#define NFC_BOOT_MODE_NORMAL 0
#define NFC_BOOT_MODE_FTM 1
#define NFC_BOOT_MODE_DOWNLOAD 2
#define NFC_BOOT_MODE_OFF_MODE_CHARGING 5



/******************************************************************************
 *
 *	Function pn544_htc_check_rfskuid:
 *	Return With(1)/Without(0) NFC chip if this SKU can get RFSKUID in kernal
 *	Return is_alive(original value) by default.
 *
 ******************************************************************************/
int pn544_htc_check_rfskuid(int in_is_alive);

/******************************************************************************
 *
 *  Function pn544_htc_get_bootmode:
 *  Return  NFC_BOOT_MODE_NORMAL            0
 *          NFC_BOOT_MODE_FTM               1
 *          NFC_BOOT_MODE_DOWNLOAD          2
 *          NFC_BOOT_MODE_OFF_MODE_CHARGING 5
 *  Return  NFC_BOOT_MODE_NORMAL by default
 *          if there's no bootmode infomation available
 ******************************************************************************/
int pn544_htc_get_bootmode(void);


/******************************************************************************
 *
 *  Function pn544_htc_get_bootmode:
 *  Get platform required GPIO number from device tree
 *  For Power off sequence and OFF_MODE_CHARGING
 *
 ******************************************************************************/
void pn544_htc_parse_dt(struct device *dev);

/******************************************************************************
 *
 *  Function pn544_htc_off_mode_charging
 *  Turn of NFC_PVDD when bootmode = NFC_BOOT_MODE_OFF_MODE_CHARGING
 *
 ******************************************************************************/
void pn544_htc_off_mode_charging (void);


/******************************************************************************
 *
 *  Function pn544_htc_pvdd_on
 *  Turn on NFC_PVDD
 *
 ******************************************************************************/
int pn544_htc_pvdd_on (void);


/******************************************************************************
 *
 *  Function pn544_htc_pvdd_off
 *  Turn off NFC_PVDD
 *
 ******************************************************************************/
int pn544_htc_pvdd_off(void);


/******************************************************************************
 *
 *  Function pn544_pll_clk_ctrl
 *  Enable or Disable PLL clock
 *
 ******************************************************************************/
int pn544_pll_clk_ctrl(int onoff);

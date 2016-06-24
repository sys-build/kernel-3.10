#include <linux/kernel.h>
#include <linux/string.h>

#include <mach/mtk_rtc.h>
#include <mach/wd_api.h>
extern void wdt_arch_reset(char);

#ifdef CONFIG_HTC_REBOOT_REASON_INFORMATION
#include <linux/htc_reboot_info.h>
#endif

#include <linux/mrdump.h>
#include <linux/aee.h>
#include <linux/htc_devices_dtb.h>

void arch_reset(char mode, const char *cmd)
{
	char reboot = NO_AUTOREBOOT_RESET;
	int res = 0;
	struct wd_api *wd_api = NULL;
#ifdef CONFIG_FPGA_EARLY_PORTING
    return ;
#else
	int ramdump_debug_en = (get_radio_flag() & 0x8?1:0);
	res = get_wd_api(&wd_api);
	pr_notice("arch_reset: cmd = %s\n", cmd ? : "NULL");
	pr_notice("arch_reset: config[8] = 0x%.8X\n", get_radio_flag());

#ifdef CONFIG_HTC_REBOOT_REASON_INFORMATION
#ifdef CONFIG_MTK_AEE_MRDUMP
	switch(mrdump_get_ramdump_reason()) {
	case AEE_REBOOT_MODE_KERNEL_OOPS:
	case AEE_REBOOT_MODE_KERNEL_PANIC:
	case AEE_REBOOT_MODE_NESTED_EXCEPTION:
	case AEE_REBOOT_MODE_WDT:
	case AEE_REBOOT_MODE_MANUAL_KDUMP:
	case AEE_REBOOT_MODE_MODEM_FATAL:
		reboot = (ramdump_debug_en?SW_RESET:HW_RESET);
		goto do_arch_reboot;
		break;
	default:
		/* HTC: If no mrdump reason exist, set normal boot. */
		htc_reboot_reason_update_by_cmd(cmd);
		mrdump_set_ramdump_reason(AEE_REBOOT_MODE_NORMAL ,cmd);
		break;
	}
#else
	htc_reboot_reason_update_by_cmd(cmd);
	mrdump_set_ramdump_reason(AEE_REBOOT_MODE_NORMAL ,cmd);
#endif /* CONFIG_MTK_AEE_MRDUMP */

#endif /* CONFIG_HTC_REBOOT_REASON_INFORMATION */
	if (cmd && !strcmp(cmd, "charger")) {
		/* do nothing */
	} else if (cmd && !strcmp(cmd, "recovery")) {
 #ifndef CONFIG_MTK_FPGA
		rtc_mark_recovery();
 #endif
	} else if (cmd && !strcmp(cmd, "bootloader")) {
 #ifndef CONFIG_MTK_FPGA
 		reboot = HW_RESET;
		rtc_mark_fast();
 #endif
	}
#ifdef CONFIG_MTK_KERNEL_POWER_OFF_CHARGING
	else if (cmd && !strcmp(cmd, "kpoc")) {
		rtc_mark_kpoc();
	}
#endif
	else if(cmd && !strncmp(cmd, "oem-", 4)){
		unsigned long code;
		code = simple_strtoul(cmd + 4, NULL, 16) & 0xff;
		/* oem-93, 94, 95, 96, 97, 98, 99 are RIL fatal, septup ramdump reason */
		if ((code >= 0x93) && (code <= 0x99)){
#ifdef CONFIG_MTK_AEE_MRDUMP
			mrdump_set_ramdump_reason(AEE_REBOOT_MODE_MODEM_FATAL, cmd);
#endif
			reboot = SW_RESET; /* for modem off-line ramdump */
		} else {
			reboot = (htc_is_need_sw_reboot() == 1?SW_RESET:HW_RESET);
		}
	}
	else if (cmd && !strncmp(cmd, "force-hard", 10)){
#ifdef CONFIG_MTK_AEE_MRDUMP
		mrdump_set_ramdump_reason(AEE_REBOOT_MODE_MANUAL_KDUMP, cmd);
#endif
		reboot = (ramdump_debug_en?SW_RESET:HW_RESET); /* for manual ramdump */
	}
	else if(reboot != SW_RESET) {
		reboot = (htc_is_need_sw_reboot() == 1?SW_RESET:HW_RESET);
	}

do_arch_reboot:

	if (res) {
		pr_notice("arch_reset, get wd api error %d\n", res);
	} else {
		wd_api->wd_sw_reset(reboot);
	}
 #endif
}

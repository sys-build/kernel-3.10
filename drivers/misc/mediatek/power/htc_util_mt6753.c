#include <linux/rtc.h>
#include <linux/tick.h>
#include <linux/time.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/pm_wakeup.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/kernel_stat.h>
#include <linux/workqueue.h>
#include <mach/htc_util.h>
#include <mach/mt_pm_ldo.h>
#include <mach/gpio_const.h>
#include <mach/upmu_common.h>
#include <mach/mt_gpio_core.h>
#include <mach/mtk_thermal_monitor.h>
#include <linux/irq.h>
#include <mach/mt_spm.h>
#include <linux/stacktrace.h>
#include <linux/slab.h>

#define MAX_PID				32768
#define POWER_PROFILE_POLLING_TIME	10000 /* 10 seconds */
#define THERMAL_LOG_SHOWN_INTERVAL	60000 /* 60 seconds */
#define NUM_BUSY_PROCESS_CHECK		5

struct _htc_kernel_top {
	struct delayed_work dwork;
	int *curr_proc_delta;
        int *curr_proc_delta_kernel;
	int *curr_proc_pid;
	unsigned int *prev_proc_stat;
        unsigned int *prev_proc_stat_kernel;
	struct task_struct **proc_ptr_array;
	struct kernel_cpustat curr_cpustat;
	struct kernel_cpustat prev_cpustat;
	unsigned long cpustat_time;
	int top_loading_pid[NUM_BUSY_PROCESS_CHECK];
	unsigned long curr_cpu_usage[8][NR_STATS];
	unsigned long prev_cpu_usage[8][NR_STATS];
	spinlock_t lock;
};

static struct workqueue_struct *htc_pm_monitor_wq = NULL;

static const char * const thermal_dev_name[MTK_THERMAL_SENSOR_COUNT] = {
	[MTK_THERMAL_SENSOR_CPU]	= "cpu",
	[MTK_THERMAL_SENSOR_ABB]	= "abb",
	[MTK_THERMAL_SENSOR_PMIC]	= "pmic",
	[MTK_THERMAL_SENSOR_BATTERY]	= "batt",
	[MTK_THERMAL_SENSOR_MD1]	= "md1",
	[MTK_THERMAL_SENSOR_MD2]	= "md2",
	[MTK_THERMAL_SENSOR_WIFI]	= "wifi",
	[MTK_THERMAL_SENSOR_BATTERY2]	= "batt2",
	[MTK_THERMAL_SENSOR_BUCK]	= "buck",
	[MTK_THERMAL_SENSOR_AP]		= "ap",
	[MTK_THERMAL_SENSOR_PCB1]	= "pcb1",
	[MTK_THERMAL_SENSOR_PCB2]	= "pcb2",
	[MTK_THERMAL_SENSOR_SKIN]	= "skin",
	[MTK_THERMAL_SENSOR_XTAL]	= "xtal"
};


static const char * const mt63xx_vreg_name[MT65XX_POWER_LDO_DEFAULT] = {
	[MT6328_POWER_LDO_VAUX18]		= "mt6328_power_ldo_vaux18",
	[MT6328_POWER_LDO_VTCXO_0]		= "mt6328_power_ldo_vtcxo0",
	[MT6328_POWER_LDO_VTCXO_1]		= "mt6328_power_ldo_vtcxo1",
	[MT6328_POWER_LDO_VAUD28]		= "mt6328_power_ldo_vauxa28",
	[MT6328_POWER_LDO_VCN28]		= "mt6328_power_ldo_vcn28",
	[MT6328_POWER_LDO_VCAMA]		= "mt6328_power_ldo_vcama",
	[MT6328_POWER_LDO_VCN33_BT]		= "mt6328_power_ldo_vcn33_bt",
	[MT6328_POWER_LDO_VCN33_WIFI] 	 	= "mt6328_power_ldo_vcn33_wifi",
	[MT6328_POWER_LDO_VUSB33]	 	= "mt6328_power_ldo_vusb33",
	[MT6328_POWER_LDO_VEFUSE]	 	= "mt6328_power_ldo_vefuse",
	[MT6328_POWER_LDO_VSIM1]		= "mt6328_power_ldo_vsim1",
	[MT6328_POWER_LDO_VSIM2]		= "mt6328_power_ldo_vsim2",
	[MT6328_POWER_LDO_VEMC33]		= "mt6328_power_ldo_vemc33",
	[MT6328_POWER_LDO_VMCH] 	    	= "mt6328_power_ldo_vmach",
	[MT6328_POWER_LDO_VTREF]	 	= "mt6328_power_ldo_vtref",
	[MT6328_POWER_LDO_VMC]		 	= "mt6328_power_ldo_vmc",
	[MT6328_POWER_LDO_VCAM_AF]	 	= "mt6328_power_ldo_vcam_af",
	[MT6328_POWER_LDO_VIO28] 	 	= "mt6328_power_ldo_vio28",
	[MT6328_POWER_LDO_VGP1] 	 	= "mt6328_power_ldo_vgp1",
	[MT6328_POWER_LDO_VIBR] 	 	= "mt6328_power_ldo_vibr",
	[MT6328_POWER_LDO_VCAMD] 	 	= "mt6328_power_ldo_vcamd",
	[MT6328_POWER_LDO_VRF18_0]		= "mt6328_power_ldo_vrf18_0",
	[MT6328_POWER_LDO_VRF18_1] 	 	= "mt6328_power_ldo_vrf18_1",
	[MT6328_POWER_LDO_VIO18]	 	= "mt6328_power_ldo_vin18",
	[MT6328_POWER_LDO_VCN18]		= "mt6328_power_ldo_vcn18",
	[MT6328_POWER_LDO_VCAM_IO]		= "mt6328_power_ldo_vcam_io",
	[MT6328_POWER_LDO_VSRAM]	    	= "mt6328_power_ldo_vsram",
	[MT6328_POWER_LDO_VM]			= "mt6328_power_ldo_vm",
};

struct _vregs {
	int vreg_id;
	char vreg_name[32];
};

struct _vregs mtk_vregs[MT65XX_POWER_LDO_DEFAULT];
static char *gpio_sleep_status_info;
static char *vreg_sleep_status_info;
static int slept = 0;
static unsigned int pon_reason = 0xFFFF;

ssize_t htc_pm_show_wakelocks(char *buf, int size, bool show_active);
extern int aee_rr_last_fiq_step(void);

char htc_boot_reason[][16] =
    { "keypad", "usb_chg", "rtc_alarm", "wdt", "reboot", "tool_reboot", "smpl", "unknow", "kpanic", "wdt_sw", "wdt_hw" };
static void htc_show_pon_poff()
{
	char *br_ptr;
	if ((pon_reason == 0xFFFF) &&
		(br_ptr = strstr(saved_command_line, "boot_reason=")) != 0) {
		/* get boot reason */
		pon_reason = br_ptr[12] - '0';
#ifdef CONFIG_MTK_RAM_CONSOLE
		if (aee_rr_last_fiq_step() != 0)
			pon_reason = 8;//BR_KE_REBOOT
#endif
	} else {
		if (pon_reason <= 10)
			printk("[K] pon: %s, poff: %s\n", htc_boot_reason[pon_reason], "NA");
	}
}

static void htc_show_wakelocks(void)
{
	char wl_buf[256];
	int print_size;
	int buf_size;

	buf_size = sizeof(wl_buf);
	print_size = htc_pm_show_wakelocks(wl_buf, buf_size, true);
	printk("wakeup sources: %s", wl_buf);
	if (print_size >= (buf_size-1))
		printk("Can't print wakelocks due to out of buf size.\n");

	return;
}

static void htc_show_thermal_temp(void)
{
	int i = 0, temp = 0;
	static int count = 0;

	if (++count >= (THERMAL_LOG_SHOWN_INTERVAL / POWER_PROFILE_POLLING_TIME)) {
		for (i = 0; i < MTK_THERMAL_SENSOR_COUNT; i++) {
			temp = mtk_thermal_get_temp(i);
			if (temp == -127000)
				continue;
			printk("[K][THERMAL] Sensor %d (%5s) = %d\n", i, thermal_dev_name[i], temp);
		}
		count = 0;
	}

	return;
}

static void sort_cputime_by_pid(int *src, int *pid_pos, int pid_cnt, int *result)
{
    int i = 0, j = 0, k = 0, l = 0;
    int pid_found = 0;

    for (i = 0; i < NUM_BUSY_PROCESS_CHECK; i++) {
        result[i] = 0;
        if (i == 0) {
            for (j = 0; j < pid_cnt; j++) {
                k = pid_pos[j];
                /* Find the largest one. */
                if(src[result[i]] < src[k]) {
                    result[i] = k;
                }
            }
        } else {
            for (j = 0; j < pid_cnt; j++) {
                k = pid_pos[j];
                /* Skip the saved PIDs. */
                for (l = 0; l < i; l++) {
                    /* Field (index k) is saved already. */
                    if (result[l] == k) {
                        pid_found = 1;
                        break;
                    }
                }
                /* Found the saved PID and skip it (index k). */
                if (pid_found) {
                    pid_found = 0;
                    continue;
                }

                /* Find the largest one from rest fields. */
                if(src[result[i]] < src[k]) {
                    result[i] = k;
                }
            }
        }
    }
}

#ifdef arch_idle_time
static cputime64_t get_idle_time(int cpu)
{
	cputime64_t idle;

	idle = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
	if (cpu_online(cpu) && !nr_iowait_cpu(cpu))
		idle += arch_idle_time(cpu);

	return idle;
}

static cputime64_t get_iowait_time(int cpu)
{
	cputime64_t iowait;

	iowait = kcpustat_cpu(cpu).cpustat[CPUTIME_IOWAIT];
	if (cpu_online(cpu) && nr_iowait_cpu(cpu))
		iowait += arch_idle_time(cpu);

	return iowait;
}
#else
static u64 get_idle_time(int cpu)
{
	u64 idle = 0;
	u64 idle_time = get_cpu_idle_time_us(cpu, NULL);

	if (idle_time == -1ULL)
		/* !NO_HZ so we can rely on cpustat.idle */
		idle = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
	else
		idle = usecs_to_cputime64(idle_time);

	return idle;
}

static u64 get_iowait_time(int cpu)
{
	u64 iowait;
	u64 iowait_time = get_cpu_iowait_time_us(cpu, NULL);

	if (iowait_time == -1ULL)
		/* !NO_HZ so we can rely on cpustat.iowait */
		iowait = kcpustat_cpu(cpu).cpustat[CPUTIME_IOWAIT];
	else
		iowait = usecs_to_cputime64(iowait_time);

	return iowait;
}
#endif

static void get_all_cpustat(struct _htc_kernel_top *ktop, struct kernel_cpustat *cpu_stat)
{
	int cpu = 0;

	if (!cpu_stat)
		return;

	memset(cpu_stat, 0, sizeof(struct kernel_cpustat));

	for_each_possible_cpu(cpu) {
		cpu_stat->cpustat[CPUTIME_USER] += kcpustat_cpu(cpu).cpustat[CPUTIME_USER];
		cpu_stat->cpustat[CPUTIME_NICE] += kcpustat_cpu(cpu).cpustat[CPUTIME_NICE];
		cpu_stat->cpustat[CPUTIME_SYSTEM] += kcpustat_cpu(cpu).cpustat[CPUTIME_SYSTEM];
		cpu_stat->cpustat[CPUTIME_IDLE] += get_idle_time(cpu);
		cpu_stat->cpustat[CPUTIME_IOWAIT] += get_iowait_time(cpu);
		cpu_stat->cpustat[CPUTIME_IRQ] += kcpustat_cpu(cpu).cpustat[CPUTIME_IRQ];
		cpu_stat->cpustat[CPUTIME_SOFTIRQ] += kcpustat_cpu(cpu).cpustat[CPUTIME_SOFTIRQ];
		cpu_stat->cpustat[CPUTIME_STEAL] += kcpustat_cpu(cpu).cpustat[CPUTIME_STEAL];
		cpu_stat->cpustat[CPUTIME_GUEST] += kcpustat_cpu(cpu).cpustat[CPUTIME_GUEST];
		cpu_stat->cpustat[CPUTIME_GUEST_NICE] += kcpustat_cpu(cpu).cpustat[CPUTIME_GUEST_NICE];
		ktop->curr_cpu_usage[cpu][CPUTIME_USER] = kcpustat_cpu(cpu).cpustat[CPUTIME_USER];
		ktop->curr_cpu_usage[cpu][CPUTIME_NICE] = kcpustat_cpu(cpu).cpustat[CPUTIME_NICE];
		ktop->curr_cpu_usage[cpu][CPUTIME_SYSTEM] = kcpustat_cpu(cpu).cpustat[CPUTIME_SYSTEM];
		ktop->curr_cpu_usage[cpu][CPUTIME_IDLE] = get_idle_time(cpu);
	}

	return;
}

static unsigned long htc_calculate_cpustat_time(struct kernel_cpustat curr_cpustat,
						struct kernel_cpustat prev_cpustat)
{
	unsigned long user_time = 0, system_time = 0, io_time = 0;
	unsigned long irq_time = 0, idle_time = 0;

	user_time = (unsigned long) ((curr_cpustat.cpustat[CPUTIME_USER] +
					curr_cpustat.cpustat[CPUTIME_NICE]) -
					(prev_cpustat.cpustat[CPUTIME_USER] +
					prev_cpustat.cpustat[CPUTIME_NICE]));
	system_time = (unsigned long) (curr_cpustat.cpustat[CPUTIME_SYSTEM] -
					prev_cpustat.cpustat[CPUTIME_SYSTEM]);
	io_time = (unsigned long) (curr_cpustat.cpustat[CPUTIME_IOWAIT] -
					prev_cpustat.cpustat[CPUTIME_IOWAIT]);
	irq_time = (unsigned long) ((curr_cpustat.cpustat[CPUTIME_IRQ] +
					curr_cpustat.cpustat[CPUTIME_SOFTIRQ]) -
					(prev_cpustat.cpustat[CPUTIME_IRQ] +
					prev_cpustat.cpustat[CPUTIME_SOFTIRQ]));
	idle_time = (unsigned long) ((curr_cpustat.cpustat[CPUTIME_IDLE] > prev_cpustat.cpustat[CPUTIME_IDLE]) ?
					curr_cpustat.cpustat[CPUTIME_IDLE] - prev_cpustat.cpustat[CPUTIME_IDLE] : 0);

	idle_time += (unsigned long) ((curr_cpustat.cpustat[CPUTIME_STEAL] +
					curr_cpustat.cpustat[CPUTIME_GUEST]) -
					(prev_cpustat.cpustat[CPUTIME_STEAL] +
					prev_cpustat.cpustat[CPUTIME_GUEST]));

	return (user_time + system_time + io_time + irq_time + idle_time);
}

static void htc_calc_kernel_top(struct _htc_kernel_top *ktop)
{
	int pid_cnt = 0;
	ulong flags;
	struct task_struct *proc;
	struct task_cputime cputime;

	if(ktop->proc_ptr_array == NULL ||
	   ktop->curr_proc_delta == NULL ||
           ktop->curr_proc_delta_kernel == NULL ||
	   ktop->curr_proc_pid == NULL ||
	   ktop->prev_proc_stat == NULL ||
	   ktop->prev_proc_stat_kernel == NULL)
		return;

	spin_lock_irqsave(&ktop->lock, flags);

	/* Calculate cpu time of each process */
	for_each_process(proc) {
		if (proc->pid < MAX_PID) {
			thread_group_cputime(proc, &cputime);
			ktop->curr_proc_delta[proc->pid] =
				(cputime.utime + cputime.stime) -
				ktop->prev_proc_stat[proc->pid];
                        ktop->curr_proc_delta_kernel[proc->pid] =
                                cputime.stime -
                                ktop->prev_proc_stat_kernel[proc->pid];
			ktop->proc_ptr_array[proc->pid] = proc;

			if (ktop->curr_proc_delta[proc->pid] > 0) {
				ktop->curr_proc_pid[pid_cnt] = proc->pid;
				pid_cnt++;
			}
		}
	}
	sort_cputime_by_pid(ktop->curr_proc_delta, ktop->curr_proc_pid, pid_cnt, ktop->top_loading_pid);

	/* Calculate cpu time of cpus */
	get_all_cpustat(ktop, &ktop->curr_cpustat);
	ktop->cpustat_time = htc_calculate_cpustat_time(ktop->curr_cpustat, ktop->prev_cpustat);

	/* Save old process cpu time info */
	for_each_process(proc) {
		if (proc->pid < MAX_PID) {
			thread_group_cputime(proc, &cputime);
			ktop->prev_proc_stat[proc->pid] = cputime.stime + cputime.utime;
			ktop->prev_proc_stat_kernel[proc->pid] = cputime.stime;
		}
	}
	memcpy(&ktop->prev_cpustat, &ktop->curr_cpustat, sizeof(struct kernel_cpustat));
	spin_unlock_irqrestore(&ktop->lock, flags);

	return;
}

#define MAX_STACK_DEPTH   64

static int print_pid_stack(struct task_struct *task)
{
	struct stack_trace trace;
	unsigned long *entries;
	int err;
	int i;

	entries = kmalloc(MAX_STACK_DEPTH * sizeof(*entries), GFP_KERNEL);
	if (!entries)
		return -ENOMEM;

	trace.nr_entries	= 0;
	trace.max_entries	= MAX_STACK_DEPTH;
	trace.entries		= entries;
	trace.skip		= 0;

	if (1) {
		save_stack_trace_tsk(task, &trace);

		for (i = 0; i < trace.nr_entries; i++) {
			printk("[K][<%pK>] %pS\n",
				   (void *)entries[i], (void *)entries[i]);
		}
	}
	kfree(entries);

	return err;
}

static void htc_show_kernel_top(struct _htc_kernel_top *ktop)
{
	int top_n_pid = 0, i;
	int top_pid = 0;
	int top_usage = 0;
        int top_usage_kernel = 0;
	struct task_struct *t;
	int cpu = 0;
	unsigned long usage = 0, total = 0, idle_time = 0, proc_usage, proc_usage_kernel;

	/* Print most time consuming processes */
	printk("[K] CPU Usage\tPID\tName\t\tCPU Time (Total: %lu)\n", ktop->cpustat_time);
	for (i = 0; i < NUM_BUSY_PROCESS_CHECK; i++) {
		if (ktop->cpustat_time > 0) {
			top_n_pid = ktop->top_loading_pid[i];
			proc_usage = ktop->curr_proc_delta[top_n_pid] * 100 / ktop->cpustat_time;
			proc_usage_kernel = ktop->curr_proc_delta_kernel[top_n_pid] * 100 / ktop->curr_proc_delta[top_n_pid];
			printk("[K]%8lu(%lu)%%\t%d\t%s\t\t%d(%d)\n",
				proc_usage,proc_usage_kernel,
				top_n_pid,
				ktop->proc_ptr_array[top_n_pid]->comm,
				ktop->curr_proc_delta[top_n_pid],ktop->curr_proc_delta_kernel[top_n_pid]);
		}
		if (i == 0){
			top_pid = ktop->top_loading_pid[i];
			top_usage = proc_usage;
			top_usage_kernel = proc_usage_kernel;
		}
	}
	if (top_usage >= 8 && top_usage_kernel >= 50){
		t = ktop->proc_ptr_array[top_pid];
		do {
			printk("[K] Name %s ,ID %d ,CALL trace:\n",t->comm,t->pid);
			print_pid_stack(t);
		}while_each_thread(ktop->proc_ptr_array[top_pid],t);

		show_state_filter(TASK_UNINTERRUPTIBLE);
	}

	/* Show cpu usage */
	printk("[K] CPU usage per core: ");
	for_each_possible_cpu(cpu) {
		usage = (ktop->curr_cpu_usage[cpu][CPUTIME_USER] -
			ktop->prev_cpu_usage[cpu][CPUTIME_USER]) +
			(ktop->curr_cpu_usage[cpu][CPUTIME_NICE] -
			ktop->prev_cpu_usage[cpu][CPUTIME_NICE]) +
			(ktop->curr_cpu_usage[cpu][CPUTIME_SYSTEM] -
			ktop->prev_cpu_usage[cpu][CPUTIME_SYSTEM]);
		idle_time = (ktop->curr_cpu_usage[cpu][CPUTIME_IDLE] > ktop->prev_cpu_usage[cpu][CPUTIME_IDLE]) ?
			     ktop->curr_cpu_usage[cpu][CPUTIME_IDLE] - ktop->prev_cpu_usage[cpu][CPUTIME_IDLE] : 0;
		total = usage + idle_time;
		printk("[%lu%%]", usage *100 / total);
		ktop->prev_cpu_usage[cpu][CPUTIME_USER] = ktop->curr_cpu_usage[cpu][CPUTIME_USER];
		ktop->prev_cpu_usage[cpu][CPUTIME_NICE] = ktop->curr_cpu_usage[cpu][CPUTIME_NICE];
		ktop->prev_cpu_usage[cpu][CPUTIME_SYSTEM] = ktop->curr_cpu_usage[cpu][CPUTIME_SYSTEM];
		ktop->prev_cpu_usage[cpu][CPUTIME_IDLE] = ktop->curr_cpu_usage[cpu][CPUTIME_IDLE];
	}
	printk("\n");
	memset(ktop->curr_proc_delta, 0, sizeof(int) * MAX_PID);
        memset(ktop->curr_proc_delta_kernel, 0, sizeof(int) * MAX_PID);
	memset(ktop->proc_ptr_array, 0, sizeof(struct task_struct *) * MAX_PID);
	memset(ktop->curr_proc_pid, 0, sizeof(int) * MAX_PID);

	return;
}

static void htc_pm_monitor_work_func(struct work_struct *work)
{
	struct _htc_kernel_top *ktop = container_of(work, struct _htc_kernel_top,
					dwork.work);
	struct timespec ts;
	struct rtc_time tm;

	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec - (sys_tz.tz_minuteswest * 60), &tm);
	printk("[K][PM] hTC PM Statistic start (%02d-%02d %02d:%02d:%02d)\n",
		tm.tm_mon +1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

       /* Show interrupt status */
       htc_show_interrupts();
#ifdef CONFIG_TIMER_STATS
       /* Show timer stats */
       htc_timer_stats_onoff('0');
       htc_timer_stats_show(300); /*Show timer events which greater than 300 every 10 sec*/
       htc_timer_stats_onoff('1');
#endif

	/* Show wakeup source */
	htc_print_active_wakeup_sources();
	/* Show temperature info of each thermal device */
	htc_show_thermal_temp();
	/* Show kernel top */
	htc_calc_kernel_top(ktop);
	htc_show_kernel_top(ktop);
	/* Show pon and poff reason */
	htc_show_pon_poff();
	printk("[K][PM] hTC PM Statistic done\n");
	queue_delayed_work(htc_pm_monitor_wq, &ktop->dwork, msecs_to_jiffies(POWER_PROFILE_POLLING_TIME));
}

static struct dentry *gpio_dbgfs_base;
static struct dentry *vreg_dbgfs_base;
static struct dentry *vreg_dbgfs;
static struct dentry *ptpod_dbgfs_base;

int mt_dump_gpios(struct seq_file *m, int curr_len, char *gpio_buffer)
{
	int idx = 0, len = 0;
	char list_gpio[128];
	char *title_msg = "------------ MTK GPIO -------------";

	if (m) {
		seq_printf(m, "%s\n", title_msg);
	} else {
                printk("%s\n", title_msg);
                curr_len += sprintf(gpio_buffer + curr_len,
				"%s\n", title_msg);
        }

	for (idx = MT_GPIO_BASE_START; idx < MT_GPIO_BASE_MAX; idx++) {
		memset(list_gpio, 0 , sizeof(list_gpio));
		len = 0;
		len += sprintf(list_gpio + len, "GPIO[%3d]: ", idx);
		len += sprintf(list_gpio + len, "[MODE]0x%d, ", mt_get_gpio_mode_base(idx));
		len += sprintf(list_gpio + len, "[DIR]%s, ",
				mt_get_gpio_dir_base(idx) >= 0 ? (mt_get_gpio_dir_base(idx) ? " OUT" : "  IN") : "NULL");
		len += sprintf(list_gpio + len, "[PULL_SEL]%s, ",
				mt_get_gpio_pull_select_base(idx) >= 0 ? (mt_get_gpio_pull_select_base(idx) ? "  UP" : "DOWN") : "NULL");
		len += sprintf(list_gpio + len, "[DIN]0x%d, ", mt_get_gpio_in_base(idx));
		len += sprintf(list_gpio + len, "[DOUT]%s, ",
				mt_get_gpio_out_base(idx) >= 0 ? (mt_get_gpio_out_base(idx) ? "HIGH" : " LOW") : "NULL");
		len += sprintf(list_gpio + len, "[PULL_EN]%s, ",
				mt_get_gpio_pull_enable_base(idx) >= 0 ? (mt_get_gpio_pull_enable_base(idx) ? "  EN" : " DIS"): "NULL");
		len += sprintf(list_gpio + len, "[IES]%s",
				mt_get_gpio_ies_base(idx) >= 0 ? (mt_get_gpio_ies_base(idx) ? "  EN" : " DIS"): "NULL");
#if 0
		len += sprintf(list_gpio + len, "[SMT]0x%d, ", mt_get_gpio_smt_base(idx));
#endif
		list_gpio[127] = '\0';
                if (m) {
                        seq_printf(m, "%s\n", list_gpio);
                } else {
                        printk("%s\n", list_gpio);
                        curr_len += sprintf(gpio_buffer + curr_len, "%s\n", list_gpio);
                }

	}

	return curr_len;
}

int dump_sleep_gpio(void)
{
	slept = 1;
	if (gpio_sleep_status_info)
		mt_dump_gpios(NULL, 0, gpio_sleep_status_info);
}

static int list_sleep_gpios_show(struct seq_file *m, void *unused)
{
	if (slept)
		seq_printf(m, gpio_sleep_status_info);
	else
		seq_printf(m, "Device haven't suspended yet or sleep gpio dump flag doesn't be enabled!\n");
        return 0;
}

static int list_sleep_gpios_open(struct inode *inode, struct file *file)
{
        return single_open(file, list_sleep_gpios_show, inode->i_private);
}

static const struct file_operations list_sleep_gpios_fops = {
        .open           = list_sleep_gpios_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

static int list_gpios_show(struct seq_file *m, void *unused)
{
        mt_dump_gpios(m, 0, NULL);
        return 0;
}

static int list_gpios_open(struct inode *inode, struct file *file)
{
        return single_open(file, list_gpios_show, inode->i_private);
}

static const struct file_operations list_gpios_fops = {
        .open           = list_gpios_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

extern int pmic_ldo_get_status(int powerId);
extern int pmic_ldo_get_voltage(int powerId);
/* MTK Vreg control */
static int vreg_en_get(void *data, u64 *val)
{
	struct _vregs *vreg = data;
	int vreg_id = vreg->vreg_id;

	*val = pmic_ldo_get_status(vreg_id);

	return 0;
}

static int vreg_en_set(void *data, u64 val)
{
	struct _vregs *vreg = data;
	int vreg_id = vreg->vreg_id;

	pmic_ldo_enable(vreg_id, val);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(vregs_en_fops, vreg_en_get,
			vreg_en_set, "%lld\n");

static int vreg_vol_get(void *data, u64 *val)
{
	struct _vregs *vreg = data;
	int vreg_id = vreg->vreg_id;

	*val = pmic_ldo_get_voltage(vreg_id);

	return 0;
}

static int vreg_vol_set(void *data, u64 val)
{
	struct _vregs *vreg = data;
	int vreg_id = vreg->vreg_id;

	pmic_ldo_vol_sel(vreg_id, val);

	return 0;
}


DEFINE_SIMPLE_ATTRIBUTE(vregs_vol_fops, vreg_vol_get,
			vreg_vol_set, "%lld\n");

#if 0
extern unsigned long htc_vcore_vol_get(void);
extern int htc_vcore_vol_set(int vcore_uv);

static int vcore_vol_get(void *data, u64 *val)
{
	*val = htc_vcore_vol_get();

	return 0;
}

static int vcore_vol_set(void *data, u64 val)
{
	/* uV */
	if (val > 1310000 || val < 700000) {
		printk("%s: not support voltage: %llu\n", __func__, val);
		return -1;
	}
	htc_vcore_vol_set(val);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(vcore_vol_fops,vcore_vol_get,
			vcore_vol_set, "%lld\n");

extern unsigned long htc_vgpu_vol_get(void);
extern int htc_vgpu_vol_set(int vgpu_uv);

static int vgpu_vol_get(void *data, u64 *val)
{
	*val = htc_vgpu_vol_get();

	return 0;
}

static int vgpu_vol_set(void *data, u64 val)
{
	/* uV */
	if (val > 1310000 || val < 700000) {
		printk("%s: not support voltage: %llu\n", __func__, val);
		return -1;
	}
	htc_vgpu_vol_set(val);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(vgpu_vol_fops,vgpu_vol_get,
			vgpu_vol_set, "%lld\n");
#endif

int mt_dump_vregs(struct seq_file *m, int curr_len, char *vreg_buffer)
{
	u32 val = 0;
	int idx = 0, len = 0;
	char list_vreg[128];
	char *title_msg = "------------ MTK VREG -------------";

	if (m) {
		seq_printf(m, "%s\n", title_msg);
	} else {
                printk("%s\n", title_msg);
                curr_len += sprintf(vreg_buffer + curr_len,
				"%s\n", title_msg);
        }

	for (idx = 0; idx < MT65XX_POWER_LDO_DEFAULT; idx++) {
		memset(list_vreg, 0 , sizeof(list_vreg));
		len = 0;
		len += sprintf(list_vreg + len, "%25s: ", mtk_vregs[idx].vreg_name);
		/* Get enable status */
		val = pmic_ldo_get_status(idx);
		len += sprintf(list_vreg + len, "[ EN] %s, ", val ? "YES" : "NO ");
		/* Get voltage */
		val = pmic_ldo_get_voltage(idx);
		len += sprintf(list_vreg + len, "[Vol] %7u %s", val, val < 100000 ? "mV" : "uV");

		list_vreg[127] = '\0';
                if (m) {
                        seq_printf(m, "%s\n", list_vreg);
                } else {
                        printk("%s\n", list_vreg);
                        curr_len += sprintf(vreg_buffer + curr_len, "%s\n", list_vreg);
                }
	}

	return curr_len;
}

static int list_vregs_show(struct seq_file *m, void *unused)
{
        mt_dump_vregs(m, 0, NULL);
        return 0;
}

static int list_vregs_open(struct inode *inode, struct file *file)
{
        return single_open(file, list_vregs_show, inode->i_private);
}

static const struct file_operations list_vregs_fops = {
        .open           = list_vregs_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

int dump_sleep_vreg(void)
{
	slept = 1;
	if (vreg_sleep_status_info)
		mt_dump_vregs(NULL, 0, vreg_sleep_status_info);
}

static int list_sleep_vregs_show(struct seq_file *m, void *unused)
{
	if (slept)
		seq_printf(m, vreg_sleep_status_info);
	else
		seq_printf(m, "Device haven't suspended yet or sleep vreg dump flag doesn't be enabled!\n");
        return 0;
}

static int list_sleep_vregs_open(struct inode *inode, struct file *file)
{
        return single_open(file, list_sleep_vregs_show, inode->i_private);
}

static const struct file_operations list_sleep_vregs_fops = {
        .open           = list_sleep_vregs_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

static int htc_vreg_create_dbgfs(void)
{
	int idx = 0;
	struct _vregs *vreg = NULL;

	/* MTK VREG debugfs */
	vreg_dbgfs_base = debugfs_create_dir("htc_vreg", NULL);
	if (!vreg_dbgfs_base)
		return -ENOMEM;

	/* Buck and LDO on mt6325 */
	for (idx = 0; idx < MT65XX_POWER_LDO_DEFAULT; idx++) {
		mtk_vregs[idx].vreg_id = idx;
		sprintf(mtk_vregs[idx].vreg_name, "%s", mt63xx_vreg_name[idx]);
		vreg_dbgfs = debugfs_create_dir(mtk_vregs[idx].vreg_name, vreg_dbgfs_base);
		if (!vreg_dbgfs)
			return -ENOMEM;
		vreg = &mtk_vregs[idx];
		if (!debugfs_create_file("enable", S_IRUGO, vreg_dbgfs,
					  vreg, &vregs_en_fops))
			return -ENOMEM;
		if (!debugfs_create_file("voltage", S_IRUGO, vreg_dbgfs,
					vreg, &vregs_vol_fops))
			return -ENOMEM;
	}
	if (!debugfs_create_file("list_vregs", S_IRUGO, vreg_dbgfs_base,
                        NULL, &list_vregs_fops))
                return -ENOMEM;

	if (!debugfs_create_file("list_sleep_vregs", S_IRUGO, vreg_dbgfs_base,
                        NULL, &list_sleep_vregs_fops))
                return -ENOMEM;
	vreg_sleep_status_info = vmalloc(25000);
	if (!vreg_sleep_status_info) {
		pr_err("[PM] vmalloc memory failed in %s\n", __func__);
	}

	return 0;
}

extern void htc_get_wakeup_source_cnt(u32 index, char *buf);
extern void htc_get_wakeup_eint_cnt(u32 index, char *buf);
static int wakeup_source_show(struct seq_file *m, void *unused)
{
	unsigned int i;
	unsigned char buf[256] = {0};

	seq_printf(m, "index      sleep\tdeepidle\t  soidle\t name\n");
	for(i=0; i<33; i++) {
		htc_get_wakeup_source_cnt(i, buf);
		seq_printf(m, "%s\n", buf);
	}
	i = 212;
	htc_get_wakeup_eint_cnt(i, buf);
	seq_printf(m, "%s\n", buf);
	i = 206;
	htc_get_wakeup_eint_cnt(i, buf);
	seq_printf(m, "%s\n", buf);
    return 0;
}

static int wakeup_source_cnt_open(struct inode *inode, struct file *file)
{
        return single_open(file, wakeup_source_show, inode->i_private);
}

static const struct file_operations wakeup_source_cnt_fops = {
        .open           = wakeup_source_cnt_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
static int htc_wakeup_cnt_dbgfs(void)
{
	if (!debugfs_create_file("htc_wakeup_source_cnt", S_IRUGO, NULL,
                        NULL, &wakeup_source_cnt_fops))
		return -ENOMEM;
}

extern int get_ptpod_status(void);
static int ptpod_enable_get(void *data, u64 *val)
{
	*val = get_ptpod_status();

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ptpod_fops, ptpod_enable_get,
			NULL, "%lld\n");

static int htc_ptpod_create_dbgfs(void)
{
	ptpod_dbgfs_base = debugfs_create_dir("htc_ptpod", NULL);
	if (!ptpod_dbgfs_base)
		return -ENOMEM;

	if (!debugfs_create_file("enable", S_IRUGO, ptpod_dbgfs_base,
			NULL, &ptpod_fops))
		return -ENOMEM;
}

static int __init htc_monitor_init(void)
{
	int cpu;
	struct _htc_kernel_top *htc_kernel_top;

	if (htc_pm_monitor_wq == NULL)
		/* Create private workqueue */
		htc_pm_monitor_wq = create_workqueue("htc_pm_monitor_wq");

	if (!htc_pm_monitor_wq) {
		pr_err("[K] Fail to create htc_pm_monitor_wq\n");
		return -1;
	}

	printk("[K] Success to create htc_pm_monitor_wq.\n");
	htc_kernel_top = vmalloc(sizeof(*htc_kernel_top));
	spin_lock_init(&htc_kernel_top->lock);

	htc_kernel_top->prev_proc_stat = vmalloc(sizeof(unsigned int) * MAX_PID);
        htc_kernel_top->prev_proc_stat_kernel = vmalloc(sizeof(unsigned int) * MAX_PID);
	htc_kernel_top->curr_proc_delta = vmalloc(sizeof(int) * MAX_PID);
        htc_kernel_top->curr_proc_delta_kernel = vmalloc(sizeof(int) * MAX_PID);
	htc_kernel_top->proc_ptr_array = vmalloc(sizeof(struct task_struct *) * MAX_PID);
	htc_kernel_top->curr_proc_pid = vmalloc(sizeof(int) * MAX_PID);

	memset(htc_kernel_top->prev_proc_stat, 0, sizeof(unsigned int) * MAX_PID);
        memset(htc_kernel_top->prev_proc_stat_kernel, 0, sizeof(unsigned int) * MAX_PID);
	memset(htc_kernel_top->curr_proc_delta, 0, sizeof(int) * MAX_PID);
        memset(htc_kernel_top->curr_proc_delta_kernel, 0, sizeof(int) * MAX_PID);
	memset(htc_kernel_top->proc_ptr_array, 0, sizeof(struct task_struct *) * MAX_PID);
	memset(htc_kernel_top->curr_proc_pid, 0, sizeof(int) * MAX_PID);

	for_each_possible_cpu(cpu) {
		memset(htc_kernel_top->curr_cpu_usage[cpu], 0, NR_STATS);
		memset(htc_kernel_top->prev_cpu_usage[cpu], 0, NR_STATS);
	}

	INIT_DELAYED_WORK(&htc_kernel_top->dwork, htc_pm_monitor_work_func);
	queue_delayed_work(htc_pm_monitor_wq, &htc_kernel_top->dwork,
					msecs_to_jiffies(POWER_PROFILE_POLLING_TIME));

	/* MTK GPIO debugfs */
	gpio_dbgfs_base = debugfs_create_dir("htc_gpio", NULL);
	if (!gpio_dbgfs_base)
		return -ENOMEM;

        if (!debugfs_create_file("list_gpios", S_IRUGO, gpio_dbgfs_base,
                        NULL, &list_gpios_fops))
                return -ENOMEM;

        if (!debugfs_create_file("list_sleep_gpios", S_IRUGO, gpio_dbgfs_base,
                        NULL, &list_sleep_gpios_fops))
                return -ENOMEM;
	gpio_sleep_status_info = vmalloc(25000);
	if (!gpio_sleep_status_info) {
		pr_err("[PM] vmalloc memory failed in %s\n", __func__);
	}

	/* TODO: Don't create while security on */
	/* MTK VREG debugfs */
	htc_vreg_create_dbgfs();
	/* HTC debugfs for MTK PTPOD */
	htc_ptpod_create_dbgfs();
	htc_wakeup_cnt_dbgfs();

	return 0;
}

static void __exit htc_monitor_exit(void)
{
	return;
}

module_init(htc_monitor_init);
module_exit(htc_monitor_exit);

MODULE_DESCRIPTION("HTC Power Utility Profile driver");
MODULE_AUTHOR("Kenny Liu <kenny_liu@htc.com>");
MODULE_LICENSE("GPL");

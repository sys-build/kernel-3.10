/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/atomic.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/atomic.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/of_fdt.h>
#include <linux/io.h>
#include <asm-generic/sizes.h>
#include <linux/msm_rtb.h>

#define SENTINEL_BYTE_1 0xFF
#define SENTINEL_BYTE_2 0xAA
#define SENTINEL_BYTE_3 0xFF

#define RTB_COMPAT_STR	"qcom,msm-rtb"

/* Write
 * 1) 11 bytes sentinel
 * 2) 1 bytes of log type
 * 3) 8 bytes of where the caller came from
 * 4) 4 bytes index
 * 4) 8 bytes extra data from the caller
 *
 * Total = 32 bytes.
 */
struct msm_rtb_layout {
	unsigned char sentinel[11];
	unsigned char log_type;
	uint32_t idx;
	uint64_t caller;
	uint64_t data;
} __attribute__ ((__packed__));


struct msm_rtb_state {
	struct msm_rtb_layout *rtb;
	phys_addr_t phys;
	int nentries;
	int size;
	int enabled;
	int initialized;
	uint32_t filter;
	int step_size;
};

#if defined(CONFIG_MSM_RTB_SEPARATE_CPUS)
DEFINE_PER_CPU(atomic_t, msm_rtb_idx_cpu);
#else
static atomic_t msm_rtb_idx;
#endif

static struct msm_rtb_state msm_rtb = {

#if defined(CONFIG_HTC_DEBUG_RTB)
	/* remove msm_rtb.filter from cmdline to control the filter here */
	.filter = (1 << LOGK_READL)|(1 << LOGK_WRITEL)|(1 << LOGK_LOGBUF)|(1 << LOGK_HOTPLUG)|(1 << LOGK_CTXID)|(1 << LOGK_IRQ)|(1 << LOGK_DIE),
#else
	.filter = 1 << LOGK_LOGBUF,
#endif
	.enabled = 1,
};

module_param_named(filter, msm_rtb.filter, uint, 0644);
module_param_named(enable, msm_rtb.enabled, int, 0644);

#if defined(CONFIG_HTC_DEBUG_RTB)

#define HTC_DEBUG_RTB_MAGIC 0x5254424D /* RTBM */

struct htc_debug_rtb {
	unsigned int magic;
	unsigned int phys;
	unsigned int cpu_idx[];
};


static int htc_debug_rtb_save(struct platform_device *pdev)
{
	struct resource *res = NULL;
	phys_addr_t phys;
	resource_size_t size;
	struct htc_debug_rtb *rtb = NULL;
	unsigned int cpu;

	if (!pdev->dev.of_node) {
		pr_err("%s: of_node not found\n", __func__);
		return -EINVAL;
	}
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
			"htc_debug_rtb_res");
	if (!res) {
		pr_err("%s: 'htc_debug_rtb_res' not found\n", __func__);
		return -EINVAL;
	}
	size = resource_size(res);
	if (!size) {
		pr_err("%s: invalid size\n", __func__);
		return -EINVAL;
	}
	phys = res->start;

	rtb = ioremap(phys, size);
#ifdef CONFIG_PHYS_ADDR_T_64BIT
	if (!rtb) {
		pr_err("%s: ioremap(0x%llx, 0x%llx) failed\n", __func__, 
					(unsigned long long)phys, (unsigned long long)size);
		return -ENOMEM;
	}
	pr_notice("htc_debug_rtb: phys: 0x%llx, size: 0x%llx\n", 
					(unsigned long long)phys, (unsigned long long)size);
#else
        if (!rtb) {
                pr_err("%s: ioremap(0x%x, 0x%x) failed\n", __func__, (uint32_t)phys, (uint32_t)size);
                return -ENOMEM;
        }
        pr_notice("htc_debug_rtb: phys: 0x%x, size: 0x%x\n", (uint32_t)phys, (uint32_t)size);	
#endif

	rtb->magic = (unsigned int) HTC_DEBUG_RTB_MAGIC;
	rtb->phys = (unsigned int) virt_to_phys(&msm_rtb);
	for (cpu = 0; cpu < msm_rtb.step_size; ++cpu)
		rtb->cpu_idx[cpu] = (unsigned int) virt_to_phys(&per_cpu(msm_rtb_idx_cpu, cpu));
	mb();

	iounmap(rtb);

	return 0;
}

void msm_rtb_disable(void)
{
	msm_rtb.enabled = 0;
	return;
}
EXPORT_SYMBOL(msm_rtb_disable);
#endif /* CONFIG_HTC_DEBUG_RTB */

static int msm_rtb_panic_notifier(struct notifier_block *this,
					unsigned long event, void *ptr)
{
	msm_rtb.enabled = 0;
	return NOTIFY_DONE;
}

static struct notifier_block msm_rtb_panic_blk = {
	.notifier_call  = msm_rtb_panic_notifier,
};

int notrace msm_rtb_event_should_log(enum logk_event_type log_type)
{
	return msm_rtb.initialized && msm_rtb.enabled &&
		((1 << (log_type & ~LOGTYPE_NOPC)) & msm_rtb.filter);
}
EXPORT_SYMBOL(msm_rtb_event_should_log);

static void msm_rtb_emit_sentinel(struct msm_rtb_layout *start)
{
	start->sentinel[0] = SENTINEL_BYTE_1;
	start->sentinel[1] = SENTINEL_BYTE_2;
	start->sentinel[2] = SENTINEL_BYTE_3;
}

static void msm_rtb_write_type(enum logk_event_type log_type,
			struct msm_rtb_layout *start)
{
	start->log_type = (char)log_type;
}

static void msm_rtb_write_caller(uint64_t caller, struct msm_rtb_layout *start)
{
	start->caller = caller;
}

static void msm_rtb_write_idx(uint32_t idx,
				struct msm_rtb_layout *start)
{
	start->idx = idx;
}

static void msm_rtb_write_data(uint64_t data, struct msm_rtb_layout *start)
{
	start->data = data;
}

static void uncached_logk_pc_idx(enum logk_event_type log_type, uint64_t caller,
				 uint64_t data, int idx)
{
	struct msm_rtb_layout *start;

	start = &msm_rtb.rtb[idx & (msm_rtb.nentries - 1)];

	msm_rtb_emit_sentinel(start);
	msm_rtb_write_type(log_type, start);
	msm_rtb_write_caller(caller, start);
	msm_rtb_write_idx(idx, start);
	msm_rtb_write_data(data, start);
	mb();

	return;
}

static void uncached_logk_timestamp(int idx)
{
	unsigned long long timestamp;

	timestamp = sched_clock();
	uncached_logk_pc_idx(LOGK_TIMESTAMP|LOGTYPE_NOPC,
			(uint64_t)lower_32_bits(timestamp),
			(uint64_t)upper_32_bits(timestamp), idx);
}

#if defined(CONFIG_MSM_RTB_SEPARATE_CPUS)
static int msm_rtb_get_idx(void)
{
	int cpu, i, offset;
	atomic_t *index;

	/*
	 * ideally we would use get_cpu but this is a close enough
	 * approximation for our purposes.
	 */
	cpu = raw_smp_processor_id();

	index = &per_cpu(msm_rtb_idx_cpu, cpu);

	i = atomic_add_return(msm_rtb.step_size, index);
	i -= msm_rtb.step_size;

	/* Check if index has wrapped around */
	offset = (i & (msm_rtb.nentries - 1)) -
		 ((i - msm_rtb.step_size) & (msm_rtb.nentries - 1));
	if (offset < 0) {
		uncached_logk_timestamp(i);
		i = atomic_add_return(msm_rtb.step_size, index);
		i -= msm_rtb.step_size;
	}

	return i;
}
#else
static int msm_rtb_get_idx(void)
{
	int i, offset;

	i = atomic_inc_return(&msm_rtb_idx);
	i--;

	/* Check if index has wrapped around */
	offset = (i & (msm_rtb.nentries - 1)) -
		 ((i - 1) & (msm_rtb.nentries - 1));
	if (offset < 0) {
		uncached_logk_timestamp(i);
		i = atomic_inc_return(&msm_rtb_idx);
		i--;
	}

	return i;
}
#endif

int notrace uncached_logk_pc(enum logk_event_type log_type, void *caller,
				void *data)
{
	int i;

	if (!msm_rtb_event_should_log(log_type))
		return 0;

	i = msm_rtb_get_idx();
	uncached_logk_pc_idx(log_type, (uint64_t)((unsigned long) caller),
				(uint64_t)((unsigned long) data), i);

	return 1;
}
EXPORT_SYMBOL(uncached_logk_pc);

noinline int notrace uncached_logk(enum logk_event_type log_type, void *data)
{
	return uncached_logk_pc(log_type, __builtin_return_address(0), data);
}
EXPORT_SYMBOL(uncached_logk);

static int __init msm_rtb_early_init(void)
{
#if defined(CONFIG_MSM_RTB_SEPARATE_CPUS)
	unsigned int cpu;
#endif
  struct device_node *np ;
  struct resource temp_res ;
  int num_reg = 0;
  memset(&temp_res, 0, sizeof(temp_res)) ;

  np = of_find_compatible_node(NULL, NULL, "qcom,msm-rtb") ;
  if(np)
  {
    if(of_can_translate_address(np))
    {
      while(of_address_to_resource(np, num_reg, &temp_res) == 0)
      {
        if(!strcmp(temp_res.name, "msm_rtb_res"))
        {
          msm_rtb.phys = temp_res.start ;
          msm_rtb.size = resource_size(&temp_res);
	        pr_notice("msm_rtb set ok: phys: 0x%llx, size: 0x%x\n", (unsigned long long)msm_rtb.phys, msm_rtb.size);
        }
        else if(!strcmp(temp_res.name, "htc_debug_rtb_res"))
        {
          //TODO
        }
        else
          pr_notice("%s: unknow rtb resource %s\n", __func__, temp_res.name) ;
        num_reg++ ;
      }
      if(!num_reg)
        pr_notice("%s: can't find address resource\n", __func__) ;
    }
    else
      pr_notice("%s, can't translate address\n", __func__) ;
  }
  else
    pr_notice("%s, can't find qcom,msm-rtb node\n", __func__) ;
	pr_notice("msm_rtb.size: 0x%x\n", msm_rtb.size);

	if (msm_rtb.phys == 0 || msm_rtb.size <= 0 || msm_rtb.size > SZ_1M)
		return -EINVAL;

	msm_rtb.rtb = ioremap(msm_rtb.phys, msm_rtb.size);

	if (!msm_rtb.rtb)
		return -ENOMEM;

	pr_notice("msm_rtb.rtb:0x%p\n", msm_rtb.rtb);

	msm_rtb.nentries = msm_rtb.size / sizeof(struct msm_rtb_layout);

	/* Round this down to a power of 2 */
	msm_rtb.nentries = __rounddown_pow_of_two(msm_rtb.nentries);

	memset(msm_rtb.rtb, 0, msm_rtb.size);


#if defined(CONFIG_MSM_RTB_SEPARATE_CPUS)
	for_each_possible_cpu(cpu) {
		atomic_t *a = &per_cpu(msm_rtb_idx_cpu, cpu);
		atomic_set(a, cpu);
	}
	msm_rtb.step_size = num_possible_cpus();
#else
	atomic_set(&msm_rtb_idx, 0);
	msm_rtb.step_size = 1;
#endif
	atomic_notifier_chain_register(&panic_notifier_list,
						&msm_rtb_panic_blk);

	msm_rtb.initialized = 1;

	return 0;
}

static int msm_rtb_probe(struct platform_device *pdev)
{
	struct msm_rtb_platform_data *d = pdev->dev.platform_data;
	struct resource *res = NULL;
#if defined(CONFIG_MSM_RTB_SEPARATE_CPUS)
	unsigned int cpu;
#endif
	int ret;

	if (!pdev->dev.of_node) {
		if (!d) {
			return -EINVAL;
		}
		msm_rtb.size = d->size;
	} else {
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "msm_rtb_res");
		if (res) {
			msm_rtb.size = resource_size(res);
		} else {
			u64 size;
			struct device_node *pnode;

			pnode = of_parse_phandle(pdev->dev.of_node,
							"linux,contiguous-region", 0);
			if (pnode != NULL) {
				const u32 *addr;

				addr = of_get_address(pnode, 0, &size, NULL);
				if (!addr) {
					of_node_put(pnode);
					return -EINVAL;
				}
				of_node_put(pnode);
			} else {
				ret = of_property_read_u32(pdev->dev.of_node,
						"qcom,rtb-size",
						(u32 *)&size);
				if (ret < 0)
					return ret;

			}
			msm_rtb.size = size;
		}
	}
	pr_notice("msm_rtb.size: 0x%x\n", msm_rtb.size);

	if (msm_rtb.size <= 0 || msm_rtb.size > SZ_1M)
		return -EINVAL;

	if (res) {
		msm_rtb.phys = res->start;
		msm_rtb.rtb = ioremap(msm_rtb.phys, msm_rtb.size);
	} else {
		msm_rtb.rtb = dma_alloc_coherent(&pdev->dev, msm_rtb.size, &msm_rtb.phys, GFP_KERNEL);
	}

	pr_notice("msm_rtb set ok: phys: 0x%llx, size: 0x%x\n", (unsigned long long)msm_rtb.phys, msm_rtb.size);

	if (!msm_rtb.rtb)
		return -ENOMEM;

	pr_notice("msm_rtb.rtb:0x%p\n", msm_rtb.rtb);

	msm_rtb.nentries = msm_rtb.size / sizeof(struct msm_rtb_layout);

	/* Round this down to a power of 2 */
	msm_rtb.nentries = __rounddown_pow_of_two(msm_rtb.nentries);

	memset(msm_rtb.rtb, 0, msm_rtb.size);


#if defined(CONFIG_MSM_RTB_SEPARATE_CPUS)
	for_each_possible_cpu(cpu) {
		atomic_t *a = &per_cpu(msm_rtb_idx_cpu, cpu);
		atomic_set(a, cpu);
	}
	msm_rtb.step_size = num_possible_cpus();
#else
	atomic_set(&msm_rtb_idx, 0);
	msm_rtb.step_size = 1;
#endif

	atomic_notifier_chain_register(&panic_notifier_list,
						&msm_rtb_panic_blk);
	msm_rtb.initialized = 1;

#if defined(CONFIG_HTC_DEBUG_RTB)
	htc_debug_rtb_save(pdev);
#endif

	return 0;
}

static struct of_device_id msm_match_table[] = {
	{.compatible = RTB_COMPAT_STR},
	{},
};

static struct platform_driver msm_rtb_driver = {
	.driver         = {
		.name = "msm_rtb",
		.owner = THIS_MODULE,
		.of_match_table = msm_match_table
	},
};

static int __init msm_rtb_init(void)
{
//	return platform_driver_probe(&msm_rtb_driver, msm_rtb_probe);
  return 0 ;
}

static void __exit msm_rtb_exit(void)
{
	//platform_driver_unregister(&msm_rtb_driver);
  return ;
}
module_init(msm_rtb_init)
module_exit(msm_rtb_exit)
console_initcall(msm_rtb_early_init) ;

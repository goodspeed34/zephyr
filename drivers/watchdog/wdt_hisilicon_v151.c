/*
 * Copyright (c) 2025, Gong Zhile
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT hisilicon_v151_watchdog

#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/sys_clock.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(wdt_hisilicon_v151, CONFIG_WDT_LOG_LEVEL);

#define HISILICON_V151_WDT_KEY		(0x5a5a5a5a)

#define HISILICON_V151_WDT_TIMEOUT_MIN	0xff
#define HISILICON_V151_WDT_TIMEOUT_MAX	0xFFFFFFFF

#define WDT_LOAD_BITS		8

#define WDT_CTRL_ENABLE_BIT	0
#define WDT_CTRL_MODE_RESET_BIT	1

struct hisilicon_v151_wdt_regs {
	uint32_t wdt_lock;
	uint32_t wdt_load;
	uint32_t wdt_restart;
	uint32_t wdt_eoi;
	uint32_t wdt_cr;
	uint32_t wdt_cnt;
	uint32_t wdt_raw_intr;
	uint32_t wdt_lpif;
	uint32_t wdt_status;
	uint32_t wdt_ccvr_en;
};

struct hisilicon_v151_wdt_config {
	mm_reg_t base;
	const struct device *clk_dev;
	clock_control_subsys_t clk_id;
};

static int wdt_hisilicon_v151_setup(const struct device *dev, uint8_t options)
{
	const struct hisilicon_v151_wdt_config *config = dev->config;
	struct hisilicon_v151_wdt_regs *regs = (void *) config->base;

	sys_write32(HISILICON_V151_WDT_KEY, (mem_addr_t) &regs->wdt_lock);

	sys_set_bit((mem_addr_t) &regs->wdt_cr, WDT_CTRL_ENABLE_BIT);
	sys_set_bit((mem_addr_t) &regs->wdt_cr, WDT_CTRL_MODE_RESET_BIT);

	sys_write32(HISILICON_V151_WDT_KEY, (mem_addr_t) &regs->wdt_restart);

	return 0;
}

static int wdt_hisilicon_v151_disable(const struct device *dev)
{
	const struct hisilicon_v151_wdt_config *config = dev->config;
	struct hisilicon_v151_wdt_regs *regs = (void *) config->base;

	sys_write32(HISILICON_V151_WDT_KEY, (mem_addr_t) &regs->wdt_lock);
	sys_clear_bit((mem_addr_t) &regs->wdt_cr, WDT_CTRL_ENABLE_BIT);

	return 0;
}

static int wdt_hisilicon_v151_install_timeout(const struct device *dev,
					      const struct wdt_timeout_cfg *cfg)
{
	const struct hisilicon_v151_wdt_config *config = dev->config;
	struct hisilicon_v151_wdt_regs *regs = (void *) config->base;
	uint32_t ref_clk, timeout;
	int err;

	if (cfg->window.min != 0U || cfg->window.max == 0U) {
		return -EINVAL;
	} else if (cfg->callback != NULL) {
		return -ENOTSUP;
	}

	err = clock_control_get_rate(config->clk_dev, config->clk_id, &ref_clk);
	if (err < 0) {
		return err;
	}

	timeout = (cfg->window.max / MSEC_PER_SEC) * ref_clk;
	if (timeout / ref_clk != (cfg->window.max / MSEC_PER_SEC)) {
		/* Overflowed */
		return -EINVAL;
	}

	if (timeout < HISILICON_V151_WDT_TIMEOUT_MIN
	    || timeout > HISILICON_V151_WDT_TIMEOUT_MAX) {
		return -EINVAL;
	}

	timeout = timeout >> WDT_LOAD_BITS;

	sys_write32(HISILICON_V151_WDT_KEY, (mem_addr_t) &regs->wdt_lock);
	sys_clear_bit((mem_addr_t) &regs->wdt_cr, WDT_CTRL_ENABLE_BIT);

	sys_write32(timeout, (mem_addr_t) &regs->wdt_load);

	sys_write32(HISILICON_V151_WDT_KEY, (mem_addr_t) &regs->wdt_restart);

	return 0;
}

static int wdt_hisilicon_v151_feed(const struct device *dev, int channel_id)
{
	const struct hisilicon_v151_wdt_config *config = dev->config;
	struct hisilicon_v151_wdt_regs *regs = (void *) config->base;

	ARG_UNUSED(channel_id);
	sys_write32(HISILICON_V151_WDT_KEY, (mem_addr_t) &regs->wdt_restart);

	return 0;
}

static int wdt_hisilicon_v151_init(const struct device *dev)
{
#ifndef CONFIG_WDT_DISABLE_AT_BOOT
	return wdt_hisilicon_v151_disable(dev);
#endif

	return 0;
}

static DEVICE_API(wdt, wdt_hisilicon_v151_driver_api) = {
	.setup = wdt_hisilicon_v151_setup,
	.disable = wdt_hisilicon_v151_disable,
	.install_timeout = wdt_hisilicon_v151_install_timeout,
	.feed = wdt_hisilicon_v151_feed,
};

#define CREATE_HISILICON_V151_WDT_DEV(inst)				\
	static const struct hisilicon_v151_wdt_config			\
	wdt_hisilicon_v151_cfg_##inst = {				\
		.base = DT_INST_REG_ADDR(inst),				\
		.clk_dev = DEVICE_DT_GET(DT_INST_CLOCKS_CTLR(inst)),	\
		.clk_id = (clock_control_subsys_t)DT_INST_PHA_BY_IDX(inst, clocks, 0, clk_id), \
	};								\
									\
	DEVICE_DT_INST_DEFINE(inst, &wdt_hisilicon_v151_init,		\
			      NULL,					\
			      NULL,					\
			      &wdt_hisilicon_v151_cfg_##inst,		\
			      PRE_KERNEL_1,				\
			      CONFIG_KERNEL_INIT_PRIORITY_DEVICE,	\
			      &wdt_hisilicon_v151_driver_api)

DT_INST_FOREACH_STATUS_OKAY(CREATE_HISILICON_V151_WDT_DEV);

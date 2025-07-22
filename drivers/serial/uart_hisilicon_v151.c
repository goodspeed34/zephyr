/*
 * Copyright (c) 2025 Gong Zhile <gongzl@stu.hebust.edu.cn>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT hisilicon_v151_uart

#include <zephyr/spinlock.h>
#include <zephyr/device.h>
#include <zephyr/drivers/syscon.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/sys_io.h>

/* HISILICON V151 UART registers offsets */
#define V151_UART_DATA	0x04
#define V151_UART_FFST	0x44

/* HISILICON V151 UART fifo status registers bits */
#define V151_UART_FFST_TX_FULL	BIT(0)
#define V151_UART_FFST_RX_EMPTY	BIT(3)

struct hisilicon_v151_uart_config {
	mm_reg_t base;
};

static int hisilicon_v151_uart_init(const struct device *dev)
{
	return 0;
}

static int hisilicon_v151_uart_poll_in(const struct device *dev,
				       unsigned char *c )
{
	const struct hisilicon_v151_uart_config *cfg = dev->config;

	if (sys_read16(cfg->base + V151_UART_FFST) & V151_UART_FFST_RX_EMPTY)
		return -1;

	*c = sys_read16(cfg->base + V151_UART_DATA) & 0xff;
	return 0;
}

static void hisilicon_v151_uart_poll_out(const struct device *dev, unsigned char c)
{
	const struct hisilicon_v151_uart_config *cfg = dev->config;

	sys_write16(c, cfg->base + V151_UART_DATA);
}

static DEVICE_API(uart, hisilicon_v151_uart_driver_api) = {
	.poll_in = hisilicon_v151_uart_poll_in,
	.poll_out = hisilicon_v151_uart_poll_out,
};



#define CREATE_HISILICON_V151_UART_DEV(inst)				\
	static const struct hisilicon_v151_uart_config			\
	hisilicon_v151_uart_cfg_##inst = {				\
		.base = DT_INST_REG_ADDR(inst),				\
	};								\
									\
	DEVICE_DT_INST_DEFINE(inst, &hisilicon_v151_uart_init,		\
			      NULL,					\
			      NULL,					\
			      &hisilicon_v151_uart_cfg_##inst,		\
			      PRE_KERNEL_1,				\
			      CONFIG_SERIAL_INIT_PRIORITY,		\
			      &hisilicon_v151_uart_driver_api)

DT_INST_FOREACH_STATUS_OKAY(CREATE_HISILICON_V151_UART_DEV)

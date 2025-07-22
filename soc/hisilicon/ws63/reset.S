/*
 * Copyright (c) 2025 Gong Zhile <gongzl@stu.hebust.edu.cn>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/toolchain.h>

/* exports */
GTEXT(__reset)

/* imports */
GTEXT(__initialize)

SECTION_FUNC(reset, __reset)
	/* Disable interrupts */
	csrw mstatus, x0
	csrw mie, x0

	/* Disable pre-initialized watchdog for now */
	li   t0, 0x40006000
	li   t1, 0x5A5A5A5A
	sw   t1, 0(t0)

	li   t0, 0x40006010
	lw   t1, 0(t0)
	li   t2, ~(1 << 0)
	and  t1, t1, t2
	sw   t1, 0(t0)

	/* Jump to __initialize */
	call __initialize

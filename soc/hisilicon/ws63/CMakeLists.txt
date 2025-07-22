# Copyright (c) 2025 Gong Zhile <gongzl@stu.hebust.edu.cn>
# SPDX-License-Identifier: Apache-2.0

zephyr_sources(
  reset.S
  soc_irq.S
)

set(SOC_LINKER_SCRIPT ${ZEPHYR_BASE}/include/zephyr/arch/riscv/common/linker.ld CACHE INTERNAL "")

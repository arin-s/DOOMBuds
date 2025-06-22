/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#include "cmsis.h"
#include "hal_bootmode.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_norflash.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hwtimer_list.h"
#include "main_entry.h"
#include "pmu.h"
#include "analog.h"
#include "hal_sysfreq.h"

#ifdef DOOMBUDS
#include "serial.h"
#endif

#ifdef RTOS
#include "cmsis_os.h"
#ifdef KERNEL_RTX
#include "rt_Time.h"
#endif
#endif

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

#ifndef FLASH_FILL
#define FLASH_FILL 1
#endif

const static unsigned char bytes[FLASH_FILL] = {
    0x1,
};

// GDB can set a breakpoint on the main function only if it is
// declared as below, when linking with STD libraries.

int MAIN_ENTRY(void) {
	int POSSIBLY_UNUSED ret;
	// Initialise timer
	hwtimer_init();
	// Open DMA
	hal_audma_open();
	hal_gpdma_open();
	// open RX lines
	hal_iomux_set_uart0();
	hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
	hal_trace_rx_reopen();
	// Initialise audio
	analog_open();
	// Enable high performance mode (300mhz)
	ret = pmu_open();
 	ASSERT(ret == 0, "Failed to open pmu");
	pmu_high_performance_mode_enable(true);
	hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
	TRACE(1, "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

	// RIP AND TEARRRRR
#ifdef DOOMBUDS
	doom_main();
#endif

  SAFE_PROGRAM_STOP();
  return 0;
}
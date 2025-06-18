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
#include "analog.h"
#include "cmsis.h"
#include "hal_bootmode.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_norflash.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hwtimer_list.h"
#include "main_entry.h"
#include "pmu.h"

#include "serial.h"

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

#define TIMER_IRQ_PERIOD_MS 2500
#define DELAY_PERIOD_MS 4000

#ifndef FLASH_FILL
#define FLASH_FILL 1
#endif

#ifdef KERNEL_RTX
#define OS_TIME_STR "[%2u/%u]"
#define OS_CUR_TIME , SysTick->VAL, os_time
#else
#define OS_TIME_STR
#define OS_CUR_TIME
#endif

#if defined(MS_TIME)
#define TIME_STR "[%u]" OS_TIME_STR
#define CUR_TIME TICKS_TO_MS(hal_sys_timer_get()) OS_CUR_TIME
#elif defined(RAW_TIME)
#define TIME_STR "[0x%X]" OS_TIME_STR
#define CUR_TIME hal_sys_timer_get() OS_CUR_TIME
#else
#define TIME_STR "[%u/0x%X]" OS_TIME_STR
#define CUR_TIME                                                               \
  TICKS_TO_MS(hal_sys_timer_get()), hal_sys_timer_get() OS_CUR_TIME
#endif

#ifndef NO_TIMER
static HWTIMER_ID hw_timer = NULL;

static void timer_handler(void *param) {
  TRACE(1, TIME_STR " Timer handler: %u", CUR_TIME, (uint32_t)param);
  hwtimer_start(hw_timer, MS_TO_TICKS(TIMER_IRQ_PERIOD_MS));
  TRACE(1, TIME_STR " Start timer %u ms", CUR_TIME, TIMER_IRQ_PERIOD_MS);
}
#endif

const static unsigned char bytes[FLASH_FILL] = {
    0x1,
};

// GDB can set a breakpoint on the main function only if it is
// declared as below, when linking with STD libraries.

int MAIN_ENTRY(void) {
  int POSSIBLY_UNUSED ret;

  hwtimer_init();
  hal_audma_open();
  hal_gpdma_open();
#ifdef DEBUG
#if (DEBUG_PORT == 3)
  hal_iomux_set_analog_i2c();
  hal_iomux_set_uart2();
  hal_trace_open(HAL_TRACE_TRANSPORT_UART2);
#elif (DEBUG_PORT == 2)
  hal_iomux_set_analog_i2c();
  hal_iomux_set_uart1();
  hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
#else
  hal_iomux_set_uart0();
  hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
#endif
#endif

  uint8_t flash_id[HAL_NORFLASH_DEVICE_ID_LEN];
  hal_norflash_get_id(HAL_NORFLASH_ID_0, flash_id, ARRAY_SIZE(flash_id));
  TRACE(3, "FLASH_ID: %02X-%02X-%02X", flash_id[0], flash_id[1], flash_id[2]);
  TRACE(1, TIME_STR " main started: filled@0x%08x", CUR_TIME, (uint32_t)bytes);

#ifndef NO_PMU
  ret = pmu_open();
  ASSERT(ret == 0, "Failed to open pmu");
#endif
  analog_open();
  hal_cmu_simu_pass();

  //------------------------------------------------------

  // Enable high performance mode (300mhz)
  pmu_high_performance_mode_enable(true);
  hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
  TRACE(1, "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

  doom_main();

  while (1) {
    osDelay(DELAY_PERIOD_MS);
    TRACE(1, TIME_STR " Delay %u ms done", CUR_TIME, DELAY_PERIOD_MS);
  }

  SAFE_PROGRAM_STOP();
  return 0;
}
#!/usr/bin/env sh

if make -j "$(nproc)" T=open_source DEBUG=1 NOAPP=1 MCU_HIGH_PERFORMANCE_MODE=1 FLASH_LOW_SPEED=1 PSRAM_LOW_SPEED=0 DOOMBUDS=1 V=1 LARGE_RAM=1 >log.txt 2>&1; then
	echo "build success"
else
	echo "build failed and call log.txt"
	grep "error:" log.txt
fi

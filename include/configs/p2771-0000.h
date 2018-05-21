/*
 * Copyright (c) 2013-2016, NVIDIA CORPORATION.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _P2771_0000_H
#define _P2771_0000_H

#include <linux/sizes.h>

#define DISABLE_DISTRO_BOOTCMD

#include "tegra186-common.h"

/* High-level configuration options */
#define CONFIG_TEGRA_BOARD_STRING	"NVIDIA P2771-0000"

/* I2C */
#define CONFIG_SYS_I2C_TEGRA

/* SD/MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_TEGRA_MMC

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		2
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)
#define CONFIG_ENV_OFFSET_REDUND	(-2*CONFIG_ENV_SIZE)

#define CONFIG_BOOT_RETRY_TIME		-1
#define CONFIG_BOOTCOUNT_ENV
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOT_RETRY_TIME_MIN	10
#define CONFIG_RESET_TO_RETRY
#define CONFIG_CONSOLE_DEV		"ttyS0"

#define BOARD_BASE_BOOTARGS "OS=l4t"

#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_EFI_PARTITION
#define CONFIG_ENV_VARS_UBOOT_CONFIG

#define CONFIG_IMAGE_FORMAT_LEGACY
#define CONFIG_CMD_BOOTI
#define MERLIN_ENV_LEGACY_SETTINGS \
	"image=Image\0" \
	"initrdfile=initrd\0" \
	"loadimage=echo Loading kernel at ${kernel_addr_r}; ext2load mmc 0:${mmcpart} ${kernel_addr_r} /boot/${image}\0" \
	"loadinitrd=echo Loading initrd at ${ramdisk_addr_r}; if ext2load mmc 0:${mmcpart} ${ramdisk_addr_r} /boot/${initrdfile}; then " \
			"setenv initrd_addr ${ramdisk_addr_r}; " \
		"else " \
			"echo No initrd present - skipping; " \
			"setenv initrd_addr -; " \
		"fi;\0" \
	"mmcboot=run loadinitrd; echo Boot args: ${bootargs}; echo Booting from eMMC...; " \
		"booti ${kernel_addr_r} ${initrd_addr} ${fdt_addr}\0"
#define MERLIN_BOOTCMD_FALLBACK \
	"if test \"${secureboot}\" != \"\"; then " \
		"echo FAIL: could not find Linux kernel; " \
	"else " \
		"if run loadimage; then " \
			"run mmcboot; " \
		"else " \
			"echo FAIL: could not find Linux kernel; " \
		"fi; " \
	"fi; "
#define MERLIN_MODULE_SIGNING

#define BOARD_EXTRA_ENV_SETTINGS					\
	"calculated_vars=kernel_addr_r fdt_addr_r scriptaddr pxefile_addr_r " \
		"ramdisk_addr_r\0" \
	"kernel_addr_r_align=00200000\0" \
	"kernel_addr_r_offset=00080000\0" \
	"kernel_addr_r_size=02000000\0" \
	"kernel_addr_r_aliases=loadaddr\0" \
	"fdt_addr_r_align=00200000\0" \
	"fdt_addr_r_offset=00000000\0" \
	"fdt_addr_r_size=00200000\0" \
	"scriptaddr_align=00200000\0" \
	"scriptaddr_offset=00000000\0" \
	"scriptaddr_size=00200000\0" \
	"pxefile_addr_r_align=00200000\0" \
	"pxefile_addr_r_offset=00000000\0" \
	"pxefile_addr_r_size=00200000\0" \
	"ramdisk_addr_r_align=00200000\0" \
	"ramdisk_addr_r_offset=00000000\0" \
	"ramdisk_addr_r_size=02000000\0" \
	"bootcount=0\0" \
	"upgrade_available=1\0" \
	"bootlimit=3\0" \
	"bootretry=" __stringify(CONFIG_BOOT_RETRY_TIME) "\0" \
	"console=" CONFIG_CONSOLE_DEV "\0" \
	"extra_bootargs=quiet loglevel=3\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk0p1\0" \
	"mmcroot_eval=setenv mmcroot /dev/mmcblk0p${mmcpart}\0" \
	"mmcpart_swap=if test ${mmcpart} -eq 2; then setenv mmcpart 1; else setenv mmcpart 2; fi; setenv bootcount 0; saveenv\0" \
	"mmcargs=run mmcroot_eval; " \
		"setenv bootargs ${cbootargs} console=${console},115200n8 " BOARD_BASE_BOOTARGS \
		" root=${mmcroot} ro rootwait " MERLIN_MODULE_SIGNING " ${extra_bootargs}\0"	\
	"fitimage=fitImage\0" \
	"loadfit=ext2load mmc 0:${mmcpart} ${pxefile_addr_r} /boot/${fitimage}\0" \
	"fitboot=echo Boot args: ${bootargs}; echo Booting FIT image...; " \
		"bootm ${pxefile_addr_r}#config@1 ${pxefile_addr_r}#config@1 ${fdt_addr}\0" \
	"altbootcmd=run mmcpart_swap; run bootcmd\0" \
	MERLIN_ENV_LEGACY_SETTINGS

#define BOARD_EXTRA_ENV_SETTINGS \
	"calculated_vars=kernel_addr_r fdt_addr_r scriptaddr pxefile_addr_r " \
		"ramdisk_addr_r\0" \
	"kernel_addr_r_align=00200000\0" \
	"kernel_addr_r_offset=00080000\0" \
	"kernel_addr_r_size=02000000\0" \
	"kernel_addr_r_aliases=loadaddr\0" \
	"fdt_addr_r_align=00200000\0" \
	"fdt_addr_r_offset=00000000\0" \
	"fdt_addr_r_size=00200000\0" \
	"scriptaddr_align=00200000\0" \
	"scriptaddr_offset=00000000\0" \
	"scriptaddr_size=00200000\0" \
	"pxefile_addr_r_align=00200000\0" \
	"pxefile_addr_r_offset=00000000\0" \
	"pxefile_addr_r_size=00200000\0" \
	"ramdisk_addr_r_align=00200000\0" \
	"ramdisk_addr_r_offset=00000000\0" \
	"ramdisk_addr_r_size=02000000\0"

#include "tegra-common-post.h"

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND \
			"mmc dev 0; if mmc rescan; then " \
				"run mmcargs; " \
				"if run loadfit; then " \
					"run fitboot; " \
				"else " \
					MERLIN_BOOTCMD_FALLBACK \
				"fi; " \
			"else " \
				"echo FAIL: mmc rescan failed; " \
			"fi"

/* Crystal is 38.4MHz. clk_m runs at half that rate */
#define COUNTER_FREQUENCY	19200000

#undef CONFIG_NR_DRAM_BANKS
#define CONFIG_NR_DRAM_BANKS	(1024 + 2)

#endif

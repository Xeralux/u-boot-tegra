/*
 * Copright (c) 2016 Sensity Systems, Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _SENSITY_MERLIN_H
#define _SENSITY_MERLIN_H

#include <linux/sizes.h>

#include "tegra210-common.h"

/* High-level configuration options */
#define CONFIG_TEGRA_BOARD_STRING	"Sensity Merlin"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTA

/* I2C */
#define CONFIG_SYS_I2C_TEGRA
#define CONFIG_SYS_VI_I2C_TEGRA

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

/* SPI */
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#define CONFIG_SF_DEFAULT_SPEED		24000000
#define CONFIG_SPI_FLASH_SIZE		(4 << 20)

/* USB2.0 Host support */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_TEGRA
#define CONFIG_USB_STORAGE

/* PCI host support */
#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_CMD_PCI

/* Crystal is 38.4MHz. clk_m runs at half that rate */
#define COUNTER_FREQUENCY	19200000

#if !defined(CONFIG_CPU_BL_IS_CBOOT)
/* Parse the board ID EEPROM and update DT */
#define CONFIG_NV_BOARD_ID_EEPROM
#define CONFIG_NV_BOARD_ID_EEPROM_OF_MAC_ADDRS
#define EEPROM_I2C_BUS		3
#define EEPROM_I2C_ADDRESS	0x50
#endif	/* !CPU_BL_IS_CBOOT */

#ifdef CONFIG_MANUFACTURING
#define CONFIG_BOOT_RETRY_TIME		-1
#define CONFIG_BOOTDELAY		5
#else
#define CONFIG_BOOT_RETRY_TIME		300
#define CONFIG_BOOTDELAY		2
#endif
#define CONFIG_BOOTCOUNT_ENV
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOT_RETRY_TIME_MIN	10
#define CONFIG_RESET_TO_RETRY
#define CONFIG_DEFAULT_FDT_FILE		"tegra210-sensity-merlin.dtb"
#define CONFIG_CONSOLE_DEV		"ttyS0"

#define BOARD_BASE_BOOTARGS "OS=l4t"

#define BOARD_EXTRA_ENV_SETTINGS \
	"bootcount=0\0" \
	"upgrade_available=1\0" \
	"bootlimit=3\0" \
	"bootretry=" __stringify(CONFIG_BOOT_RETRY_TIME) "\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"console=" CONFIG_CONSOLE_DEV "\0" \
	"extra_bootargs=quiet loglevel=3\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk0p1\0" \
	"mmcroot_eval=setenv mmcroot /dev/mmcblk0p${mmcpart}\0" \
	"mmcpart_swap=if test ${mmcpart} -eq 2; then setenv mmcpart 1; else setenv mmcpart 2; fi; setenv bootcount 0; saveenv\0" \
	"mmcargs=run mmcroot_eval; " \
		"setenv bootargs ${cbootargs} console=${console},115200n8 " BOARD_BASE_BOOTARGS " root=${mmcroot} ro rootwait ${extra_bootargs}\0" \
	"image=Image\0" \
	"initrdfile=initrd\0" \
	"loadimage=echo Loading kernel at ${kernel_addr_r}; ext2load mmc 0:${mmcpart} ${kernel_addr_r} /boot/${image}\0" \
	"loadfdt=echo Loading FDT at ${fdt_addr_r}; ext2load mmc 0:${mmcpart} ${fdt_addr_r} /boot/${fdtfile}\0" \
	"loadinitrd=echo Loading initrd at ${ramdisk_addr_r}; if ext2load mmc 0:${mmcpart} ${ramdisk_addr_r} /boot/${initrdfile}; then " \
			"setenv initrd_addr ${ramdisk_addr_r}; " \
		"else " \
			"echo No initrd present - skipping; " \
			"setenv initrd_addr -; " \
		"fi;\0" \
	"defaultfdt=ext2load mmc 0:${mmcpart} ${fdt_addr_r} /boot/" CONFIG_DEFAULT_FDT_FILE "\0" \
	"mmcboot=run loadinitrd; echo Boot args: ${bootargs}; echo Booting from eMMC...; " \
		"if run loadfdt; then booti ${kernel_addr_r} ${initrd_addr} ${fdt_addr_r}; " \
			"else if run defaultfdt; then booti ${kernel_addr_r} ${initrd_addr} ${fdt_addr_r}; " \
				"else echo FAIL: could not load FDT; " \
		"fi; fi;\0" \
	 "altbootcmd=run mmcpart_swap; run bootcmd\0"

#include "tegra-common-usb-gadget.h"
#include "tegra-common-post.h"
/* Clear BOOTENV set by config_distro_bootcmd.h */
#undef BOOTENV
#define BOOTENV

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND \
			"mmc dev 0; if mmc rescan; then " \
				"run mmcargs; if run loadimage; then " \
					"run mmcboot; " \
				"else " \
					"echo FAIL: could not find Linux kernel; " \
				"fi; " \
			"else " \
				"echo FAIL: mmc rescan failed; " \
			"fi"

#endif /* _SENSITY_MERLIN_H */

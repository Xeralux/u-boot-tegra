/*
 * (C) Copyright 2013-2016
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <pca953x.h>
#include <asm/gpio.h>
#include <asm/arch/pinmux.h>
#include "../../nvidia/p2571/max77620_init.h"
#include "pinmux-config-merlin.h"

void pin_mux_mmc(void)
{
	struct udevice *dev;
	uchar val;
	int ret;

	/* Turn on MAX77620 LDO2 to 3.3V for SD card power */
	debug("%s: Set LDO2 for VDDIO_SDMMC_AP power to 3.3V\n", __func__);
	ret = i2c_get_chip_for_busnum(0, MAX77620_I2C_ADDR_7BIT, 1, &dev);
	if (ret) {
		printf("%s: Cannot find MAX77620 I2C chip\n", __func__);
		return;
	}
	/* 0xF2 for 3.3v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	val = 0xF2;
	ret = dm_i2c_write(dev, MAX77620_CNFG1_L2_REG, &val, 1);
	if (ret)
		printf("i2c_write 0 0x3c 0x27 failed: %d\n", ret);

	/* Disable LDO4 discharge */
	ret = dm_i2c_read(dev, MAX77620_CNFG2_L4_REG, &val, 1);
	if (ret) {
		printf("i2c_read 0 0x3c 0x2c failed: %d\n", ret);
	} else {
		val &= ~BIT(1); /* ADE */
		ret = dm_i2c_write(dev, MAX77620_CNFG2_L4_REG, &val, 1);
		if (ret)
			printf("i2c_write 0 0x3c 0x2c failed: %d\n", ret);
	}

	/* Set MBLPD */
	ret = dm_i2c_read(dev, MAX77620_CNFGGLBL1_REG, &val, 1);
	if (ret) {
		printf("i2c_write 0 0x3c 0x00 failed: %d\n", ret);
	} else {
		val |= BIT(6); /* MBLPD */
		ret = dm_i2c_write(dev, MAX77620_CNFGGLBL1_REG, &val, 1);
		if (ret)
			printf("i2c_write 0 0x3c 0x00 failed: %d\n", ret);
	}
}

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_clear_tristate_input_clamping();

	gpio_config_table(merlin_gpio_inits,
			  ARRAY_SIZE(merlin_gpio_inits));

	pinmux_config_pingrp_table(merlin_pingrps,
				   ARRAY_SIZE(merlin_pingrps));

	pinmux_config_drvgrp_table(merlin_drvgrps,
				   ARRAY_SIZE(merlin_drvgrps));
}

void arch_preboot_os(void)
{
	if (!gpio_request(TEGRA_GPIO(H, 6), "status-led")) {
		gpio_set_value(TEGRA_GPIO(H, 6), 0);
		gpio_free(TEGRA_GPIO(H, 6));
	}
	if (!gpio_request(TEGRA_GPIO(K, 6), "socfpga-por")) {
		gpio_set_value(TEGRA_GPIO(K, 6), 1);
		if (!gpio_request(TEGRA_GPIO(V, 6), "spiflash-reset")) {
			gpio_set_value(TEGRA_GPIO(V, 6), 1);
			gpio_free(TEGRA_GPIO(V, 6));
			printf("-- Released SPI flash from reset\n");
		}
		if (!gpio_request(TEGRA_GPIO(H, 7), "spi-muxsel")) {
			gpio_set_value(TEGRA_GPIO(H, 7), 1);
			gpio_free(TEGRA_GPIO(H, 7));
			udelay(10);
			printf("-- Released SPI bus to socfpga\n");
		}
		gpio_free(TEGRA_GPIO(K, 6));
		printf("-- Released socfpga from POR\n");
	}
}

#ifdef CONFIG_PCI_TEGRA
int tegra_pcie_board_init(void)
{
	struct udevice *dev;
	uchar val;
	int ret;

	/* Turn on MAX77620 LDO1 to 1.05V for PEX power */
	debug("%s: Set LDO1 for PEX power to 1.05V\n", __func__);
	ret = i2c_get_chip_for_busnum(0, MAX77620_I2C_ADDR_7BIT, 1, &dev);
	if (ret) {
		printf("%s: Cannot find MAX77620 I2C chip\n", __func__);
		return -1;
	}
	/* 0xCA for 1.05v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	val = 0xCA;
	ret = dm_i2c_write(dev, MAX77620_CNFG1_L1_REG, &val, 1);
	if (ret)
		printf("i2c_write 0 0x3c 0x25 failed: %d\n", ret);

	return 0;
}
#endif /* PCI */

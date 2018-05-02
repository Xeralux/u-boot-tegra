/*
 * Copyright (c) 2016, NVIDIA CORPORATION
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <i2c.h>
#include <asm/gpio.h>
#include "../../nvidia/p2571/max77620_init.h"

int tegra_board_init(void)
{
	struct udevice *dev;
	uchar val;
	int ret;

	/* Turn on MAX77620 LDO3 to 3.3V for SD card power */
	debug("%s: Set LDO3 for VDDIO_SDMMC_AP power to 3.3V\n", __func__);
	ret = i2c_get_chip_for_busnum(0, MAX77620_I2C_ADDR_7BIT, 1, &dev);
	if (ret) {
		printf("%s: Cannot find MAX77620 I2C chip\n", __func__);
		return ret;
	}
	/* 0xF2 for 3.3v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	val = 0xF2;
	ret = dm_i2c_write(dev, MAX77620_CNFG1_L3_REG, &val, 1);
	if (ret) {
		printf("i2c_write 0 0x3c 0x27 failed: %d\n", ret);
		return ret;
	}

	return 0;
}

int tegra_pcie_board_init(void)
{
	struct udevice *dev;
	uchar val;
	int ret;

	/* Turn on MAX77620 LDO7 to 1.05V for PEX power */
	debug("%s: Set LDO7 for PEX power to 1.05V\n", __func__);
	ret = i2c_get_chip_for_busnum(0, MAX77620_I2C_ADDR_7BIT, 1, &dev);
	if (ret) {
		printf("%s: Cannot find MAX77620 I2C chip\n", __func__);
		return -1;
	}
	/* 0xC5 for 1.05v, enabled: bit7:6 = 11 = enable, bit5:0 = voltage */
	val = 0xC5;
	ret = dm_i2c_write(dev, MAX77620_CNFG1_L7_REG, &val, 1);
	if (ret)
		printf("i2c_write 0 0x3c 0x31 failed: %d\n", ret);

	return 0;
}

void arch_preboot_os(void)
{
#ifdef NOT_YET
	/* need to figure out the mapping, since NVIDIA doesn't provide a TEGRA_GPIO() macro for 186 */
	if (!gpio_request(TEGRA_GPIO(B, 5), "status-led")) {
		gpio_set_value(TEGRA_GPIO(B, 5), 0);
		gpio_free(TEGRA_GPIO(B, 5));
	}
#endif
#ifdef NOT_YET_MAYBE_NEVER
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
#endif /* NOT_YET_MAYBE_NEVER */
}

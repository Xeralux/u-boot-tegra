/*
 * merlin-eeprom.c
 *
 * Reads an NVIDIA-style module/board ID EEPROM.
 *
 * Copyright (c) 2018 Verizon Communications, Inc.
 *
 */
#include <stdlib.h>
#include <common.h>
#include <inttypes.h>
#include <asm/byteorder.h>
#include <dm.h>
#include <i2c.h>
#include <fdt_support.h>

#define BOARD_EEPROM_BUS	2
#define BOARD_EEPROM_CHIP	0x50
#define BOARD_EEPROM_ADDRLEN	2

#define LAYOUT_VERSION	1
struct module_eeprom_v1_raw {
	uint16_t version;
	uint16_t length; // no longer used
	uint8_t  reserved_1__[16];
	char     partnumber[22];
	uint8_t  padding[8]; // either 0 or FF
	uint8_t  factory_default_wifi_mac[6]; // little-endian
	uint8_t  factory_default_bt_mac[6];
	uint8_t  factory_default_wifi_alt_mac[6];
	uint8_t  factory_default_ether_mac[6];
	char     asset_id[15]; // string padded with 0 or FF
	uint8_t  reserved_2__[61];
	char     cfgblk_sig[4];
	uint16_t cfgblk_len;
	char     macfmt_tag[2];
	uint16_t macfmt_version;
	uint8_t  vendor_wifi_mac[6];
	uint8_t  vendor_bt_mac[6];
	uint8_t  vendor_ether_mac[6];
	uint8_t  reserved_3__[77];
	uint8_t  crc8;
} __attribute__((packed));

static const uint8_t crc_table[256] =
{
	0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83, 0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
	0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e, 0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
	0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0, 0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
	0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d, 0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
	0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5, 0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
	0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58, 0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
	0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6, 0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
	0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b, 0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
	0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f, 0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
	0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92, 0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
	0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c, 0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
	0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1, 0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
	0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49, 0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
	0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4, 0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
	0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a, 0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
	0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7, 0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35
};

static uint8_t
calc_crc8 (uint8_t *buf, size_t buflen)
{
	uint8_t crc = 0;
	while (buflen-- > 0)
		crc = crc_table[(crc ^ *buf++) & 0xff];
	return crc;

} /* calc_crc8 */

/*
 * eeprom_data_valid
 *
 * Verify CRC and check that the version and tag fields
 * are ones we recognize.
 */
static int
eeprom_data_valid (struct module_eeprom_v1_raw *data)
{
	if (data->crc8 != calc_crc8((uint8_t *) data, 255))
		return 0;
	if (le16_to_cpu(data->version) != LAYOUT_VERSION)
		return 0;
	return 1;

} /* eeprom_data_valid */

/*
 * strings in the EEPROM fields may be padded with either
 * nulls or 0xff
 */
static void
extract_string (char *dst, const char *src, size_t maxlen)
{
	size_t len = maxlen;

	if (src[len-1] == 0xff)
		while (len > 0 && src[len-1] == 0xff)
			len -= 1;
	else if (src[len-1] == '\0')
		while (len > 0 && src[len-1] == '\0')
			len -= 1;
	if (len > 0)
		memcpy(dst, src, len);
	if (len < maxlen)
		memset(dst+len, 0, maxlen-len);

} /* extract_string */

/*
 * cboot_init_late_extra
 *
 * Read an NVIDIA-style device ID EEPROM and update
 * the plugin manager ids based on its contents.
 *
 */
void
cboot_init_late_extra (void *blob)
{
	struct module_eeprom_v1_raw rawdata;
	int ret;
	int pnoffset;
	int offset, parent;
	struct udevice *dev;
	char board_partnum[23];

	ret = i2c_get_chip_for_busnum(BOARD_EEPROM_BUS, BOARD_EEPROM_CHIP, BOARD_EEPROM_ADDRLEN, &dev);
	if (!ret)
		ret = dm_i2c_read(dev, 0, (uint8_t *) &rawdata, sizeof(rawdata));
	if (!ret && eeprom_data_valid(&rawdata)) {
		if (rawdata.partnumber[0] == 0xcc) {
			extract_string(board_partnum, rawdata.partnumber+1, sizeof(rawdata.partnumber)-1);
			pnoffset = 0;
		} else {
			extract_string(board_partnum, rawdata.partnumber, sizeof(rawdata.partnumber));
			pnoffset = 5; // strip off the leading "699-x" prefix
		}
		printf("Board EEPROM id: %s\n", board_partnum + pnoffset);
	} else {
		printf("No board EEPROM, using default\n");
		strcpy(board_partnum, "VRZN-M274-0000-A01 A");
		pnoffset = 0;
	}
	parent = 0;
	offset = fdt_path_offset(blob, "/chosen");
	if (offset < 0) {
		offset = fdt_add_subnode(blob, parent, "/chosen");
		if (offset < 0)
			goto print_warning;
	}

	parent = offset;
	offset = fdt_path_offset(blob, "/chosen/plugin-manager");
	if (offset < 0) {
		offset = fdt_add_subnode(blob, parent, "plugin-manager");
		if (offset < 0)
			goto print_warning;
	}

	parent = offset;
	offset = fdt_path_offset(blob, "/chosen/plugin-manager/ids");
	if (offset < 0) {
		offset = fdt_add_subnode(blob, parent, "ids");
		if (offset < 0)
			goto print_warning;
	}

	fdt_setprop_string(blob, offset, board_partnum + pnoffset, "");

	return;

  print_warning:
	printf("ERROR: could not add board ID to device tree: %s\n",
	       fdt_strerror(offset));

} /* cboot_init_late_extra */

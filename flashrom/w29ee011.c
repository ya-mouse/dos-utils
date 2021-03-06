/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2007 Markus Boas <ryven@ryven.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <string.h>
#include "flash.h"
#include "chipdrivers.h"

int probe_w29ee011(struct flashchip *flash)
{
	chipaddr bios = flash->virtual_memory;
	uint8_t id1, id2;

	if (!chip_to_probe || strcmp(chip_to_probe, "W29EE011")) {
		msg_cdbg("Probing disabled for Winbond W29EE011 because "
			     "the probing sequence puts the AMIC A49LF040A in "
			     "a funky state. Use 'flashrom -c W29EE011' if you "
			     "have a board with this chip.\n");
		return 0;
	}

	/* Issue JEDEC Product ID Entry command */
	chip_writeb(0xAA, bios + 0x5555);
	programmer_delay(10);
	chip_writeb(0x55, bios + 0x2AAA);
	programmer_delay(10);
	chip_writeb(0x80, bios + 0x5555);
	programmer_delay(10);
	chip_writeb(0xAA, bios + 0x5555);
	programmer_delay(10);
	chip_writeb(0x55, bios + 0x2AAA);
	programmer_delay(10);
	chip_writeb(0x60, bios + 0x5555);
	programmer_delay(10);

	/* Read product ID */
	id1 = chip_readb(bios);
	id2 = chip_readb(bios + 0x01);

	/* Issue JEDEC Product ID Exit command */
	chip_writeb(0xAA, bios + 0x5555);
	programmer_delay(10);
	chip_writeb(0x55, bios + 0x2AAA);
	programmer_delay(10);
	chip_writeb(0xF0, bios + 0x5555);
	programmer_delay(10);

	msg_cdbg("%s: id1 0x%02x, id2 0x%02x\n", __func__, id1, id2);

	if (id1 == flash->manufacture_id && id2 == flash->model_id)
		return 1;

	return 0;
}

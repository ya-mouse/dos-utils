/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
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

#include <stdlib.h>
#include <string.h>
#include "flash.h"
#include "programmer.h"

uint32_t io_base_addr;
struct pci_access *pacc;
struct pci_dev *pcidev_dev = NULL;

uint32_t pcidev_validate(struct pci_dev *dev, uint32_t bar,
			 const struct pcidev_status *devs)
{
	int i;
	/* FIXME: 64 bit memory BARs need a 64 bit addr. */
	uint32_t addr;

	for (i = 0; devs[i].device_name != NULL; i++) {
		if (dev->device_id != devs[i].device_id)
			continue;

		/*
		 * Don't use dev->base_addr[x] (as value for 'bar'), won't
		 * work on older libpci.
		 */
		addr = pci_read_long(dev, bar);
		
		msg_pinfo("Found \"%s %s\" (%04x:%04x, BDF %02x:%02x.%x).\n",
		       devs[i].vendor_name, devs[i].device_name,
		       dev->vendor_id, dev->device_id, dev->bus, dev->dev,
		       dev->func);
		msg_pdbg("Requested BAR is %s", (addr & 0x1) ? "IO" : "MEM");
		if (addr & 0x1) {
			/* Mask off IO space indicator and reserved bit. */
			msg_pdbg("\n");
			addr &= ~0x3;
		} else {
			msg_pdbg(", %sbit, %sprefetchable\n",
				 ((addr & 0x6) == 0x0) ? "32" :
				 (((addr & 0x6) == 0x4) ? "64" : "reserved"),
				 (addr & 0x8) ? "" : "not ");
			/* Mask off Mem space indicator, 32/64bit type indicator
			 * and Prefetchable indicator.
			 */
			addr &= ~0xf;
		}

		if (devs[i].status == NT) {
			msg_pinfo("===\nThis PCI device is UNTESTED. Please "
				  "report the 'flashrom -p xxxx' output \n"
				  "to flashrom@flashrom.org if it works "
				  "for you. Please add the name of your\n"
				  "PCI device to the subject. Thank you for "
				  "your help!\n===\n");
		}

		return addr;
	}

	return 0;
}

uint32_t pcidev_init(uint16_t vendor_id, uint32_t bar,
		     const struct pcidev_status *devs)
{
	struct pci_dev *dev;
	struct pci_filter filter;
	char *pcidev_bdf;
	char *msg = NULL;
	int found = 0;
	uint32_t addr = 0, curaddr = 0;

	pacc = pci_alloc();     /* Get the pci_access structure */
	pci_init(pacc);         /* Initialize the PCI library */
	pci_scan_bus(pacc);     /* We want to get the list of devices */
	pci_filter_init(pacc, &filter);

	/* Filter by vendor and also bb:dd.f (if supplied by the user). */
	filter.vendor = vendor_id;
	pcidev_bdf = extract_programmer_param("pci");
	if (pcidev_bdf != NULL) {
		if ((msg = pci_filter_parse_slot(&filter, pcidev_bdf))) {
			msg_perr("Error: %s\n", msg);
			exit(1);
		}
	}
	free(pcidev_bdf);

	for (dev = pacc->devices; dev; dev = dev->next) {
		if (pci_filter_match(&filter, dev)) {
			if ((addr = pcidev_validate(dev, bar, devs)) != 0) {
				curaddr = addr;
				pcidev_dev = dev;
				found++;
			}
		}
	}

	/* Only continue if exactly one supported PCI dev has been found. */
	if (found == 0) {
		msg_perr("Error: No supported PCI device found.\n");
		exit(1);
	} else if (found > 1) {
		msg_perr("Error: Multiple supported PCI devices found. "
			"Use 'flashrom -p xxxx:pci=bb:dd.f' \n"
			"to explicitly select the card with the given BDF "
			"(PCI bus, device, function).\n");
		exit(1);
	}

	return curaddr;
}

void print_supported_pcidevs(const struct pcidev_status *devs)
{
	int i;

	msg_pinfo("PCI devices:\n");
	for (i = 0; devs[i].vendor_name != NULL; i++) {
		msg_pinfo("%s %s [%04x:%04x]%s\n", devs[i].vendor_name,
		       devs[i].device_name, devs[i].vendor_id,
		       devs[i].device_id,
		       (devs[i].status == NT) ? " (untested)" : "");
	}
}

enum pci_write_type {
	pci_write_type_byte,
	pci_write_type_word,
	pci_write_type_long,
};

struct undo_pci_write_data {
	struct pci_dev dev;
	int reg;
	enum pci_write_type type;
	union {
		uint8_t bytedata;
		uint16_t worddata;
		uint32_t longdata;
	};
};

void undo_pci_write(void *p)
{
	struct undo_pci_write_data *data = p;
	msg_pdbg("Restoring PCI config space for %02x:%02x:%01x reg 0x%02x\n",
		 data->dev.bus, data->dev.dev, data->dev.func, data->reg);
	switch (data->type) {
	case pci_write_type_byte:
		pci_write_byte(&data->dev, data->reg, data->bytedata);
		break;
	case pci_write_type_word:
		pci_write_word(&data->dev, data->reg, data->worddata);
		break;
	case pci_write_type_long:
		pci_write_long(&data->dev, data->reg, data->longdata);
		break;
	}
	/* p was allocated in register_undo_pci_write. */
	free(p);
}

#define register_undo_pci_write(a, b, c) 				\
{									\
	struct undo_pci_write_data *undo_pci_write_data;		\
	undo_pci_write_data = malloc(sizeof(struct undo_pci_write_data)); \
	undo_pci_write_data->dev = *a;					\
	undo_pci_write_data->reg = b;					\
	undo_pci_write_data->type = pci_write_type_##c;			\
	undo_pci_write_data->c##data = pci_read_##c(dev, reg);		\
	register_shutdown(undo_pci_write, undo_pci_write_data);		\
}

#define register_undo_pci_write_byte(a, b) register_undo_pci_write(a, b, byte)
#define register_undo_pci_write_word(a, b) register_undo_pci_write(a, b, word)
#define register_undo_pci_write_long(a, b) register_undo_pci_write(a, b, long)

int rpci_write_byte(struct pci_dev *dev, int reg, uint8_t data)
{
	register_undo_pci_write_byte(dev, reg);
	return pci_write_byte(dev, reg, data);
}
 
int rpci_write_word(struct pci_dev *dev, int reg, uint16_t data)
{
	register_undo_pci_write_word(dev, reg);
	return pci_write_word(dev, reg, data);
}
 
int rpci_write_long(struct pci_dev *dev, int reg, uint32_t data)
{
	register_undo_pci_write_long(dev, reg);
	return pci_write_long(dev, reg, data);
}

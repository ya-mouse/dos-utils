/*
 * CMOS/NV-RAM driver for Linux
 *
 * Copyright (C) 1997 Roman Hodek <Roman.Hodek@informatik.uni-erlangen.de>
 * idea by and with help from Richard Jelinek <rj@suse.de>
 * Portions copyright (c) 2001,2002 Sun Microsystems (thockin@sun.com)
 *
 * This driver allows you to access the contents of the non-volatile memory in
 * the mc146818rtc.h real-time clock. This chip is built into all PCs and into
 * many Atari machines. In the former it's called "CMOS-RAM", in the latter
 * "NVRAM" (NV stands for non-volatile).
 *
 * The data are supplied as a (seekable) character device, /dev/nvram. The
 * size of this file is dependent on the controller.  The usual size is 114,
 * the number of freely available bytes in the memory (i.e., not used by the
 * RTC itself).
 *
 * Checksums over the NVRAM contents are managed by this driver. In case of a
 * bad checksum, reads and writes return -EIO. The checksum can be initialized
 * to a sane state either by ioctl(NVRAM_INIT) (clear whole NVRAM) or
 * ioctl(NVRAM_SETCKS) (doesn't change contents, just makes checksum valid
 * again; use with care!)
 *
 * This file also provides some functions for other parts of the kernel that
 * want to access the NVRAM: nvram_{read,write,check_checksum,set_checksum}.
 * Obviously this can be used only if this driver is always configured into
 * the kernel and is not a module. Since the functions are used by some Atari
 * drivers, this is the case on the Atari.
 *
 *
 * 	1.1	Cesar Barros: SMP locking fixes
 * 		added changelog
 * 	1.2	Erik Gilling: Cobalt Networks support
 * 		Tim Hockin: general cleanup, Cobalt support
 * 	1.3	Wim Van Sebroeck: convert PRINT_PROC to seq_file
 */
#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>

#include "asm.mc146818rtc.h"
#include "nvram.h"

#define NVRAM_VERSION	"1.3"

/* RTC in a PC */

/* On PCs, the checksum is built only over bytes 2..31 */
#define PC_CKS_RANGE_START	2
#define PC_CKS_RANGE_END	31
#define PC_CKS_LOC		32
#define NVRAM_BYTES		(256-NVRAM_FIRST_BYTE)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Note that *all* calls to CMOS_READ and CMOS_WRITE must be done with
 * rtc_lock held. Due to the index-port/data-port design of the RTC, we
 * don't want two different things trying to get to it at once. (e.g. the
 * periodic 11 min sync from time.c vs. this driver.)
 */

/*
 * These functions are provided to be called internally or by other parts of
 * the kernel. It's up to the caller to ensure correct checksum before reading
 * or after writing (needs to be done only once).
 *
 * It is worth noting that these functions all access bytes of general
 * purpose memory in the NVRAM - that is to say, they all add the
 * NVRAM_FIRST_BYTE offset.  Pass them offsets into NVRAM as if you did not
 * know about the RTC cruft.
 */

unsigned char __nvram_read_byte(int i)
{
	return CMOS_READ(NVRAM_FIRST_BYTE + i);
}

/* This races nicely with trying to read with checksum checking (nvram_read) */
void __nvram_write_byte(unsigned char c, int i)
{
	CMOS_WRITE(c, NVRAM_FIRST_BYTE + i);
}

/*
 * Machine specific functions
 */

static int __nvram_check_checksum(void)
{
	int i;
	unsigned short sum = 0;
	unsigned short expect;

	for (i = PC_CKS_RANGE_START; i <= PC_CKS_RANGE_END; ++i)
		sum += __nvram_read_byte(i);
	expect = __nvram_read_byte(PC_CKS_LOC)<<8 |
	    __nvram_read_byte(PC_CKS_LOC+1);
	return (sum & 0xffff) == expect;
}

static void __nvram_set_checksum(void)
{
	int i;
	unsigned short sum = 0;

	for (i = PC_CKS_RANGE_START; i <= PC_CKS_RANGE_END; ++i)
		sum += __nvram_read_byte(i);
	__nvram_write_byte(sum >> 8, PC_CKS_LOC);
	__nvram_write_byte(sum & 0xff, PC_CKS_LOC + 1);
}

static int __nvram_is_256()
{
	unsigned char val;

	val = __nvram_read_byte(0x80 + 0x10);
	__nvram_write_byte(0x55, 0x80 + 0x10);
	if (__nvram_read_byte(0x80 + 0x10) != 0x55) {
		__nvram_write_byte(val, 0x80 + 0x10);
		return 0;
	}
	__nvram_write_byte(val, 0x80 + 0x10);
	return 1;
}

/*
 * The are the file operation function for user access to /dev/nvram
 */

static ssize_t __nvram_read(unsigned char *buf)
{
	unsigned i = 0;
	unsigned char *tmp;
	unsigned toread = NVRAM_BYTES;

	if (!__nvram_check_checksum())
		goto checksum_err;

	if (!__nvram_is_256())
		toread -= 128;

	for (tmp = buf; i < toread; ++i, ++tmp)
		*tmp = __nvram_read_byte(i);

	return tmp - buf;

checksum_err:
	return -1;
}

static ssize_t __nvram_write(const unsigned char *buf)
{
	unsigned i = 0;
	const unsigned char *tmp;
	unsigned towrite = NVRAM_BYTES;

	if (!__nvram_is_256())
		towrite -= 128;

	for (tmp = buf; i < towrite; ++i, ++tmp)
		__nvram_write_byte(*tmp, i);

	__nvram_set_checksum();

	return tmp - buf;
}

static int __nvram_ioctl(unsigned int cmd)
{
	int i;

	switch (cmd) {

	case NVRAM_INIT:
		for (i = 0; i < NVRAM_BYTES; ++i)
			__nvram_write_byte(0, i);
		__nvram_set_checksum();

		return 0;

	case NVRAM_SETCKS:
		/* just set checksum, contents unchanged (maybe useful after
		 * checksum garbaged somehow...) */
		__nvram_set_checksum();
		return 0;

	default:
		return -1;
	}
}

static char *floppy_types[] = {
	"none", "5.25'' 360k", "5.25'' 1.2M", "3.5'' 720k", "3.5'' 1.44M",
	"3.5'' 2.88M", "3.5'' 2.88M"
};

static char *gfx_types[] = {
	"EGA, VGA, ... (with BIOS)",
	"CGA (40 cols)",
	"CGA (80 cols)",
	"monochrome",
};

static void __nvram_infos(unsigned char *nvram)
{
	int checksum;
	int type;

	checksum = __nvram_check_checksum();

	printf("Checksum status: %svalid\r\n", checksum ? "" : "not ");

	printf("# floppies     : %d\r\n",
	    (nvram[6] & 1) ? (nvram[6] >> 6) + 1 : 0);
	printf("Floppy 0 type  : ");
	type = nvram[2] >> 4;
	if (type < ARRAY_SIZE(floppy_types))
		printf("%s\r\n", floppy_types[type]);
	else
		printf("%d (unknown)\r\n", type);
	printf("Floppy 1 type  : ");
	type = nvram[2] & 0x0f;
	if (type < ARRAY_SIZE(floppy_types))
		printf("%s\r\n", floppy_types[type]);
	else
		printf("%d (unknown)\r\n", type);

	printf("HD 0 type      : ");
	type = nvram[4] >> 4;
	if (type)
		printf("%02x\r\n", type == 0x0f ? nvram[11] : type);
	else
		printf("none\r\n");

	printf("HD 1 type      : ");
	type = nvram[4] & 0x0f;
	if (type)
		printf("%02x\r\n", type == 0x0f ? nvram[12] : type);
	else
		printf("none\r\n");

	printf("HD type 48 data: %d/%d/%d C/H/S, precomp %d, lz %d\r\n",
	    nvram[18] | (nvram[19] << 8),
	    nvram[20], nvram[25],
	    nvram[21] | (nvram[22] << 8), nvram[23] | (nvram[24] << 8));
	printf("HD type 49 data: %d/%d/%d C/H/S, precomp %d, lz %d\r\n",
	    nvram[39] | (nvram[40] << 8),
	    nvram[41], nvram[46],
	    nvram[42] | (nvram[43] << 8), nvram[44] | (nvram[45] << 8));

	printf("DOS base memory: %d kB\r\n", nvram[7] | (nvram[8] << 8));
	printf("Extended memory: %d kB (configured), %d kB (tested)\r\n",
	    nvram[9] | (nvram[10] << 8), nvram[34] | (nvram[35] << 8));

	printf("Gfx adapter    : %s\r\n",
	    gfx_types[(nvram[6] >> 4) & 3]);

	printf("FPU            : %sinstalled\r\n",
	    (nvram[6] & 2) ? "" : "not ");

	return;
}

int main(int argc, char *argv[])
{
	int ch;
	FILE *fp;
	ssize_t l;
	unsigned char buf[NVRAM_BYTES];

	memset(buf, 0, sizeof(buf));
	while ((ch = getopt(argc, argv, "r:w:cip?")) != EOF)
		switch (ch)
		{
		case 'r':
			fprintf(stderr, "Reading NVRAM\n");
			if ((l = __nvram_read(buf)) < 0)
				goto error;
			if (*optarg == '-') {
				fp = stdout;
				setmode(fileno(fp), O_BINARY);
			} else {
				fp = fopen(optarg, "wb+");
			}
			if (fp == NULL)
				goto file_error;
			fwrite(buf, 1, l, fp);
			if (fp != stdout)
				fclose(fp);
			break;

		case 'w':
			fprintf(stderr, "Writing NVRAM image\n");
			if (*optarg == '-') {
				fp = stdin;
				setmode(fileno(fp), O_BINARY);
			} else {
				fp = fopen(optarg, "rb");
			}
			if (fp == NULL)
				goto file_error;
			fread(buf, 1, NVRAM_BYTES, fp);
			if (fp != stdin)
				fclose(fp);
			if (__nvram_write(buf) < 0)
				goto error;
			break;

		case 'c':
			fprintf(stderr, "Setting checksum\n");
			__nvram_ioctl(NVRAM_SETCKS);
			break;

		case 'i':
			fprintf(stderr, "Initializing NVRAM\n");
			__nvram_ioctl(NVRAM_INIT);
			break;

		case 'p':
			if (__nvram_read(buf) < 0)
				goto error;
			__nvram_infos(buf);
			break;
		case '?':
		default:
			printf("Usage: nvram [-r|-w|-p|-c|-i]\r\n");
			return 1;
	}

	return 0;

error:
	fprintf(stderr, "Operation failed\r\n");
	return 2;

file_error:
	fprintf(stderr, "File operation failed\r\n");
	return 3;
}

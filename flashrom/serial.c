/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2009 Urja Rannikko <urjaman@gmail.com>
 * Copyright (C) 2009,2010 Carl-Daniel Hailfinger
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <inttypes.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif
#include "flash.h"
#include "programmer.h"

fdtype sp_fd;

void __attribute__((noreturn)) sp_die(char *msg)
{
	perror(msg);
	exit(1);
}

#ifndef _WIN32
struct baudentry {
	int flag;
	unsigned int baud;
};

/* I'd like if the C preprocessor could have directives in macros */
#define BAUDENTRY(baud) { B##baud, baud },
static const struct baudentry sp_baudtable[] = {
	BAUDENTRY(9600)
	BAUDENTRY(19200)
	BAUDENTRY(38400)
	BAUDENTRY(57600)
	BAUDENTRY(115200)
#ifdef B230400
	BAUDENTRY(230400)
#endif
#ifdef B460800
	BAUDENTRY(460800)
#endif
#ifdef B500000
	BAUDENTRY(500000)
#endif
#ifdef B576000
	BAUDENTRY(576000)
#endif
#ifdef B921600
	BAUDENTRY(921600)
#endif
#ifdef B1000000
	BAUDENTRY(1000000)
#endif
#ifdef B1152000
	BAUDENTRY(1152000)
#endif
#ifdef B1500000
	BAUDENTRY(1500000)
#endif
#ifdef B2000000
	BAUDENTRY(2000000)
#endif
#ifdef B2500000
	BAUDENTRY(2500000)
#endif
#ifdef B3000000
	BAUDENTRY(3000000)
#endif
#ifdef B3500000
	BAUDENTRY(3500000)
#endif
#ifdef B4000000
	BAUDENTRY(4000000)
#endif
	{0, 0}			/* Terminator */
};
#endif

fdtype sp_openserport(char *dev, unsigned int baud)
{
#ifdef _WIN32
	HANDLE fd;
	char *dev2 = dev;
	if ((strlen(dev) > 3) && (tolower((unsigned char)dev[0]) == 'c') &&
	    (tolower((unsigned char)dev[1]) == 'o') &&
	    (tolower((unsigned char)dev[2]) == 'm')) {
		dev2 = malloc(strlen(dev) + 5);
		strcpy(dev2, "\\\\.\\");
		strcpy(dev2 + 4, dev);
	}
	fd = CreateFile(dev2, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, 0, NULL);
	if (dev2 != dev)
		free(dev2);
	if (fd == INVALID_HANDLE_VALUE) {
		sp_die("Error: cannot open serial port");
	}
	DCB dcb;
	if (!GetCommState(fd, &dcb)) {
		sp_die("Error: Could not fetch serial port configuration");
	}
	switch (baud) {
		case 9600: dcb.BaudRate = CBR_9600; break;
		case 19200: dcb.BaudRate = CBR_19200; break;
		case 38400: dcb.BaudRate = CBR_38400; break;
		case 57600: dcb.BaudRate = CBR_57600; break;
		case 115200: dcb.BaudRate = CBR_115200; break;
		default: sp_die("Error: Could not set baud rate");
	}
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	if (!SetCommState(fd, &dcb)) {
		sp_die("Error: Could not change serial port configuration");
	}
	return fd;
#else
	struct termios options;
	int fd, i;
	fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0)
		sp_die("Error: cannot open serial port");
	fcntl(fd, F_SETFL, 0);
	tcgetattr(fd, &options);
	for (i = 0;; i++) {
		if (sp_baudtable[i].baud == 0) {
			close(fd);
			msg_perr("Error: cannot configure for baudrate %d\n",
				 baud);
			exit(1);
		}
		if (sp_baudtable[i].baud == baud) {
			cfsetispeed(&options, sp_baudtable[i].flag);
			cfsetospeed(&options, sp_baudtable[i].flag);
			break;
		}
	}
	options.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
	options.c_cflag |= (CS8 | CLOCAL | CREAD);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | IGNCR | INLCR);
	options.c_oflag &= ~OPOST;
	tcsetattr(fd, TCSANOW, &options);
	return fd;
#endif
}

void sp_flush_incoming(void)
{
#ifdef _WIN32
	PurgeComm(sp_fd, PURGE_RXCLEAR);
#else
	tcflush(sp_fd, TCIFLUSH);
#endif
	return;
}

int serialport_shutdown(void)
{
#ifdef _WIN32
	CloseHandle(sp_fd);
#else
	close(sp_fd);
#endif
	return 0;
}

int serialport_write(unsigned char *buf, unsigned int writecnt)
{
	long tmp = 0;

	while (writecnt > 0) {
#ifdef _WIN32
		WriteFile(sp_fd, buf, writecnt, &tmp, NULL);
#else
		tmp = write(sp_fd, buf, writecnt);
#endif
		if (tmp == -1) {
			msg_perr("Serial port write error!\n");
			return 1;
		}
		if (!tmp)
			msg_pdbg("Empty write\n");
		writecnt -= tmp; 
		buf += tmp;
	}

	return 0;
}

int serialport_read(unsigned char *buf, unsigned int readcnt)
{
	long tmp = 0;

	while (readcnt > 0) {
#ifdef _WIN32
		ReadFile(sp_fd, buf, readcnt, &tmp, NULL);
#else
		tmp = read(sp_fd, buf, readcnt);
#endif
		if (tmp == -1) {
			msg_perr("Serial port read error!\n");
			return 1;
		}
		if (!tmp)
			msg_pdbg("Empty read\n");
		readcnt -= tmp;
		buf += tmp;
	}

	return 0;
}

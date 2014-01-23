/*
 * RTC related functions
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "linux.mc146818rtc.h"

/* For two digit years assume time is always after that */
#define CMOS_YEARS_OFFS 2000

static unsigned bcd2bin(unsigned char val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

static unsigned char bin2bcd(unsigned val)
{
	return ((val / 10) << 4) + val % 10;
}

/* REP NOP (PAUSE) is a good thing to insert into busy-wait loops. */
static inline void rep_nop(void)
{
	__asm
	{
		nop rep;
	}
}

static inline void outb(unsigned char b, unsigned short addr)
{
	__asm
	{
		mov dx, addr
		mov al, b
		out dx, al
	}
}

static inline unsigned char inb(unsigned short addr)
{
	unsigned char b = 0;
	__asm
	{
		mov dx, addr
		in al, dx
		mov b, al
	}
	return b;
}

/*
 * In order to set the CMOS clock precisely, set_rtc_mmss has to be
 * called 500 ms after the second nowtime has started, because when
 * nowtime is written into the registers of the CMOS clock, it will
 * jump to the next second precisely 500 ms later. Check the Motorola
 * MC146818A or Dallas DS12887 data sheet for details.
 *
 * BUG: This routine does not handle hour overflow properly; it just
 *      sets the minutes. Usually you'll only notice that after reboot!
 */
int mach_set_rtc_mmss(unsigned long nowtime)
{
	int real_seconds, real_minutes, cmos_minutes;
	unsigned char save_control, save_freq_select;
	int retval = 0;

	 /* tell the clock it's being set */
	save_control = CMOS_READ(RTC_CONTROL);
	CMOS_WRITE((save_control|RTC_SET), RTC_CONTROL);

	/* stop and reset prescaler */
	save_freq_select = CMOS_READ(RTC_FREQ_SELECT);
	CMOS_WRITE((save_freq_select|RTC_DIV_RESET2), RTC_FREQ_SELECT);

	cmos_minutes = CMOS_READ(RTC_MINUTES);
	if (!(save_control & RTC_DM_BINARY) || RTC_ALWAYS_BCD)
		cmos_minutes = bcd2bin(cmos_minutes);

	/*
	 * since we're only adjusting minutes and seconds,
	 * don't interfere with hour overflow. This avoids
	 * messing with unknown time zones but requires your
	 * RTC not to be off by more than 15 minutes
	 */
	real_seconds = nowtime % 60;
	real_minutes = nowtime / 60;
	/* correct for half hour time zone */
	if (((abs(real_minutes - cmos_minutes) + 15)/30) & 1)
		real_minutes += 30;
	real_minutes %= 60;

	if (abs(real_minutes - cmos_minutes) < 30) {
		if (!(save_control & RTC_DM_BINARY) || RTC_ALWAYS_BCD) {
			real_seconds = bin2bcd(real_seconds);
			real_minutes = bin2bcd(real_minutes);
		}
		CMOS_WRITE(real_seconds, RTC_SECONDS);
		CMOS_WRITE(real_minutes, RTC_MINUTES);
	} else {
		printf(
		       "set_rtc_mmss: can't update from %d to %d\n",
		       cmos_minutes, real_minutes);
		retval = -1;
	}

	/* The following flags have to be released exactly in this order,
	 * otherwise the DS12887 (popular MC146818A clone with integrated
	 * battery and quartz) will not reset the oscillator and will not
	 * update precisely 500 ms later. You won't find this mentioned in
	 * the Dallas Semiconductor data sheets, but who believes data
	 * sheets anyway ...                           -- Markus Kuhn
	 */
	CMOS_WRITE(save_control, RTC_CONTROL);
	CMOS_WRITE(save_freq_select, RTC_FREQ_SELECT);

	return retval;
}

unsigned long mach_get_cmos_time(void)
{
	struct tm tm;
	unsigned int status, century = 0;

	/*
	 * If UIP is clear, then we have >= 244 microseconds before
	 * RTC registers will be updated.  Spec sheet says that this
	 * is the reliable way to read RTC - registers. If UIP is set
	 * then the register access might be invalid.
	 */
	while ((CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP))
		rep_nop();

	tm.tm_sec = CMOS_READ(RTC_SECONDS);
	tm.tm_min = CMOS_READ(RTC_MINUTES);
	tm.tm_hour = CMOS_READ(RTC_HOURS);
	tm.tm_mday = CMOS_READ(RTC_DAY_OF_MONTH);
	tm.tm_mon = CMOS_READ(RTC_MONTH);
	tm.tm_year = CMOS_READ(RTC_YEAR);

#ifdef CONFIG_ACPI
	if (acpi_gbl_FADT.header.revision >= FADT2_REVISION_ID &&
	    acpi_gbl_FADT.century)
		century = CMOS_READ(acpi_gbl_FADT.century);
#endif

	status = CMOS_READ(RTC_CONTROL);

	if (RTC_ALWAYS_BCD || !(status & RTC_DM_BINARY)) {
		tm.tm_sec = bcd2bin(tm.tm_sec);
		tm.tm_min = bcd2bin(tm.tm_min);
		tm.tm_hour = bcd2bin(tm.tm_hour);
		tm.tm_mday = bcd2bin(tm.tm_mday);
		tm.tm_mon = bcd2bin(tm.tm_mon);
		tm.tm_year = bcd2bin(tm.tm_year);
	}

	if (century) {
		century = bcd2bin(century);
		tm.tm_year += century * 100;
		printf("Extended CMOS year: %d\n", century * 100);
	} else
		tm.tm_year += CMOS_YEARS_OFFS;

	return mktime(&tm);
}

/* Routines for accessing the CMOS RAM/RTC. */
unsigned char rtc_cmos_read(unsigned char addr)
{
	unsigned char val;

	outb(addr < 0x80 ? addr : addr - 0x80, RTC_PORT(addr < 0x80 ? 0 : 2));
	val = inb(RTC_PORT(addr < 0x80 ? 1 : 3));

	return val;
}

void rtc_cmos_write(unsigned char val, unsigned char addr)
{
	outb(addr < 0x80 ? addr : addr - 0x80, RTC_PORT(addr < 0x80 ? 0 : 2));
	outb(val, RTC_PORT(addr < 0x80 ? 1 : 3));
}

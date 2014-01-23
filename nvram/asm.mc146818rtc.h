/*
 * Machine dependent access functions for RTC registers.
 */
#ifndef _ASM_X86_MC146818RTC_H
#define _ASM_X86_MC146818RTC_H

#define RTC_PORT(x)	(0x70 + (x))
#define RTC_ALWAYS_BCD	1	/* RTC operates in binary mode */

/*
 * All of these below must be called with interrupts off, preempt
 * disabled, etc.
 */

/*
 * The yet supported machines all access the RTC index register via
 * an ISA port access but the way to access the date register differs ...
 */
#define CMOS_READ(addr) rtc_cmos_read(addr)
#define CMOS_WRITE(val, addr) rtc_cmos_write(val, addr)
unsigned char rtc_cmos_read(unsigned char addr);
void rtc_cmos_write(unsigned char val, unsigned char addr);

extern int mach_set_rtc_mmss(unsigned long nowtime);
extern unsigned long mach_get_cmos_time(void);

#define RTC_IRQ 8

#endif /* _ASM_X86_MC146818RTC_H */

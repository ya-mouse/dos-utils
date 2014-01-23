#ifndef _LINUX_NVRAM_H
#define _LINUX_NVRAM_H

/* /dev/nvram ioctls */
#define NVRAM_INIT	0x40 /* initialize NVRAM and set checksum */
#define NVRAM_SETCKS	0x41 /* recalculate checksum */

/* for all current systems, this is where NVRAM starts */
#define NVRAM_FIRST_BYTE    14
/* all these functions expect an NVRAM offset, not an absolute */
#define NVRAM_OFFSET(x)   ((x)-NVRAM_FIRST_BYTE)

#endif  /* _LINUX_NVRAM_H */

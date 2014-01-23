# Makefile for zlib
# OpenWatcom flat model
# Last updated: 28-Dec-2005

# To use, do "wmake -f watcom_f.mak"

CC       = wcc386
LINKER   = wcl386
CFLAGS   = -zq -mf -3r -fp3 -s -bt=dos -oilrtfm -fr=nul -wx -fpi

.extensions:
.extensions: .exe .o .c

.c.o:
        $(CC) $(CFLAGS) $[@

all: nvram.exe

nvram.exe: nvram.o rtc.o .AUTODEPEND
	$(LINKER) -ldos32a -fe=$^@ nvram.o rtc.o
	upx $^@

clean: .SYMBOLIC
          rm -f *.o
          rm -f nvram.exe
          @echo Cleaning done

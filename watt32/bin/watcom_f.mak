# Makefile for zlib
# OpenWatcom flat model
# Last updated: 28-Dec-2005

# To use, do "wmake -f watcom_f.mak"

CC       = wcc386
LINKER   = wcl386
CFLAGS   = -zq -mf -3r -fp3 -s -bt=dos -oilrtfm -fr=nul -wx -fpi -I../inc -I../../zlib
LIB = ../lib/wattcpwf.lib ../../zlib/zlib_f.lib

.extensions:
.extensions: .exe .lib .o .c

.c.o:
        $(CC) $(CFLAGS) $[@

all: tcpinfo.exe

tcpinfo.exe: tcpinfo.o ../lib/wattcpwf.lib
	$(LINKER) -ldos32a -fe=$^@ tcpinfo.o $(LIB)
	upx $^@

clean: .SYMBOLIC
          rm -f *.o
          rm -f tcpinfo.exe
          @echo Cleaning done

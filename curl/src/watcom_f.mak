# Makefile for zlib
# OpenWatcom flat model
# Last updated: 28-Dec-2005

# To use, do "wmake -f watcom_f.mak"
WATCOM   = ../../watt32
ZLIB     = ../../zlib

CC       = wcc386
AR       = wlib
LINKER   = wcl386
CFLAGS   = -zq -mf -3r -fp3 -s -bt=dos -oilrtfm -fr=nul -wx -fpi
CFLAGS  += -I../include -I$(WATCOM)/inc -I$(ZLIB) -I../lib
LIB      = ../lib/libcurl.lib $(WATCOM)/lib/wattcpwf.lib $(ZLIB)/zlib_f.lib

CFLAGS  += -DNDEBUG -DBUILDING_LIBCURL -DHAVE_CONFIG_H -DMSDOS -D__WATCOM_LFN__
CFLAGS  += -DHAVE_LIBZ -DHAVE_ZLIB_H -DUSE_IPV6 -DENABLE_IPV6 -D_GETOPT_H

top_srcdir = ..

!include Makefile.inc

CSOURCES = $(CURL_CFILES) $(CURLX_ONES)
OBJS     = $(CSOURCES:.c=.o)

.extensions:
.extensions: .exe .lib .o .c

.c.o: .AUTODEPEND
        $(CC) $(CFLAGS) $[@

all: curl.exe

curl.exe: $(OBJS) $(WATCOM)/lib/wattcpwf.lib
	$(LINKER) -ldos32a -fe=$^@ $(OBJS) $(LIB)
	upx $^@

clean: .SYMBOLIC
          rm -f *.o curl.exe
          @echo Cleaning done

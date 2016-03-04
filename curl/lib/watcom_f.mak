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
CFLAGS  += -I../include -I$(WATCOM)/inc -I$(ZLIB) -I.
LIB      = $(WATCOM)/lib/wattcpwf.lib $(ZLIB)/zlib_f.lib

CFLAGS  += -DNDEBUG -DBUILDING_LIBCURL -DHAVE_CONFIG_H -DMSDOS
CFLAGS  += -DHAVE_LIBZ -DHAVE_ZLIB_H -DUSE_IPV6 -D_GETOPT_H -D__WATCOM_LFN__

!include Makefile.inc

OBJS     = $(CSOURCES:.c=.o)
OBJS     = $(OBJS:vtls/=)

.extensions:
.extensions: .exe .lib .o .c

.c : vtls

.c.o: .AUTODEPEND
        $(CC) $(CFLAGS) $[@

all: libcurl.lib

libcurl.lib: $(OBJS) wlib.arg
	$(AR) -q -b -c $^@ @$]@

wlib.arg: $(__MAKEFILES__)
	%create $^@
	@for %f in ($(OBJS)) do @%append $^@ +- %f

clean: .SYMBOLIC
	rm -f libcurl.lib wlib.arg
	rm -f *.o wget.exe
	@echo Cleaning done

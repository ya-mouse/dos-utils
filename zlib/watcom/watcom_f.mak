# Makefile for zlib
# OpenWatcom flat model
# Last updated: 28-Dec-2005

# To use, do "wmake -f watcom_f.mak"

C_SOURCE =  adler32.c  compress.c crc32.c   deflate.c    &
            gzio.c     infback.c  inffast.c inflate.c    &
            inftrees.c trees.c    uncompr.c zutil.c

OBJS =      adler32.o  compress.o crc32.o   deflate.o    &
            gzio.o     infback.o  inffast.o inflate.o    &
            inftrees.o trees.o    uncompr.o zutil.o

CC       = wcc386
LINKER   = wcl386
CFLAGS   = -DNO_FSEEKO -zq -mf -3r -fp3 -s -bt=dos -oilrtfm -fr=nul -wx
ZLIB_LIB = zlib_f.lib

.extensions:
.extensions: .exe .o .c .lib

.c.o:
        $(CC) $(CFLAGS) $[@

all: $(ZLIB_LIB) example.exe minigzip.exe

$(ZLIB_LIB): $(OBJS)
	wlib -b -c $(ZLIB_LIB) -+adler32.o  -+compress.o -+crc32.o
        wlib -b -c $(ZLIB_LIB) -+deflate.o  -+gzio.o     -+infback.o
        wlib -b -c $(ZLIB_LIB) -+inffast.o  -+inflate.o  -+inftrees.o
        wlib -b -c $(ZLIB_LIB) -+trees.o    -+uncompr.o  -+zutil.o

example.exe: $(ZLIB_LIB) example.o
	$(LINKER) -ldos32a -fe=example.exe example.o $(ZLIB_LIB)

minigzip.exe: $(ZLIB_LIB) minigzip.o
	$(LINKER) -ldos32a -fe=minigzip.exe minigzip.o $(ZLIB_LIB)

clean: .SYMBOLIC
          rm -f *.o
          rm -f *.exe
          rm -f $(ZLIB_LIB)
          @echo Cleaning done

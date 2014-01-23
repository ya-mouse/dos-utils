# Makefile for pciutil
# OpenWatcom flat model

# To use, do "wmake -f watcom_f.mak"

ZLIB     = ../zlib

CC       = wcc386
LINKER   = wcl386
CFLAGS   = -zq -mf -3r -fp3 -s -bt=dos -oilrtfm -fr=nul -wx -fpi
LFLAGS   = lib/pci_f.lib $(ZLIB)/zlib_f.lib

.extensions:
.extensions: .exe .o .c

.c.o:
        $(CC) $(CFLAGS) $[@

all: lspci.exe pci.idz compat/getopt.o .SYMBOLIC
	mkdir -p include/pci
	for %h in (config.h header.h internal.h pci.h sysdep.h types.h) do &
		@ln -sf ../../lib/%h include/pci/%h

pci.idz: pci.ids
	gzip -9 -c pci.ids > $^@

compat/getopt.o: compat/getopt.c
	$(CC) $(CFLAGS) -fo=$^@ compat/getopt.c

lspci.exe: lspci.o common.o ls-vpd.o ls-caps.o ls-ecaps.o ls-kernel.o ls-tree.o ls-map.o
	$(LINKER) $(LFLAGS) -ldos32a -fe=$^@ lspci.o common.o ls-vpd.o ls-caps.o ls-ecaps.o ls-kernel.o ls-tree.o ls-map.o

clean: .SYMBOLIC
          rm -f *.o lib/*.o pci.idz lib/wlib.arg lib/pci_f.lib nul.err
          rm -f compat/*.o lspci.exe
          rm -rf include
          @echo Cleaning done

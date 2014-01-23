# Makefile for pciutil
# OpenWatcom flat model

# To use, do "wmake -f watcom_f.mak"

CHIP_OBJS = jedec.o stm50flw0x0x.o w39.o w29ee011.o &
            sst28sf040.o m29f400bt.o 82802ab.o pm49fl00x.o &
            sst49lfxxxc.o sst_fwhub.o flashchips.o &
            spi.o spi25.o sharplhf00l04.o

LIB_OBJS = layout.o util.o

CLI_OBJS = flashrom.o cli_classic.o cli_output.o print.o

PROGRAMMER_OBJS = udelay.o programmer.o

CFLAGS =

NEED_PCI = no

# Always enable internal/onboard support for now.
CONFIG_INTERNAL = yes

# RayeR SPIPGM hardware support
CONFIG_RAYER_SPI = yes

# Always enable 3Com NICs for now.
CONFIG_NIC3COM = yes

# Enable NVIDIA graphics cards. Note: write and erase do not work properly.
CONFIG_GFXNVIDIA = no

# Always enable SiI SATA controllers for now.
CONFIG_SATASII = no

# Highpoint (HPT) ATA/RAID controller support.
# IMPORTANT: This code is not yet working!
CONFIG_ATAHPT = no

# Always enable FT2232 SPI dongles for now.
CONFIG_FT2232_SPI = no

# Always enable dummy tracing for now.
CONFIG_DUMMY = no

# Always enable Dr. Kaiser for now.
CONFIG_DRKAISER = yes

# Always enable Realtek NICs for now.
CONFIG_NICREALTEK = yes

# Disable National Semiconductor NICs until support is complete and tested.
CONFIG_NICNATSEMI = no

# Always enable SPI on Intel NICs for now.
CONFIG_NICINTEL_SPI = yes

# Always enable SPI on OGP cards for now.
CONFIG_OGP_SPI = yes

# Always enable Bus Pirate SPI for now.
CONFIG_BUSPIRATE_SPI = no

# Disable Dediprog SF100 until support is complete and tested.
CONFIG_DEDIPROG = no

# Disable wiki printing by default. It is only useful if you have wiki access.
CONFIG_PRINT_WIKI = no

# Bitbanging SPI infrastructure, default off unless needed.
!ifeq CONFIG_RAYER_SPI yes
CONFIG_BITBANG_SPI = yes
!else
!ifeq CONFIG_INTERNAL yes
!error 'asd'
CONFIG_BITBANG_SPI = yes
!else
!ifeq CONFIG_NICINTEL_SPI yes
CONFIG_BITBANG_SPI = yes
!else
!ifeq CONFIG_OGP_SPI yes
CONFIG_BITBANG_SPI = yes
!else
CONFIG_BITBANG_SPI = no
!endif
!endif
!endif
!endif

!ifeq CONFIG_INTERNAL yes
CFLAGS += -DCONFIG_INTERNAL=1
PROGRAMMER_OBJS += processor_enable.o chipset_enable.o board_enable.o cbtable.o dmi.o internal.o
# FIXME: The PROGRAMMER_OBJS below should only be included on x86.
PROGRAMMER_OBJS += it87spi.o ichspi.o sb600spi.o wbsio_spi.o mcp6x_spi.o
NEED_PCI = yes
!endif

!ifeq CONFIG_RAYER_SPI yes
CFLAGS += -DCONFIG_RAYER_SPI=1
PROGRAMMER_OBJS += rayer_spi.o
# Actually, NEED_PCI is wrong. NEED_IOPORT_ACCESS would be more correct.
NEED_PCI = yes
!endif

!ifeq CONFIG_BITBANG_SPI yes
CFLAGS += -DCONFIG_BITBANG_SPI=1
PROGRAMMER_OBJS += bitbang_spi.o
!endif

!ifeq CONFIG_NIC3COM yes
CFLAGS += -DCONFIG_NIC3COM=1
PROGRAMMER_OBJS += nic3com.o
NEED_PCI = yes
!endif

!ifeq CONFIG_GFXNVIDIA yes
CFLAGS += -DCONFIG_GFXNVIDIA=1
PROGRAMMER_OBJS += gfxnvidia.o
NEED_PCI = yes
!endif

!ifeq CONFIG_SATASII yes
CFLAGS += -DCONFIG_SATASII=1
PROGRAMMER_OBJS += satasii.o
NEED_PCI = yes
!endif

!ifeq CONFIG_ATAHPT yes
CFLAGS += -DCONFIG_ATAHPT=1
PROGRAMMER_OBJS += atahpt.o
NEED_PCI = yes
!endif

!ifeq CONFIG_DRKAISER yes
CFLAGS += -DCONFIG_DRKAISER=1
PROGRAMMER_OBJS += drkaiser.o
NEED_PCI = yes
!endif

!ifeq CONFIG_NICREALTEK yes
CFLAGS += -DCONFIG_NICREALTEK=1
PROGRAMMER_OBJS += nicrealtek.o
NEED_PCI = yes
!endif

!ifeq CONFIG_NICNATSEMI yes
CFLAGS += -DCONFIG_NICNATSEMI=1
PROGRAMMER_OBJS += nicnatsemi.o
NEED_PCI = yes
!endif

!ifeq CONFIG_NICINTEL_SPI yes
CFLAGS += -DCONFIG_NICINTEL_SPI=1
PROGRAMMER_OBJS += nicintel_spi.o
NEED_PCI = yes
!endif

!ifeq CONFIG_OGP_SPI yes
CFLAGS += -DCONFIG_OGP_SPI=1
PROGRAMMER_OBJS += ogp_spi.o
NEED_PCI = yes
!endif

!ifeq CONFIG_BUSPIRATE_SPI yes
CFLAGS += -DCONFIG_BUSPIRATE_SPI=1
PROGRAMMER_OBJS += buspirate_spi.o
NEED_SERIAL = yes
!endif

!ifeq NEED_PCI yes
CFLAGS += -DNEED_PCI=1
PROGRAMMER_OBJS += pcidev.o physmap.o hwaccess.o
!endif

!ifeq CONFIG_PRINT_WIKI yes
CFLAGS += -DCONFIG_PRINT_WIKI=1
CLI_OBJS += print_wiki.o
!endif

OBJS     = $(CHIP_OBJS) $(LIB_OBJS) $(CLI_OBJS) $(PROGRAMMER_OBJS)

SVNVERSION = 1352
# $(shell LC_ALL=C svnversion -cn . 2>/dev/null | sed -e "s/.*://" -e "s/\([0-9]*\).*/\1/" | grep "[0-9]" || LC_ALL=C svn info . 2>/dev/null | awk '/^Revision:/ {print $$2 }' | grep "[0-9]" || LC_ALL=C git svn info . 2>/dev/null | awk '/^Revision:/ {print $$2 }' | grep "[0-9]" || echo unknown)

RELEASE = 0.9.3
VERSION = $(RELEASE)-r$(SVNVERSION)
RELEASENAME = $(VERSION)

SVNDEF = -DFLASHROM_VERSION="\"$(VERSION)\""

ZLIB     = ../zlib
PCI      = ../pciutils

CC       = wcc386
LINKER   = wcl386
CFLAGS  += -zq -mf -3r -fp3 -s -bt=dos -oilrtfm -fr=nul -wx -fpi
CFLAGS  += -I$(ZLIB) -I$(PCI)/include -I$(PCI)/compat $(SVNDEF)
LFLAGS   = $(PCI)/compat/getopt.o $(PCI)/lib/pci_f.lib $(ZLIB)/zlib_f.lib

.extensions:
.extensions: .exe .o .c

.c.o:
        $(CC) $(CFLAGS) $[@

all: flashrom.exe

flashrom.exe: $(OBJS)
	$(LINKER) $(LFLAGS) -ldos32a -fe=$^@ $(OBJS)
	upx $^@

clean: .SYMBOLIC
          rm -f *.o
          rm -f lspci.exe
          @echo Cleaning done

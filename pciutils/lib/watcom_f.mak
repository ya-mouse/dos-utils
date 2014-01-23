# OpenWatcom flat model

# To use, do "wmake -f watcom_f.mak"

C_SOURCE =  init.c access.c generic.c dump.c names.c filter.c &
            names-hash.c names-parse.c names-net.c names-cache.c &
            params.c caps.c i386-ports.c

OBJS     = $(C_SOURCE:.c=.o)

ZLIB     = ../../zlib

CC       = wcc386
AR       = wlib
LINKER   = wcl386
CFLAGS   = -DNO_FSEEKO -zq -mf -3r -fp3 -s -bt=dos -oilrtfm -fr=nul -wx
CFLAGS  += -I$(ZLIB)
PCI_LIB  = pci_f.lib

.extensions:
.extensions: .exe .o .c .lib

.c.o:
        $(CC) $(CFLAGS) $[@

all: $(PCI_LIB)

$(PCI_LIB): $(OBJS) wlib.arg
	$(AR) -q -b -c $^@ @$]@

wlib.arg: $(__MAKEFILES__)
	%create $^@
	@for %f in ($(OBJS)) do @%append $^@ +- %f

clean: .SYMBOLIC
          rm -f *.o
          rm -f $(PCI_LIB)
          @echo Cleaning done

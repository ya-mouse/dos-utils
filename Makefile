SUBDIRS=ow zlib watt32 curl nvram getargs pciutils flashrom

BUILD := [ ! -e "$(CURDIR)/ow/watcomenv" ] || { . $(CURDIR)/ow/watcomenv && wmake -u -f watcom_f.mak $(MAKECMDGOALS); }

all: $(SUBDIRS)

ow:
	$(MAKE) -C $@ $(MAKECMDGOALS)

ow/watcomenv:
	$(MAKE) -C ow watcomenv

zlib: ow/watcomenv
	cd $@; [ ! -e "$(CURDIR)/ow/watcomenv" ] || { . $(CURDIR)/ow/watcomenv && wmake -u -f watcom/watcom_f.mak $(MAKECMDGOALS); }

nvram: ow/watcomenv
	@cd $@; $(BUILD)

getargs: ow/watcomenv
	@cd $@; $(BUILD)

watt32: zlib
	@cd $@/src; $(BUILD)
	@cd $@/bin; $(BUILD)

curl: watt32
	@cd $@/lib; $(BUILD)
	@cd $@/src; $(BUILD)

pciutils: zlib
	@cd $@/lib; $(BUILD)
	@cd $@; $(BUILD)

flashrom: pciutils
	@cd $@; $(BUILD)

clean: $(SUBDIRS:%ow=)
	+make -C ow clean

install:

distclean: clean

.PHONY: all install clean distclean $(SUBDIRS)

# Makefile for zlib
# OpenWatcom flat model
# Last updated: 28-Dec-2005

# To use, do "wmake -f watcom_f.mak"

all: getargs.com

getargs.com: getargs.c .AUTODEPEND
	wcl -3 -osx -mt getargs.c

clean: .SYMBOLIC
	rm -f *.o
	rm -f getargs.com
	@echo Cleaning done

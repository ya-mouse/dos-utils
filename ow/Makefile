all: watcomenv

linux: open-watcom-c-linux-1.9 dos32a.exe
	rm -rf linux; unzip open-watcom-c-linux-1.9 -d $@
	cd linux && \
	cp -f $(CURDIR)/dos32a.exe binl/dos32a.exe && \
	rm -rf binnt binp binw docs h/nt h/os2* h/win lib286/os2 lib286/win \
	       lib386/linux lib386/netware lib386/nt lib386/os2 lib386/rdos lib386/win \
	       nlm rdos samples src setup.inf watcom.ico && \
	file -i binl/* | fgrep application/x-executable | sed 's,:.*,,' | xargs chmod +x

watcomenv: linux
	echo "#!/bin/sh" > $@
	echo "export WATCOM=$$PWD/linux" >> $@
	echo 'export PATH=$$PATH:$$WATCOM/binl:$$WATCOM/lib386:$$WATCOM/lib386/dos' >> $@
	echo 'export INCLUDE=$$WATCOM/h' >> $@
	echo 'export EDPATH=$$WATCOM/eddat' >> $@
	echo 'export WIPFC=$$WATCOM/wipfc' >> $@

dos32a.exe: dos32a-912-bin.zip
	unzip -p $^ binw/dos32a.exe > dos32a.tmp
	xdelta3 -d -s dos32a.tmp dos32a.exe.xdelta
	rm -f dos32a.tmp

open-watcom-c-linux-1.9:
	wget -c -O $@ ftp://ftp.heanet.ie/pub/openwatcom/open-watcom-c-linux-1.9

dos32a-912-bin.zip:
	wget -c -O $@ http://download.narechk.net/dos32a-912-bin.zip

clean:
	rm -f watcomenv dos32a.exe
	rm -rf linux

distclean: clean
	rm -f open-watcom-c-linux-1.9

.PHONY: all clean distclean

#
# NB! THIS MAKEFILE WAS AUTOMATICALLY GENERATED FROM MAKEFILE.ALL.
#     DO NOT EDIT.
#
# Makefile for Waterloo TCP/IP kernel
#

ASM_SOURCE = asmpkt.asm chksum0.asm cpumodel.asm

CORE_SOURCE = bsdname.c  btree.c    chksum.c   country.c  crc.c      &
              echo.c     fortify.c  getopt.c   gettod.c   highc.c    &
              ip4_frag.c ip4_in.c   ip4_out.c  ip6_in.c   ip6_out.c  &
              language.c lookup.c   loopback.c misc.c     netback.c  &
              oldstuff.c pc_cbrk.c  pcarp.c    pcbootp.c  powerpak.c &
              pcbuf.c    pcconfig.c pcdbug.c   pcdhcp.c   pcicmp.c   &
              pcicmp6.c  pcintr.c   pcmulti.c  pcping.c   pcpkt.c    &
              pcpkt32.c  pcqueue.c  pcrarp.c   pcrecv.c   pcsed.c    &
              pcstat.c   pctcp.c    ports.c    ppp.c      pppoe.c    &
              qmsg.c     rs232.c    settod.c   sock_dbu.c sock_in.c  &
              sock_ini.c sock_io.c  sock_prn.c sock_scn.c sock_sel.c &
              split.c    strings.c  tcp_fsm.c  tftp.c     timer.c    &
              udp_dom.c  udp_rev.c  version.c  wdpmi.c    x32vm.c    &
              pcsarp.c   idna.c     punycode.c tcp_md5.c  dynip.c    &
              winpcap.c  winmisc.c  packet32.c

BSD_SOURCE = accept.c   adr2asc.c  asc2adr.c  bind.c     bsddbug.c  &
             close.c    connect.c  fcntl.c    fsext.c    get_ai.c   &
             get_ni.c   get_ip.c   geteth.c   gethost.c  gethost6.c &
             getname.c  getnet.c   getprot.c  getput.c   getserv.c  &
             get_xbyr.c ioctl.c    linkaddr.c listen.c   netaddr.c  &
             neterr.c   nettime.c  nsapaddr.c poll.c     presaddr.c &
             printk.c   receive.c  select.c   shutdown.c signal.c   &
             socket.c   sockopt.c  stream.c   syslog.c   syslog2.c  &
             transmit.c

BIND_SOURCE = res_comp.c res_data.c res_debu.c res_init.c res_loc.c &
              res_mkqu.c res_quer.c res_send.c

C_SOURCE = $(CORE_SOURCE) $(BSD_SOURCE) $(BIND_SOURCE)


OBJS = &
       $(OBJDIR)/chksum0.o  $(OBJDIR)/cpumodel.o  &
       $(OBJDIR)/accept.o   $(OBJDIR)/adr2asc.o   &
       $(OBJDIR)/asc2adr.o  $(OBJDIR)/bind.o      &
       $(OBJDIR)/bsddbug.o  $(OBJDIR)/bsdname.o   &
       $(OBJDIR)/btree.o    $(OBJDIR)/chksum.o    &
       $(OBJDIR)/close.o    $(OBJDIR)/connect.o   &
       $(OBJDIR)/country.o  $(OBJDIR)/crc.o       &
       $(OBJDIR)/echo.o     $(OBJDIR)/fcntl.o     &
       $(OBJDIR)/fortify.o  $(OBJDIR)/get_ai.o    &
       $(OBJDIR)/get_ni.o   $(OBJDIR)/geteth.o    &
       $(OBJDIR)/gethost.o  $(OBJDIR)/gethost6.o  &
       $(OBJDIR)/getname.o  $(OBJDIR)/getnet.o    &
       $(OBJDIR)/getopt.o   $(OBJDIR)/getprot.o   &
       $(OBJDIR)/getput.o   $(OBJDIR)/getserv.o   &
       $(OBJDIR)/gettod.o   $(OBJDIR)/ioctl.o     &
       $(OBJDIR)/ip4_frag.o $(OBJDIR)/ip4_in.o    &
       $(OBJDIR)/ip4_out.o  $(OBJDIR)/ip6_in.o    &
       $(OBJDIR)/ip6_out.o  $(OBJDIR)/language.o  &
       $(OBJDIR)/linkaddr.o $(OBJDIR)/listen.o    &
       $(OBJDIR)/lookup.o   $(OBJDIR)/loopback.o  &
       $(OBJDIR)/misc.o     $(OBJDIR)/netaddr.o   &
       $(OBJDIR)/netback.o  $(OBJDIR)/neterr.o    &
       $(OBJDIR)/nettime.o  $(OBJDIR)/nsapaddr.o  &
       $(OBJDIR)/oldstuff.o $(OBJDIR)/pc_cbrk.o   &
       $(OBJDIR)/pcarp.o    $(OBJDIR)/pcbootp.o   &
       $(OBJDIR)/powerpak.o $(OBJDIR)/pcbuf.o     &
       $(OBJDIR)/pcconfig.o $(OBJDIR)/pcdbug.o    &
       $(OBJDIR)/pcdhcp.o   $(OBJDIR)/pcicmp.o    &
       $(OBJDIR)/pcicmp6.o  $(OBJDIR)/pcintr.o    &
       $(OBJDIR)/pcmulti.o  $(OBJDIR)/pcping.o    &
       $(OBJDIR)/pcpkt.o    $(OBJDIR)/pcpkt32.o   &
       $(OBJDIR)/pcqueue.o  $(OBJDIR)/pcrarp.o    &
       $(OBJDIR)/pcrecv.o   $(OBJDIR)/pcsed.o     &
       $(OBJDIR)/pcstat.o   $(OBJDIR)/pctcp.o     &
       $(OBJDIR)/poll.o     $(OBJDIR)/ports.o     &
       $(OBJDIR)/ppp.o      $(OBJDIR)/pppoe.o     &
       $(OBJDIR)/presaddr.o $(OBJDIR)/printk.o    &
       $(OBJDIR)/qmsg.o     $(OBJDIR)/receive.o   &
       $(OBJDIR)/res_comp.o $(OBJDIR)/res_data.o  &
       $(OBJDIR)/res_debu.o $(OBJDIR)/res_init.o  &
       $(OBJDIR)/res_loc.o  $(OBJDIR)/res_mkqu.o  &
       $(OBJDIR)/res_quer.o $(OBJDIR)/res_send.o  &
       $(OBJDIR)/select.o   $(OBJDIR)/settod.o    &
       $(OBJDIR)/shutdown.o $(OBJDIR)/signal.o    &
       $(OBJDIR)/sock_dbu.o $(OBJDIR)/sock_in.o   &
       $(OBJDIR)/sock_ini.o $(OBJDIR)/sock_io.o   &
       $(OBJDIR)/sock_prn.o $(OBJDIR)/sock_scn.o  &
       $(OBJDIR)/sock_sel.o $(OBJDIR)/socket.o    &
       $(OBJDIR)/sockopt.o  $(OBJDIR)/split.o     &
       $(OBJDIR)/stream.o   $(OBJDIR)/strings.o   &
       $(OBJDIR)/syslog.o   $(OBJDIR)/syslog2.o   &
       $(OBJDIR)/tcp_fsm.o  $(OBJDIR)/get_xbyr.o  &
       $(OBJDIR)/tftp.o     $(OBJDIR)/timer.o     &
       $(OBJDIR)/transmit.o $(OBJDIR)/udp_dom.o   &
       $(OBJDIR)/udp_rev.o  $(OBJDIR)/version.o   &
       $(OBJDIR)/fsext.o    $(OBJDIR)/wdpmi.o     &
       $(OBJDIR)/x32vm.o    $(OBJDIR)/rs232.o     &
       $(OBJDIR)/get_ip.o   $(OBJDIR)/pcsarp.o    &
       $(OBJDIR)/idna.o     $(OBJDIR)/punycode.o  &
       $(OBJDIR)/tcp_md5.o  $(OBJDIR)/dynip.o     &
       $(OBJDIR)/winpcap.o  $(OBJDIR)/winmisc.o   &
       $(OBJDIR)/packet32.o


O = obj

PKT_STUB = pkt_stub.h

########################################################################


.EXTENSIONS:
.EXTENSIONS: .exe .o .c .lib .asm .l

CC      = wcc386
CFLAGS   = -zq -mf -3r -fp3 -s -bt=dos -oilrtfm -fr=nul -wx -I../../zlib
AFLAGS  = -bt=dos -3r -dDOSX
TARGET  = ../lib/wattcpwf.lib
OBJDIR  = watcom/flat
MAKEFIL = watcom_f.mak


LIBARG  = $(OBJDIR)/wlib.arg
LINKARG = $(OBJDIR)/wlink.arg
C_ARGS  = $(OBJDIR)/cc.arg

AFLAGS += -zq -fr=nul -w3 -d1
CFLAGS += -zq -fr=nul -wx -fpi -DWATT32_BUILD -I../inc

#
# WCC386-flags used:
#   -m{s,l,f} memory model; small, large or flat
#   -3s       optimise for 386, stack call convention
#   -3r       optimise for 386, register calls
#   -s        no stack checking
#   -zq       quiet compiling
#   -d3       generate full debug info
#   -fpi      inline math + emulation
#   -fr       write errors to file (and stdout)
#   -bt=dos   target system - dos
#   -zm       place each function in separate segment
#   -oilrtfm  optimization flags
#     i:      expand intrinsics
#     l:      loop optimisations
#     r:      reorder instructions
#     t:      favor execution time
#     f:      always use stack frames
#     m:      generate inline code for math functions
#
#  This should make the smallest code on a 386
#    -oahkrs -s -em -zp1 -3r -fp3
#
#  WCC-flags for small/large model:
#    -zc      place const data into the code segment
#    -os      optimization flags
#      s:     favour code size over execution time
#

AS = wasm

all: $(PKT_STUB) $(C_ARGS) $(TARGET)

$(TARGET): $(OBJS) $(LIBARG)
      wlib -q -b -c $^@ @$(LIBARG)

-!include "watcom/watt32.dep"

$(OBJDIR)/asmpkt.o:   asmpkt.asm
$(OBJDIR)/chksum0.o:  chksum0.asm
$(OBJDIR)/cpumodel.o: cpumodel.asm

.ERASE
.c{$(OBJDIR)}.o: .AUTODEPEND
          *$(CC) $[@ @$(C_ARGS) -fo=$@

.ERASE
.asm{$(OBJDIR)}.o:
          *$(AS) $[@ $(AFLAGS) -fo=$@

$(C_ARGS)::
          @mkdir -p $(OBJDIR)
          %create $^@
          %append $^@ $(CFLAGS)

#language.c: language.l
#          flex -8 -o$@ $[@

clean: .SYMBOLIC
          - @rm -f $(OBJDIR)/*.o
          - @rm -f $(TARGET)
          - @rm -f $(C_ARGS)
          - @rm -f $(LIBARG)
          @echo Cleaning done

$(LIBARG): $(__MAKEFILES__)
          %create $^@
          for %f in ($(OBJS) $(ZLIB_OBJS)) do %append $^@ +- %f


########################################################################


########################################################################


########################################################################

doxygen:
	doxygen doxyfile


$(OBJDIR)/pcpkt.o: asmpkt.nas

$(PKT_STUB): asmpkt.nas
        nasm -f bin -l asmpkt.lst -o asmpkt.bin asmpkt.nas
        ../util/bin2c asmpkt.bin > $@

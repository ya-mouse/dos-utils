/******************************************************************************

  TCPINFO - display configuration info to the screen

  Copyright (C) 1991, University of Waterloo
  portions Copyright (C) 1990, National Center for Supercomputer Applications

  This program is free software; you can redistribute it and/or modify
  it, but you may not sell it.

  This program is distributed in the hope that it will be useful,
  but without any warranty; without even the implied warranty of
  merchantability or fitness for a particular purpose.

      Erick Engelke                   or via E-Mail
      Faculty of Engineering
      University of Waterloo          Erick@development.watstar.uwaterloo.ca
      200 University Ave.,
      Waterloo, Ont., Canada
      N2L 3G1

  Numerous additions by G. Vanem <giva@bgnett.no>
  1995-2003

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <io.h>
#include <netdb.h>

#include "../src/wattcp.h"
#include "../src/pcconfig.h"
#include "../src/pcdbug.h"
#include "../src/pctcp.h"
#include "../src/pcarp.h"
#include "../src/pcsed.h"
#include "../src/pcpkt.h"
#include "../src/pcbootp.h"
#include "../src/pcdhcp.h"
#include "../src/pcdbug.h"
#include "../src/bsdname.h"
#include "../src/udp_dom.h"
#include "../src/sock_ini.h"
#include "../src/printk.h"
#include "../src/misc.h"
#include "../src/timer.h"
#include "../src/netaddr.h"
#include "../src/get_xby.h"
#include "../src/ip6_in.h"
#include "../src/ip6_out.h"
#include "../src/pcicmp6.h"

#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif

/*
 * Fix trouble with multiple defined symbols,
 */
#define perror(str)   perror_s(str)
#define strerror(x)   strerror_s(x)

#if defined(USE_BSD_API)
static void BSD_api_stats (void);
#endif

char buffer [512], buf2 [512];

char *EtherAddr (const void *eth_adr)
{
  static char buf[20];
  const char *eth = (const char*)eth_adr;

  if (_pktdevclass == PDCLASS_ARCNET)
       sprintf (buf, "%02X", eth[0] & 255);
  else sprintf (buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                eth[0] & 255, eth[1] & 255, eth[2] & 255,
                eth[3] & 255, eth[4] & 255, eth[5] & 255);
  return (buf);
}

#if defined(USE_DEBUG)
void dump_gateways (void)
{
  struct gate_entry gw_list [10];
  int    i, num_gw = _arp_list_gateways (&gw_list[0], DIM(gw_list));

  if (num_gw == 0)
  {
    puts ("NONE\n");
    return;
  }

  puts ("GATEWAY'S IP     SUBNET           SUBNET MASK");

  for (i = 0 ; i < num_gw; i++)
  {
    char gate[20], snet[20], mask[20];

    strcpy (gate, _inet_ntoa(NULL,gw_list[i].gate_ip));
    strcpy (snet, _inet_ntoa(NULL,gw_list[i].subnet));
    strcpy (mask, _inet_ntoa(NULL,gw_list[i].mask));

    printf ("                 : %-15s  %-15s  %-15s\n", gate,
            gw_list[i].subnet ? snet : "DEFAULT",
            gw_list[i].mask   ? mask : "DEFAULT");
  }
  puts ("");
}

void dump_arp_cache (void)
{
  struct arp_entry arp_list [100];
  int    i, num_arp = _arp_list_cache (arp_list, DIM(arp_list));

  if (num_arp == 0)
  {
    puts ("                   <None>           <None>\n");
    return;
  }

  for (i = 0; i < num_arp; i++)
  {
    char type[20];

    printf ("                 : %-15s  %s  ",
            _inet_ntoa(NULL, arp_list[i].ip),
            EtherAddr ((void*)&arp_list[i].hardware));

    if (arp_list[i].flags & ARP_FIXED)
       strcpy (type, "fixed  ");
    else if (arp_list[i].flags & ARP_DYNAMIC)
       strcpy (type, "dynamic");
    else if (arp_list[i].flags & ARP_PENDING)
       strcpy (type, "pending");
    else
       sprintf (type, "0x%02X?  ", arp_list[i].flags);
    printf ("%s ", type);

    if (arp_list[i].expiry)
    {
      long timeout = arp_list[i].expiry - set_timeout(0);
      if (timeout < 0)
           puts ("< 0 ms");
      else printf ("%ld ms\n", timeout);
    }
    else
      puts ("N/A");
  }
  puts ("");
}
#endif

const char *pkt_class_name (WORD class)
{
  return (class == PDCLASS_ETHER  ? "Ethernet" :
          class == PDCLASS_TOKEN  ? "Token"    :
          class == PDCLASS_ARCNET ? "Arcnet"   :
          class == PDCLASS_SLIP   ? "SLIP"     :
          class == PDCLASS_AX25   ? "AX25"     :
          class == PDCLASS_FDDI   ? "FDDI"     :
          class == PDCLASS_PPP    ? "PPP"      :
          "Unknown");
}

const char *pkt_driver_ver (void)
{
  static char buf[20];
  BYTE   ver[4];

  if (!pkt_get_drvr_ver((DWORD*)&ver))
     return ("unknown");

#if defined(WIN32)  /* NPF.SYS ver e.g. 3.1.0.22 */
  sprintf (buf, "%d.%d.%d.%d", ver[3], ver[2], ver[1], ver[0]);
#else
  sprintf (buf, "%d.%02d", ver[1], ver[0]);
#endif
  return (buf);
}

const char *pkt_api_ver (void)
{
  static char buf[10];
  DWORD  ver;

  if (pkt_get_api_ver(&ver))
  {
    if (ver >= 255)  /* assumed to be MSB.LSB (NDIS version under Win32) */
         sprintf (buf, "%d.%02d", (BYTE)(ver >> 8), (int)(ver & 255));
    else sprintf (buf, "%lu", ver);
    return (buf);
  }
  return strcpy (buf, "?");
}

void dump_pktdrvr_info (void)
{
#if !defined(WIN32)
  printf ("PKTDRVR Name     : ");
  if (!_pkt_inf)
  {
    puts ("<NO DRIVER>");
    return;
  }

  printf ("%s, version %s, API %s, intr 0x%02X\n"
          "        Class    : %s, level %u",
          _pktdrvrname, pkt_driver_ver(), pkt_api_ver(), pkt_get_vector(),
          pkt_class_name(_pktdevclass), _pktdevlevel);

  if (_pktdevlevel >= 5)  /* high-performance driver */
  {
    struct PktParameters params;

    memset (&params, 0, sizeof(params));
    pkt_get_params (&params);
    printf (", addr-len %d, MTU %d, MC avail %d\n"
            "        RX bufs  : %d, TX bufs %d, EOI intr %d",
            params.addr_len, params.mtu, params.multicast_avail,
            params.rcv_bufs + 1, params.xmt_bufs + 1, params.int_num);
  }
#else
  printf ("WinPcap Name     : ");
  if (!_pkt_inf)
  {
    puts ("<NO DRIVER>");
    return;
  }

  printf ("%s\n                 : %s\n"
          "                 : NPF.SYS %s, NDIS %s, Class %s",
          _pktdrvrname, _pktdrvr_descr, "??" /*PacketGetDriverVersion() */, pkt_api_ver(),
          pkt_class_name(_pktdevclass));
#endif

#if defined(USE_MULTICAST) || defined(USE_IPV6)
  if (!_pktserial)
     printf (", RX mode %d", pkt_get_rcv_mode());
#endif

  printf ("\n        Address  : %s\n\n", EtherAddr(&_eth_addr));
}

void usage (void)
{
  printf (" Usage: tcpinfo [-d]\n"
          "   -d sets debug-mode (WATTCP.CFG \"debug_on = 1\")\n");
  exit (0);
}

void check_setup (void)
{
  const char *watt_env  = getenv ("WATTCP.CFG");
  const char *watt_root = getenv ("WATT_ROOT");
  char  cfg_name [_MAX_PATH];

  if (!tcp_config_name(cfg_name,sizeof(cfg_name)))
       printf ("Failed to find WATTCP.CFG; %s\n", strerror(errno));
  else printf ("Reading configuration file `%s'\n", cfg_name);

  if (!watt_root)
     puts ("\7Warning: %WATT_ROOT% not set.\n"
           "This is okay if you're not planning to program with Watt-32");
  else
  {
    char path [_MAX_PATH];

    strcpy (path, watt_root);
    strcat (path, "\\.");
    if (access(path,0) != 0)
       printf ("\7Warning: WATT_ROOT incorrectly set: `%s'.\n", watt_root);
  }

  if (watt_root && strchr(watt_root,'/'))
     puts ("\7Warning: %WATT_ROOT% contains forward ('/') slashes.\n"
           "This may break some Makefiles.");

  if (watt_env && strchr(watt_env,'/'))
     puts ("\7Warning: %WATTCP.CFG% contains forward slashes ('/').\n"
           "This will break non-djgpp Watt-32 programs.");
}

int main (int argc, char **argv)
{
  int  i;
  char name[50];

  if (argc >= 2 && (!strncmp(argv[1],"-?",2) || !strncmp(argv[1],"/?",2)))
     usage();

  if (argc >= 2 && (!stricmp(argv[1],"-d") || !stricmp(argv[1],"/d")))
     debug_on = 2;

  check_setup();

  survive_eth   = 1;  /* needed to not exit if pkt_eth_init() fails */
  survive_bootp = 1;  /* ditto for BOOTP */
  survive_dhcp  = 1;  /* ditto for DHCP/RARP */
  survive_rarp  = 1;

  if (debug_on)
     dbug_init();

  watt_sock_init (0, 0);

#if defined(USE_IPV6) && !defined(WIN32)
  _ip6_pkt_init();
#endif

  if (debug_on >= 2)
     puts ("");

  dump_pktdrvr_info();

  if (multihomes)
       printf ("IP Addresses     : %s - %s\n",
               _inet_ntoa (buf2,  _gethostid()),
               _inet_ntoa (buffer,_gethostid()+multihomes));
  else printf ("IP Address       : %s\n", _inet_ntoa (buffer,_gethostid()));

  printf ("Network Mask     : %s\n\n", _inet_ntoa(NULL,sin_mask));

#if defined(USE_DEBUG)
  printf ("Gateways         : ");
  dump_gateways();

  printf ("ARP Cache        : IP Address       MAC Address        Type    Timeout\n");
  dump_arp_cache();
#endif

  printf ("Host name        : ");
  if (gethostname(name,sizeof(name)) == 0)
       puts (name);
  else puts ("<NONE>");

  printf ("Domain name      : ");
  if (getdomainname(name,sizeof(name)) == 0)
       puts (name);
  else puts ("<NONE>");

#if 0
  printf ("Cookieserver%c    : ", (last_cookie < 2) ? ' ' : 's');
  if (!last_cookie)
     puts ("NONE DEFINED");

  for (i = 0 ; i < last_cookie; i++)
  {
    if (i)
       printf ("                 : ");
    printf ("%s\n", _inet_ntoa(NULL, cookies[i]));
  }
  puts("");
#endif

  printf ("Nameserver%c      : ", (last_nameserver < 2) ? ' ' : 's');
  if (!last_nameserver)
     puts ("NONE DEFINED\n");

  for (i = 0 ; i < last_nameserver; i++)
  {
    unsigned timeout = dns_timeout ? dns_timeout :
                       (unsigned)sock_delay << 2;
    if (i)
       printf ("                 : ");
    printf ("%s  Timeout %us\n",
            _inet_ntoa(NULL, def_nameservers[i]), timeout);
  }

  if (_bootp_on)
  {
    puts("");
    printf ("BOOTP            : Enabled and %s\n", _gethostid() ? "SUCCEEDED" : "FAILED");
    printf ("BOOTP Server     : %s\n",           _inet_ntoa(NULL,_bootp_host));
    printf ("BOOTP Timeout    : %i seconds\n\n", _bootp_timeout);
  }
#if defined(USE_DHCP)
  else if (_dhcp_on)
  {
    puts("");
    printf ("DHCP             : Enabled and %s\n", _gethostid() ? "SUCCEEDED" : "FAILED");
    printf ("DHCP Server      : %s\n",           _inet_ntoa(NULL,DHCP_get_server()));
  }
#endif

#if defined(USE_IPV6)
  puts ("");
  printf ("IPv6-address     : %s\n", _inet6_ntoa(&in6addr_my_ip));
  printf ("6-to-4 gateway   : %s\n", _inet_ntoa(NULL,icmp6_6to4_gateway));
#endif

  puts ("");
  printf ("Max Seg Size,MSS : %u bytes\n",   _mss);
  printf ("Max Transmit,MTU : %u bytes\n\n", _mtu);

  printf ("TCP timers       : ");
#if defined(USE_UDP_ONLY)
  puts ("TCP not compiled in");
#else
  printf ("Sock delay %us, Inactivity %us, Keep-alive %ds/%ds\n",
          sock_delay, sock_inactive, tcp_keep_idle, tcp_keep_intvl);
  printf ("                 : Open %ums, Close %ums, RST time %ums\n",
          tcp_OPEN_TO, tcp_CLOSE_TO, tcp_RST_TIME);
  printf ("                 : RTO base %ums, RTO add %ums, Retrans %ums\n\n",
          tcp_RTO_BASE, tcp_RTO_ADD, tcp_RETRAN_TIME);
#endif

  printf ("_tcp_Socket size : %u bytes\n",   (unsigned)sizeof(_tcp_Socket));
  printf ("_udp_Socket size : %u bytes\n\n", (unsigned)sizeof(_udp_Socket));

#if defined(USE_BSD_API)
  BSD_api_stats();
#endif

  printf ("Version info     : %s\n", wattcpVersion());
  printf ("Capabilities     : %s\n", wattcpCapabilities());
  return (0);
}


#if defined(USE_BSD_API)
/*
 * Returning -1 means uninitialsed
 */
static int NumHostsEntries (void)
{
  int num = -1;

  while (gethostent())
        num++;
  if (num > -1)
      num++;
  return (num);
}

#if defined(USE_IPV6)
static int NumHosts6Entries (void)
{
  int num = -1;

  while (gethostent6())
        num++;
  if (num > -1)
      num++;
  return (num);
}
#endif

static int NumServEntries (void)
{
  int num = -1;

  while (getservent())
        num++;
  if (num > -1)
      num++;
  return (num);
}

static int NumProtoEntries (void)
{
  int num = -1;

  while (getprotoent())
        num++;
  if (num > -1)
      num++;
  return (num);
}

static int NumNetEntries (void)
{
  int num = -1;

  while (getnetent())
        num++;
  if (num > -1)
      num++;
  return (num);
}

static void BSD_api_stats (void)
{
  const char *name;

  printf ("HOSTS file       : ");
  name = GetHostsFile();
  if (name)
       printf ("%-40s %4d entries\n", name, NumHostsEntries());
  else puts ("<NONE>");

#if defined(USE_IPV6)
  printf ("HOSTS6 file      : ");
  name = GetHosts6File();
  if (name)
       printf ("%-40s %4d entries\n", name, NumHosts6Entries());
  else puts ("<NONE>");
#endif

  printf ("SERVICES file    : ");
  name = GetServFile();
  if (name)
       printf ("%-40s %4d entries\n", name, NumServEntries());
  else puts ("<NONE>");

  printf ("PROTOCOL file    : ");
  name = GetProtoFile();
  if (name)
       printf ("%-40s %4d entries\n", name, NumProtoEntries());
  else puts ("<NONE>");

  printf ("NETWORKS file    : ");
  name = GetNetFile();
  if (name)
       printf ("%-40s %4d entries\n", name, NumNetEntries());
  else puts ("<NONE>");

  printf ("ETHERS file      : ");
  name = GetEthersFile();
  if (name)
       printf ("%-40s %4d entries\n", name, NumEtherEntries());
  else puts ("<NONE>");
  puts ("");
}
#endif  /* USE_BSD_API */


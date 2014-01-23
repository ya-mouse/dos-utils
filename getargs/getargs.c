/* -*- c -*- ------------------------------------------------------------- *
 *   
 *   Copyright 2003 Murali Krishnan Ganapathy - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
 *   Bostom MA 02111-1307, USA; either version 2 of the License, or
 *   (at your option) any later version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */
/* memdiskinfo and related code taken from mdiskchk.c by HPA */

/* This program can be compiled for DOS with the OpenWatcom compiler
 * (http://www.openwatcom.org/):
 *
 * wcl -3 -osx -mt getargs.c
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <i86.h>

/*
 *
 * Calculate the boot time arguments passed in 
 * and output a batch file which sets the appropriate
 * boot time arguments in environment variables
 *
 */

#define BUFSIZE 513 // Max size of command line

struct memdiskinfo {
  uint16_t bytes;               /* Bytes from memdisk */
  uint16_t version;             /* Memdisk version */
  uint32_t base;                /* Base of disk in high memory */
  uint32_t size;                /* Size of disk in sectors */
  char far * cmdline;           /* Command line */
  void far * oldint13;          /* Old INT 13h */
  void far * oldint15;          /* Old INT 15h */
  uint16_t olddosmem;

  /* We add our own fields at the end */
  int cylinders;
  int heads;
  int sectors;
};

struct memdiskinfo * query_memdisk(int drive)
{
  static struct memdiskinfo mm;
  uint32_t _eax, _ebx, _ecx, _edx;
  uint16_t _es, _di;
  unsigned char _dl = drive;

  __asm {
    .386 ;
    mov eax, 454d0800h ;
    mov ecx, 444d0000h ;
    mov edx, 53490000h ;
    mov dl, _dl ;
    mov ebx, 3f4b0000h ;
    int 13h ;
    mov _eax, eax ;
    mov _ecx, ecx ;
    mov _edx, edx ;
    mov _ebx, ebx ;
    mov _es, es ;
    mov _di, di
      }
  
  if ( _eax >> 16 != 0x4d21 ||
       _ecx >> 16 != 0x4d45 ||
       _edx >> 16 != 0x4944 ||
       _ebx >> 16 != 0x4b53 )
    return NULL;

  _fmemcpy((void far *)&mm, (void far *)MK_FP(_es,_di), 26);
  mm.cylinders = ((_ecx >> 8) & 0xff) + ((_ecx & 0xc0) << 2) + 1;
  mm.heads     = ((_edx >> 8) & 0xff) + 1;
  mm.sectors   = (_ecx & 0x3f);
  
  return &mm;
}

unsigned int argarray(unsigned char *cmdline, unsigned char ***args)
     /*
      * Return an array of pointer to chars (strings) which 
      *   the identify different args
      * Some spaces in cmdline replaced with \0x0 to indicate 
      *   end of argument
      */
{
  unsigned char **ans;
  unsigned char *ptr;
  unsigned char *pch;
  unsigned int num = 0;
  unsigned int curr = 0;

  ptr = cmdline;
  while (*ptr) {
    while ((*ptr) && (*ptr == ' ')) ptr++; // Ignore initial spaces
    if (*ptr) num++;
    while ((*ptr) && (*ptr != ' ')) ptr++; // Ignore all till next space
  }
  // num = number of arguments found
  *args = malloc(num * sizeof (char *)); // Allocate space  
  ptr = cmdline;
  curr = 0; // Setup the current pointer
  while (*ptr) {
    while ((*ptr) && (*ptr == ' ')) ptr++; // Ignore initial spaces
    if (*ptr) (*args)[curr++] = ptr;
    while ((*ptr) && (*ptr != ' ')) ptr++; // Ignore all till next space
    (*ptr) = 0;
    ptr++;
  }
  return num;
}

void parsecmdline(unsigned char __far *cmdline, const char *val)
{
  unsigned char **args;
  unsigned int num,ctr;
  unsigned char *buffer;

  buffer = malloc(BUFSIZE* (sizeof(unsigned char)));
  _fstrncpy(buffer,cmdline,BUFSIZE-2); // Copy cmdline into our buffer
  buffer[BUFSIZE-1] = 0; // For safety

  num = argarray(buffer,&args);
  if (!num)
    return;
  if (!val)
    puts("@echo off");
  for (ctr=0; ctr < num; ctr++)
    {
      unsigned char *p;
      p = strchr(args[ctr],'=');
      if (p)
        *(p++) = '\0';
      if (val != NULL && strcmp(args[ctr], val))
        continue;
      if (p) {
        if (val)
          printf("%s='%s'\n", args[ctr], p);
        else
          printf("SET %s=%s\n", args[ctr], p);
      } else {
        printf("%s%s=1\n", (val ? "" : "SET "), args[ctr]);
      }
    }
}

int main(int argc, char *argv[])
{
  int d;
  int found = 0;
  struct memdiskinfo *m;

  for ( d = 0 ; d <= 0xff ; d++ ) {
    if ( (m = query_memdisk(d)) != NULL ) {
      parsecmdline(m->cmdline, argv[1]);
      found++;
    }
  }

  //if (!found) printf("No MEMDISK drive found\n");
  return found;
}

/*
 *	The PCI Library -- Access to i386 I/O ports on WATCOM
 *
 *	Copyright (c) 2011 Anton D. Kachalov <mouse@yandex.ru>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

static inline void
outb(unsigned char b, unsigned short addr)
{
  __asm
  {
    mov dx, addr
    mov al, b
    out dx, al
  }
}

static inline void
outw(unsigned short int b, unsigned short addr)
{
  __asm
  {
    mov dx, addr
    mov ax, b
    out dx, ax
  }
}

static inline void
outl(unsigned long b, unsigned short addr)
{
  __asm
  {
    mov dx,  addr
    mov eax, b
    out dx,  eax
  }
}

static inline unsigned char
inb(unsigned short addr)
{
  unsigned char b = 0;
  __asm
  {
    mov dx, addr
    in al,  dx
    mov b,  al
  }
  return b;
}

static inline unsigned short int
inw(unsigned short addr)
{
  unsigned short int b = 0;
  __asm
  {
    mov dx, addr
    in ax,  dx
    mov b,  ax
  }
  return b;
}

static inline unsigned long
inl(unsigned short addr)
{
  unsigned long b = 0;
  __asm
  {
    mov dx,  addr
    in  eax, dx
    mov b,   eax
  }
  return b;
}

static int
intel_setup_io(struct pci_access *a UNUSED)
{
  return 1;
}

static inline int
intel_cleanup_io(struct pci_access *a UNUSED)
{
  return 1;
}

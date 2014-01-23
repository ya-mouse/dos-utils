/*
 * ffs -- vax ffs instruction
 */
int
ffs(int mask)
{
	int bit;
	unsigned int r = mask;
	static const signed char t[16] = {
		-28, 1, 2, 1,
		  3, 1, 2, 1,
		  4, 1, 2, 1,
		  3, 1, 2, 1
	};

	bit = 0;
	if (!(r & 0xffff)) {
		bit += 16;
		r >>= 16;
	}
	if (!(r & 0xff)) {
		bit += 8;
		r >>= 8;
	}
	if (!(r & 0xf)) {
		bit += 4;
		r >>= 4;
	}

	return (bit + t[ r & 0xf ]);
}

void
outb(unsigned char b, unsigned short addr)
{
  __asm
  {
    mov dx, addr
    mov al, b
    out dx, al
  }
}

void
outw(unsigned short int b, unsigned short addr)
{
  __asm
  {
    mov dx, addr
    mov ax, b
    out dx, ax
  }
}

void
outl(unsigned long b, unsigned short addr)
{
  __asm
  {
    mov dx,  addr
    mov eax, b
    out dx,  eax
  }
}

unsigned char
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

unsigned short int
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

unsigned long
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

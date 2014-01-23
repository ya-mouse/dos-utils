; !\file cpumodel.asm
;
;  This file contains all assembly code for the Intel CPU identification.
;  It is based on linux cpu detection code.
;
;  Intel also provides public similar code in the book
;  called :
;
;  Pentium Processor Family
;      Developer Family
;  Volume  3 : Architecture and Programming Manual
;
;  At the following place :
;
;  Chapter 5 : Feature determination
;  Chapter 25: CPUID instruction
;
;  COPYRIGHT (c) 1998 valette@crf.canon.fr
;
;  The license and distribution terms for this file may be
;  found in the file LICENSE in this distribution or at
;  http://www.OARcorp.com/rtems/license.html.
;
;  $Id: cpumodel.S,v 1.1 1998/08/05 15:15:46 joel Exp $
;
;  Rewritten for tasm/wasm by G. Vanem 2000 for
;  the Watt-32 tcp/ip stack.
;

PAGE 66, 132

ifdef DOSX  ; only for 32-bit target (including WIN32)

;
; All '__w32_' and '_x86' symbols are only needed for Borland and
; Digital Mars (until it fixes it's bug in the "pragma alias" handling).
;

PUBLIC x86_capability,    _x86_capability
PUBLIC x86_type,          _x86_type
PUBLIC x86_mask,          _x86_mask
PUBLIC x86_model,         _x86_model
PUBLIC x86_have_cpuid,    _x86_have_cpuid
PUBLIC x86_hard_math,     _x86_hard_math
PUBLIC x86_vendor_id,     _x86_vendor_id

PUBLIC _w32_CheckCpuType, __w32_CheckCpuType
PUBLIC _w32_get_cpuid,    __w32_get_cpuid
PUBLIC _w32_asm_ffs,      __w32_asm_ffs
PUBLIC _w32_get_rdtsc,    __w32_get_rdtsc
PUBLIC _w32_get_rdtsc2,   __w32_get_rdtsc2

PUBLIC _w32_MY_CS,        __w32_MY_CS
PUBLIC _w32_MY_DS,        __w32_MY_DS
PUBLIC _w32_MY_ES,        __w32_MY_ES
PUBLIC _w32_MY_SS,        __w32_MY_SS
PUBLIC _w32_Get_CR4,      __w32_Get_CR4
PUBLIC _w32_SelReadable , __w32_SelReadable
PUBLIC _w32_SelWriteable, __w32_SelWriteable

ifdef ??version   ; Turbo Assembler
  .486p
  .487
else
  .586p
  .387
endif

ifdef X32VM       ; FlashTek's X32 isn't flat-model, but SMALL
  .MODEL SMALL,C
else
  .MODEL FLAT,C
endif

ifdef ??version   ; Turbo Assembler
  _CPUID       equ  <db 0Fh, 0A2h>   ; TASM v3.2 lacks CPUID etc.
  _RDTSC       equ  <db 0Fh, 31h>
  _MOV_EAX_CR4 equ  <db 0Fh, 20h, 0E0h>
else              ; Watcom or ML assembler
  _CPUID       equ  cpuid
  _RDTSC       equ  rdtsc
  _MOV_EAX_CR4 equ  mov eax, CR4
endif

EFLAGS_ALIGN_CHECK = 040000h
EFLAGS_ID          = 200000h

.DATA

ALIGN 4

_x86_have_cpuid label dword
 x86_have_cpuid dd 0

_x86_hard_math  label dword
 x86_hard_math  dd 0

_x86_capability label dword
 x86_capability dd 0

_x86_type       label byte
 x86_type       db 0

_x86_model      label byte
 x86_model      db 0

_x86_mask       label byte
 x86_mask       db 0

_x86_vendor_id  label byte
 x86_vendor_id  db 13 dup (0)


.CODE

;
; Check Processor type: 386, 486, 6x86(L) or CPUID capable processor
;
 _w32_CheckCpuType:
__w32_CheckCpuType:
    push ebp
    mov  ebp, esp
    push ebx
    mov  x86_type, 3                ;  Assume 386 for now

    ; Start using the EFAGS AC bit determination method described in
    ; the book mentioned above page 5.1. If this bit can be set we
    ; have a 486 or above.

    pushfd                          ; save EFLAGS
    pushfd                          ; Get EFLAGS in EAX
    pop  eax

    mov  ecx, eax                   ; save original EFLAGS in ECX
    xor  eax, EFLAGS_ALIGN_CHECK    ; flip AC bit in EAX
    push eax                        ; set EAX as EFLAGS
    popfd
    pushfd                          ; Get new EFLAGS in EAX
    pop  eax

    xor  eax, ecx                   ; check if AC bit changed
    and  eax, EFLAGS_ALIGN_CHECK
    je   is386                      ; If not : we have a 386

    mov  x86_type, 4                ; Assume 486 for now
    mov  eax, ecx                   ; Restore orig EFLAGS in EAX
    xor  eax, EFLAGS_ID             ; flip  ID flag
    push eax                        ; set EAX as EFLAGS
    popfd
    pushfd                          ; Get new EFLAGS in EAX
    pop  eax

    xor  eax, ecx                   ; check if ID bit changed
    and  eax, EFLAGS_ID

    ;
    ; If we are on a straight 486DX, SX, or 487SX we can't
    ; change it. OTOH 6x86MXs and MIIs check OK.
    ; Also if we are on a Cyrix 6x86(L)

    je is486x

isnew:
    popfd                           ; restore original EFLAGS
    inc x86_have_cpuid              ; we have CPUID instruction

    ; use it to get :
    ;  processor type,
    ;  processor model,
    ;  processor mask,
    ; by using it with EAX = 1

    mov  eax, 1
   _CPUID

    mov  cl, al                     ; save reg for future use

    and  ah, 0Fh                    ; mask processor family
    mov  x86_type, ah               ; put result in x86_type (0..15)
    and  x86_type, 7

    and  al, 0F0h                   ; get model
    shr  al, 4
    mov  x86_model, al              ; store it in x86_model

    and  cl, 0Fh                    ; get mask revision
    mov  x86_mask, cl               ; store it in x86_mask

    mov  x86_capability, edx        ; store feature flags

    ;
    ; Get vendor info by using CPUID with EAX = 0
    ;
    xor eax, eax
   _CPUID

    ;
    ; Store results contained in EBX, EDX and ECX in x86_vendor_id[].
    ;
    mov  dword ptr x86_vendor_id, ebx
    mov  dword ptr x86_vendor_id+4, edx
    mov  dword ptr x86_vendor_id+8, ecx
    call check_x87
    jmp  end_CheckCpuType


;
; Now we test if we have a Cyrix 6x86(L). We didn't test before to avoid
; clobbering the new BX chipset used with the Pentium II, which has a
; register at the same addresses as those used to access the Cyrix special
; configuration registers (CCRs).

    ;
    ; A Cyrix/IBM 6x86(L) preserves flags after dividing 5 by 2
    ; (and it _must_ be 5 divided by 2) while other CPUs change
    ; them in undefined ways. We need to know this since we may
    ; need to enable the CPUID instruction at least.
    ; We couldn't use this test before since the PPro and PII behave
    ; like Cyrix chips in this respect.
    ;
is486x:
    xor ax, ax
    sahf
    mov ax, 5
    mov bx, 2
    div bl
    lahf
    cmp ah, 2
    jne ncyrix

    ;
    ; N.B. The pattern of accesses to 0x22 and 0x23 is *essential*
    ;      so do not try to "optimize" it! For the same reason we
    ;      do all this with interrupts off.

setCx86 MACRO reg, val
        mov ax, reg
        out 22h, ax
        mov ax, val
        out 23h, ax
        ENDM

getCx86 MACRO reg
        mov ax, reg
        out 22h, ax
        in  ax, 23h
        ENDM

    cli
    getCx86 0C3h             ; get CCR3
    mov cx, ax               ; Save old value
    mov bx, ax
    and bx, 0Fh              ; Enable access to all config registers
    or  bx, 10h              ; by setting bit 4
    setCx86 0C3h, bx

    getCx86 0E8h             ; now we can get CCR4
    or ax, 80h               ; and set bit 7 (CPUIDEN)
    mov bx, ax               ; to enable CPUID execution
    setCx86 0E8h, bx

    getCx86 0FEh             ; DIR0 : let's check this is a 6x86(L)
    and ax, 0F0h             ; should be 3xh
    cmp ax, 30h
    jne n6x86
    getCx86 0E9h             ; CCR5 : we reset the SLOP bit
    and ax, 0FDh             ; so that udelay calculation
    mov bx, ax               ; is correct on 6x86(L) CPUs
    setCx86 0E9h, bx

    setCx86 0C3h, cx         ; Restore old CCR3
    sti
    jmp isnew                ; We enabled CPUID now

n6x86:
    setCx86 0C3h, cx         ; Restore old CCR3
    sti

ncyrix:
    popfd                    ; restore original EFLAGS
    call check_x87
    jmp  end_CheckCpuType

is386:
    popfd                    ; restore original EFLAGS
    call check_x87

end_CheckCpuType:
    pop  ebx
    pop  ebp
    ret


;
; This checks for 287/387.
;
check_x87:
    fninit
    fstsw ax
    cmp al, 0
    je  is_x87
    mov x86_hard_math, 0
    ret

is_x87:
    mov x86_hard_math, 1
    ret

;
; void cdecl _w32_get_cpuid (DWORD val, DWORD *_eax, DWORD *_ebx,
;                            DWORD *_ecx, DWORD *_edx);
;
; Don't call this if x86_have_cpuid == FALSE
;
 _w32_get_cpuid:
__w32_get_cpuid:
    enter 0, 0
    push ebx
    push esi
    mov  eax, [ebp+8]        ; EAX = val
   _CPUID
    mov  esi, [ebp+12]
    mov  [esi], eax          ; *_eax = EAX
    mov  esi, [ebp+16]
    mov  [esi], ebx          ; *_ebx = EBX
    mov  esi, [ebp+20]
    mov  [esi], ecx          ; *_ecx = ECX
    mov  esi, [ebp+24]
    mov  [esi], edx          ; *_edx = EDX
    pop  esi
    pop  ebx
    leave
    ret


;
; uint64 cdecl _w32_get_rdtsc (void);
;
 _w32_get_rdtsc:
__w32_get_rdtsc:
   _RDTSC
    ret

;
; void cdecl _w32_get_rdtsc2 (struct ulong_long *tsc);
;
 _w32_get_rdtsc2:
__w32_get_rdtsc2:
    enter 0, 0
    push  esi
    push  edx
    mov   esi, [ebp+8]
   _RDTSC
    mov   [esi], eax
    mov   [esi+4], edx
    pop   edx
    pop   esi
    leave
    ret

;
; int cdecl _w32_asm_ffs (int val)
;
 _w32_asm_ffs:
__w32_asm_ffs:
    bsf eax, [esp+4]
    jnz ok
    mov eax, -1
ok: inc eax
    ret

;
; WORD cdecl MY_CS (void);
;
 _w32_MY_CS:
__w32_MY_CS:
    mov ax, cs
    ret

;
; WORD cdecl MY_DS (void);
;
 _w32_MY_DS:
__w32_MY_DS:
    mov ax, ds
    ret

;
; WORD cdecl MY_ES (void);
;
 _w32_MY_ES:
__w32_MY_ES:
    mov ax, es
    ret

;
; WORD cdecl MY_SS (void);
;
 _w32_MY_SS:
__w32_MY_SS:
    mov ax, ss
    ret

;
; DWORD cdecl Get_CR4 (void);
;
; Is virtualised under Win32 (always returns 0), hence of no use.
; Don't call this function w/o checking for a true Pentium first
; (see RDTSC_enabled() in misc.c). Returns 0 if CPL != 0.
;
 _w32_Get_CR4:
__w32_Get_CR4:
    mov  ax, cs
    test ax, 3
    jnz  not_cpl0
    _MOV_EAX_CR4
    ret

not_cpl0:
    xor eax, eax
    ret

;
; BOOL _w32_SelWriteable (WORD sel)
;
 _w32_SelWriteable:
__w32_SelWriteable:
    mov   ax, [esp+4]
    verw  ax
    sete  al
    movzx eax, al
    ret

;
; BOOL _w32_SelReadable (WORD sel)
;
 _w32_SelReadable:
__w32_SelReadable:
    mov   ax, [esp+4]
    verr  ax
    sete  al
    movzx eax, al
    ret


comment ~   ; not yet (problem with linking this)

; ************************************************************************
;
; 16 bit versions of some functions.
; Maybe not worth the effort; tlink have problems linking this module).
;

PUBLIC _w32_get_rdtsc2, __w32_get_rdtsc2

.MODEL LARGE,C
.CODE
.386

ALIGN 2

_x86_have_cpuid label dword
 x86_have_cpuid dd 0

_x86_hard_math  label dword
 x86_hard_math  dd 0

_x86_capability label dword
 x86_capability dd 0

_x86_type       label byte
 x86_type       db 0

_x86_model      label byte
 x86_model      db 0

_x86_mask       label byte
 x86_mask       db 0

_x86_vendor_id  label byte
 x86_vendor_id  db 13 dup (0)

;
; void far cdecl get_rdtsc2 (struct ulong_long far *res)
; RDTSC is always enabled in real-mode
;
 _w32_get_rdtsc2:
__w32_get_rdtsc2:
    enter 0, 0
    push  es
    les   bx, [bp+6]
    db    0Fh, 31h   ; RDTSC op-ode
    mov   es:[bx], eax
    mov   es:[bx+4], edx
    pop   es
    leave
    retf

 _w32_CheckCpuType:
__w32_CheckCpuType:
    mov _x86_type, 4  ; test !! to-do
    retf

;
; void cdecl _w32_get_cpuid (DWORD val, DWORD *_eax, DWORD *_ebx, DWORD *_ecx, DWORD *_edx);
;
 _w32_get_cpuid:
__w32_get_cpuid:
    enter 0, 0
    push  es
    push  di
    mov   eax, dword ptr [bp+6]        ; EAX = val

    db 0Fh, 0A2h             ; CPUID

    les   di, [bp+8]
    mov   es:[di], ax        ; *_eax = EAX
    shr   eax, 16
    mov   es:[di+4], ax

    les   di, [bp+12]
    mov   es:[di], bx        ; *_ebx = EBX
    shr   ebx, 16
    mov   es:[di+4], bx

    les   di, [bp+16]
    mov   es:[di], cx        ; *_ecx = ECX
    shr   ecx, 16
    mov   es:[di+4], cx

    les   di, [bp+20]
    mov   es:[di], dx        ; *_edx = EDX
    shr   edx, 16
    mov   es:[di+4], dx

    pop   di
    pop   es
    leave
    retf

;
; int FAR cdecl _w32_asm_ffs (int val)
;
 _w32_asm_ffs:
__w32_asm_ffs:
    bsf ax, [bp+4]
    jnz @f
    mov eax, -1
@f: inc ax
    retf

~

endif ; DOSX

end


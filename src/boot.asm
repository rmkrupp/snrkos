; boot.asm - entry assembly code
;
; Author: Rebecca Krupp (beka.krupp@gmail.com)
;

bits 32 ; force 32 bit object code

MBALIGN     equ 1
MEMINFO     equ 2
FLAGS       equ MBALIGN | MEMINFO
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

VGA_PTR     equ 0xB8000

; multiboot header
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .data
align 4

section .text
align 4

global _start ; entrypoint

_start:
    mov edx, dword VGA_PTR
    mov ax, 0x0768
    mov [edx], ax

    cli
.hang:
    hlt
    jmp .hang

section .bss progbits noalloc noexec write
align 16
stack_bottom:
    times 16384 db 0
stack_top:

section .text
global _start
extern kmain
_start:
    mov eax, 0xB8000
    mov [eax], word 0x044F

    mov esp, stack_top
    sub esp, 0x8
    mov [esp], ebx
    call kmain

    cli

.halt:
    hlt
    jmp .halt

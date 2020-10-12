/*
 * Copyright (C) 2013-2019, The AROS Development Team
 * All right reserved.
 * Author: Jason S. McMullan <jason.mcmullan@gmail.com>
 *
 * Licensed under the AROS PUBLIC LICENSE (APL) Version 1.1
 * 
 * $Id$
 */

#include <aros/m68k/asm.h>
#include <aros/config.h>

Version = 46
Revision= 12

	.section .aros.startup
	.chip 68020
	.align 4 

_start:
software_reset:
	jmp rom_init

    // make programs that peek rom header happy

    .word 0x0000

    .word 0x0000
    .word 0xffff
    .word Version
    .word Revision
    .word Version
    .word Revision
    .word 0xffff
    .word 0xffff
    .asciz "AROS ROM Operating System and Libraries"
    .asciz "Copyright (c) 1995-2020 "
    .asciz "The AROS Development Team, "
    .asciz "All Rights Reserved."
    .asciz ADATE

    /* Magic reset vector, used by some expansion ROMs and
     * softkick programs */
    .align 4
    .org   0xd0
    .global rom_magic_reset
rom_magic_reset:
    reset
    jmp rom_init

    .text

rom_init:
    lea _ss_end, %sp
    pea 0.w
    pea (%a6)
    jsr c_boot
    lea 4(%sp),%sp
    rts

Early_TrapHandler:
    pea     0.w
    rts

#define STACK_OFFSET (6*4)
	.globl Early_Exception
Early_Exception:
	or.w	#0x0700,%sr
#if AROS_SERIAL_DEBUG
	movem.l	%d0-%d2/%a0-%a2,-(%sp)

	lea		DebugPutHex,%a2

	move.l	STACK_OFFSET+2(%sp),-(%sp)
	pea		.earlystr1(%pc)
	jsr		(%a2)
	addq.l	#8,%sp
	
	moveq	#0,%d0
	move.w	STACK_OFFSET+6(%sp),%d0
	move.l	%d0,-(%sp)
	pea		.earlystr2(%pc)
	jsr		(%a2)
	addq.l	#8,%sp

	move.l	STACK_OFFSET+8(%sp),-(%sp)
	pea		.earlystr3(%pc)
	jsr		(%a2)
	addq.l	#8,%sp

	movem.l	(%sp)+,%d0-%d2/%a0-%a2
#endif
	jmp		Early_TrapHandler

        .section .rodata
.earlystr1:
	.string "Early Exception!\nPC"
.earlystr2:
	.string "Vector"
.earlystr3:
	.string "Address"


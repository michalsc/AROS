/*
    Copyright � 1995-2020, The AROS Development Team. All rights reserved.
    $Id$

    Desc: m68k-raspi bootstrap to exec.
    Lang: english
 */

#define DEBUG 0
#include <aros/debug.h>

#include <aros/kernel.h>
#include <exec/memory.h>
#include <exec/resident.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <hardware/cpu/memory.h>

#include "memory.h"

#include "kernel_romtags.h"
#include "kernel_base.h"

#define __AROS_KERNEL__
#include "exec_intern.h"

#include "devicetree.h"
#include "serialdebug.h"

#define SS_STACK_SIZE	0x02000

extern const struct Resident Exec_resident;
extern struct ExecBase *AbsExecBase;

#define _AS_STRING(x)	#x
#define AS_STRING(x)	_AS_STRING(x)

static BOOL iseven(APTR p)
{
    return (((ULONG)p) & 1) == 0;
}

/* Create a sign extending call stub:
 * foo:
 *   jsr AROS_SLIB_ENTRY(funcname, libname, funcid)
 *     0x4eb9 .... ....
 *   ext.w %d0	// EXT_BYTE only
 *     0x4880	
 *   ext.l %d0	// EXT_BYTE and EXT_WORD
 *     0x48c0
 *   rts
 *     0x4e75
 */
#define EXT_BYTE(lib, libname, funcname, funcid) \
	do { \
		void libname##_##funcname##_Wrapper(void) \
		{ asm volatile ( \
			"jsr " AS_STRING(AROS_SLIB_ENTRY(funcname, libname, funcid)) "\n" \
			"ext.w %d0\n" \
			"ext.l %d0\n" \
			"rts\n"); } \
		/* Insert into the library's jumptable */ \
		__AROS_SETVECADDR(lib, funcid, libname##_##funcname##_Wrapper); \
	} while (0)
#define EXT_WORD(lib, libname, funcname, funcid) \
	do { \
		void libname##_##funcname##_Wrapper(void) \
		{ asm volatile ( \
			"jsr " AS_STRING(AROS_SLIB_ENTRY(funcname, libname, funcid)) "\n" \
			"ext.l %d0\n" \
			"rts\n"); } \
		/* Insert into the library's jumptable */ \
		__AROS_SETVECADDR(lib, funcid, libname##_##funcname##_Wrapper); \
	} while (0)

/*
 * Create a register preserving call stub:
 * foo:
 *   movem.l %d0-%d1/%a0-%a1,%sp@-
 *     0x48e7 0xc0c0
 *   jsr AROS_SLIB_ENTRY(funcname, libname, funcid)
 *     0x4eb9 .... ....
 *   movem.l %sp@+,%d0-%d1/%d0-%a1
 *     0x4cdf 0x0303
 *   rts
 *     0x4e75
 */
#define PRESERVE_ALL(lib, libname, funcname, funcid) \
	do { \
		void libname##_##funcname##_Wrapper(void) \
	        { asm volatile ( \
	        	"movem.l %d0-%d1/%a0-%a1,%sp@-\n" \
	        	"jsr " AS_STRING(AROS_SLIB_ENTRY(funcname, libname, funcid)) "\n" \
	        	"movem.l %sp@+,%d0-%d1/%a0-%a1\n" \
	        	"rts\n" ); } \
		/* Insert into the library's jumptable */ \
		__AROS_SETVECADDR(lib, funcid, libname##_##funcname##_Wrapper); \
	} while (0)

/* Inject arbitrary code into the jump table
 * Used for GetCC and nano-stubs
 */
#define FAKE_IT(lib, libname, funcname, funcid, ...) \
	do { \
		UWORD *asmcall = (UWORD *)__AROS_GETJUMPVEC(lib, funcid); \
		const UWORD code[] = { __VA_ARGS__ }; \
		asmcall[0] = code[0]; \
		asmcall[1] = code[1]; \
		asmcall[2] = code[2]; \
	} while (0)
/* Inject a 'move.w #value,%d0; rts" sequence into the
 * jump table, to fake an private syscall.
 */
#define FAKE_ID(lib, libname, funcname, funcid, value) \
	FAKE_IT(lib, libname, funcname, funcid, 0x303c, value, 0x4e75)

extern BYTE __rom_start;
extern BYTE __rom_end;
extern BYTE _ss;
extern BYTE _ss_end;

static BOOL IsSysBaseValidNoVersion(struct ExecBase *sysbase)
{
	if (!iseven(sysbase))
		return FALSE;
    if (sysbase == NULL || (((ULONG)sysbase) & 0x80000001))
        return FALSE;
    if (sysbase->ChkBase != ~(IPTR)sysbase)
        return FALSE;
    return GetSysBaseChkSum(sysbase) == 0xffff;
}

static UWORD GetAttnFlags()
{
    /* Convert CPU/FPU flags to AttnFlags */
    UWORD attnflags = AFF_68010 | AFF_68020 | AFF_68881 | AFF_68882 | AFF_FPU | AFF_ADDR32;

    DEBUGPUTS(("[BOOT] CPU: "));
    if (attnflags & AFF_68080)
        DEBUGPUTS(("Apollo Core 68080"));
    else if (attnflags & AFF_68060)
        DEBUGPUTS(("68060"));
    else if (attnflags & AFF_68040)
        DEBUGPUTS(("68040"));
    else if (attnflags & AFF_68030)
        DEBUGPUTS(("68030"));
    else if (attnflags & AFF_68020) {
        if (attnflags & AFF_ADDR32)
            DEBUGPUTS(("68020"));
        else
            DEBUGPUTS(("68EC020"));
    } else if (attnflags & AFF_68010)
        DEBUGPUTS(("68010"));
    else
        DEBUGPUTS(("68000"));
    DEBUGPUTS((" FPU: "));
    if (attnflags & AFF_FPU40) {
        if (attnflags & AFF_68060)
            DEBUGPUTS(("68060"));
        else if (attnflags & AFF_68040)
            DEBUGPUTS(("68040"));
        else
            DEBUGPUTS(("-"));
    } else if (attnflags & AFF_68882)
        DEBUGPUTS(("68882"));
    else if (attnflags & AFF_68881)
        DEBUGPUTS(("68881"));
    else
        DEBUGPUTS(("-"));
    DEBUGPUTS(("\n"));

    return attnflags;
}

static void RomInfo(IPTR rom)
{
    APTR ptr = (APTR)rom;
    CONST_STRPTR str;

    if ((*(UWORD *)(ptr + 8) == 0x0000) &&
        (*(UWORD *)(ptr + 10) == 0xffff) &&
        (*(UWORD *)(ptr + 12) == *(UWORD *)(ptr + 16)) &&
        (*(UWORD *)(ptr + 14) == *(UWORD *)(ptr + 18)) &&
        (*(UWORD *)(ptr + 20) == 0xffff) &&
        (*(UWORD *)(ptr + 22) == 0xffff)) {
        DEBUGPUTHEX(("[BOOT] ROM Location", rom));
        DEBUGPUTD(("[BOOT]      Version", *(UWORD *)(ptr + 12)));
        DEBUGPUTD(("[BOOT]     Revision", *(UWORD *)(ptr + 14)));
        str = (ptr + 24);
        DEBUGPUTS(("[BOOT]     ROM Type: ")); DEBUGPUTS((str)); DEBUGPUTS(("\n"));
        str += strlen(str) + 1;
        DEBUGPUTS(("[BOOT]    Copyright: ")); DEBUGPUTS((str));
        str += strlen(str) + 1;
        DEBUGPUTS((str));
        str += strlen(str) + 1;
        DEBUGPUTS((str));
        DEBUGPUTS(("\n"));
        str += strlen(str) + 1;
        DEBUGPUTS(("[BOOT]    ROM Model: ")); DEBUGPUTS((str));
        DEBUGPUTS(("\n"));
    }
}

/*
    The rom_init.S code brought us here. Prepare initial setup based on the device tree,
    set up serial debug over console and start booting m68k AROS.
*/

struct TagItem bootmsg[] = {
    { KRN_CmdLine,              0               },
    { KRN_OpenFirmwareTree,     0               },
    { KRN_FlattenedDeviceTree,  0               },
    { KRN_BootLoader,           (IPTR)"Emu68"   },
    { KRN_ProtAreaStart,        (IPTR)&__rom_start },
    { KRN_ProtAreaEnd,          (IPTR)&__rom_end},
    { KRN_KernelLowest,         (IPTR)&__rom_start },
    { KRN_KernelPhysLowest,     (IPTR)&__rom_start },
    { KRN_KernelHighest,        (IPTR)&__rom_end},
    { TAG_END                                   },
};

void c_boot(void *fdt)
{
    void Early_Exception(void);
    struct TagItem *bootmsgptr = bootmsg;
    volatile APTR *trap;
    int i;
    BOOL wasvalid, arosbootstrapmode;
    UWORD *kickrom[8];
    struct MemHeader *mh;
    LONG oldLastAlert[4];
    ULONG oldmem;
    UWORD attnflags;
    APTR ColdCapture = NULL, CoolCapture = NULL, WarmCapture = NULL;
    APTR KickMemPtr = NULL, KickTagPtr = NULL, KickCheckSum = NULL;
    struct BootStruct *BootS = NULL;
    /* We can't use the global 'SysBase' symbol, since
     * the compiler does not know that PrepareExecBase
     * may change it out from under us.
     */
    struct ExecBase *oldSysBase = *(APTR *)4;
#define SysBase CANNOT_USE_SYSBASE_SYMBOL_HERE

    /* Parse the device tree. The openfirmware.resource will use it later on */
    bootmsg[2].ti_Data = (IPTR)fdt;
    bootmsg[1].ti_Data = (IPTR)dt_parse(fdt);

    /* Fetch command line for the boot message */
    bootmsg[0].ti_Data = (IPTR)dt_find_property(dt_find_node("/chosen"), "bootargs")->op_value;

    trap = (APTR *)(NULL);

    /* Set all the exceptions to the Early_Exception */
    for (i = 2; i < 64; i++) {
        if (i != 31) // Do not overwrite NMI
            trap[i] = Early_Exception;
    }

    /* Set up serial debug */
    DebugInit();

    DEBUGPUTS(("\n[BOOT] M68K AROS for RaspberryPi\n"));
    DEBUGPUTS(("[BOOT] Device: "));
    DEBUGPUTS((dt_get_prop_value(dt_find_property(dt_find_node("/"), "model"))));
    DEBUGPUTS(("\n"));

    RomInfo((IPTR)&__rom_start);
    RomInfo(0xf80000);
    RomInfo(0xe00000);
    RomInfo(0xf00000);

    attnflags = GetAttnFlags();
    
    /* Zap out old SysBase if invalid */
    arosbootstrapmode = FALSE;
    wasvalid = IsSysBaseValid(oldSysBase);
    if (wasvalid) {
        DEBUGPUTHEX(("[BOOT] SysBase was at", (ULONG)oldSysBase));
    } else {
        DEBUGPUTHEX(("[BOOT] SysBase invalid at", (ULONG)oldSysBase));
        wasvalid = FALSE;
    }

    if (bootmsgptr[0].ti_Tag == KRN_CmdLine) {
        DEBUGPUTS(("[BOOT] kernel commandline '"));
        DEBUGPUTS(((CONST_STRPTR)bootmsgptr[0].ti_Data));
        DEBUGPUTS(("'\n"));
    }

    if (wasvalid) {
        /* Save reset proof vectors */
        ColdCapture  = oldSysBase->ColdCapture;
        CoolCapture  = oldSysBase->CoolCapture;
        WarmCapture  = oldSysBase->WarmCapture;
        KickMemPtr   = oldSysBase->KickMemPtr; 
        KickTagPtr   = oldSysBase->KickTagPtr;
        KickCheckSum = oldSysBase->KickCheckSum;

        /* Mark the oldSysBase as processed */
        oldSysBase = NULL;
    }

    /* Look for 'HELP' at address 0 - we're recovering
     * from a fatal alert
     */
    if (trap[0] == (APTR)0x48454c50) {
        for (i = 0; i < 4; i++)
            oldLastAlert[i] = (LONG)trap[64 + i];

        DEBUGPUTHEX(("[BOOT] LastAlert Alert", oldLastAlert[0]));
        DEBUGPUTHEX(("[BOOT] LastAlert  Task", oldLastAlert[1]));
    } else {
        oldLastAlert[0] = (LONG)-1;
        oldLastAlert[1] = 0;
        oldLastAlert[2] = 0;
        oldLastAlert[3] = 0;
    }

    /* Clear alert marker */
    trap[0] = 0;

    kickrom[0] = (UWORD*)&__rom_start;
    kickrom[1] = (UWORD*)&__rom_end;
    kickrom[2] = (UWORD*)~0;
    kickrom[3] = (UWORD*)~0;
    kickrom[4] = (UWORD*)~0;
    kickrom[5] = (UWORD*)~0;
    kickrom[6] = (UWORD*)~0;
    kickrom[7] = (UWORD*)~0;

    IPTR *memory = dt_get_prop_value(dt_find_property(dt_find_node("/memory@0"), "reg"));
    IPTR memlo = memory[0];
    IPTR memhi = memory[0] + memory[1] - 1;

    if (memlo < 0x1000)
        memlo = 0x1000;
    if (memhi > ((IPTR)&__rom_start - 0x10000))
        memhi = ((IPTR)&__rom_start - 0x10000);

    DEBUGPUTHEX(("[BOOT] RAM lower", memlo));
    DEBUGPUTHEX(("[BOOT] RAM upper", memhi));

    mh = (struct MemHeader *)memlo;

    if (bootmsg[0].ti_Data && strstr((const char *)(bootmsg[0].ti_Data), "notlsf"))
    {
        DEBUGPUTS(("[BOOT] Preparing classic memory header\n"));
        krnCreateMemHeader("System Memory", 0, mh, (memhi - memlo), MEMF_FAST | MEMF_PUBLIC | MEMF_KICK | MEMF_LOCAL);
    }
    else
    {
        DEBUGPUTS(("[BOOT] Preparing TLSF memory header\n"));
        krnCreateTLSFMemHeader("System Memory", 0, mh, (memhi - memlo), MEMF_FAST | MEMF_PUBLIC | MEMF_KICK | MEMF_LOCAL);
    }
    
    for (i = 0; kickrom [i] != (UWORD*)~0; i += 2) {
        DEBUGPUTHEX(("[BOOT] Resident start", (ULONG)kickrom[i]));
        DEBUGPUTHEX(("[BOOT] Resident end  ", (ULONG)kickrom[i + 1]));
    }

    krnPrepareExecBase(kickrom, mh, bootmsgptr);

#undef SysBase
    PrivExecBase(SysBase)->KernelBase = NULL;
    DEBUGPUTHEX(("[SysBase] at", (ULONG)SysBase));

    PrivExecBase(SysBase)->PlatformData.BootMsg = bootmsgptr;
    SysBase->ThisTask->tc_SPLower = &_ss;
    SysBase->ThisTask->tc_SPUpper = &_ss_end;

    if (wasvalid) {
        SysBase->ColdCapture = ColdCapture;
        SysBase->CoolCapture = CoolCapture;
        SysBase->WarmCapture = WarmCapture;
        SysBase->ChkSum = 0;
        SysBase->ChkSum = GetSysBaseChkSum(SysBase) ^ 0xffff; 
        SysBase->KickMemPtr = KickMemPtr;
        SysBase->KickTagPtr = KickTagPtr;
        SysBase->KickCheckSum = KickCheckSum;
    }

    SysBase->SysStkUpper    = (APTR)&_ss_end;
    SysBase->SysStkLower    = (APTR)&_ss;

    /* Mark what the last alert was */
    for (i = 0; i < 4; i++)
        SysBase->LastAlert[i] = oldLastAlert[i];

    SysBase->AttnFlags = attnflags;

    /* Inject code for GetCC, depending on CPU model */
    if (SysBase->AttnFlags & AFF_68010) {
        /* move.w %ccr,%d0; rts; nop */
        FAKE_IT(SysBase, Exec, GetCC, 88, 0x42c0, 0x4e75, 0x4e71);
    } else {
        /* move.w %sr,%d0; rts; nop */
        FAKE_IT(SysBase, Exec, GetCC, 88, 0x40c0, 0x4e75, 0x4e71);
    }

#ifdef THESE_ARE_KNOWN_SAFE_ASM_ROUTINES
    PRESERVE_ALL(SysBase, Exec, Disable, 20);
    PRESERVE_ALL(SysBase, Exec, Enable, 21);
    PRESERVE_ALL(SysBase, Exec, Forbid, 22);
#endif
    PRESERVE_ALL(SysBase, Exec, Permit, 23);
    PRESERVE_ALL(SysBase, Exec, ObtainSemaphore, 94);
    PRESERVE_ALL(SysBase, Exec, ReleaseSemaphore, 95);
    PRESERVE_ALL(SysBase, Exec, ObtainSemaphoreShared, 113);

    /* Functions that need sign extension */
    EXT_BYTE(SysBase, Exec, SetTaskPri, 50);
    EXT_BYTE(SysBase, Exec, AllocSignal, 55);

    krnCreateROMHeader("Kickstart ROM", (APTR)&__rom_start, (APTR)&__rom_start);

    // Add extra memory here (if there is any)

    /* Now that we have a valid SysBase, we can call ColdCapture */
    //if (wasvalid)
    //    doColdCapture();

    /* Seal up SysBase's critical variables */
    SetSysBaseChkSum();

    /* Set privilege violation trap - we
     * need this to support the Exec/Supervisor call
     */
    //trap[8] = (SysBase->AttnFlags & AFF_68010) ? Exec_Supervisor_Trap : Exec_Supervisor_Trap_00;

    /* SysBase is complete, now we can enable instruction caches safely. */
    //CacheControl(CACRF_EnableI, CACRF_EnableI);
    //CacheClearU();

    oldmem = AvailMem(MEMF_FAST);
    
    /* Ok, let's start the system. We have to
     * do this in Supervisor context, since some
     * expansions ROMs (Cyperstorm PPC) expect it.
     */
    DEBUGPUTS(("[start] InitCode(RTF_SINGLETASK, 0)\n"));
    InitCode(RTF_SINGLETASK, 0);

    DEBUGPUTS(("[BOOT] We were not supposed to get here...\n\n"));

    while(1);
}

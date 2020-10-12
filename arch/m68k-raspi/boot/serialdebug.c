/* Simple serial debug - uses PL011 interface and assumes it to be already preconfigured */

#include <aros/macros.h>
#include "devicetree.h"

#define UARTDR      0x000
#define UARTRSR     0x004
#define UARTECR     0x004
#define UARTFR      0x018
#define UARTILPR    0x020
#define UARTIBRD    0x024
#define UARTFBRD    0x028

#define UARTFR_CTS  0x001
#define UARTFR_DSR  0x002
#define UARTFR_DCD  0x004
#define UARTFR_BUSY 0x008
#define UARTFR_RXFE 0x010
#define UARTFR_TXFF 0x020
#define UARTFR_RXFF 0x040
#define UARTFR_TXFE 0x080
#define UARTFR_RI   0x100

static void *pl011_base;

static inline ULONG rd32le(void *addr)
{
    return AROS_LE2LONG(*(volatile ULONG *)addr);
}

static inline void wr32le(void *addr, ULONG val)
{
    *(volatile ULONG*)addr = AROS_LONG2LE(val);
}

static void waitSerOUT()
{
    while(1)
    {
       if ((rd32le(pl011_base + UARTFR) & UARTFR_TXFF) == 0) break;
    }
}

int DebugPutChar(register int chr)
{
	if (pl011_base)
    {
        waitSerOUT();

        if (chr == '\n')
        {
            wr32le(pl011_base + UARTDR, '\r');
            waitSerOUT();
        }
        wr32le(pl011_base + UARTDR, chr);
    }
}

int DebugMayGetChar(void)
{
    if (pl011_base)
    {
        if ((rd32le(pl011_base + UARTFR) & UARTFR_RXFE) == 0)
        {
            return rd32le(pl011_base + UARTDR) & 0xff;
        }
    }

    return -1;
}

void DebugInit(void)
{
	of_node_t *aliases = dt_find_node("/aliases");
    if (aliases)
    {
        of_property_t *seralias = dt_find_property(aliases, "serial1");
        if (seralias)
        {
            of_node_t *serial = dt_find_node(seralias->op_value);

            if (serial)
            {
                ULONG *reg = dt_find_property(serial, "reg")->op_value;
                IPTR phys_base = reg[0];
                ULONG *ranges = dt_find_property(serial->on_parent, "ranges")->op_value;
                phys_base -= ranges[0];
                phys_base += ranges[1];
                
                pl011_base = (void *)phys_base;
            }
        }
    }
}

void DebugPutStr(register const char *buff)
{
	for (; *buff != 0; buff++)
		DebugPutChar(*buff);
}

void DebugPutDec(const char *what, ULONG val)
{
	int i, num;
	DebugPutStr(what);
	DebugPutStr(": ");
	if (val == 0) {
	    DebugPutChar('0');
	    DebugPutChar('\n');
	    return;
	}

	for (i = 1000000000; i > 0; i /= 10) {
	    if (val == 0) {
	    	DebugPutChar('0');
	    	continue;
	    }

	    num = val / i;
	    if (num == 0)
	    	continue;

	    DebugPutChar("0123456789"[num]);
	    val -= num * i;
	}
	DebugPutChar('\n');
}

void DebugPutHexVal(ULONG val)
{
	int i;
	for (i = 0; i < 8; i ++) {
		DebugPutChar("0123456789abcdef"[(val >> (28 - (i * 4))) & 0xf]);
	}
	DebugPutChar(' ');
}

void DebugPutHex(const char *what, ULONG val)
{
	DebugPutStr(what);
	DebugPutStr(": ");
	DebugPutHexVal(val);
	DebugPutChar('\n');
}

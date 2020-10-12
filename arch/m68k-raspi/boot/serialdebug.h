#ifndef _SERIALDEBUG_H
#define _SERIALDEBUG_H

#include <exec/types.h>

void DebugInit(void);
int DebugPutChar(register int chr);
int DebugMayGetChar(void);

void DebugPutStr(register const char *buff);
void DebugPutDec(const char *what, ULONG val);
void DebugPutHexVal(ULONG val);
void DebugPutHex(const char *what, ULONG val);

#define DEBUGPUTS(x) do { DebugPutStr x; } while(0)
#define DEBUGPUTD(x) do { DebugPutDec x; } while(0)
#define DEBUGPUTHEX(x) do { DebugPutHex x; } while(0)

#endif /* _SERIALDEBUG_H */

#ifndef __EXL_PRINT_H
#define __EXL_PRINT_H

#include "exl_Types.h"
#include "exl_DllExport.h"
#include <stdarg.h>

#ifdef EXL_PLATFORM_GFL

#ifdef EXL_DESMUME

INLINE void __exlDesmumePrint(const char* str) {
    asm("MOV R0, %0" : : "r" (str));
    asm("SWI 0xFC");
}

#define __EXL_PRINT(str) __exlDesmumePrint(str)
#else
#define __EXL_PRINT(str)
#endif
#elif defined(EXL_PLATFORM_WIN32)
#include "stdio.h"
#define __EXL_PRINT(str) printf(str)
#else
#define __EXL_PRINT(str)
#endif

INLINE void exlPrint(const char* string) {
    __EXL_PRINT(string);
}

EXL_PUBLIC void exlPrintf(const char* format, ...);

#endif
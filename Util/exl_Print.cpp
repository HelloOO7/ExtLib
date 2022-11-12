#ifndef __EXL_PRINT_CPP
#define __EXL_PRINT_CPP

#include "exl_Types.h"
#include <stdarg.h>
#include "exl_Print.h"

#ifdef EXL_PLATFORM_GFL

#ifdef EXL_DESMUME

INLINE void __exlDesmumePrint(const char* str) {
    asm("MOVS R0, %0" : : "r" (str)); //the compiler may not recognize the need to keep R0 intact when inlining this method
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

void exlPrint(const char* str) {
    __EXL_PRINT(str);
}

void exlPrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char outBuffer[1024];
    vsnprintf(outBuffer, 1024, format, args);
    exlPrint(outBuffer); //make sure R0 is in the proper place
    va_end(args);
}

#endif
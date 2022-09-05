#ifndef __EXL_PRINT_CPP
#define __EXL_PRINT_CPP

#include "exl_Types.h"
#include <stdarg.h>
#include "exl_Print.h"

void exlPrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char outBuffer[1024];
    vsnprintf(outBuffer, 1024, format, args);
    __EXL_PRINT(outBuffer);
    va_end(args);
}

#endif
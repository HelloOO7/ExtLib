#ifndef __EXL_PRINT_H
#define __EXL_PRINT_H

#include "exl_Types.h"
#include "exl_DllExport.h"
#include <stdarg.h>

EXL_PUBLIC void exlPrint(const char* string);

EXL_PUBLIC void exlPrintf(const char* format, ...);

#endif
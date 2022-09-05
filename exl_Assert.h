#ifndef __EXL_ASSERT_H
#define __EXL_ASSERT_H

#include "exl_DebugPrint.h"

#define EXL_ASSERT(expression) EXL_DEBUG_PRINTABLE((void)(                                                       \
            (!!(expression)) ||                                                              \
            (EXL_DEBUG_PRINTF("Assertion failed!\nIn file %s, line %d:\n%s", __FILE__, (unsigned)(__LINE__)), #expression) \
        ))

#ifdef EXL_PLATFORM_WIN32
#include <stdlib.h>
#define EXL_EXIT(code) exit(code)
#else
#define EXL_EXIT(code) while (1) {;}
#endif

#endif
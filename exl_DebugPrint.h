/**
 * @file exl_DebugPrint.h
 * @author Hello007
 * @brief Debug print macros.
 * @version 0.1
 * @date 2022-03-19
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __EXL_DEBUGPRINT_H
#define __EXL_DEBUGPRINT_H

//#define DEBUG

#ifdef DEBUG
#include "Util/exl_Print.h"
#define EXL_DEBUG_PRINTF(...) exlPrintf(__VA_ARGS__)
#define EXL_DEBUG_PRINTABLE(str) str
#else
#define EXL_DEBUG_PRINTF(...)
#define EXL_DEBUG_PRINTABLE(str)
#endif

#endif
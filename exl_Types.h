/**
 * @file exl_Types.h
 * @author Hello007
 * @brief ExtLib standard types.
 * @version 0.1
 * @date 2022-03-18
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __EXL_TYPES_H
#define __EXL_TYPES_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief Counts the number of elements in an array.
 */
#define NELEMS(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

/**
 * @brief Force-inlined function.
 */
#ifndef INLINE
#if defined(_MSC_VER)
#define INLINE inline __forceinline
#elif defined(__GNUC__)
#define INLINE inline __attribute__((always_inline))
#endif
#endif

#if defined(_MSC_VER)
#define _PRAGMA_HELPER(str) _Pragma(#str)
#define ALIGNED_TYPE(definition, alignment) _PRAGMA_HELPER(pack(push, alignment)) \
definition;\
_Pragma("pack(pop)")
#elif defined(__GNUC__)
#define ALIGNED_TYPE(definition, alignment) definition __attribute__ ((aligned(alignment)));
#endif

typedef uint64_t u64;

typedef int64_t s64;

typedef uint32_t u32;

typedef int32_t s32;

typedef uint16_t u16;

typedef int16_t s16;

typedef uint8_t u8;

typedef int8_t s8;

#endif
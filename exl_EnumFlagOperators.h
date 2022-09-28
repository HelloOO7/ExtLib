/**
 * @file exl_EnumFlagOperators.h
 * @author Hello007
 * @brief WinAPI header for bitwise enum operations in C++.
 * @version 0.1
 * @date 2022-09-28
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_ENUMFLAGOPERATORS_H
#define __RPM_ENUMFLAGOPERATORS_H

#ifndef CPP_ENUM_OPS_DEFINED
#define CPP_ENUM_OPS_DEFINED

#include <stdint.h>

template <size_t S>
struct _ENUM_FLAG_INTEGER_FOR_SIZE;

template <>
struct _ENUM_FLAG_INTEGER_FOR_SIZE<1>
{
    typedef uint8_t type;
};

template <>
struct _ENUM_FLAG_INTEGER_FOR_SIZE<2>
{
    typedef uint16_t type;
};

template <>
struct _ENUM_FLAG_INTEGER_FOR_SIZE<4>
{
    typedef uint32_t type;
};

template <class T>
struct _ENUM_FLAG_SIZED_INTEGER
{
    typedef typename _ENUM_FLAG_INTEGER_FOR_SIZE<sizeof(T)>::type type;
};

#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
inline constexpr ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) | ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) |= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) & ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) &= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE operator ~ (ENUMTYPE a) { return ENUMTYPE(~((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a)); } \
inline constexpr ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) ^ ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) ^= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \

#endif

#endif
#ifndef __EXL_FIXEDPOINTMATH_H
#define __EXL_FIXEDPOINTMATH_H

#ifndef EXL_FIXED_POINT_SHIFT
#define EXL_FIXED_POINT_SHIFT 12
#endif

/*
Macros for fixed point operation.
*/

typedef signed int fix32;

#define FIX_ONE (1 << EXL_FIXED_POINT_SHIFT)
#define FIX_HALF (FIX_ONE >> 1)

#define __FIX_ROUND(a) ((a) + (FIX_HALF >> 1))

#define I2FIX(i) ((fix32)((i) << EXL_FIXED_POINT_SHIFT))
#define F2FIX(i) ((fix32)((i) * (1 << EXL_FIXED_POINT_SHIFT)))
#define FIX2I(f) ((s32)((f) >> EXL_FIXED_POINT_SHIFT))

#define FIXFLOOR(f) ((s32)(((f) & ((1 << EXL_FIXED_POINT_SHIFT) - 1)) >> EXL_FIXED_POINT_SHIFT))
#define FIXCEIL(f) ((s32)(((f) + ((1 << EXL_FIXED_POINT_SHIFT) - 1)) >> EXL_FIXED_POINT_SHIFT))

#define FIXADD(a, b) ((a) + (b))
#define FIXSUB(a, b) ((a) - (b))

#define FIXMUL(a, b) (fix32)(__FIX_ROUND((long long)(a) * (long long)(b)) >> EXL_FIXED_POINT_SHIFT)
#define FIXMUL_LOW(a, b) (__FIX_ROUND((a) * (b)) >> EXL_FIXED_POINT_SHIFT)

#ifdef EXL_PLATFORM_GFL
    #include "std/stdfx.h"
    #include "std/stddiv.h"

    #if EXL_FIXED_POINT_SHIFT == 12
        #define FIXDIV(a, b) fx_div(a, b)
    #else
        INLINE fix32 __exl_fixed_div(fix32 numerator, fix32 denominator) {
            fx_div_req(numerator, denominator); //this will shift numerator 32 bits left, then request division
            return (fix32)((div_get_result() + (1 << (32 - EXL_FIXED_POINT_SHIFT - 1))) >> (32 - EXL_FIXED_POINT_SHIFT));
        }

        #define FIXDIV(a, b) __exl_fixed_div(a, b)
    #endif
#else
    #define FIXDIV(a, b) (fix32)(((((long long)(a) << 32LL) / (b)) + (1 << (32 - EXL_FIXED_POINT_SHIFT - 1))) >> (32 - EXL_FIXED_POINT_SHIFT))
#endif

#endif
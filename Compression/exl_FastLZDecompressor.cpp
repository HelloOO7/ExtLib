#ifndef __EXL_FASTLZDECODER_CPP
#define __EXL_FASTLZDECODER_CPP

/*
  FastLZ - Byte-aligned LZ77 compression library
  Copyright (C) 2005-2020 Ariya Hidayat <ariya.hidayat@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdint.h>

#include "exl_FastLZDecompressor.h"

/*
 * Always check for bound when decompressing.
 * Generally it is best to leave it defined.
 */
#define FASTLZ_SAFE
#if defined(FASTLZ_USE_SAFE_DECOMPRESSOR) && (FASTLZ_USE_SAFE_DECOMPRESSOR == 0)
#undef FASTLZ_SAFE
#endif

/*
 * Give hints to the compiler for branch prediction optimization.
 */
#if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 2))
#define FASTLZ_LIKELY(c) (__builtin_expect(!!(c), 1))
#define FASTLZ_UNLIKELY(c) (__builtin_expect(!!(c), 0))
#else
#define FASTLZ_LIKELY(c) (c)
#define FASTLZ_UNLIKELY(c) (c)
#endif

/*
 * Specialize custom 64-bit implementation for speed improvements.
 */
#if defined(__x86_64__) || defined(_M_X64)
#define FLZ_ARCH64
#endif

#if defined(FASTLZ_SAFE)
#define FASTLZ_BOUND_CHECK(cond) \
  if (FASTLZ_UNLIKELY(!(cond))) return 0;
#else
#define FASTLZ_BOUND_CHECK(cond) \
  do {                           \
  } while (0)
#endif

#include <string.h>

#if defined(FLZ_ARCH64)

#endif /* FLZ_ARCH64 */

#define MAX_COPY 32
#define MAX_LEN 264 /* 256 + 8 */
#define MAX_L1_DISTANCE 8192
#define MAX_L2_DISTANCE 8191
#define MAX_FARDISTANCE (65535 + MAX_L2_DISTANCE - 1)

#define HASH_LOG 14
#define HASH_SIZE (1 << HASH_LOG)
#define HASH_MASK (HASH_SIZE - 1)

namespace exl {
    namespace compress {
        int FastLZDecompressor::Decompress(const void* src, void* dest, size_t compSize, size_t uncompSize) {
            /* magic identifier for compression level */
            int level = ((*(const uint8_t*)src) >> 5) + 1;

            if (level == 1) return DecompressL1(src, compSize, dest, uncompSize);
            if (level == 2) return DecompressL2(src, compSize, dest, uncompSize);

            /* unknown level, trigger error */
            return 0;
        }

        static void fastlz_memmove(uint8_t* dest, const uint8_t* src, uint32_t count) {
            do {
                *dest++ = *src++;
            } while (--count);
        }

        int FastLZDecompressor::DecompressL1(const void* input, int length, void* output, int maxout) {
            const uint8_t* ip = (const uint8_t*)input;
            const uint8_t* ip_limit = ip + length;
            const uint8_t* ip_bound = ip_limit - 2;
            uint8_t* op = (uint8_t*)output;
            uint8_t* op_limit = op + maxout;
            uint32_t ctrl = (*ip++) & 31;

            while (1) {
                if (ctrl >= 32) {
                    uint32_t len = (ctrl >> 5) - 1;
                    uint32_t ofs = (ctrl & 31) << 8;
                    const uint8_t* ref = op - ofs - 1;
                    if (len == 7 - 1) {
                        FASTLZ_BOUND_CHECK(ip <= ip_bound);
                        len += *ip++;
                    }
                    ref -= *ip++;
                    len += 3;
                    FASTLZ_BOUND_CHECK(op + len <= op_limit);
                    FASTLZ_BOUND_CHECK(ref >= (uint8_t*)output);
                    fastlz_memmove(op, ref, len);
                    op += len;
                } else {
                    ctrl++;
                    FASTLZ_BOUND_CHECK(op + ctrl <= op_limit);
                    FASTLZ_BOUND_CHECK(ip + ctrl <= ip_limit);
                    memcpy(op, ip, ctrl);
                    ip += ctrl;
                    op += ctrl;
                }

                if (FASTLZ_UNLIKELY(ip > ip_bound)) {
                    break;
                }
                ctrl = *ip++;
            }

            return op - (uint8_t*)output;
        }

        int FastLZDecompressor::DecompressL2(const void* input, int length, void* output, int maxout) {
            const uint8_t* ip = (const uint8_t*)input;
            const uint8_t* ip_limit = ip + length;
            const uint8_t* ip_bound = ip_limit - 2;
            uint8_t* op = (uint8_t*)output;
            uint8_t* op_limit = op + maxout;
            uint32_t ctrl = (*ip++) & 31;

            while (1) {
                if (ctrl >= 32) {
                    uint32_t len = (ctrl >> 5) - 1;
                    uint32_t ofs = (ctrl & 31) << 8;
                    const uint8_t* ref = op - ofs - 1;

                    uint8_t code;
                    if (len == 7 - 1) do {
                        FASTLZ_BOUND_CHECK(ip <= ip_bound);
                        code = *ip++;
                        len += code;
                    } while (code == 255);
                    code = *ip++;
                    ref -= code;
                    len += 3;

                    /* match from 16-bit distance */
                    if (FASTLZ_UNLIKELY(code == 255)) {
                        if (FASTLZ_LIKELY(ofs == (31 << 8))) {
                            FASTLZ_BOUND_CHECK(ip < ip_bound);
                            ofs = (*ip++) << 8;
                            ofs += *ip++;
                            ref = op - ofs - MAX_L2_DISTANCE - 1;
                        }
                    }

                    FASTLZ_BOUND_CHECK(op + len <= op_limit);
                    FASTLZ_BOUND_CHECK(ref >= (uint8_t*)output);
                    fastlz_memmove(op, ref, len);
                    op += len;
                } else {
                    ctrl++;
                    FASTLZ_BOUND_CHECK(op + ctrl <= op_limit);
                    FASTLZ_BOUND_CHECK(ip + ctrl <= ip_limit);
                    memcpy(op, ip, ctrl);
                    ip += ctrl;
                    op += ctrl;
                }

                if (FASTLZ_UNLIKELY(ip >= ip_limit)) {
                    break;
                }
                ctrl = *ip++;
            }

            return op - (uint8_t*)output;
        }
    }
}

#endif
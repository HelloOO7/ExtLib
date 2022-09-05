#ifndef __EXL_DECOMPRESSOR_H
#define __EXL_DECOMPRESSOR_H

#include "exl_Compression.h"
#include "exl_FastLZDecompressor.h"
#include <string.h>

namespace exl {
    namespace compress {
        typedef int (* DecompressFunc)(const void* src, void* dest, size_t compSize, size_t uncompSize);

        INLINE static int DecompressNONE(const void* src, void* dest, size_t compSize, size_t uncompSize) {
            memcpy(dest, src, compSize);
            return compSize;
        }
        static const DecompressFunc DECOMPRESS_FUNCTIONS[] = {
            DecompressNONE,
            FastLZDecompressor::Decompress
        };

        class Decompressor {
        public:
            EXL_PUBLIC INLINE static int Decompress(CompressionType compressionType, const void* src, void* dest, size_t compSize, size_t uncompSize) {
                return DECOMPRESS_FUNCTIONS[compressionType](src, dest, compSize, uncompSize);
            }
        };
    }
}

#endif
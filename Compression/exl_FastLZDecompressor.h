#ifndef __EXL_FASTLZDECOMPRESSOR_H
#define __EXL_FASTLZDECOMPRESSOR_H

#include "exl_Types.h"
#include "exl_DllExport.h"

namespace exl {
    namespace compress {
        class FastLZDecompressor {
        public:
            EXL_PUBLIC static int Decompress(const void* src, void* dest, size_t compSize, size_t uncompSize);
        
        private:
            static int DecompressL1(const void* input, int length, void* output, int maxout);
            static int DecompressL2(const void* input, int length, void* output, int maxout);
        };
    }
}

#endif
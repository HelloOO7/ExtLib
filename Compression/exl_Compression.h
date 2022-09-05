#ifndef __EXL_COMPRESSION_H
#define __EXL_COMPRESSION_H

namespace exl {
    namespace compress {
        enum CompressionType : uint8_t {
            NONE,
            FASTLZ
        };
    }
}

#endif
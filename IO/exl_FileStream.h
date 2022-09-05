#ifndef __EXL_FILESTREAM_H
#define __EXL_FILESTREAM_H

#include "exl_Stream.h"

#ifdef EXL_PLATFORM_GFL
#include "nds/fs.h"
#undef bool
#else
#include <stdio.h>
#endif

namespace exl {
    namespace io {
        class FileStream : public Stream {
        public:
            enum OpenMode {
                READ,
                READ_WRITE
            };

        private:
            #ifdef EXL_PLATFORM_GFL
            FSFile __file;
            FSFile* file;
            #else
            FILE* file;
            #endif

        public:
            EXL_PUBLIC FileStream(const char* path, OpenMode mode);

            EXL_PUBLIC INLINE FileStream(const char* path) : FileStream(path, READ) {};

            EXL_PUBLIC u32 Read(void* dest, u32 elementSize, u32 elementCount);

            EXL_PUBLIC u32 Write(void* src, u32 elementSize, u32 elementCount);

            EXL_PUBLIC bool Seek(s32 pos, SeekOrigin origin);
            
            EXL_PUBLIC u32 Tell();

            EXL_PUBLIC void Close();
        };
    }
}

#endif
#ifndef __EXL_FILESTREAM_CPP
#define __EXL_FILESTREAM_CPP

#include "exl_FileStream.h"

#ifdef EXL_PLATFORM_GFL
enum SeekOrigin
{
    IO_SEEK_SET = 0x0,
    IO_SEEK_CUR = 0x1,
    IO_SEEK_END = 0x2,
};

extern "C" {
    extern __NDSFSFile*finit(__NDSFSFile* file);
    extern int         romfs_fopen(__NDSFSFile* file, const char* path);
    extern int         romfs_fclose(__NDSFSFile* file);
    extern int         romfs_fseek(__NDSFSFile* file, u32 offset, SeekOrigin origin);
    extern u32         romfs_ftell(__NDSFSFile* file);
    extern u32         romfs_fread(__NDSFSFile* file, void* dest, u32 length);
}

typedef SeekOrigin __NDSSeekOrigin;
#else
#include <stdio.h>
#endif

namespace exl {
    namespace io {
        FileStream::FileStream(const char* path, OpenMode mode) {
            #ifdef EXL_PLATFORM_GFL
            file = &__file;
            finit(file);
            if (!romfs_fopen(file, path)) {
                file = nullptr;
            }
            #else
            file = fopen(path, mode == READ ? "rb" : "w+b");
            #endif
        }

        u32 FileStream::Read(void* dest, u32 elementSize, u32 elementCount) {
            #ifdef EXL_PLATFORM_GFL
            if (!file) {
                return 0;
            }
            return romfs_fread(file, dest, elementSize * elementCount);
            #else
            return fread(dest, elementSize, elementCount, file);
            #endif
        }

        u32 FileStream::Write(void* src, u32 elementSize, u32 elementCount) {
            #ifdef EXL_PLATFORM_GFL
            //UNSUPPORTED
            return -1;
            #else
            return fwrite(src, elementSize, elementCount, file);
            #endif
        }

        bool FileStream::Seek(s32 pos, SeekOrigin origin) {
            #ifdef EXL_PLATFORM_GFL
            static const __NDSSeekOrigin SEEKORIGIN_LUT_NDS[] = {
                IO_SEEK_SET,
                IO_SEEK_CUR,
                IO_SEEK_END
            };

            if (!file) {
                return false;
            }

            return romfs_fseek(file, pos, SEEKORIGIN_LUT_NDS[(u32)origin]);
            #else
            static const u32 SEEKORIGIN_LUT_STDIO[] = {
                SEEK_SET,
                SEEK_CUR,
                SEEK_END
            };

            return fseek(file, pos, SEEKORIGIN_LUT_STDIO[(u32)origin]) == 0;
            #endif
        }

        u32 FileStream::Tell() {
            #ifdef EXL_PLATFORM_GFL
            if (!file) {
                return 0;
            }

            return romfs_ftell(file);
            #else
            return ftell(file);
            #endif
        }

        void FileStream::Close() {
            if (file) {
                #ifdef EXL_PLATFORM_GFL
                romfs_fclose(file);
                #else
                fclose(file);
                #endif
                file = nullptr;
            }
        }
    }
}

#endif
#ifndef __EXL_FSARCHIVEACCESSOR_H
#define __EXL_FSARCHIVEACCESSOR_H

#include "exl_Types.h"
#include "IO/exl_Stream.h"
#include "FileSystem/exl_FSArchive.h"
#include "Util/exl_HashTree.h"
#include "Heap/exl_Allocator.h"
#include "Util/exl_HashCode.h"

namespace exl {
    namespace fs {
        class FSArchiveAccessor {
        private:
            exl::io::Stream* m_Stream;
            exl::util::HashTree* m_FileHashMap;
            u32 m_FileTableOffset;

            u32 m_NowFileStart;
            u32 m_NowFileEnd;
        
        public:
            FSArchiveAccessor(exl::io::Stream* stream, exl::heap::Allocator* allocator);

            ~FSArchiveAccessor();

            bool OpenFile(exl::util::Hash hash);

            INLINE bool OpenFile(const char* path) {
                return OpenFile(exl::util::HashCode(path));
            }

            bool SeekFile(s32 offset, exl::io::Stream::SeekOrigin origin);

            u32 ReadFile(void* dest, u32 elementSize, u32 elementCount);
        };
    }
}

#endif
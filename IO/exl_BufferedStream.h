#ifndef __EXL_BUFFEREDSTREAM_H
#define __EXL_BUFFEREDSTREAM_H

#include "exl_Types.h"
#include "exl_Stream.h"
#include "Heap/exl_Allocator.h"

namespace exl {
    namespace io {
        class BufferedStream : public exl::io::Stream {
        private:
            exl::io::Stream*    m_SubStream;
            char*               m_Buffer;
            size_t              m_BufferSize;
            char*               m_BufferPos;
            char*               m_BufferEnd;
            char*               m_BufferLimit;
            size_t              m_BufferStreamPos;
            bool                m_FreeBufOnDelete;

        public:
            EXL_PUBLIC BufferedStream(exl::io::Stream* subStream, void* buffer, size_t bufSize);

            EXL_PUBLIC BufferedStream(exl::io::Stream* subStream, exl::heap::Allocator* allocator, size_t bufSize);

            EXL_PUBLIC u32 Read(void* dest, u32 elementSize, u32 elementCount);

            EXL_PUBLIC bool Seek(s32 pos, SeekOrigin origin);
            
            EXL_PUBLIC u32 Tell();

            EXL_PUBLIC void Close();
        
        private:
            void bufcpy(char* dest, size_t size);

            INLINE size_t LimitMaxRead(size_t reqRead) {
                size_t max = m_BufferLimit - m_BufferPos;
                return reqRead > max ? max : reqRead;
            }

            INLINE void RefillAfterSeek(u32 seekPos) {
                m_BufferStreamPos = seekPos;
                m_BufferLimit = m_Buffer + m_SubStream->Read(m_Buffer, 1, m_BufferSize);
                m_BufferPos = m_Buffer;
            }
        };
    }
}

#endif
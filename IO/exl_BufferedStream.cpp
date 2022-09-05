#ifndef __EXL_BUFFEREDSTREAM_CPP
#define __EXL_BUFFEREDSTREAM_CPP

#include "exl_Types.h"
#include "exl_Stream.h"
#include "Heap/exl_Allocator.h"
#include "exl_BufferedStream.h"
#include "exl_DebugPrint.h"

namespace exl {
    namespace io {       
        BufferedStream::BufferedStream(exl::io::Stream* subStream, void* buffer, size_t bufSize) {
            m_SubStream = subStream;
            m_Buffer = static_cast<char*>(buffer);
            m_BufferSize = bufSize;
            m_BufferEnd = m_Buffer + m_BufferSize;
            m_BufferPos = m_Buffer;
            m_BufferStreamPos = 0;
            m_BufferLimit = m_Buffer + subStream->Read(m_Buffer, 1, m_BufferSize); //initial stream fill
        }

        BufferedStream::BufferedStream(exl::io::Stream* subStream, exl::heap::Allocator* allocator, size_t bufSize) : BufferedStream(subStream, allocator->Alloc(bufSize), bufSize) {
            m_FreeBufOnDelete = true;
        }

        void BufferedStream::bufcpy(char* dst, size_t size) {
            if (size == 1) {
                *dst = *m_BufferPos;
            }
            else {
                memcpy(dst, m_BufferPos, size);
            }
            m_BufferPos += size;
        }

        u32 BufferedStream::Read(void* dest, u32 elementSize, u32 elementCount) {
            u32 remainBytesRead = elementCount * elementSize;
            if (remainBytesRead) {
                char* dest8 = static_cast<char*>(dest);
                u32 available = m_BufferEnd - m_BufferPos;
                if (remainBytesRead > available && m_BufferLimit == m_BufferEnd) { //if the original stream ends within the buffer, no more refills are possible
                    if (available) {
                        //Finish up remaining buffer data
                        bufcpy(dest8, available);
                        dest8 += available;
                        remainBytesRead -= available;
                    }
                    if (remainBytesRead < m_BufferSize) {
                        //Refill buffer, read from buffer
                        m_BufferStreamPos += m_BufferSize; //reading from here
                        u32 bytesRead = m_SubStream->Read(m_Buffer, 1, m_BufferSize);
                        m_BufferLimit = m_Buffer + bytesRead;
                        m_BufferPos = m_Buffer;
                        bufcpy(dest8, remainBytesRead);
                        return bytesRead + available;
                    }
                    else {
                        //Read directly to destination
                        u32 readResult = m_SubStream->Read(dest8, 1, remainBytesRead);
                        //Then set buffer to read after that
                        m_BufferStreamPos += readResult - m_BufferStreamPos;
                        m_BufferPos = m_BufferEnd; //next read will advance buffer stream pos correctly, right now buffer is set to full
                        m_BufferLimit = m_BufferEnd;
                        return readResult + available;
                    }
                }
                else {
                    remainBytesRead = LimitMaxRead(remainBytesRead);
                    bufcpy(dest8, remainBytesRead);
                    return remainBytesRead;
                }
            }
            return 0;
        }

        bool BufferedStream::Seek(s32 pos, SeekOrigin origin) {
            if (origin == SeekOrigin::SET) {
                s32 delta = pos - m_BufferStreamPos;
                if (delta < (m_BufferLimit - m_Buffer)) {
                    m_BufferPos = m_Buffer + delta;
                    return true;
                }
                else {
                    if (m_SubStream->SeekSet(pos)) {
                        RefillAfterSeek(pos);
                        return true;
                    }
                }
            }
            else if (origin == SeekOrigin::CUR) {
                if (!pos) {
                    return true;
                }
                u32 outBufPos = m_BufferPos - m_Buffer + pos;
                if (outBufPos > 0 && outBufPos < m_BufferSize) {
                    m_BufferPos = m_Buffer + outBufPos;
                    return true;
                }
                else {
                    //abs strm pos desired: m_BufferStreamPos + (m_BufferPos - m_Buffer) + pos | actual: m_BufferStreamPos + (m_BufferLimit - m_Buffer)
                    //rel strm pos = actual - (m_BufferLimit - m_Buffer) + (m_BufferPos - m_Buffer) + pos
                    //             = actual - m_BufferLimit + m_BufferPos + pos
                    u32 seekPos = m_BufferStreamPos + (m_BufferPos - m_Buffer) + pos;
                    if (m_SubStream->SeekSet(seekPos)) {
                        RefillAfterSeek(seekPos);
                        return true;
                    }
                }
            }
            else if (origin == SeekOrigin::END) {
                if (m_SubStream->SeekEnd(pos)) {
                    RefillAfterSeek(Tell());
                    return true;
                }
            }
            return false;
        }
        
        u32 BufferedStream::Tell() {
            return m_BufferStreamPos + (m_BufferPos - m_Buffer);
        }

        void BufferedStream::Close() {
            if (m_FreeBufOnDelete) {
                exl::heap::Allocator::FreeStatic(m_Buffer);
            }
            m_SubStream->Close();
        }
    }
}

#endif
#ifndef __EXL_FSARCHIVEACCESSOR_CPP
#define __EXL_FSARCHIVEACCESSOR_CPP

#include "exl_Types.h"
#include "IO/exl_Stream.h"
#include "FileSystem/exl_FSArchive.h"
#include "Util/exl_HashTree.h"
#include "Heap/exl_Allocator.h"
#include "exl_FSArchiveAccessor.h"
#include "Util/exl_Offset.h"
#include "exl_DebugPrint.h"

namespace exl {
    namespace fs {
        FSArchiveAccessor::FSArchiveAccessor(exl::io::Stream* stream, exl::heap::Allocator* allocator) {
            m_Stream = stream;
            
            exl::fs::ArchiveHeader arcHeader;
            stream->Read(&arcHeader, sizeof(arcHeader), 1);
            m_FileTableOffset = arcHeader.Files.GetOffset(&arcHeader);
            u32 FileHashTableOffset = arcHeader.FileNameLookup.GetOffset(&arcHeader);
            stream->SeekSet(FileHashTableOffset);
            u32 hashTreeNodeCount;
            stream->Read(&hashTreeNodeCount, sizeof(u32), 1);
            EXL_DEBUG_PRINTF("File hash tree size %d\n", hashTreeNodeCount);
            m_FileHashMap = new(allocator, hashTreeNodeCount) exl::util::HashTree();
            m_FileHashMap->NodeCount = hashTreeNodeCount;
            stream->Read(&m_FileHashMap->Nodes, sizeof(exl::util::HashTreeNode), hashTreeNodeCount);
            for (int i=  0; i < hashTreeNodeCount; i++) {
                EXL_DEBUG_PRINTF("hash tree node %d: hash %x\n", m_FileHashMap->Nodes[i].Value, m_FileHashMap->Nodes[i].Hash);
            }

            m_NowFileStart = -1;
            m_NowFileEnd = -1;
        }

        FSArchiveAccessor::~FSArchiveAccessor() {
            delete m_FileHashMap;
        }

        bool FSArchiveAccessor::OpenFile(exl::util::Hash hash) {
            u32 index = m_FileHashMap->Find(hash);
            if (index != exl::util::HashTree::NOT_FOUND) {
                u32 filePtrOffset = m_FileTableOffset + sizeof(EXL_OFFSET(File)) * index;
                m_Stream->SeekSet(filePtrOffset);
                EXL_OFFSET(File) fileInfoOffset;
                m_Stream->Read(&fileInfoOffset, sizeof(fileInfoOffset), 1);
                u32 fileInfoPos = fileInfoOffset.GetOffset(filePtrOffset);
                m_Stream->SeekSet(fileInfoPos);
                FileInfo fileInfo;
                m_Stream->Read(&fileInfo, sizeof(fileInfo), 1);
                m_NowFileStart = fileInfo.Data.GetOffset(fileInfoPos + offsetof(FileInfo, Data));
                m_NowFileEnd = m_NowFileStart + fileInfo.Size;
                m_Stream->SeekSet(m_NowFileStart);
                return true;
            }
            return false;
        }

        bool FSArchiveAccessor::SeekFile(s32 offset, exl::io::Stream::SeekOrigin origin) {
            if (m_NowFileStart != -1) {
                switch (origin) {
                    case exl::io::Stream::SeekOrigin::SET:
                    {
                        u32 absOffs = m_NowFileStart + offset;
                        if (absOffs <= m_NowFileEnd) {
                            m_Stream->SeekSet(absOffs);
                            return true;
                        }
                        break;
                    }
                    case exl::io::Stream::SeekOrigin::END:
                        if (offset < 0 && -offset < (m_NowFileEnd - m_NowFileStart)) {
                            m_Stream->SeekSet(m_NowFileEnd + offset);
                            return true;
                        }
                        break;
                    case exl::io::Stream::SeekOrigin::CUR:
                        m_Stream->SeekCur(offset);
                        return true;
                }
            }
            return false;
        }

        u32 FSArchiveAccessor::ReadFile(void* dest, u32 elementSize, u32 elementCount) {
            if (m_NowFileStart) {
                return m_Stream->Read(dest, elementSize, elementCount);
            }
            return -1;
        }
    }
}

#endif
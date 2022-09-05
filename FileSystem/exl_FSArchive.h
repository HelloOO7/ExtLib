#ifndef __EXL_FSARCHIVE_H
#define __EXL_FSARCHIVE_H

#include "exl_Types.h"
#include "Compression/exl_Compression.h"
#include "Util/exl_Offset.h"
#include "Util/exl_HashTree.h"
#include "Util/exl_HashCode.h"

#ifdef EXL_FSARC_DEBUG
#define EXL_FSARC_PRIVATE public
#else
#define EXL_FSARC_PRIVATE private
#endif

namespace exl {
    namespace fs {
        class File;

        struct Directory {
        EXL_FSARC_PRIVATE:
            EXL_OFFSET(char)                    Name;
            EXL_OFFSET(Directory)               Parent;
            u32                                 SubDirectoryCount;
            EXL_OFFSET(EXL_OFFSET(Directory))   SubDirectories;
            u32                                 FileCount;
            EXL_OFFSET(EXL_OFFSET(File))        Files;

            INLINE const char* GetName() {
                return Name.Get();
            }
            
            INLINE const Directory* GetParent() {
                return Parent.Get();
            }
        };

        struct FileInfo {
            EXL_OFFSET(char)                  Name;
            EXL_OFFSET(Directory)             Parent;
            size_t                            Size;
            EXL_OFFSET(void)                  Data;
            size_t                            UncompSize;
            exl::compress::CompressionType    Compression;
        };

        class File {
        EXL_FSARC_PRIVATE:
            FileInfo m_Info;

        public:
            INLINE const char* GetName() const {
                return m_Info.Name.Get();
            }
            
            INLINE const Directory* GetParent() const {
                return m_Info.Parent.Get();
            }

            INLINE size_t GetSize() const {
                return m_Info.Size;
            }

            INLINE const void* GetRawData() const {
                return m_Info.Data.Get();
            }

            INLINE size_t GetUncompSize() const {
                return m_Info.UncompSize;
            }

            INLINE exl::compress::CompressionType GetCompression() const {
                return m_Info.Compression;
            }
        };

        struct ArchiveHeader {
            u32                                 Magic;
            u32                                 FileVersion;
            EXL_OFFSET(exl::util::HashTree)     DirNameLookup;
            EXL_OFFSET(exl::util::HashTree)     FileNameLookup;
            u32                                 DirCount;
            EXL_OFFSET(EXL_OFFSET(Directory))   Directories;
            u32                                 FileCount;
            EXL_OFFSET(EXL_OFFSET(File))        Files;
        };

        class Archive {
        EXL_FSARC_PRIVATE:
            ArchiveHeader m_Header;

        public:
            const File* GetFile(exl::util::Hash hash) const;
            const Directory* GetDirectory(exl::util::Hash hash) const;

            INLINE const File* GetFile(const char* name) const {
                return GetFile(exl::util::HashCode(name));
            }

            INLINE const Directory* GetDirectory(const char* name) const {
                return GetDirectory(exl::util::HashCode(name));
            }

            INLINE static const Archive* Open(void* buffer) {
                return reinterpret_cast<Archive*>(buffer);
            }
        };
    }
}

#endif
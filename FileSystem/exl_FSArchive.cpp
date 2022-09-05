#ifndef __EXL_FSARCHIVE_CPP
#define __EXL_FSARCHIVE_CPP

#include "exl_Types.h"
#include "exl_FSArchive.h"
#include "Util/exl_HashTree.h"
#include "Util/exl_HashCode.h"

namespace exl {
    namespace fs {
        const File* Archive::GetFile(u32 hash) const {
            u32 index = m_Header.FileNameLookup.Get()->Find(hash);

            if (index != exl::util::HashTree::NOT_FOUND) {
                return m_Header.Files.Get()[index].Get();
            }

            return nullptr;
        }

        const Directory* Archive::GetDirectory(u32 hash) const {
            u32 index = m_Header.DirNameLookup.Get()->Find(hash);

            if (index != exl::util::HashTree::NOT_FOUND) {
                return m_Header.Directories.Get()[index].Get();
            }

            return nullptr;
        }
    }
}

#endif
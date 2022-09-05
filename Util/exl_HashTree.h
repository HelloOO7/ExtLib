#ifndef __EXL_HASHTREE_H
#define __EXL_HASHTREE_H

#include "exl_Types.h"
#include "exl_HashCode.h"
#include "Heap/exl_Allocator.h"

namespace exl {
    namespace util {
        struct HashTreeNode {
            u32 Hash;
            u32 Value;
        };

        struct HashTree {
            u32 NodeCount;
            HashTreeNode Nodes[0];

            static const u32 NOT_FOUND = 0xFFFFFFFF;

        public:
            u32 Find(u32 hash) const;

            EXL_PUBLIC INLINE u32 Find(const char* name) const {
                return Find(HashCode(name));
            }

            EXL_PUBLIC INLINE void* operator new(size_t size, exl::heap::Allocator* allocator, u32 NodeCount) {
				return allocator->Alloc(size + NodeCount * sizeof(HashTreeNode));
			}
        };
    }
}

#endif
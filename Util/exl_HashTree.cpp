#ifndef __EXL_HASHTREE_CPP
#define __EXL_HASHTREE_CPP

#include "exl_Types.h"
#include "exl_HashTree.h"

namespace exl {
    namespace util {
        u32 HashTree::Find(u32 hash) const {
            u32 low = 0;
            u32 high = NodeCount - 1;

            //java.util.Arrays.binarySearch
            while (low <= high) {
                u32 mid = (low + high) >> 1;
                u32 midVal = Nodes[mid].Hash;

                if (midVal < hash) {
                    low = mid + 1;
                }
                else if (midVal > hash) {
                    high = mid - 1;
                }
                else {
                    return Nodes[mid].Value;
                }
            }

            return HashTree::NOT_FOUND;
        }
    }
}

#endif
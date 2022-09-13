#ifndef __EXL_OSALLOCATOR_CPP
#define __EXL_OSALLOCATOR_CPP

#include "exl_OSAllocator.h"
#include "exl_DebugPrint.h"

#ifndef EXL_PLATFORM_GFL
#include <stdlib.h>
#endif

namespace exl {
    namespace heap {
        Allocator* OSAllocator::GetInstance()  {
            static Allocator* g_Instance;
            
            if (!g_Instance) {
                g_Instance = new OSAllocator();
            }
            return g_Instance;
        }

        void* OSAllocator::Alloc(size_t size) {
            if (size == 0) {
                return nullptr;
            }
            OSHeapHeader* mainBuf = static_cast<OSHeapHeader*>(OS_ALLOC(sizeof(OSHeapHeader) + size));
            if (!mainBuf) {
                EXL_DEBUG_PRINTF("OSAllocator allocation for size %d failed!", size);
                return 0;
            }
            #ifdef EXL_PLATFORM_GFL
            mainBuf->BlockSize = size;
            #endif
            mainBuf->Allocator = this;
            return static_cast<void*>(mainBuf + 1);
        }

        void* OSAllocator::Realloc(void* p, size_t newSize) {
            #ifdef EXL_PLATFORM_GFL
            //GFL_HeapResize is super dangerous! It fails if realloc should return a new pointer.
            //That's why we have extra 4 bytes in the Block to remember its size, as GFL does not retain it and I don't want to use the internal struct fields.
            OSHeapHeader* h = p ? (static_cast<OSHeapHeader*>(p) - 1) : nullptr; //do not try to find header if pointer is null
            size_t oldSize = h ? h->BlockSize : 0; //sizes are of the actual block, excluding OSHeapHeader
            if (oldSize != 0) {
                if (newSize > oldSize) {
                    void* newAlloc = OSAllocator::Alloc(newSize);
                    memcpy(newAlloc, p, oldSize);
                    OSAllocator::Free(p);
                    return newAlloc;
                }
                else if (newSize != oldSize) {
                    //block is smaller, can simply shrink it
                    //resize the header pointer since that's where the GFL alloc starts
                    GFL_HeapResize(h, sizeof(OSHeapHeader) + newSize);
                    return p;
                }
                else {
                    return p;
                }
            }
            else {
                return Alloc(newSize);
            }
            #else
            p = p ? (static_cast<OSHeapHeader*>(p) - 1) : p; //do not try to find header if pointer is null
            return static_cast<void*>
            (
                static_cast<OSHeapHeader*>(
                    realloc(p, sizeof(OSHeapHeader) + newSize)
                ) + 1
            ); //the old block already has the allocator*, no need to redo
            #endif
        }

        void OSAllocator::Free(void* p) {
            if (p) {
                OS_FREE(static_cast<OSHeapHeader*>(p) - 1);
            }
        }
    }
}

#endif
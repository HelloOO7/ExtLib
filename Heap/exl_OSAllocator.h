#ifndef __EXL_OSALLOCATOR_H
#define __EXL_OSALLOCATOR_H

#include "exl_Allocator.h"

#ifdef EXL_PLATFORM_GFL
extern "C" {
    extern void* GFL_HeapAllocate(u16 heapId, u32 size, int32_t calloc, const char* sourceFile, u16 lineNo);
    extern void  GFL_HeapFree(void* p);
    extern void  GFL_HeapResize(void* p, u32 newSize);
}
#undef bool

#define OS_ALLOC(size) GFL_HeapAllocate(1, size, false, __FILE__, __LINE__)
#define OS_FREE(p) GFL_HeapFree(p)

#else
#include <stdlib.h>

#define OS_ALLOC(size) malloc(size)
#define OS_FREE(p) free(p)

#endif

namespace exl {
    namespace heap {
        class OSAllocator : public Allocator {
        private:
            struct OSHeapHeader {
                #ifdef EXL_PLATFORM_GFL
                size_t BlockSize;
                #endif
                EXL_ALLOCATOR_BLOCK_END_REQUIRE;
            };
            #ifdef EXL_PLATFORM_GFL
            u16 m_HeapID;
            #endif
        
        public:
            EXL_PUBLIC OSAllocator();
            #ifdef EXL_PLATFORM_GFL
            EXL_PUBLIC OSAllocator(u16 heapId);
            #endif

            EXL_PUBLIC void* Alloc(size_t size) override;
	
			EXL_PUBLIC void* Realloc(void* p, size_t newSize) override;

			EXL_PUBLIC void Free(void* p) override;

            EXL_PUBLIC static Allocator* GetInstance();

			EXL_PUBLIC INLINE void* operator new(size_t size) {
				return OS_ALLOC(size);
			}

            EXL_PUBLIC INLINE void* operator new(size_t size, void* p) {
				return p;
			}

			EXL_PUBLIC INLINE void operator delete(void* p) {
				OS_FREE(p);
			}
        };
    }
}

#endif
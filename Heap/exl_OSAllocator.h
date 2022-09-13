#ifndef __EXL_OSALLOCATOR_H
#define __EXL_OSALLOCATOR_H

#include "exl_Allocator.h"

#ifdef EXL_PLATFORM_GFL
extern "C" {
    extern void* GFL_HeapAllocate(int heapId, u32 size, int calloc, const char* sourceFile, u16 lineNo);
    extern void  GFL_HeapFree(void* p);
    extern void  GFL_HeapResize(void* p, size_t newSize);
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
        
        public:
            EXL_PUBLIC void* Alloc(size_t size);
	
			EXL_PUBLIC void* Realloc(void* p, size_t newSize);

			EXL_PUBLIC void Free(void* p);

            EXL_PUBLIC static Allocator* GetInstance();

			EXL_PUBLIC INLINE void* operator new(size_t size) {
				return OS_ALLOC(size);
			}

			EXL_PUBLIC INLINE void operator delete(void* p) {
				OS_FREE(p);
			}
        };
    }
}

#endif
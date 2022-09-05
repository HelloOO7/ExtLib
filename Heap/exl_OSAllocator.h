#ifndef __EXL_OSALLOCATOR_H
#define __EXL_OSALLOCATOR_H

#include "exl_Allocator.h"

#ifdef EXL_PLATFORM_GFL
#include "gfl/core/gfl_heap.h"
#undef bool

#define OS_ALLOC(size) GFL_MALLOC(1, size)
#define OS_FREE(p) GFL_FREE(p)

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
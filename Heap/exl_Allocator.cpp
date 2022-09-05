#ifndef __EXL_ALLOCATOR_CPP
#define __EXL_ALLOCATOR_CPP

#include "exl_Types.h"
#include "exl_Allocator.h"
#include "exl_DebugPrint.h"

namespace exl {
	namespace heap {
		void* Allocator::FillAlloc(size_t size, u8 value) {
			void* data = Alloc(size);
			if (data) {
				memset(data, value, size);
			}
			else if (size) {
				EXL_DEBUG_PRINTF("FillAlloc failed due to unsuccessful allocation!!");
			}
			return data;
		}

		#define GET_ALLOCATOR_REFERENCE(p) (reinterpret_cast<Allocator**>(p)[-1])

		Allocator* Allocator::GetAllocator(void* p) {
			return GET_ALLOCATOR_REFERENCE(p);
		}

		void* Allocator::ReallocStatic(void* p, size_t newSize) {
			if (p) {
				return GET_ALLOCATOR_REFERENCE(p)->Realloc(p, newSize);
			}
			return nullptr;
		}

		void Allocator::FreeStatic(void* p) {
			if (p) {
				GET_ALLOCATOR_REFERENCE(p)->Free(p);
			}
		}
	}
}

#endif
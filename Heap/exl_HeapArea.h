/**
 * @file exl_HeapArea.h
 * @author Hello007
 * @brief ExtLib Heap manager.
 * @version 0.1
 * @date 2022-03-19
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __EXL_HEAPAREA_H
#define __EXL_HEAPAREA_H

namespace exl {
	namespace heap {
		class HeapArea;
	}
}

#include "exl_Types.h"
#include "exl_MemOperators.h"
#include "exl_DllExport.h"
#include <cstring>

namespace exl {
	namespace heap {
		/**
		 * @brief Simple and versatile heap memory manager for dynamic allocation.
		 */
		class HeapArea : public Allocator {
		private:
			struct BlockHeader {
				size_t 			Size;
				BlockHeader* 	Next;
				int 			AlignPadding;
				EXL_ALLOCATOR_BLOCK_END_REQUIRE;

				INLINE void SetNext(BlockHeader* next) {
					if (next == this) {
						while (true) { //BAD!!
							;
						}
					}
					Next = next;
				}

				INLINE void* EndAddress() {
					return reinterpret_cast<char*>(this + 1) + Size;
				}
			};

			static_assert(sizeof(BlockHeader) % 8 == 0);

			void*  m_HeapBase;
			size_t m_TotalSize;

			BlockHeader* m_FreeBlocks;

			bool  m_DeleteHeapOnDestroy;
			//Keep debug fields for size consistency in dynamically linked modules
			//#ifdef DEBUG
			const char* m_MgrName;
			u32   m_AllocCount;
			//#endif

			void InsertFreeBlock(BlockHeader* block);

		public:
			/**
			 * @brief Wraps a block of pre-allocated heap space into an anonymous HeapArea instance.
			 * 
			 * @param heap Pointer to the heap space.
			 * @param heapSize Size of the heap space.
			 */
			EXL_PUBLIC HeapArea(void* heap, size_t heapSize);

			/**
			 * @brief Wraps a block of pre-allocated heap space into a named HeapArea instance.
			 * 
			 * @param areaName Name of the area for debugging.
			 * @param heap Pointer to the heap space.
			 * @param heapSize Size of the heap space.
			 */
			EXL_PUBLIC HeapArea(const char* areaName, void* heap, size_t heapSize);

			EXL_PUBLIC static HeapArea* CreateIn(const char* areaName, void* heap, size_t size);
			EXL_PUBLIC static INLINE HeapArea* CreateIn(void* heap, size_t size) {
				return CreateIn(nullptr, heap, size);
			}

			EXL_PUBLIC static HeapArea* CreateFrom(exl::heap::Allocator* allocator, const char* areaName, size_t size);

			EXL_PUBLIC static INLINE HeapArea* CreateFrom(exl::heap::Allocator* allocator, size_t size) {
				return CreateFrom(allocator, nullptr, size);
			}

			~HeapArea();

			EXL_PUBLIC void* Alloc(size_t size);
	
			EXL_PUBLIC void* Realloc(void* p, size_t newSize);

			EXL_PUBLIC void Free(void* p);

			#ifdef DEBUG
			/**
			 * @brief Get the size of the wrapped heap.
			 * 
			 * @return Heap size in bytes.
			 */
			INLINE size_t GetHeapSize() {
				return m_TotalSize;
			}

			/**
			 * @brief Get the pointer to the wrapped heap.
			 * 
			 * @return m_HeapBase
			 */
			INLINE void* GetHeapPtr() {
				return m_HeapBase;
			}
			#endif

			/**
			 * @brief New operator for initialization of a HeapArea in a preallocated block.
			 * 
			 * @param size Size of the block, should be sizeof(HeapArea).
			 * @param buf The buffer to initialize the HeapArea in.
			 * @return 'buf' as the new operator result.
			 */
			EXL_PUBLIC INLINE void* operator new(size_t size, void* buf) {
				return buf;
			}

			/**
			 * @brief Placeholder delete operator for HeapArea. NOT TO BE USED.
			 * 
			 * HeapArea should be freed using the counterpart function that created its memory block.
			 */
			EXL_PUBLIC INLINE void operator delete(void* p) {
				while (true) {
					; //use free externally
				}
			}
		};
	}
}

#endif
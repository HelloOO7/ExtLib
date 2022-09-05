#ifndef __EXL_HEAPAREA_CPP
#define __EXL_HEAPAREA_CPP

#include "exl_HeapArea.h"
#include "exl_Types.h"
#include "exl_DebugPrint.h"
#include "exl_Assert.h"

namespace exl {
	namespace heap {
		#define VOIDPTR_BYTEADD(ptr, bytes) static_cast<void*>((static_cast<u8*>((ptr))) + ((bytes) / sizeof(u8)))
		#define VOIDPTR_BYTESUB(ptr, bytes) static_cast<void*>((static_cast<u8*>((ptr))) - ((bytes) / sizeof(u8)))

		#define SCROLL_HEAP_HEADER(ptr) VOIDPTR_BYTESUB((ptr), HEAP_HEADER_SIZE)

		//vtable at 0x2235694
		HeapArea::HeapArea(void* heap, size_t size) : HeapArea::HeapArea(NULL, heap, size) {
			
		}

		HeapArea::HeapArea(const char* mgrName, void* heap, size_t size) {
			#ifdef DEBUG
			if (mgrName) {
				m_MgrName = mgrName;
			}
			else {
				m_MgrName = "<?>";
			}
			m_AllocCount = 0;
			#endif
			m_HeapBase = heap;
			m_DeleteHeapOnDestroy = false;
			m_HeapMax = VOIDPTR_BYTEADD(heap, size);
			
			HeapHeader* header = static_cast<HeapHeader*>(m_HeapBase);
			header->Allocator = this;
			header->Size = HEAP_HEADER_SIZE;
			header->PrevOffset = 0;
			header->NextOffset = size;
			EXL_DEBUG_PRINTF("HeapArea::ctor %s | Done initializing. Heap size: %d.\n", m_MgrName, size);
			EXL_DEBUG_PRINTF("HeapArea::ctor %s | Initialized heap area at 0x%p, end at 0x%p\n", m_MgrName, m_HeapBase, m_HeapMax);
		}

		HeapArea::~HeapArea() {
			if (m_DeleteHeapOnDestroy) {
				if (m_HeapBase) {
					GetAllocator(m_HeapBase)->Free(m_HeapBase);
				}
			}
		}

		HeapArea* HeapArea::CreateFrom(exl::heap::Allocator* allocator, const char* areaName, size_t size) {
			HeapArea* area = new(allocator->Alloc(sizeof(HeapArea))) HeapArea(areaName, allocator->Alloc(size), size);
			area->m_DeleteHeapOnDestroy = true;
			return area;
		}

		void* HeapArea::Alloc(size_t size) {
			EXL_DEBUG_PRINTF("HeapArea::Alloc %s | Beginning allocation. Current number of allocations: %d.\n", m_MgrName, m_AllocCount);
			EXL_DEBUG_PRINTF("HeapArea::Alloc %s | Aligning allocation size 0x%x to 0x%x.\n", m_MgrName, size, ALIGN16(size));
			size_t reqSize = ALIGN16(size) + HEAP_HEADER_SIZE;
			void* heapPtr = m_HeapBase;
			HeapHeader* header;

			u32 allocBlk = 1;
			while (heapPtr < m_HeapMax) {
				header = static_cast<HeapHeader*>(heapPtr);

				size_t next = header->NextOffset;
				if (next - header->Size >= reqSize) {
					#ifdef DEBUG
					m_AllocCount++;
					#endif

					if (header->PrevOffset || header->Size != HEAP_HEADER_SIZE) {
						EXL_DEBUG_PRINTF("HeapArea::Alloc %s | Allocating at block index %d size 0x%x\n", m_MgrName, allocBlk, reqSize);

						HeapHeader* prevHeader = header;

						void* newHeapPtr = VOIDPTR_BYTEADD(heapPtr, prevHeader->Size);
						
						HeapHeader* newHeader = static_cast<HeapHeader*>(newHeapPtr);
						newHeader->Allocator = this;
						newHeader->Size = reqSize;
						newHeader->PrevOffset = prevHeader->Size;
						newHeader->NextOffset = prevHeader->NextOffset - prevHeader->Size;

						void* nextHeapPtr = VOIDPTR_BYTEADD(heapPtr, prevHeader->NextOffset);
						if (nextHeapPtr < m_HeapMax) {
							EXL_DEBUG_PRINTF("HeapArea::Alloc %s | Fixing up next header.\n", m_MgrName);
							HeapHeader* nextHeader = static_cast<HeapHeader*>(nextHeapPtr);
							nextHeader->PrevOffset = newHeader->PrevOffset - prevHeader->Size;
						}

						prevHeader->NextOffset = prevHeader->Size;

						EXL_DEBUG_PRINTF("HeapArea::Alloc %s | Done.\n", m_MgrName);

						return VOIDPTR_BYTEADD(newHeapPtr, HEAP_HEADER_SIZE);
					}
					else { //use existing initial header
						EXL_DEBUG_PRINTF("HeapArea::Alloc %s | First allocation! - total block size 0x%x\n", m_MgrName, reqSize);
						header->Size = reqSize;
						return VOIDPTR_BYTEADD(heapPtr, HEAP_HEADER_SIZE);
					}
				}
				allocBlk++;

				heapPtr = VOIDPTR_BYTEADD(heapPtr, header->NextOffset);
				if (header->NextOffset) {
					//EXL_DEBUG_PRINTF("skipblk add %d\n", header->NextOffset);
				}
			}
			EXL_DEBUG_PRINTF("HeapArea::Alloc %s | Memory allocation failure!! | Heap start: %p / Size: 0x%x", m_MgrName, m_HeapBase, static_cast<u8*>(m_HeapMax) - static_cast<u8*>(m_HeapBase));
			volatile char ALLOC_FAIL_MARKER[] = "HEAP SPACE FULL!!";
			EXL_EXIT(0xA110CE44);

			return NULL;
		}

		void* HeapArea::Realloc(void* p, size_t newSize) {
			if (p) {
				size_t fullSize = newSize + HEAP_HEADER_SIZE;
				HeapHeader* header = static_cast<HeapHeader*>(SCROLL_HEAP_HEADER(p));
				if (fullSize <= header->NextOffset) {
					header->Size = fullSize;
					return p;
				}
			}
			
			void* newLoc = HeapArea::Alloc(newSize);
			//copy in blocks of size_t bytes (usually 32/64 by platform)
			size_t* dst = static_cast<size_t*>(newLoc);
			size_t* src = static_cast<size_t*>(p);
			for (size_t idx = 0; idx < newSize; idx += sizeof(size_t)) {
				*dst = *src;
				dst++;
				src++;
			}
			HeapArea::Free(p);
			return newLoc;
		}

		void HeapArea::Free(void* p) {
			if (!p) {
				return;
			}
			EXL_DEBUG_PRINTF("HeapArea::Free BEGIN | Now alloc count %d\n", m_AllocCount);
			void* headerPtr = SCROLL_HEAP_HEADER(p);
			void* nextHeaderPtr = 0;
			void* prevHeaderPtr = 0;

			HeapHeader* header = static_cast<HeapHeader*>(headerPtr);
			nextHeaderPtr = VOIDPTR_BYTEADD(headerPtr, header->NextOffset);
			if (header->PrevOffset) {
				prevHeaderPtr = VOIDPTR_BYTESUB(headerPtr, header->PrevOffset);
			}
			EXL_DEBUG_PRINTF("HeapArea::Free | Is first alloc %d\n", headerPtr == m_HeapBase);

			if (nextHeaderPtr < m_HeapMax) {
				EXL_DEBUG_PRINTF("HeapArea::Free | Syncing next header.\n");
				if (prevHeaderPtr) {
					HeapHeader* nextHeader = static_cast<HeapHeader*>(nextHeaderPtr);

					nextHeader->PrevOffset += header->PrevOffset;
				}
				else {
					//If this is zero, this is the first allocated memory block
					//In such a case, the old header persists, but its size is reset to allow allocations
					header->Size = HEAP_HEADER_SIZE;
				}
			}

			if (prevHeaderPtr) {
				EXL_DEBUG_PRINTF("HeapArea::Free | Syncing prev header.\n");
				HeapHeader* prevHeader = static_cast<HeapHeader*>(prevHeaderPtr);
				prevHeader->NextOffset += header->NextOffset;
			}

			#ifdef DEBUG
			m_AllocCount--;
			#endif
		}
	}
}

#endif
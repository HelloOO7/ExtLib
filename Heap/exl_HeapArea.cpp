#ifndef __EXL_HEAPAREA_CPP
#define __EXL_HEAPAREA_CPP

#include "exl_HeapArea.h"
#include "exl_Types.h"
#include "exl_DebugPrint.h"
#include "exl_Assert.h"

namespace exl {
	namespace heap {
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
			m_TotalSize = size;
			m_DeleteHeapOnDestroy = false;
			
			BlockHeader* header = static_cast<BlockHeader*>(heap);
			header->Allocator = this;
			header->Size = size - sizeof(BlockHeader);
			header->Next = nullptr;
			m_FreeBlocks = header;
			EXL_DEBUG_PRINTF("HeapArea::ctor %s | Done initializing. Heap size: %d.\n", m_MgrName, size);
			EXL_DEBUG_PRINTF("HeapArea::ctor %s | Initialized heap area at %p, end at %p\n", m_MgrName, heap, static_cast<char*>(heap) + size);
		}

		HeapArea::~HeapArea() {
			if (m_DeleteHeapOnDestroy) {
				if (m_HeapBase) {
					GetAllocator(m_HeapBase)->Free(m_HeapBase);
				}
			}
		}

		HeapArea* HeapArea::CreateIn(const char* areaName, void* heap, size_t size) {
			char* newStart = reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(heap) + sizeof(HeapArea) + 7) & ~7);
			return new(heap) HeapArea(areaName, newStart, size - (reinterpret_cast<char*>(heap) - newStart));
		}

		HeapArea* HeapArea::CreateFrom(exl::heap::Allocator* allocator, const char* areaName, size_t size) {
			HeapArea* area = new(allocator->Alloc(sizeof(HeapArea))) HeapArea(areaName, allocator->Alloc(size), size);
			area->m_DeleteHeapOnDestroy = true;
			return area;
		}

		void HeapArea::InsertFreeBlock(BlockHeader* block) {
			BlockHeader* nextFree = m_FreeBlocks;
			BlockHeader* prevFree = nullptr;
			while (nextFree && nextFree < block) {
				prevFree = nextFree;
				nextFree = nextFree->Next;
			}
			EXL_DEBUG_PRINTF("HeapArea::InsertFreeBlock | begin prev free %p next free %p\n", prevFree, nextFree);
			BlockHeader* last = block;
			if (prevFree) {
				if (prevFree->EndAddress() == block) {
					prevFree->Size += sizeof(*block) + block->Size; //coalesce from left	
					last = prevFree;
				}
				else {
					block->SetNext(prevFree->Next);
					prevFree->SetNext(block);
				}
			}
			else {
				block->SetNext(m_FreeBlocks);
				m_FreeBlocks = block;
			}
			if (last->Next && last->EndAddress() == last->Next) {
				last->Size += sizeof(*last->Next) + last->Next->Size;
				last->SetNext(last->Next->Next); //coalesce right
			}
		}

		INLINE static constexpr size_t AlignAllocSize(size_t size) {
			return (size + 7) & ~7;
		}

		void* HeapArea::Alloc(size_t size) {
			EXL_DEBUG_PRINTF("HeapArea::Alloc %s | Beginning allocation. Current number of allocations: %d.\n", m_MgrName, m_AllocCount);
			EXL_DEBUG_PRINTF("HeapArea::Alloc %s | Aligning allocation size 0x%x to 0x%x.\n", m_MgrName, size, AlignAllocSize(size));
			
			size = AlignAllocSize(size);

			BlockHeader* prevBlock = nullptr;
			BlockHeader* block = m_FreeBlocks; //first fit
			while (block) {
				if (block->Size >= size) {
					break;
				}
				prevBlock = block;
				block = block->Next;
			}

			if (!block) {
				EXL_DEBUG_PRINTF("HeapArea::Alloc %s | Memory allocation failure!! | Heap start: %p / Size: 0x%x", m_MgrName, m_HeapBase, GetHeapSize());
				volatile char ALLOC_FAIL_MARKER[] = "HEAP SPACE FULL!!";
				EXL_EXIT(0xA110CE44);

				return nullptr;
			}
			else {
				if (prevBlock) {
					prevBlock->SetNext(block->Next);
				}
				else {
					m_FreeBlocks = block->Next;
				}
				if (block->Size >= size + sizeof(BlockHeader) + AlignAllocSize(1)) {
					BlockHeader* newFreeBlock = reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(block + 1) + size);
					newFreeBlock->Size = block->Size - size - sizeof(BlockHeader);
					InsertFreeBlock(newFreeBlock);

					block->Size = size;
				}
				//else keep block size wasted
				block->Allocator = this;
				#ifdef DEBUG
				m_AllocCount++;
				#endif
				return block + 1;
			}
		}

		void* HeapArea::Realloc(void* p, size_t newSize) {
			if (p) {
				size_t alignSize = AlignAllocSize(newSize);
				BlockHeader* header = static_cast<BlockHeader*>(p) - 1;
				if (newSize <= header->Size) {
					if (header->Size - alignSize >= sizeof(BlockHeader) + AlignAllocSize(1)) {
						size_t newFreeSize = header->Size - alignSize - sizeof(BlockHeader) - AlignAllocSize(1);
						header->Size = alignSize;
						BlockHeader* free2 = reinterpret_cast<BlockHeader*>(header->EndAddress());
						free2->Size = newFreeSize;
						InsertFreeBlock(free2);
						return p;
					}
					else {
						return p; //keep header as is - we'll just have up to 23 bytes of wasted space
					}
				}
				else {
					size_t remainSize = alignSize - header->Size;

					BlockHeader* f = m_FreeBlocks;
					BlockHeader* pf = nullptr;
					while (f && f < header) {
						pf = f;
						f = f->Next;
					}
					if (f && f == header->EndAddress() && f->Size + sizeof(BlockHeader) >= remainSize) {
						EXL_DEBUG_PRINTF("Compact realloc possible, performing... (remainsize %d)\n", remainSize);
						//move the next free block a bit
						size_t newFreeSize = f->Size + sizeof(BlockHeader) - remainSize;
						if (newFreeSize >= sizeof(BlockHeader) + AlignAllocSize(1)) {
							BlockHeader* newf = reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(f) + remainSize);
							newf->Size = f->Size - remainSize;
							newf->SetNext(f->Next);
							if (pf) {
								pf->SetNext(newf);
							}
							else {
								m_FreeBlocks = newf;
							}
							header->Size = alignSize;
						}
						else {
							header->Size += f->Size + sizeof(BlockHeader);
							if (pf) {
								pf->SetNext(f->Next);
							}
							else {
								m_FreeBlocks = f->Next;
							}
						}
						return p;
					}
				}
			}

			Fallback:
			
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
			InsertFreeBlock(reinterpret_cast<BlockHeader*>(p) - 1);

			#ifdef DEBUG
			m_AllocCount--;
			#endif
		}
	}
}

#endif
#include "Media/exl_CinepakDecoder.h"
#include "IO/exl_FileStream.h"
#include "IO/exl_BufferedStream.h"
#include "Heap/exl_OSAllocator.h"
#include "Heap/exl_HeapArea.h"

#include <stdio.h>

int main(int argc, char** argv) {
    int heapSize = 0x80000;
    exl::heap::Allocator* osAlloc = exl::heap::OSAllocator::GetInstance();
    void* heap = osAlloc->Alloc(heapSize);

    exl::heap::HeapArea* area = exl::heap::HeapArea::CreateIn("TestHeap", heap, heapSize);

    /*void* a = area->FillAlloc(123, 'A');
    void* b = area->FillAlloc(123, 'B');
    void* c = area->FillAlloc(123, 'C');
    void* d = area->FillAlloc(123, 'D');
    void* e = area->FillAlloc(123, 'E');
    void* f = area->FillAlloc(123, 'F');

    area->Free(c);
    area->Free(e);
    area->Free(d);

    void* _0 = area->FillAlloc(123 * 4, '0');
    area->Free(a);
    void* _1 = area->FillAlloc(140, '1');
    void* _2 = area->FillAlloc(123, '2');

    area->Free(_1);
    area->Free(_2);
    area->Free(b);
    area->Free(f);
    area->Free(_0);

    void* ea = area->FillAlloc(2048, 0xEA);

    void* bb = area->FillAlloc(128, 0xBB);
    void* cc = area->FillAlloc(128, 0xCC);
    area->Free(bb);
    void* ea2 = area->Realloc(ea, 2048+128-7);
    memset(ea2, 0xEB, 2048+128-7);
    area->Free(ea2);
    void* ec = area->FillAlloc(2048+128, 0xEC);*/

    void* allocations[128];
    memset(allocations, 0, sizeof(allocations));
    int alcount = 0;

    for (int i = 0; i < 128; i++) {
        if (alcount != 128 && (rand() & 1 || !alcount)) {
            void* a = area->Alloc(rand() % 128 + 128);
            for (int i = 0; i < 128; i++) {
                if (!allocations[i]) {
                    allocations[i] = a;
                    alcount++;
                    break;
                }
            }
        }
        else {
            bool real = rand() & 1;

            auto idx = rand() % alcount;
            int ali = 0;
            for (int i = 0; i < 128; i++) {
                if (allocations[i]) {
                    if (ali == idx) {
                        if (real) {
                            allocations[i] = area->Realloc(allocations[i], rand() % 128 + 128);
                        }
                        else {
                            area->Free(allocations[i]);
                            allocations[i] = nullptr;
                            alcount--;
                        }
                        break;
                    }
                    ali++;
                }
            }
        }
    }

    exl::io::FileStream out("heap.bin", exl::io::FileStream::READ_WRITE);
    out.Write(heap, 1, heapSize);
    out.Close();

    osAlloc->Free(heap);
    return 0;
}
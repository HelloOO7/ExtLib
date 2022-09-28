#include "Media/exl_CinepakDecoder.h"
#include "IO/exl_FileStream.h"
#include "IO/exl_BufferedStream.h"
#include "Heap/exl_OSAllocator.h"
#include "Heap/exl_HeapArea.h"

#include <stdio.h>

int main(int argc, char** argv) {
    int heapSize = 0x80000;
    exl::heap::Allocator* osAlloc = exl::heap::OSAllocator::GetInstance();
    void* alal = osAlloc->Alloc(sizeof(exl::heap::HeapArea));
    void* heap = osAlloc->Alloc(heapSize);
    printf("Allocation done.\n");
    exl::heap::Allocator* allocator = new (alal) exl::heap::HeapArea(heap, heapSize);
    exl::io::FileStream* fileStream = new (allocator) exl::io::FileStream("D:/_REWorkspace/pokescript_genv/codeinjection_new/CinePlayer/vid/GameFreak.avi");
    //exl::io::BufferedStream* bufStream = new (allocator) exl::io::BufferedStream(fileStream, allocator, 0x8000);

    exl::media::CinepakDecoder* decoder = new(allocator) exl::media::CinepakDecoder(allocator, fileStream, EXL_CINEPAK_VIDEO);

    u32 w;
    u32 h;
    decoder->GetVideoDimensions(&w, &h);
    printf("Stream dimensions %dx%d\n", w, h);

    void* decodeBuf = allocator->Alloc(256 * 192 * 2);
    
    const char* basePath = "D:/_REWorkspace/pokescript_genv/codeinjection_new/extlib/Tests/framesCp/";
    char* path = (char*)osAlloc->Alloc(strlen(basePath) + 7 + 1);

    for (int frame = 0; frame < 60; frame++) {
        if (!decoder->NextFrame()) {
            printf("Decode error! frame %d\n", frame);
            return 1;
        }
        decoder->DecodeFrameRGB5A1(decodeBuf, 256, 192);
        
        sprintf(path, "%s%03d.bin", basePath, frame);

        exl::io::FileStream* outFile = new(osAlloc) exl::io::FileStream(path, exl::io::FileStream::READ_WRITE);
        outFile->Write(decodeBuf, 1, 256 * 192 * 2);
        outFile->Close();
        delete outFile;
    }
    osAlloc->Free(path);
    return 0;
}
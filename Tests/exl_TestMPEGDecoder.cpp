#include "Media/exl_MPEGDecoder.h"
#include "IO/exl_FileStream.h"
#include "Heap/exl_OSAllocator.h"
#include "Heap/exl_HeapArea.h"

#include <stdio.h>

int main(int argc, char** argv) {
    exl::heap::Allocator* osAlloc = exl::heap::OSAllocator::GetInstance();
    void* alal = osAlloc->Alloc(sizeof(exl::heap::HeapArea));
    void* heap = osAlloc->Alloc(0x47000);
    printf("Allocation done.\n");
    exl::heap::Allocator* allocator = new (alal) exl::heap::HeapArea(heap, 0x49000);
    exl::io::FileStream* fileStream = new (allocator) exl::io::FileStream("D:/_REWorkspace/CTRMapProjects/PMC/vfs/data/video/EleventhDoctorDanceoff.morbiclip");

    exl::media::MPEGDecoder* decoder = new(allocator) exl::media::MPEGDecoder(allocator, fileStream);

    u32 w;
    u32 h;
    decoder->GetVideoDimensions(&w, &h);
    printf("Stream dimensions %dx%d\n", w, h);

    void* decodeBuf = allocator->Alloc(256 * 192 * 2);
    
    const char* basePath = "D:/_REWorkspace/pokescript_genv/codeinjection_new/extlib/Tests/frames/";
    char* path = (char*)osAlloc->Alloc(strlen(basePath) + 7 + 1);

    /*bool seekResult = decoder->SeekTime(EXL_TO_MPEG_DECIMAL(4046.71), true);
    printf("Seek %d\n", seekResult);*/
    printf("Now time %d frame %d\n", EXL_FROM_MPEG_DECIMAL(decoder->GetTime()), decoder->GetFrame());

    for (int frame = 0; frame < 60; frame++) {
        decoder->DecodeFrameRGB5A1(decodeBuf, 256, 192);
        
        sprintf(path, "%s%03d.bin", basePath, frame);

        exl::io::FileStream* outFile = new(osAlloc) exl::io::FileStream(path, exl::io::FileStream::READ_WRITE);
        outFile->Write(decodeBuf, 1, 256 * 192 * 2);
        outFile->Close();
        delete outFile;
    }
    osAlloc->Free(path);
}
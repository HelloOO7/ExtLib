#include <stdlib.h>

#define EXL_FSARC_DEBUG

#include "exl_Types.h"
#include "FileSystem/exl_FSArchive.h"
#include "Compression/exl_Compression.h"
#include "Compression/exl_Decompressor.h"
#include "Util/exl_HashCode.h"
#include "Heap/exl_Allocator.h"
#include "Heap/exl_OSAllocator.h"
#include "IO/exl_Stream.h"
#include "IO/exl_FileStream.h"
#include "FileSystem/exl_FSArchiveAccessor.h"
#include "Heap/exl_HeapArea.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Not enough arguments!\n\n");

        printf("Usage:\n");
        printf("    exl_TestFsArchive <archive path>                |    Dump archive info\n");
        printf("    exl_TestFsArchive <archive path> <filename>     |    Extract file from archive\n");

        return 1;
    }

    exl::heap::Allocator* osAllocator = exl::heap::OSAllocator::GetInstance();
    void* mgrAllocatorArea = osAllocator->Alloc(1000000);
    exl::heap::Allocator* allocator = new(osAllocator) exl::heap::HeapArea(mgrAllocatorArea, 1000000);

    const char* path = argv[1];

    exl::io::Stream* stream = new(allocator) exl::io::FileStream(path);

    if (argc == 3) {
        //extract file
        const char* filePath = argv[2];

        exl::fs::FSArchiveAccessor* accessor = new(allocator) exl::fs::FSArchiveAccessor(stream, allocator);

        if (accessor->OpenFile(filePath)) {
            printf("openfile success!");
            delete accessor;
            delete stream;
           /* FILE* outstream = fopen("out.bin", "wb+");

            const void* fileData = file->GetRawData();
            switch (file->GetCompression()) {
                case exl::compress::CompressionType::NONE:
                    fwrite(fileData, 1, file->GetSize(), outstream);

                    break;
                default:
                    void* uncomp = malloc(file->GetUncompSize());

                    exl::compress::Decompressor::Decompress(file->GetCompression(), fileData, uncomp, file->GetSize(), file->GetUncompSize());

                    fwrite(uncomp, 1, file->GetUncompSize(), outstream);

                    free(uncomp);

                    break;
            }

            fclose(outstream);*/
        }
        else {
            printf("File %s not found in archive!\n", filePath);
            return 2;
        }
    }
    else {
        stream->SeekEnd(0);
        size_t len = stream->Tell();

        stream->SeekSet(0);

        void* buf = allocator->Alloc(len);
        stream->Read(buf, 1, len);

        const exl::fs::Archive* arc = exl::fs::Archive::Open(buf);

        //dump archive info
        printf("Dumping archive %s (size %d)\n", path, len);
        printf("Magic: %.4s\n", &arc->m_Header.Magic);
        printf("Version: %d\n\n", arc->m_Header.FileVersion);
        
        printf("Directory count: %d\n", arc->m_Header.DirCount);
        printf("Directories:\n");
        for (int i = 0; i < arc->m_Header.DirCount; i++) {
            const exl::fs::Directory* dir = arc->m_Header.Directories.Get()[i].Get();
            printf("Name: %s\n", dir->Name.Get());
            if (dir->Parent.Get()) {
                printf("Parent directory: %s\n", dir->Parent.Get()->Name.Get());
            }
            for (u32 childIndex = 0; childIndex < dir->FileCount; childIndex++) {
                printf("Child file %s\n", dir->Files.Get()[childIndex].Get()->m_Info.Name.Get());
            }
            printf("\n");
        }

        printf("File count: %d\n", arc->m_Header.FileCount);
        printf("Files:\n");
        for (int i = 0; i < arc->m_Header.FileCount; i++) {
            const exl::fs::File* file = arc->m_Header.Files.Get()[i].Get();
            printf("Name: %s\n", file->m_Info.Name.Get());
            if (file->m_Info.Parent.Get()) {
                printf("Parent directory: %s\n", file->m_Info.Parent.Get()->Name.Get());
            }
            printf("Size: %d (uncompressed: %d)\n", file->m_Info.Size, file->m_Info.UncompSize);
            printf("Compression: %d\n\n", file->m_Info.Compression);
        }
    }

    return 0;
}
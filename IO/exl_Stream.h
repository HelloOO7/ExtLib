#ifndef __EXL_STREAM_H
#define __EXL_STREAM_H

#include "exl_Types.h"
#include "exl_DllExport.h"

namespace exl {
    namespace io {
        class Stream {
        public:
            enum class SeekOrigin {
                SET,
                CUR,
                END
            };

        public:
            ~Stream() {
                Close();
            }

            virtual u32 Read(void* dest, u32 elementSize, u32 elementCount) = 0;

            virtual bool Seek(s32 pos, SeekOrigin origin) = 0;

            EXL_PUBLIC INLINE bool SeekSet(s32 pos) {
                return Seek(pos, SeekOrigin::SET);
            }

            EXL_PUBLIC INLINE bool SeekCur(s32 pos) {
                return Seek(pos, SeekOrigin::CUR);
            }

            EXL_PUBLIC INLINE bool SeekEnd(s32 pos) {
                return Seek(pos, SeekOrigin::END);
            }

            virtual u32 Tell() = 0;

            virtual void Close() {}
        };
    }
}

#endif
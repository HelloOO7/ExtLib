#ifndef __EXL_MPEGDECODER_H
#define __EXL_MPEGDECODER_H

#include "IO/exl_Stream.h"

#ifdef EXL_PLATFORM_GFL
#define EXL_MPEG_FIXED_MATH
#endif

#ifdef EXL_MPEG_FIXED_MATH
#define PLM_USE_FIXED_MATH
#endif
#include "detail/pl_mpeg.h"

#define EXL_MPEG_BUFFER_SIZE 24 * 1024

#define EXL_MPEG_DECIMAL PLM_DECIMAL
#define EXL_TO_MPEG_DECIMAL(a) PLM_DECCONST(a)
#define EXL_FROM_MPEG_DECIMAL(a) PLM_DEC2I(a)

namespace exl {
    namespace media {
        class MPEGDecoder {
        private:
            exl::io::Stream*    m_Stream;
            plm_t*              m_Plm;

        public:
            EXL_PUBLIC MPEGDecoder(exl::heap::Allocator* allocator, exl::io::Stream* stream);
            EXL_PUBLIC ~MPEGDecoder();

            EXL_PUBLIC void GetVideoDimensions(u32* pWidth, u32* pHeight);

            EXL_PUBLIC u32 GetFrame();

            EXL_PUBLIC bool SeekFrame(u32 frame, bool exact);

            EXL_PUBLIC EXL_MPEG_DECIMAL GetTime();

            EXL_PUBLIC bool SeekTime(EXL_MPEG_DECIMAL time, bool exact);

            #ifdef EXL_MPEG_FIXED_MATH
            EXL_PUBLIC INLINE bool SeekTime(float time) {
                return SeekTime(EXL_TO_MPEG_DECIMAL(time));
            }
            #endif

            EXL_PUBLIC bool DecodeFrameRGB5A1(void* destRgb555, u32 outWidth, u32 outHeight);
        };
    }
}

#endif
#ifndef __EXL_CINEPAKDECODER_H
#define __EXL_CINEPAKDECODER_H

#include "exl_Types.h"
#include "IO/exl_Stream.h"
#include "detail/avi_player.h"

namespace exl {
    namespace media {
        class CinepakDecoder {
        private:
            exl::io::Stream*    m_Stream;
            AVI_Player          m_Player;
            uint8_t*            m_LastYUV;          

        public:
            EXL_PUBLIC CinepakDecoder(exl::heap::Allocator* allocator, exl::io::Stream* stream);
            EXL_PUBLIC ~CinepakDecoder();

            EXL_PUBLIC void GetVideoDimensions(u32* pWidth, u32* pHeight);

            EXL_PUBLIC u32 GetFrame();

            EXL_PUBLIC void Restart();

            EXL_PUBLIC bool DecodeFrameRGB5A1(void* destRgb555, u32 outWidth, u32 outHeight);
        private:
            static void ConvY2R(void* yuv, void* rgb, u32 srcW, u32 srcH, u32 dstW, u32 dstH);
            static void VideoDecodeCallback(void* supervisor, uint8_t* yuv);
        };
    }
}

#endif
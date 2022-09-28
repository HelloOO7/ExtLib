#ifndef __EXL_CINEPAKDECODER_H
#define __EXL_CINEPAKDECODER_H

#include "exl_Types.h"
#include "IO/exl_Stream.h"
#include "exl_AudioFormat.h"
#include "detail/avi_player.h"

namespace exl {
    namespace media {
        #define EXL_CINEPAK_VIDEO 1
        #define EXL_CINEPAK_AUDIO 2

        class CinepakDecoder {
        private:
            exl::io::Stream*    m_Stream;
            AVI_Player          m_Player;
            uint8_t*            m_LastYUV;          

        public:
            EXL_PUBLIC CinepakDecoder(exl::heap::Allocator* allocator, exl::io::Stream* stream, int streams);
            EXL_PUBLIC ~CinepakDecoder();

            EXL_PUBLIC void GetVideoDimensions(u32* pWidth, u32* pHeight);
            EXL_PUBLIC void GetAudioFormat(exl::media::AudioFormat& dest);

            EXL_PUBLIC u32 GetFrame();

            EXL_PUBLIC void Restart();

            EXL_PUBLIC bool NextFrame();
            EXL_PUBLIC void DecodeFrameRGB5A1(void* destRgb555, u32 outWidth, u32 outHeight);
            
            EXL_PUBLIC u32 GetAvailableSamples();
            EXL_PUBLIC void GetSamples(void* dest, u32 count, u8 channel);
            EXL_PUBLIC void DiscardSamples(u32 count);
        private:
            static void ConvY2R(void* yuv, void* rgb, u32 srcW, u32 srcH, u32 dstW, u32 dstH);
            static void VideoDecodeCallback(void* supervisor, uint8_t* yuv);
            static void AudioDecodeCallback(void* supervisor, AVI_SoundBufferQueue* sbq);
        };
    }
}

#endif
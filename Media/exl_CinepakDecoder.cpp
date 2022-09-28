#ifndef __EXL_CINEPAKDECODER_CPP
#define __EXL_CINEPAKDECODER_CPP

#include "exl_CinepakDecoder.h"
#include "IO/exl_Stream.h"
#include "detail/avi_player.h"
#include "Util/exl_Print.h"
#include "exl_DebugPrint.h"
#include "exl_AudioFormat.h"

namespace exl {
    namespace media {
        CinepakDecoder::CinepakDecoder(exl::heap::Allocator* allocator, exl::io::Stream* stream, int streams) : m_Player(allocator) {
            m_Stream = stream;
            m_Player.setSupervisor(this);
            int aviStreams = 0;
            if (streams & EXL_CINEPAK_VIDEO) {
                aviStreams |= AVI_DECODE_VIDEO;
            }
            if (streams & EXL_CINEPAK_AUDIO) {
                aviStreams |= AVI_DECODE_AUDIO;
            }
            if (!m_Player.openStream(m_Stream, aviStreams)) {
                EXL_DEBUG_PRINTF("Failed to open AVI file!\n");
            }
            else {
                m_Player.setVideoCallback(VideoDecodeCallback);
                m_Player.setAudioCallback(AudioDecodeCallback);
            }
            m_LastYUV = nullptr;
        }

        CinepakDecoder::~CinepakDecoder() {
            m_Player.close();
            m_Stream->Close();
        }

        void CinepakDecoder::GetVideoDimensions(u32* pWidth, u32* pHeight) {
            int iw;
            int ih;
            m_Player.getVideoDimensions(&iw, &ih);
            *pWidth = (u32) iw;
            *pHeight = (u32) ih;
        }

        void CinepakDecoder::GetAudioFormat(exl::media::AudioFormat& dest) {
            int bps;
            int nch;
            int rate;
            m_Player.getAudioInfo(&nch, &rate, &bps);
            if (nch == 0) {
                dest.Encoding == INVALID;
            }
            else {
                dest.Encoding = AudioEncoding::PCM;
                dest.BitsPerSample = bps;
                dest.ChannelCount = nch;
                dest.SampleRate = rate;
            }
        }

        u32 CinepakDecoder::GetFrame() {
            return m_Player.getFrame();
        }

        void CinepakDecoder::Restart() {
            m_Player.restart();
        }

        bool CinepakDecoder::NextFrame() {
            return m_Player.readNextFrame();
        }

        void CinepakDecoder::DecodeFrameRGB5A1(void* dest, u32 outWidth, u32 outHeight) {
            u32 srcW;
            u32 srcH;
            GetVideoDimensions(&srcW, &srcH);
            ConvY2R(m_LastYUV, dest, srcW, srcH, outWidth, outHeight);
        }

        u32 CinepakDecoder::GetAvailableSamples() {
            return m_Player.getAvailableSamples();
        }

        void CinepakDecoder::GetSamples(void* dest, u32 count, u8 channel) {
            m_Player.getSamples(dest, count, channel);
        }

        void CinepakDecoder::DiscardSamples(u32 count) {
            m_Player.discardSamples(count);
        }

        struct UYVY {
            u8 U;
            u8 Y1;
            u8 V;
            u8 Y2; 
        };

        #ifdef EXL_PLATFORM_GFL
        extern "C" void uyvy422_2_rgb555_interwork(void* rgb, void* uyvy, u32 srcW, u32 srcH, u32 dstW, u32 dstH);

        void CinepakDecoder::ConvY2R(void* yuv, void* rgb, u32 srcW, u32 srcH, u32 dstW, u32 dstH) {
            uyvy422_2_rgb555_interwork(rgb, yuv, srcW, srcH, dstW, dstH);
        }
        #else
        INLINE int clip(int value) {
            return value < 0 ? 0 : (value > 255 ? 255 : value);
        }

        INLINE void set565(u16* dest, int c, int d, int e) {
            *dest = 0x8000
                | ((clip(( 298 * c + 409 * e + 128) >> 8) >> 3)           <<  0) //Red
                | ((clip(( 298 * c - 100 * d - 208 * e + 128) >> 8) >> 3) <<  5) //Green
                | ((clip(( 298 * c + 516 * d + 128) >> 8) >> 3)           << 10) //Blue
            ;
        }

        void CinepakDecoder::ConvY2R(void* yuv, void* rgb, u32 srcW, u32 srcH, u32 dstW, u32 dstH) {
            UYVY* in  = static_cast<UYVY*> (yuv);
            u32*  out = static_cast<u32*>  (rgb);

            int minW = ((srcW < dstW) ? srcW : dstW) >> 1;
            int minH = (srcH < dstH) ? srcH : dstH;
            u32 strideAddend = (dstW < srcW) ? ((srcW - dstW) >> 1) : 0;
            //EXL_DEBUG_PRINTF("Y2R begin minW %d minH %d stride %d\n", minW, minH, strideAddend);
            //EXL_DEBUG_PRINTF("Debug data: UYVY increment %d total expected UYVY size %x\n", sizeof(UYVY), minH * minW * sizeof(UYVY));
            int y0;
            int u0;
            int y1;
            int v0;
            int rbase;
            int gbase;
            int bbase;
            for (int yPos = 0; yPos < minH; yPos++) {
                for (int xPos = 0; xPos < minW; xPos++) {
                    y0 = ((int)(in->Y1) - 16) * 298;
                    u0 = (int)(in->U) - 128;
                    y1 = ((int)(in->Y2) - 16) * 298;
                    v0 = (int)(in->V) - 128;
                    in++;
                    rbase = 409 * v0 + 128;
                    gbase = -100 * u0 -208 * v0 + 128;
                    bbase = 516 * u0 + 128;
                    
                    *out = 
                        ((0x8000
                        | ((clip(( y0 + rbase) >> 8) >> 3) <<  0) //R1
                        | ((clip(( y0 + gbase) >> 8) >> 3) <<  5) //G1
                        | ((clip(( y0 + bbase) >> 8) >> 3) << 10) //B1
                        ) << 0) //Color 1
                        |
                        ((0x8000
                        | ((clip(( y1 + rbase) >> 8) >> 3) <<  0) //R2
                        | ((clip(( y1 + gbase) >> 8) >> 3) <<  5) //G2
                        | ((clip(( y1 + bbase) >> 8) >> 3) << 10) //B2
                        ) << 16) //Color2
                    ;
                    out++;
                }
                if (strideAddend) {
                    in += strideAddend;
                }
            }
            //EXL_DEBUG_PRINTF("Y2R bytes written 0x%x\n", (u8*)out - (u8*)rgb);
        }
        #endif

        void CinepakDecoder::VideoDecodeCallback(void* supervisor, uint8_t* yuv) {
            static_cast<CinepakDecoder*>(supervisor)->m_LastYUV = yuv;
        }

        void CinepakDecoder::AudioDecodeCallback(void* supervisor, AVI_SoundBufferQueue* sbq) {
            //dummy
        }
    }
}

#endif
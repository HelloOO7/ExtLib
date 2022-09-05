#ifndef __EXL_MPEGDECODER_CPP
#define __EXL_MPEGDECODER_CPP

#include "exl_MPEGDecoder.h"
#include "IO/exl_Stream.h"
#include "Heap/exl_Allocator.h"

#define PL_MPEG_IMPLEMENTATION
#include "detail/pl_mpeg.h"
#undef PL_MPEG_IMPLEMENTATION

namespace exl {
    namespace media {
        MPEGDecoder::MPEGDecoder(exl::heap::Allocator* allocator, exl::io::Stream* stream) {
            m_Stream = stream;
            m_Plm = plm_create_with_stream(allocator, stream, EXL_MPEG_BUFFER_SIZE, true, PLM_PLAYBACK_VIDEO);
        }

        MPEGDecoder::~MPEGDecoder() {
            plm_destroy(m_Plm);
        }

        void MPEGDecoder::GetVideoDimensions(u32* pWidth, u32* pHeight) {
            *pWidth = plm_get_width(m_Plm);
            *pHeight = plm_get_height(m_Plm);
        }

        u32 MPEGDecoder::GetFrame() {
            return m_Plm->video_decoder->frames_decoded;
        }

        bool MPEGDecoder::SeekFrame(u32 frame, bool exact) {
            //integer * decimal = decimal, no need for PLM_DECMUL
            return plm_seek(m_Plm, frame * plm_get_inv_framerate(m_Plm), exact ? 1 : 0);
        }

        EXL_MPEG_DECIMAL MPEGDecoder::GetTime() {
            return plm_get_time(m_Plm);
        }

        bool MPEGDecoder::SeekTime(EXL_MPEG_DECIMAL time, bool exact) {
            return plm_seek(m_Plm, time, exact ? 1 : 0);
        }

        bool MPEGDecoder::DecodeFrameRGB5A1(void* dest, u32 outWidth, u32 outHeight) {
            plm_frame_t* frame = plm_decode_video(m_Plm);
            if (!frame) {
                return false;
            }
            plm_frame_to_rgb555(frame, dest, outWidth, outHeight);
            return true;
        }
    }
}

#endif
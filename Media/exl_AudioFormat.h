#ifndef __EXL_AUDIOFORMAT_H
#define __EXL_AUDIOFORMAT_H

#include "exl_Types.h"

namespace exl {
    namespace media {
        enum AudioEncoding : u8 {
            INVALID,
            PCM
        };

        struct AudioFormat {
            AudioEncoding Encoding;
            u8            ChannelCount;
            u16           BitsPerSample;
            u32           SampleRate;
        };
    }
}
#endif
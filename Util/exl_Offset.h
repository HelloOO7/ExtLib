#ifndef __EXL_OFFSET_H
#define __EXL_OFFSET_H

#include "exl_Types.h"

#define EXL_OFFSET(type) exl::util::Offset<type>

namespace exl {
    namespace util {
        template<typename T>
        struct Offset {
        private:
            s32 Value;
        public:
            INLINE const T* Get() const
            {
                if (Value == 0) {
                    return nullptr;
                }
                return reinterpret_cast<const T*>(reinterpret_cast<const u8*>(this) + Value);
            }

            INLINE const u32 GetOffset(void* base) {
                if (Value == 0) {
                    return 0xFFFFFFFF;
                }
                return reinterpret_cast<char*>(this) + Value - reinterpret_cast<char*>(base);
            }

            INLINE const u32 GetOffset(u32 myPosition) {
                if (Value == 0) {
                    return 0xFFFFFFFF;
                }
                return myPosition + Value;
            }
        };
    }
}

#endif
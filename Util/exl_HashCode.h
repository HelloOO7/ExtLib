#ifndef __EXL_HASHCODE_H
#define __EXL_HASHCODE_H

#include "exl_Types.h"
#include "exl_DllExport.h"

namespace exl {
    namespace util {
        typedef u32 Hash;

        EXL_PUBLIC constexpr Hash HashCode(const char* cstring) {
            if (!cstring) {
                return 0;
            }
            Hash hash = 0x811C9DC5; //offset_basis
            while (true) {
                char c = *cstring;
                if (!c) {
                    break;
                }
                hash = (hash ^ c) * 16777619; //FNV_prime
                cstring++;
            }
            return hash;
        }

        EXL_PUBLIC constexpr Hash HashCode(const char* data, size_t dataLen) {
            if (!dataLen) {
                return 0;
            }
            Hash hash = 0x811C9DC5; //offset_basis
            while (dataLen) {
                char c = *data;
                hash = (hash ^ c) * 16777619; //FNV_prime
                data++;
                dataLen--;
            }
            return hash;
        }

        namespace detail {
            INLINE constexpr Hash fnv1a_32(char const *s, std::size_t count) {
                return count ? (fnv1a_32(s, count - 1) ^ s[count - 1]) * 16777619u : 0x811C9DC5;
            }
        }
    }
}

constexpr exl::util::Hash operator"" _hash(char const* s, std::size_t count)
{
    return exl::util::detail::fnv1a_32(s, count);
}

#endif
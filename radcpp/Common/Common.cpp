#include "radcpp/Common/Common.h"

#include <intrin.h>

uint32_t BitScanReverse32(uint32_t mask)
{
#if defined(_WIN32)
    unsigned long index = 0;
    ::_BitScanReverse(&index, static_cast<uint32_t>(mask));
    return static_cast<uint32_t>(index);
#elif defined(__GNUC__)
    static_assert(sizeof(uint32_t) == sizeof(unsigned int));
    return (31u - __builtin_clz(static_cast<uint32_t>(mask)));
#else
    uint32_t index = 31u;
    if (mask != 0)
        for (; (((mask >> index) & 0x1) == 0); --index);
    return index;
#endif
}

uint32_t BitScanReverse64(uint64_t mask)
{
#if defined(_WIN64)
    unsigned long index = 0;
    ::_BitScanReverse64(&index, mask);
    return static_cast<uint32_t>(index);
#elif defined(_WIN32)
    unsigned long index = 0;
    const uint32_t highPart = HighPart(mask);
    ::_BitScanReverse(&index, (highPart != 0) ? highPart : LowPart(mask));
    return (highPart != 0) ? (index + 32u) : index;
#elif defined(__GNUC__)
    static_assert(sizeof(uint64_t) == sizeof(unsigned long long));
    return (63u - __builtin_clzll(mask));
#else
    uint32_t index = 63u;
    if (mask != 0)
        for (; (((mask >> index) & 0x1) == 0); --index);
    return index;
#endif
}

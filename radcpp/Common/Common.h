#ifndef RADCPP_COMMON_H
#define RADCPP_COMMON_H
#pragma once

// C Standard Library header files
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <cinttypes>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>

// C++ Standard Library headers
#include <any>
#include <bitset>
#include <chrono>
#include <compare>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <initializer_list>
#include <optional>
#include <source_location>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <variant>
#include <version>

template<class T>
constexpr std::underlying_type_t<T> UnderlyingCast(T t) noexcept
{
    return static_cast<std::underlying_type_t<T>>(t);
}

constexpr uint32_t HighPart(uint64_t value)
{
    return (value & 0xFFFFFFFF00000000) >> 32;
}

constexpr uint32_t LowPart(uint64_t value)
{
    return (value & 0x00000000FFFFFFFF);
}

constexpr uint16_t HighPart(uint32_t value)
{
    return (value & 0xFFFF0000) >> 16;
}

constexpr uint16_t LowPart(uint32_t value)
{
    return (value & 0x0000FFFF);
}

constexpr bool HasBits(uint32_t flags, uint32_t bits)
{
    return ((flags & bits) == bits);
}

constexpr bool HasAnyBit(uint32_t flags, uint32_t bits)
{
    return ((flags & bits) != 0);
}

constexpr bool HasNoBit(uint32_t flags, uint32_t bits)
{
    return ((flags & bits) == 0);
}

// Search the mask data for the most significant set bit (1 bit); If a most significant 1 bit is found, its bit index is returned;
// If mask is 0, the result is undefined.
uint32_t BitScanReverse32(uint32_t mask);

// Search the mask data for the most significant set bit (1 bit); If a most significant 1 bit is found, its bit index is returned;
// If mask is 0, the result is undefined.
uint32_t BitScanReverse64(uint64_t mask);

constexpr bool IsPow2(uint64_t x)
{
    return (x == 0) ? false : ((x & (x - 1)) == 0);
}

template <typename T>
T Pow2AlignUp(T value, uint64_t alignment)
{
    assert(IsPow2(alignment));
    return ((value + static_cast<T>(alignment) - 1) & ~(static_cast<T>(alignment) - 1));
}

template <typename T>
T Pow2AlignDown(T value, uint64_t alignment)
{
    assert(IsPow2(alignment));
    return (value & ~(alignment - 1));
}

inline
uint32_t RoundUpToPow2(uint32_t x)
{
    if (x != 0)
    {
        return uint32_t(0x2) << BitScanReverse32(x);
    }
    else
    {
        return 0;
    }
}

inline
uint64_t RoundUpToPow2(uint64_t x)
{
    if (x != 0)
    {
        return uint64_t(0x2) << BitScanReverse64(x);
    }
    else
    {
        return 0;
    }
}

template<typename T>
constexpr T RoundUpToMultiple(T value, T alignment)
{
    return (((value + (alignment - 1)) / alignment) * alignment);
}

template <typename T>
constexpr T RoundDownToMultiple(T value, T alignment)
{
    return ((value / alignment) * alignment);
}

inline
uint32_t FloatBitsToUint(float f32)
{
    union Convertor
    {
        float f32;
        uint32_t u32;
    } convertor;

    convertor.f32 = f32;
    return convertor.u32;
}

inline
float UintBitsToFloat(uint32_t u32)
{
    union Convertor
    {
        uint32_t u32;
        float f32;
    } convertor;

    convertor.u32 = u32;
    return convertor.f32;
}

#endif RADCPP_COMMON_H
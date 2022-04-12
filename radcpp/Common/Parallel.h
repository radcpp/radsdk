#ifndef RADCPP_PARALLEL_H
#define RADCPP_PARALLEL_H
#pragma once

#include "radcpp/Common/Common.h"

#include <atomic>
#include <thread>
#include <condition_variable>

#ifdef _WIN32
#include <sdkddkver.h>
#endif
#include <boost/asio.hpp>

// Please refer to: https://github.com/mmp/pbrt-v3/blob/master/src/core/parallel.h
class AtomicFloat
{
public:
    explicit AtomicFloat(float f = 0) { m_bits = FloatBitsToUint(f); }
    ~AtomicFloat() {}
    float operator=(float f)
    {
        m_bits = FloatBitsToUint(f);
        return f;
    }

    operator float() const { return UintBitsToFloat(m_bits); }

    void Add(float rhs)
    {
        uint32_t oldBits = m_bits;
        uint32_t newBits;
        do {
            newBits = FloatBitsToUint(UintBitsToFloat(oldBits) + rhs);
        } while (!m_bits.compare_exchange_weak(oldBits, newBits));
    }

private:
    std::atomic<uint32_t> m_bits;

}; // class AtomicFloat

#endif // RADCPP_PARALLEL_H
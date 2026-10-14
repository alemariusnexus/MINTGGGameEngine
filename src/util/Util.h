/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "../Globals.h"

#include <cassert>


#if defined(MINTGGGAMEENGINE_PORT_ARDUINO)  ||  defined(MINTGGGAMEENGINE_PORT_ESPIDF)
#   define MINTGGGAMEENGINE_TIMER_HAVE_US_RESOLUTION
#endif


namespace MINTGGGameEngine
{


typedef int64_t timer_mstick_t;
typedef int64_t timer_ustick_t;

enum AdvancedAllocFlags
{
    ADVANCED_ALLOC_FLAG_UNALIGNED_ACCESS    = 0x00000001,
    ADVANCED_ALLOC_FLAG_DMA_ACCESSIBLE      = 0x00000002,
    ADVANCED_ALLOC_FLAG_EXTERNAL_RAM        = 0x00000004,
    ADVANCED_ALLOC_FLAG_INTERNAL_RAM        = 0x00000008
};


void TimerInit();
void TimerShutdown();

timer_mstick_t TimerGetTickcountMs();
timer_ustick_t TimerGetTickcountUs();

void TimerDelaySpinMs(unsigned int ms);
void TimerDelaySpinUs(unsigned int us);


void DelayTaskMs(uint32_t delayMs);


void* AdvancedAlloc(size_t size, uint32_t flags);
void* AdvancedAlignedAlloc(size_t alignment, size_t size, uint32_t flags);
void AdvancedFree(void* d);


void ExtractArduinoPinMode(uint8_t pinMode, bool* output, bool* puEnabled, bool* pdEnabled);

inline uint16_t SwapEndianness(uint16_t v)
{
#ifdef __GNUC__
    return __builtin_bswap16(v);
#else
    return (v << 8) | (v >> 8);
#endif
}

inline uint32_t SwapEndianness(uint32_t v)
{
#ifdef __GNUC__
    return __builtin_bswap32(v);
#else
    return (v << 24) | (v >> 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8);
#endif
}

inline uint64_t SwapEndianness(uint64_t v)
{
#ifdef __GNUC__
    return __builtin_bswap64(v);
#else
    return  (v << 56)
        |   ((v & 0xFF00) << 40)
        |   ((v & 0xFF0000) << 24)
        |   ((v & 0xFF000000) << 8)
        |   ((v & 0xFF00000000) >> 8)
        |   ((v & 0xFF0000000000) >> 24)
        |   ((v & 0xFF000000000000) >> 40)
        |   (v >> 56);
#endif
}


#ifdef MINTGGGAMEENGINE_LITTLE_ENDIAN
#   define ToLittleEndian(v) (v)
#   define FromLittleEndian(v) (v)
#   define ToBigEndian(v) SwapEndianness((v))
#   define FromBigEndian(v) SwapEndianness((v))
#else
#   define ToLittleEndian(v) SwapEndianness((v))
#   define FromLittleEndian(v) SwapEndianness((v))
#   define ToBigEndian() (v)
#   define FromBigEndian() (v)
#endif

template <typename IntT>
inline IntT ToNativeByteOrder(IntT v, ByteOrder fromByteOrder)
{
#ifdef MINTGGGAMEENGINE_LITTLE_ENDIAN
    switch (fromByteOrder) {
    case ByteOrder::BigEndian:
        return SwapEndianness(v);
    default:
        return v;
    }
#else
    switch (fromByteOrder) {
    case ByteOrder::LittleEndian:
        return SwapEndianness(v);
    default:
        return v;
    }
#endif
}

template <typename IntT>
inline IntT FromNativeByteOrder(IntT v, ByteOrder toByteOrder)
{
    return ToNativeByteOrder<IntT>(v, toByteOrder);
}

inline uint8_t CalcPixelFormatBytesPerPixel(PixelFormat pixelFormat)
{
    switch (pixelFormat) {
    case PixelFormat::RGB565:
    case PixelFormat::BGR565:
        return 2;
    case PixelFormat::RGB888:
    case PixelFormat::BGR888:
        return 3;
    case PixelFormat::RGBX8888:
    case PixelFormat::XBGR8888:
        return 4;
    default:
        assert(false);
        return 0;
    }
}

const char* PixelFormatToString(PixelFormat pixelFormat);

}

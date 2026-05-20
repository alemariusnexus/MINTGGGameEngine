#pragma once

#include "../Globals.h"


// TODO: Actually detect it. Currently we're just assuming.
#define MINTGGGAMEENGINE_LITTLE_ENDIAN


namespace MINTGGGameEngine
{


typedef uint64_t timer_mstick_t;
typedef uint64_t timer_ustick_t;


void TimerInit();
void TimerShutdown();

timer_mstick_t TimerGetTickcountMs();
timer_ustick_t TimerGetTickcountUs();

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

}

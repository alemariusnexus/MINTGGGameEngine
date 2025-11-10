#include "Color.h"

namespace MINTGGGameEngine
{

const Color Color::BLACK(0x0000);
const Color Color::WHITE(0xFFFF);


Color::Color(uint8_t r, uint8_t g, uint8_t b)
        : rgb565(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
{
}

}

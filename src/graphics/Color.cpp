#include "Color.h"

namespace MINTGGGameEngine
{

const Color Color::BLACK(0, 0, 0);
const Color Color::WHITE(255, 255, 255);
const Color Color::RED(255, 0, 0);
const Color Color::GREEN(0, 255, 0);
const Color Color::BLUE(0, 0, 255);


Color::Color(uint8_t r, uint8_t g, uint8_t b)
        : rgb565(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
{
}

}

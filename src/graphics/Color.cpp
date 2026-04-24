#include "Color.h"

namespace MINTGGGameEngine
{

const Color Color::BLACK(0x0000);
const Color Color::WHITE(0xFFFF);

const Color Color::RED(0xF800);
const Color Color::GREEN(0x07E0);
const Color Color::BLUE(0x001F);

const Color Color::YELLOW(0xFFE0);
const Color Color::MAGENTA(0xF81F);
const Color Color::CYAN(0x07FF);


Color::Color(uint8_t r, uint8_t g, uint8_t b)
        : rgb565(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
{
}

}

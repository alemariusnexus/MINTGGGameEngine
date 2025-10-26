#pragma once

#include "Globals.h"


namespace MINTGGGameEngine
{

class Color
{
public:
    static const Color BLACK;
    static const Color WHITE;

public:
    Color() : rgb565(0x0000) {}
    Color(uint16_t rgb565) : rgb565(rgb565) {}
    Color(const Color& other) : rgb565(other.rgb565) {}

    uint16_t toRGB565() const { return rgb565; }
    operator uint16_t() const { return toRGB565(); }

private:
    uint16_t rgb565;
};

}

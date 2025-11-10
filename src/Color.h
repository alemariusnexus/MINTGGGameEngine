#pragma once

#include "Globals.h"


namespace MINTGGGameEngine
{

/**
 * \brief Represents a color.
 *
 * This class currently uses RGB565 format internally, but provides some helpers
 * for converting to and from RGB888.
 */
class Color
{
public:
    /**
     * The color black (RGB565: 0x0000)
     */
    static const Color BLACK;
    
    /**
     * The color white (RGB565: 0xFFFF)
     */
    static const Color WHITE;

public:
    /**
     * Create black color.
     */
    Color() : rgb565(0x0000) {}
    
    /**
     * Create color from the given RGB565 value.
     */
    Color(uint16_t rgb565) : rgb565(rgb565) {}
    
    /**
     * Create color from the given RGB888 values.
     */
    Color(uint8_t r, uint8_t g, uint8_t b);
    
    /**
     * Copy constructor.
     */
    Color(const Color& other) : rgb565(other.rgb565) {}

    /**
     * Return the color in RGB565 format.
     */
    uint16_t toRGB565() const { return rgb565; }
    
    /**
     * Return the color in RGB565 format.
     */
    operator uint16_t() const { return toRGB565(); }

private:
    uint16_t rgb565;
};

}

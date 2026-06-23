#pragma once

#include "../Globals.h"


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
     * \brief The color black (RGB565: 0x0000)
     */
    static const Color BLACK;
    
    /**
     * \brief The color white (RGB565: 0xFFFF)
     */
    static const Color WHITE;

    /**
     * \brief The color red (RGB565: 0xF800)
     */
    static const Color RED;

    /**
     * \brief The color green (RGB565: 0x07E0)
     */
    static const Color GREEN;

    /**
     * \brief The color blue (RGB565: 0x001F)
     */
    static const Color BLUE;

    /**
     * \brief The color yellow (RGB565: 0xFFE0)
     */
    static const Color YELLOW;

    /**
     * \brief The color magenta (RGB565: 0xF81F)
     */
    static const Color MAGENTA;

    /**
     * \brief The color cyan (RGB565: 0x07FF)
     */
    static const Color CYAN;

public:
    /**
     * \brief Create black color.
     */
    Color() : rgb565(0x0000) {}
    
    /**
     * \brief Create color from the given RGB565 value.
     */
    Color(uint16_t rgb565) : rgb565(rgb565) {}
    
    /**
     * \brief Create color from the given RGB888 values.
     *
     * \param r Red value (range 0-255).
     * \param g Green value (range 0-255).
     * \param b Blue value (range 0-255).
     */
    Color(uint8_t r, uint8_t g, uint8_t b);
    
    /**
     * \brief Copy constructor.
     */
    Color(const Color& other) : rgb565(other.rgb565) {}

    /**
     * \brief Return the color in RGB565 format.
     */
    uint16_t toRGB565() const { return rgb565; }
    
    /**
     * \brief Return the color in RGB565 format.
     */
    operator uint16_t() const { return toRGB565(); }

    Color & operator=(const Color &other)
    {
        if (this == &other)
            return *this;
        rgb565 = other.rgb565;
        return *this;
    }

private:
    uint16_t rgb565;
};

}

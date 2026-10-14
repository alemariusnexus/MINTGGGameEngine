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
    static Color fromRGB565(uint16_t rgb565);
    static Color fromBGR565(uint16_t bgr565);

    static Color fromBuffer(const void* buf, PixelFormat pixelFormat, ByteOrder byteOrder);

    static void convertBufferToBuffer (
        const void* fromBuf, PixelFormat fromPixelFormat, ByteOrder fromByteOrder,
        void* toBuf, PixelFormat toPixelFormat, ByteOrder toByteOrder,
        size_t numPixels
        );
    static void convertBufferToBuffer (
        const void* fromBuf, PixelFormat fromPixelFormat, ByteOrder fromByteOrder,
        void* toBuf, PixelFormat toPixelFormat, ByteOrder toByteOrder
        );

public:
    /**
     * \brief Create black color.
     */
    Color() : r(0), g(0), b(0), a(255) {}
    
    /**
     * \brief Create color from the given RGB888 values.
     *
     * \param r Red value (range 0-255).
     * \param g Green value (range 0-255).
     * \param b Blue value (range 0-255).
     */
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    
    /**
     * \brief Copy constructor.
     */
    Color(const Color& other) : r(other.r), g(other.g), b(other.b), a(other.a) {}

    uint8_t getRed() const { return r; }
    uint8_t getGreen() const { return g; }
    uint8_t getBlue() const { return b; }
    uint8_t getAlpha() const { return a; }

    /**
     * \brief Return the color in RGB565 format.
     */
    uint16_t toRGB565() const { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }

    /**
     * \brief Return the color in BGR565 format.
     */
    uint16_t toBGR565() const { return ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3); }

    void toRGB888(uint8_t& r, uint8_t& g, uint8_t& b) const { r = this->r; g = this->g; b = this->b; }
    void toRGBA8888(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const
            { r = this->r; g = this->g; b = this->b; a = this->a; }

    size_t toBuffer(void* buf, PixelFormat pixelFormat, ByteOrder byteOrder) const;
    size_t toBuffer(void* buf, PixelFormat pixelFormat, ByteOrder byteOrder, size_t numPixels) const;

    Color & operator=(const Color &other)
    {
        if (this == &other)
            return *this;
        r = other.r;
        g = other.g;
        b = other.b;
        a = other.a;
        return *this;
    }

private:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

}

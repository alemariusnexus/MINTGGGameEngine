#include "ScreenHAGL.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF

#include <fontx.h>
#include <font6x9.h>

#include "driver/spi_master.h"
#include "util/Log.h"
#include "util/Util.h"


namespace MINTGGGameEngine
{

ScreenHAGL::ScreenHAGL()
    : display(nullptr)
{
}

void ScreenHAGL::begin(hagl_backend_t* display)
{
    this->display = display;
}

void ScreenHAGL::fillScreen(const Color& color)
{
    if (!display) {
        return;
    }
    const hagl_color_t hcolor = color.toRGB565();
    hagl_color_t* bptr = reinterpret_cast<hagl_color_t*>(bb.buffer);
    hagl_color_t* bptrEnd = bptr + getWidth()*getHeight();
    while (bptr != bptrEnd) {
        *bptr = hcolor;
        bptr++;
    }
    //hagl_fill_rectangle(display, 0, 0, display->width-1, display->height-1, color.toRGB565());
}

void ScreenHAGL::drawPixel(int16_t x, int16_t y, const Color& color)
{
    if (!display) {
        return;
    }
    hagl_put_pixel(display, x, y, color.toRGB565());
}

void ScreenHAGL::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const Color& color)
{
    if (!display) {
        return;
    }
    hagl_draw_line(display, x0, y0, x1, y1, color.toRGB565());
}

void ScreenHAGL::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color, bool filled)
{
    if (!display) {
        return;
    }
    if (filled) {
        hagl_fill_rectangle_xywh(display, x, y, w, h, color.toRGB565());
    } else {
        hagl_draw_rectangle_xywh(display, x, y, w, h, color.toRGB565());
    }
}

void ScreenHAGL::drawCircle(int16_t cx, int16_t cy, int16_t r, const Color& color, bool filled)
{
    if (!display) {
        return;
    }
    if (filled) {
        hagl_fill_circle(display, cx, cy, r, color.toRGB565());
    } else {
        hagl_draw_circle(display, cx, cy, r, color.toRGB565());
    }
}

void ScreenHAGL::drawBitmap(int16_t x, int16_t y, const Bitmap& bitmap, FlipDir flipDir)
{
    if (!display) {
        return;
    }
    drawBitmapHelper(x, y, bitmap, flipDir, &ScreenHAGL::drawBitmapHelper_drawPixel, &ScreenHAGL::drawBitmapHelper_drawPixels, display);
}

void ScreenHAGL::drawText(const Text& text, int16_t ox, int16_t oy)
{
    if (!display) {
        return;
    }

    std::string content = text.getText();

    size_t bufLen = content.length()+1;
    wchar_t* wcontent = new wchar_t[bufLen];
    mbstowcs(wcontent, content.c_str(), bufLen);

    // TODO: Support text size setting and transparent background

    putText(wcontent, text.getX()+ox, text.getY()+oy, text.getColor().toRGB565(), font6x9);
}

void ScreenHAGL::commit()
{
    if (!display) {
        return;
    }

    // Swap endianness for pixels. This is needed at least for ST7735, and probably for all MIPI compatible displays,
    // because they expect pixels in big-endian format, while ESP32 is little-endian.
    // TODO: Find a better way. It doesn't seem possible to have the hardware do this for SPI. Another option would be
    //  to just store RGB565 values in big-endian order all the time, or maybe make this configurable. But is that a
    //  good idea?
    hagl_color_t* bptr = reinterpret_cast<hagl_color_t*>(bb.buffer);
    hagl_color_t* bptrEnd = bptr + getWidth()*getHeight();
    while (bptr != bptrEnd) {
        *bptr = __builtin_bswap16(*bptr);
        bptr++;
    }

    hagl_flush(display);
}

bool ScreenHAGL::saveScreenshot(const char* path)
{
    // TODO: Support this
    return false;
}



// putChar() and putText() below were adapted from hagl_put_char() and hagl_put_text(), to support transparent
// background in text rendering.

uint8_t ScreenHAGL::putChar (
    wchar_t code, int16_t x0, int16_t y0, hagl_color_t color,
    const uint8_t *font
) {
    //static uint8_t *buffer = NULL;
    uint8_t set, status;
    //hagl_bitmap_t bitmap;
    fontx_glyph_t glyph;

    status = fontx_glyph(&glyph, code, font);

    if (0 != status) {
        return 0;
    }

    /* Initialize character buffer when first called. */
    /*if (NULL == buffer) {
        buffer = static_cast<uint8_t*>(calloc(HAGL_CHAR_BUFFER_SIZE, sizeof(uint8_t)));
    }

    hagl_bitmap_init(
        &bitmap, glyph.width, glyph.height, display->depth, (uint8_t *)buffer
    );*/

    //hagl_color_t *ptr = (hagl_color_t *)bitmap.buffer;

    for (uint8_t y = 0; y < glyph.height; y++) {
        for (uint8_t x = 0; x < glyph.width; x++) {
            set = *(glyph.buffer + x / 8) & (0x80 >> (x % 8));
            if (set) {
                hagl_put_pixel(display, x0+x, y0+y, color);
            }
            /*if (set) {
                *(ptr++) = color;
            } else {
                *(ptr++) = 0x0000;
            }*/
        }
        glyph.buffer += glyph.pitch;
    }

    //hagl_blit(surface, x0, y0, &bitmap);

    return glyph.width;
}

// Adapted from hagl_put_text()
uint16_t ScreenHAGL::putText (
    const wchar_t *str, int16_t x0, int16_t y0, hagl_color_t color,
    const unsigned char *font
) {
    wchar_t temp;
    uint8_t status;
    uint16_t original = x0;
    fontx_meta_t meta;

    status = fontx_meta(&meta, font);
    if (0 != status) {
        return 0;
    }

    do {
        temp = *str++;
        if (13 == temp || 10 == temp) {
            x0 = 0;
            y0 += meta.height;
        } else {
            x0 += putChar(temp, x0, y0, color, font);
        }
    } while (*str != 0);

    return x0 - original;
}

}

#endif

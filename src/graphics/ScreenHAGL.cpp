#include "ScreenHAGL.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF

#include "util/Util.h"


namespace MINTGGGameEngine
{


// TODO: Maybe try to handle coordinate overflow/underflow more gracefully


ScreenHAGL::ScreenHAGL()
    : display(nullptr)
{
}

void ScreenHAGL::begin()
{
    begin(hagl_init());
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

void ScreenHAGL::drawPixel(int32_t x, int32_t y, const Color& color)
{
    if (!display) {
        return;
    }
    hagl_put_pixel(display, static_cast<int16_t>(x), static_cast<int16_t>(y), color.toRGB565());
}

void ScreenHAGL::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color)
{
    if (!display) {
        return;
    }
    hagl_draw_line (
        display,
        static_cast<int16_t>(x0), static_cast<int16_t>(y0),
        static_cast<int16_t>(x1), static_cast<int16_t>(y1),
        color.toRGB565()
        );
}

void ScreenHAGL::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled)
{
    if (!display) {
        return;
    }
    if (filled) {
        hagl_fill_rectangle_xywh (
            display,
            static_cast<int16_t>(x), static_cast<int16_t>(y),
            static_cast<uint16_t>(w), static_cast<uint16_t>(h),
            color.toRGB565()
            );
    } else {
        hagl_draw_rectangle_xywh (
            display,
            static_cast<int16_t>(x), static_cast<int16_t>(y),
            static_cast<uint16_t>(w), static_cast<uint16_t>(h),
            color.toRGB565()
            );
    }
}

void ScreenHAGL::drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled)
{
    if (!display) {
        return;
    }
    if (filled) {
        hagl_fill_circle (
            display,
            static_cast<int16_t>(cx), static_cast<int16_t>(cy), static_cast<int16_t>(r),
            color.toRGB565()
            );
    } else {
        hagl_draw_circle (
            display,
            static_cast<int16_t>(cx), static_cast<int16_t>(cy), static_cast<int16_t>(r),
            color.toRGB565()
            );
    }
}

void ScreenHAGL::drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir)
{
    if (!display) {
        return;
    }
    drawBitmapHelper (
        x, y,
        bitmap,
        flipDir,
        &ScreenHAGL::drawBitmapHelper_drawPixel,
        &ScreenHAGL::drawBitmapHelper_drawPixels,
        display
        );
}

Color ScreenHAGL::readPixel(int32_t x, int32_t y)
{
    return Color(hagl_get_pixel(display, static_cast<int16_t>(x), static_cast<int16_t>(y)));
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


}

#endif

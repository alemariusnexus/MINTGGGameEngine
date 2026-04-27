#pragma once

#include "../Globals.h"
#include "Screen.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF

#include <hagl.h>
#include <hagl_hal.h>


extern "C" {
extern hagl_bitmap_t bb;
}


namespace MINTGGGameEngine
{

class ScreenHAGL : public Screen
{
public:
    ScreenHAGL();

    void begin(hagl_backend_t* display);

    uint16_t getWidth() const override { return display->width; }
    uint16_t getHeight() const override { return display->height; }

    void fillScreen(const Color& color) override;
    void drawPixel(int16_t x, int16_t y, const Color& color) override;
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const Color& color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color, bool filled = false) override;
    void drawCircle(int16_t cx, int16_t cy, int16_t r, const Color& color, bool filled = false) override;
    void drawBitmap(int16_t x, int16_t y, const Bitmap& bitmap, FlipDir flipDir = FlipDir::None) override;
    void drawText(const Text& text, int16_t ox = 0, int16_t oy = 0) override;
    
    bool saveScreenshot(const char* path) override;
    
    void commit() override;

private:
    uint8_t putChar (
        wchar_t code, int16_t x0, int16_t y0, hagl_color_t color,
        const uint8_t *font
        );
    uint16_t putText (
        const wchar_t *str, int16_t x0, int16_t y0, hagl_color_t color,
        const unsigned char *font
        );

    static void drawBitmapHelper_drawPixel(hagl_backend_t* d, int16_t x, int16_t y, hagl_color_t c)
    {
        hagl_color_t* ptr = reinterpret_cast<hagl_color_t*>(bb.buffer) + x + y*d->width;
        *ptr = c;
    }

    static void drawBitmapHelper_drawPixels(hagl_backend_t* d, int16_t x, int16_t y, const hagl_color_t* c, uint16_t w)
    {
        memcpy(reinterpret_cast<hagl_color_t*>(bb.buffer) + y*d->width + x, c, w*sizeof(hagl_color_t));
    }

private:
    hagl_backend_t* display;
};

}

#endif

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

    void begin();
    void begin(hagl_backend_t* display);

    uint16_t getWidth() const override { return display->width; }
    uint16_t getHeight() const override { return display->height; }

    void fillScreen(const Color& color) override;
    void drawPixel(int32_t x, int32_t y, const Color& color) override;
    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color) override;
    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled = false) override;
    void drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled = false) override;
    void drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir = FlipDir::None) override;

    Color readPixel(int32_t x, int32_t y) override;
    
    void commit() override;

private:
    static void drawBitmapHelper_drawPixel(hagl_backend_t* d, int32_t x, int32_t y, hagl_color_t c)
    {
        hagl_color_t* ptr = reinterpret_cast<hagl_color_t*>(bb.buffer) + x + y*d->width;
        *ptr = c;
    }

    static void drawBitmapHelper_drawPixels(hagl_backend_t* d, int32_t x, int32_t y, const hagl_color_t* c, int32_t w)
    {
        memcpy(reinterpret_cast<hagl_color_t*>(bb.buffer) + y*d->width + x, c, w*sizeof(hagl_color_t));
    }

private:
    hagl_backend_t* display;
};

}

#endif

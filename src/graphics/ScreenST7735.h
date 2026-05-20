#pragma once

#include "../Globals.h"
#include "Screen.h"

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>


namespace MINTGGGameEngine
{

class ScreenST7735 : public Screen
{
public:
    ScreenST7735(Adafruit_ST7735& tft) : tft(&tft), canvas(getWidth(), getHeight()) {}

    void begin(int rotation = 3);

    uint16_t getWidth() const override { return 160; }
    uint16_t getHeight() const override { return 128; }

    void fillScreen(const Color& color) override;
    void drawPixel(int32_t x, int32_t y, const Color& color) override;
    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color) override;
    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled = false) override;
    void drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled = false) override;
    void drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir = FlipDir::None) override;

    Color readPixel(int32_t x, int32_t y) override;
    
    void commit() override;

private:
    static void drawBitmapHelper_drawPixel(GFXcanvas16* canvas, int32_t x, int32_t y, uint16_t c)
    {
        canvas->writePixel(static_cast<int16_t>(x), static_cast<int16_t>(y), c);
    }

    static void drawBitmapHelper_drawPixels(GFXcanvas16* canvas, int32_t x, int32_t y, const uint16_t* c, uint32_t w)
    {
        for (int32_t dx = 0 ; dx < w ; dx++) {
            canvas->writePixel(static_cast<int16_t>(x+dx), static_cast<int16_t>(y), c[dx]);
        }
    }

private:
    Adafruit_ST7735* tft;
    GFXcanvas16 canvas;
};

}

#endif

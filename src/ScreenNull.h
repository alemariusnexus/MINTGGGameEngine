#pragma once

#include "Globals.h"
#include "Screen.h"


namespace MINTGGGameEngine
{

class ScreenNull : public Screen
{
public:
    ScreenNull(uint16_t width = 160, uint16_t height = 128);

    uint16_t getWidth() const override { return width; }
    uint16_t getHeight() const override { return height; }

    void fillScreen(const Color& color) override;
    void drawPixel(int16_t x, int16_t y, const Color& color) override;
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const Color& color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color, bool filled = false) override;
    void drawCircle(int16_t cx, int16_t cy, int16_t r, const Color& color, bool filled = false) override;
    void drawBitmap(int16_t x, int16_t y, const Bitmap& bitmap, FlipDir flipDir = FlipDir::None) override;
    void drawText(const Text& text, int16_t ox = 0, int16_t oy = 0) override;
    
    void commit() override;

private:
    uint16_t width;
    uint16_t height;
};

}

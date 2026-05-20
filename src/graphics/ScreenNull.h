#pragma once

#include "../Globals.h"
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
    void drawPixel(int32_t x, int32_t y, const Color& color) override;
    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color) override;
    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled = false) override;
    void drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled = false) override;
    void drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir = FlipDir::None) override;
    
    void commit() override;

private:
    uint16_t width;
    uint16_t height;
};

}

#pragma once

#include "Globals.h"
#include "Bitmap.h"
#include "Color.h"
#include "Text.h"


namespace MINTGGGameEngine
{

class Screen
{
public:
    virtual uint16_t getWidth() const = 0;
    virtual uint16_t getHeight() const = 0;

    virtual void fillScreen(const Color& color) = 0;
    virtual void drawPixel(int16_t x, int16_t y, const Color& color) = 0;
    virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const Color& color) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color, bool filled = false) = 0;
    virtual void drawCircle(int16_t cx, int16_t cy, int16_t r, const Color& color, bool filled = false) = 0;
    virtual void drawBitmap(int16_t x, int16_t y, const Bitmap& bitmap, FlipDir flipDir = FlipDir::None) = 0;
    virtual void drawText(const Text& text, int16_t ox = 0, int16_t oy = 0) = 0;
    
    virtual bool saveScreenshot(const char* path) { return false; }
    
    virtual void commit() = 0;
};

}

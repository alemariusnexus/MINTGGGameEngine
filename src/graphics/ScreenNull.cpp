#include "ScreenNull.h"


namespace MINTGGGameEngine
{


ScreenNull::ScreenNull(uint16_t width, uint16_t height)
    : width(width), height(height)
{
}

void ScreenNull::fillScreen(const Color& color)
{
}

void ScreenNull::drawPixel(int16_t x, int16_t y, const Color& color)
{
}

void ScreenNull::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const Color& color)
{
}

void ScreenNull::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color, bool filled)
{
}

void ScreenNull::drawCircle(int16_t cx, int16_t cy, int16_t r, const Color& color, bool filled)
{
}

void ScreenNull::drawBitmap(int16_t x, int16_t y, const Bitmap& bitmap, FlipDir flipDir)
{
}

void ScreenNull::drawText(const Text& text, int16_t ox, int16_t oy)
{
}

void ScreenNull::commit()
{
}


}

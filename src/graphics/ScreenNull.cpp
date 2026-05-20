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

void ScreenNull::drawPixel(int32_t x, int32_t y, const Color& color)
{
}

void ScreenNull::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color)
{
}

void ScreenNull::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled)
{
}

void ScreenNull::drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled)
{
}

void ScreenNull::drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir)
{
}

void ScreenNull::commit()
{
}


}

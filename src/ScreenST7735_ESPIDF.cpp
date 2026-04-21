#include "ScreenST7735.h"


#ifdef ESP_PLATFORM

namespace MINTGGGameEngine
{

void ScreenST7735::begin(int rotation)
{
}

void ScreenST7735::fillScreen(const Color& color)
{
}

void ScreenST7735::drawPixel(int16_t x, int16_t y, const Color& color)
{
}

void ScreenST7735::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const Color& color)
{
}

void ScreenST7735::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color, bool filled)
{
}

void ScreenST7735::drawCircle(int16_t cx, int16_t cy, int16_t r, const Color& color, bool filled)
{
}

void ScreenST7735::drawBitmap(int16_t x, int16_t y, const Bitmap& bitmap, FlipDir flipDir)
{
}

void ScreenST7735::drawText(const Text& text, int16_t ox, int16_t oy)
{
}

void ScreenST7735::commit()
{
}

bool ScreenST7735::saveScreenshot(const char* path)
{
    return false;
}

}

#endif

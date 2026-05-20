#include "ScreenST7735.h"


#ifdef MINTGGGAMEENGINE_PORT_ARDUINO

namespace MINTGGGameEngine
{

// TODO: Handle coordinate overflow/underflow more gracefully

void ScreenST7735::begin(int rotation)
{
    tft->initR(INITR_BLACKTAB);
    tft->setSPISpeed(20000000);
    tft->setRotation(rotation);

    tft->fillScreen(ST77XX_WHITE);
}

void ScreenST7735::fillScreen(const Color& color)
{
    canvas.fillScreen(color);
}

void ScreenST7735::drawPixel(int32_t x, int32_t y, const Color& color)
{
    canvas.drawPixel(static_cast<int16_t>(x), static_cast<int16_t>(y), color);
}

void ScreenST7735::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color)
{
    canvas.drawLine (
        static_cast<int16_t>(x0), static_cast<int16_t>(y0),
        static_cast<int16_t>(x1), static_cast<int16_t>(y1),
        color
        );
}

void ScreenST7735::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled)
{
    if (filled) {
        canvas.fillRect (
            static_cast<int16_t>(x), static_cast<int16_t>(y),
            static_cast<int16_t>(w), static_cast<int16_t>(h),
            color
            );
    } else {
        canvas.drawRect (
            static_cast<int16_t>(x), static_cast<int16_t>(y),
            static_cast<int16_t>(w), static_cast<int16_t>(h),
            color
            );
    }
}

void ScreenST7735::drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled)
{
    if (filled) {
        canvas.fillCircle (
            static_cast<int16_t>(cx), static_cast<int16_t>(cy),
            static_cast<int16_t>(r),
            color
            );
    } else {
        canvas.drawCircle (
            static_cast<int16_t>(cx), static_cast<int16_t>(cy),
            static_cast<int16_t>(r),
            color
            );
    }
}

void ScreenST7735::drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir)
{
    canvas.startWrite();
    drawBitmapHelper(
        x, y,
        bitmap,
        flipDir,
        &ScreenST7735::drawBitmapHelper_drawPixel,
        &ScreenST7735::drawBitmapHelper_drawPixels,
        &canvas
        );
    canvas.endWrite();
}

Color ScreenST7735::readPixel(int32_t x, int32_t y)
{
    return Color(canvas.getPixel(static_cast<int16_t>(x), static_cast<int16_t>(y)));
}

void ScreenST7735::commit()
{
    uint16_t w = getWidth();
    uint16_t h = getHeight();
    uint16_t* d = canvas.getBuffer();
    
    tft->startWrite();
    tft->setAddrWindow(0, 0, w, h);
    tft->writePixels(d, w*h, true, false);
    tft->endWrite();
}

}

#endif

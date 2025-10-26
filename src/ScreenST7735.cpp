#include "ScreenST7735.h"


namespace MINTGGGameEngine
{

void ScreenST7735::begin(int rotation)
{
    tft->initR(INITR_BLACKTAB);
    tft->setSPISpeed(40000000);
    tft->setRotation(rotation);

    tft->fillScreen(ST77XX_WHITE);
}

void ScreenST7735::fillScreen(const Color& color)
{
    canvas.fillScreen(color);
}

void ScreenST7735::drawPixel(int16_t x, int16_t y, const Color& color)
{
    canvas.drawPixel(x, y, color);
}

void ScreenST7735::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const Color& color)
{
    canvas.drawLine(x0, y0, x1, y1, color);
}

void ScreenST7735::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color, bool filled)
{
    if (filled) {
        canvas.fillRect(x, y, w, h, color);
    } else {
        canvas.drawRect(x, y, w, h, color);
    }
}

void ScreenST7735::drawCircle(int16_t cx, int16_t cy, int16_t r, const Color& color, bool filled)
{
    if (filled) {
        canvas.fillCircle(cx, cy, r, color);
    } else {
        canvas.drawCircle(cx, cy, r, color);
    }
}

void ScreenST7735::drawBitmap(int16_t x, int16_t y, const Bitmap& bitmap, FlipDir flipDir)
{
    uint16_t w = bitmap.getWidth();
    uint16_t h = bitmap.getHeight();
    const uint16_t* d = bitmap.getData();
    const uint8_t* m = bitmap.getMask();
    
    int16_t origX = x;
    
    canvas.startWrite();
    
    int16_t byStart, byEnd, byStep;
    int16_t bxStart, bxEnd, bxStep;
    if (flipDir == FlipDir::None) {
        byStart = 0;
        byEnd = h;
        byStep = 1;
        bxStart = 0;
        bxEnd = w;
        bxStep = 1;
    } else if (flipDir == FlipDir::Horizontal) {
        byStart = 0;
        byEnd = h;
        byStep = 1;
        bxStart = w-1;
        bxEnd = -1;
        bxStep = -1;
    } else if (flipDir == FlipDir::Vertical) {
        byStart = h-1;
        byEnd = -1;
        byStep = -1;
        bxStart = 0;
        bxEnd = w;
        bxStep = 1;
    } else {
        byStart = h-1;
        byEnd = -1;
        byStep = -1;
        bxStart = w-1;
        bxEnd = -1;
        bxStep = -1;
    }
    
    if (m) {
        uint16_t mw = (w+7) / 8;
        for (int16_t by = byStart ; by != byEnd ; by += byStep, y++) {
            x = origX;
            for (int16_t bx = bxStart ; bx != bxEnd ; bx += bxStep, x++) {
                if (m[by*mw + (bx>>3)] & (0x80 >> (bx&7))) {
                    canvas.writePixel(x, y, d[by*w + bx]);
                }
            }
        }
    } else {
        for (int16_t by = byStart ; by != byEnd ; by += byStep, y++) {
            x = origX;
            for (int16_t bx = bxStart ; bx != bxEnd ; bx += bxStep, x++) {
                canvas.writePixel(x, y, d[by*w + bx]);
            }
        }
    }
    
    canvas.endWrite();
}

void ScreenST7735::drawText(const Text& text)
{
    canvas.setCursor(text.getX(), text.getY());
    canvas.setTextSize(text.getSize());
    canvas.setTextColor(text.getColor());
    canvas.print(text.getText());
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

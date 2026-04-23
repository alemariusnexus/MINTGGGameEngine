#include "ScreenST7735.h"


#ifdef ARDUINO

namespace MINTGGGameEngine
{

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
    canvas.startWrite();
    drawBitmapHelper(x, y, bitmap, flipDir, [this](uint16_t x, uint y, uint16_t c) { canvas.writePixel(x, y, c); });
    canvas.endWrite();
}

void ScreenST7735::drawText(const Text& text, int16_t ox, int16_t oy)
{
    canvas.setCursor(text.getX()+ox, text.getY()+oy);
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

bool ScreenST7735::saveScreenshot(const char* path)
{
    if (SD.exists(path)) {
        SD.remove(path);
    }
    File f = SD.open(path, FILE_WRITE);
    if (!f) {
        return false;
    }
    const uint8_t* data = reinterpret_cast<const uint8_t*>(canvas.getBuffer());
    size_t totalSize = getWidth()*getHeight()*2;
    size_t writtenSize = 0;
    do {
        writtenSize += f.write(data + writtenSize, totalSize-writtenSize);
    } while (writtenSize < totalSize);
    f.flush();
    f.close();
    return true;
}

}

#endif

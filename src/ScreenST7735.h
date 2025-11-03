#pragma once

#include "Globals.h"
#include "Screen.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>


namespace MINTGGGameEngine
{

class ScreenST7735 : public Screen
{
public:
    ScreenST7735(Adafruit_ST7735* tft) : tft(tft), canvas(getWidth(), getHeight()) {}

    void begin(int rotation = 3);

    uint16_t getWidth() const override { return 160; }
    uint16_t getHeight() const override { return 128; }

    void fillScreen(const Color& color) override;
    void drawPixel(int16_t x, int16_t y, const Color& color) override;
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const Color& color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color, bool filled = false) override;
    void drawCircle(int16_t cx, int16_t cy, int16_t r, const Color& color, bool filled = false) override;
    void drawBitmap(int16_t x, int16_t y, const Bitmap& bitmap, FlipDir flipDir = FlipDir::None) override;
    void drawText(const Text& text) override;
    
    bool saveScreenshot(const char* path) override;
    
    void commit() override;

public:
    Adafruit_ST7735* tft;
    GFXcanvas16 canvas;
};

}

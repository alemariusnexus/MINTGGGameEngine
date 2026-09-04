#pragma once

#include "Screen.h"

#include "../surface/MemDrawSurface.h"


namespace MINTGGGameEngine
{

class BufferedScreen : public Screen
{
public:
    BufferedScreen(uint8_t* buffer, uint16_t width, uint16_t height, const MemDrawSurface::delete_fn_t& del, bool swapEndianness = false);
    ~BufferedScreen() override;

    uint16_t getWidth() const override;
    uint16_t getHeight() const override;

    void fill(const Color& color, bool honorClipRegion) override;
    void drawPixel(int32_t x, int32_t y, const Color& color) override;
    void drawHLine(int32_t x0, int32_t y0, int32_t w, const Color& color) override;
    void drawVLine(int32_t x0, int32_t y0, int32_t h, const Color& color) override;
    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color) override;
    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled) override;
    void drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled) override;
    void drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir) override;

    void setClipRegion(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch) override;
    void getClipRegion(uint16_t* cx, uint16_t* cy, uint16_t* cw, uint16_t* ch) override;

    Color readPixel(int32_t x, int32_t y) override;

    void drawText(const Text& text, int32_t ox, int32_t oy) override;

    bool saveScreenshot(const char* path) override;

protected:
    MemDrawSurface fb;
};

}

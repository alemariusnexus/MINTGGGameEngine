#pragma once

#include "../Globals.h"
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

    virtual Color readPixel(int16_t x, int16_t y) = 0;

    virtual bool saveScreenshot(const char* path);
    
    virtual void commit() = 0;

protected:
    template <typename DrawPixelT, typename DrawPixelsT, typename ContextT>
    void drawBitmapHelper (
        int16_t x, int16_t y,
        const Bitmap& bitmap,
        FlipDir flipDir,
        DrawPixelT drawPixel,
        DrawPixelsT drawPixels,
        ContextT userPtr
        );
};


template <typename DrawPixelT, typename DrawPixelsT, typename ContextT>
void Screen::drawBitmapHelper (
    int16_t x, int16_t y,
    const Bitmap& bitmap,
    FlipDir flipDir,
    DrawPixelT drawPixel,
    DrawPixelsT drawPixels,
    ContextT context
) {
    uint16_t sw = getWidth();
    uint16_t sh = getHeight();

    uint16_t w = bitmap.getWidth();
    uint16_t h = bitmap.getHeight();

    const uint16_t* d = bitmap.getData();
    const uint8_t* m = bitmap.getMask();

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

    if (x >= sw  ||  x+w < 0  ||  y >= sh  ||  y+h < 0) {
        // Bitmap is completely off-screen
        return;
    }

    if (x < 0) {
        // Left side of bitmap is off-screen
        bxStart -= x*bxStep;
        x = 0;
    } else if (x+w > sw) {
        // Right side of bitmap is off-screen
        bxEnd -= (x+w-sw)*bxStep;
    }
    if (y < 0) {
        // Top side of bitmap is off-screen
        byStart -= y*byStep;
        y = 0;
    } else if (y+h > sh) {
        // Bottom side of bitmap is off-screen
        byEnd -= (y+h-sh)*byStep;
    }

    const int16_t origX = x;

    if (m) {
        uint16_t mw = (w+7) / 8;
        for (int16_t by = byStart ; by != byEnd ; by += byStep, y++) {
            const uint16_t* dptr = d + (by*w) + bxStart;
            const uint8_t* mptr = m + by*mw;
            x = origX;
            for (int16_t bx = bxStart ; bx != bxEnd ; bx += bxStep, x++) {
                if (mptr[bx>>3] & (0x80 >> (bx&7))) {
                    drawPixel(context, x, y, *dptr);
                }
                dptr += bxStep;
            }
        }
    } else {
        if (drawPixels  &&  bxStep == 1) {
            for (int16_t by = byStart ; by != byEnd ; by += byStep, y++) {
                const uint16_t* dptr = d + (by*w) + bxStart;
                drawPixels(context, x, y, dptr, bxEnd-bxStart);
            }
        } else {
            for (int16_t by = byStart ; by != byEnd ; by += byStep, y++) {
                const uint16_t* dptr = d + (by*w) + bxStart;
                x = origX;
                for (int16_t bx = bxStart ; bx != bxEnd ; bx += bxStep, x++) {
                    drawPixel(context, x, y, *dptr);
                    dptr += bxStep;
                }
            }
        }
    }
}

}

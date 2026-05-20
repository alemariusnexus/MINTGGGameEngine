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
    virtual void drawPixel(int32_t x, int32_t y, const Color& color) = 0;
    virtual void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color) = 0;
    virtual void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled = false) = 0;
    virtual void drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled = false) = 0;
    virtual void drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir = FlipDir::None) = 0;

    virtual Color readPixel(int32_t x, int32_t y) = 0;

    virtual void drawText(const Text& text, int32_t ox = 0, int32_t oy = 0);

    virtual bool saveScreenshot(const char* path);
    
    virtual void commit() = 0;

protected:
    virtual void drawGlyph (
        int32_t x, int32_t y,
        const uint8_t* d, uint8_t w, uint8_t h,
        uint16_t scale,
        const Color& color
        );

    template <typename DrawPixelT, typename DrawPixelsT, typename ContextT>
    void drawBitmapHelper (
        int32_t x, int32_t y,
        const Bitmap& bitmap,
        FlipDir flipDir,
        DrawPixelT drawPixel,
        DrawPixelsT drawPixels,
        ContextT userPtr
        );

private:
    template <bool forward>
    void drawTextLinear(const Text& text, int32_t px, int32_t py);

    void drawTextCenteredTC(const Text& text, int32_t px, int32_t py);
};


template <typename DrawPixelT, typename DrawPixelsT, typename ContextT>
void Screen::drawBitmapHelper (
    int32_t x, int32_t y,
    const Bitmap& bitmap,
    FlipDir flipDir,
    DrawPixelT drawPixel,
    DrawPixelsT drawPixels,
    ContextT context
) {
    const uint16_t sw = getWidth();
    const uint16_t sh = getHeight();

    const uint16_t w = bitmap.getWidth();
    const uint16_t h = bitmap.getHeight();

    const uint16_t* d = bitmap.getData();
    const uint8_t* m = bitmap.getMask();

    if (!d) {
        return;
    }

    int32_t byStart, byEnd, byStep;
    int32_t bxStart, bxEnd, bxStep;
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

    const int32_t origX = x;

    if (m) {
        uint16_t mw = (w+7) / 8;
        for (int32_t by = byStart ; by != byEnd ; by += byStep, y++) {
            const uint16_t* dptr = d + (by*w) + bxStart;
            const uint8_t* mptr = m + by*mw;
            x = origX;
            for (int32_t bx = bxStart ; bx != bxEnd ; bx += bxStep, x++) {
                if (mptr[bx>>3] & (0x80 >> (bx&7))) {
                    drawPixel(context, x, y, *dptr);
                }
                dptr += bxStep;
            }
        }
    } else {
        if (drawPixels  &&  bxStep == 1) {
            for (int32_t by = byStart ; by != byEnd ; by += byStep, y++) {
                const uint16_t* dptr = d + (by*w) + bxStart;
                drawPixels(context, x, y, dptr, bxEnd-bxStart);
            }
        } else {
            for (int32_t by = byStart ; by != byEnd ; by += byStep, y++) {
                const uint16_t* dptr = d + (by*w) + bxStart;
                x = origX;
                for (int32_t bx = bxStart ; bx != bxEnd ; bx += bxStep, x++) {
                    drawPixel(context, x, y, *dptr);
                    dptr += bxStep;
                }
            }
        }
    }
}

template <bool forward>
void Screen::drawTextLinear(const Text& text, int32_t px, int32_t py)
{
    const std::string& str = text.getText();
    const char* cptr;
    const char* cptrEnd;
    if constexpr(forward) {
        cptr = str.data();
        cptrEnd = cptr + str.length();
    } else {
        cptrEnd = str.data()-1;
        cptr = cptrEnd + str.length();
    }

    const Font& font = text.getFont();

    const uint8_t glyphWidth = font.getGlyphWidth();
    const uint8_t glyphHeight = font.getGlyphHeight();
    const uint16_t scaleFactor = text.getScaleFactor();
    const Color& color = text.getColor();

    const int32_t scaledGlyphWidth = static_cast<int32_t>(glyphWidth)*scaleFactor;
    const int32_t scaledGlyphHeight = static_cast<int32_t>(glyphHeight)*scaleFactor;

    if constexpr(!forward) {
        px -= scaledGlyphWidth;
        py -= scaledGlyphHeight;
    }

    const int32_t pxLineStart = px;

    while (cptr != cptrEnd) {
        const char c = *cptr;

        if (c == '\n') {
            px = pxLineStart;
            if constexpr(forward) {
                py += scaledGlyphHeight;
            } else {
                py -= scaledGlyphHeight;
            }
        } else {
            drawGlyph(px, py, font.getGlyphBuffer(c), glyphWidth, glyphHeight, scaleFactor, color);
            if constexpr(forward) {
                px += scaledGlyphWidth;
            } else {
                px -= scaledGlyphWidth;
            }
        }

        if constexpr(forward) {
            cptr++;
        } else {
            cptr--;
        }
    }
}

}

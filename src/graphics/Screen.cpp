#include "Screen.h"

namespace MINTGGGameEngine
{


void Screen::drawBitmapHelper (
    int16_t x, int16_t y,
    const Bitmap& bitmap,
    FlipDir flipDir,
    const std::function<void (int16_t x, int16_t y, uint16_t c)>& drawPixel
) {
    uint16_t w = bitmap.getWidth();
    uint16_t h = bitmap.getHeight();
    const uint16_t* d = bitmap.getData();
    const uint8_t* m = bitmap.getMask();

    int16_t origX = x;

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
                    drawPixel(x, y, d[by*w + bx]);
                }
            }
        }
    } else {
        for (int16_t by = byStart ; by != byEnd ; by += byStep, y++) {
            x = origX;
            for (int16_t bx = bxStart ; bx != bxEnd ; bx += bxStep, x++) {
                drawPixel(x, y, d[by*w + bx]);
            }
        }
    }
}


}

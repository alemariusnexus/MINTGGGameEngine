#include "Screen.h"

#include <cassert>

#include "../storage/File.h"


LOG_USE_TAG("Screen")


namespace MINTGGGameEngine
{


void Screen::drawText(const Text& text, int32_t ox, int32_t oy)
{
    const Text::HAlign halign = text.getHAlign();

    int32_t ax = text.getX();
    int32_t ay = text.getY();

    if (halign == Text::HAlign::Left) {
        text.transformAnchorPosition(Text::Anchor::TopLeft, &ax, &ay);
        drawTextLinear<true>(text, ax+ox, ay+oy);
    } else if (halign == Text::HAlign::Center) {
        text.transformAnchorPosition(Text::Anchor::TopCenter, &ax, &ay);
        drawTextCenteredTC(text, ax+ox, ay+oy);
    } else if (halign == Text::HAlign::Right) {
        text.transformAnchorPosition(Text::Anchor::BottomRight, &ax, &ay);
        drawTextLinear<false>(text, ax+ox, ay+oy);
    } else {
        assert(false);
    }
}

bool Screen::saveScreenshot(const char* path)
{
    // TODO: Support writing BMP files (based on extension maybe)
    File f(path);
    if (!f.open(File::WriteOnly)) {
        return false;
    }
    const uint16_t w = getWidth();
    const uint16_t h = getHeight();
    for (uint16_t y = 0 ; y < h ; y++) {
        for (uint16_t x = 0 ; x < w ; x++) {
            uint16_t pixel = readPixel(static_cast<int32_t>(x), static_cast<int32_t>(y)).toRGB565();
            f.write(&pixel, sizeof(pixel));
        }
    }
    f.flush();
    f.close();
    return true;
}

void Screen::drawGlyph (
    int32_t x, int32_t y,
    const uint8_t* d, uint8_t w, uint8_t h,
    uint16_t scale,
    const Color& color
) {
    const uint8_t byteW = (w+7) / 8;
    int32_t py = y;
    for (uint8_t oy = 0 ; oy < h ; oy++) {
        int32_t px = x;
        for (uint8_t ox = 0 ; ox < w ; ox++) {
            if (d[oy*byteW + (ox>>3)] & (0x80 >> (ox&0x7))) {
                // Draw single glyph pixel (possibly multiple display pixels if scaled)
                for (int32_t fy = py ; fy < py+scale ; fy++) {
                    for (int32_t fx = px ; fx < px+scale ; fx++) {
                        drawPixel(fx, fy, color);
                    }
                }
            }
            px += scale;
        }
        py += scale;
    }
}

void Screen::drawTextCenteredTC(const Text& text, int32_t px, int32_t py)
{
    const int32_t pxStart = px;

    const std::string& str = text.getText();
    const Font& font = text.getFont();

    const uint8_t glyphWidth = font.getGlyphWidth();
    const uint8_t glyphHeight = font.getGlyphHeight();
    const uint16_t scaleFactor = text.getScaleFactor();
    const Color& color = text.getColor();

    const int32_t scaledGlyphWidth = static_cast<int32_t>(glyphWidth)*scaleFactor;
    const int32_t scaledGlyphHeight = static_cast<int32_t>(glyphHeight)*scaleFactor;

    const char* cptr = str.data();
    const char* cptrEnd = cptr + str.length();

    const char* eolPtr;
    do {
        // Find end of line
        eolPtr = static_cast<const char*>(memchr(cptr, '\n', cptrEnd-cptr));
        if (!eolPtr) {
            eolPtr = cptrEnd;
        }

        // Calculate line width
        size_t lineNumChars = eolPtr - cptr;
        int32_t lineWidth = static_cast<int32_t>(lineNumChars * scaledGlyphWidth);

        // Draw single line
        px = pxStart - lineWidth/2;
        while (cptr != eolPtr) {
            const char c = *cptr;
            drawGlyph(px, py, font.getGlyphBuffer(c), glyphWidth, glyphHeight, scaleFactor, color);
            px += scaledGlyphWidth;
            cptr++;
        }

        cptr++;

        py += scaledGlyphHeight;
    } while (eolPtr != cptrEnd);
}


}

/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "DrawSurface.h"

#include <cassert>

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <QImage>
#   include <QString>
#endif

#include "../../storage/File.h"
#include "../../storage/StorageEngine.h"
#include "../../util/MathUtils.h"


LOG_USE_TAG("DrawSurface")


namespace MINTGGGameEngine
{

void DrawSurface::resetClipRegion()
{
    setClipRegion(0, 0, getWidth(), getHeight());
}

void DrawSurface::drawText(const Text& text, int32_t ox, int32_t oy)
{
    if (text.getText().empty()) {
        return;
    }

    uint16_t clipX, clipY, clipW, clipH;
    getClipRegion(&clipX, &clipY, &clipW, &clipH);

    const Text::HAlign halign = text.getHAlign();

    int32_t ax = text.getX();
    int32_t ay = text.getY();

    // TODO: Maybe implement better clipping. We could calculate the bounding rect, but that would require us to
    //  calculate font metrics, which can itself be kinda costly (transformAnchorPosition() below sometimes does it,
    //  but only if a transform is necessary).
    if (ax >= clipX+clipW  ||  ay >= clipY+clipH) {
        return;
    }

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

bool DrawSurface::saveScreenshot(const std::string_view& path)
{
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    const auto w = getWidth();
    const auto h = getHeight();

    QImage screenImg(w, h, QImage::Format_RGB888);

    for (uint16_t y = 0 ; y < h ; y++) {
        for (uint16_t x = 0 ; x < w ; x++) {
            Color pixel = readPixel(x, y);
            screenImg.setPixelColor(x, y, QColor(pixel.getRed(), pixel.getGreen(), pixel.getBlue()));
        }
    }

    return screenImg.save(QString::fromStdString(
        StorageEngine::getInstance().resolvePath(path)
        ));
#else
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
#endif
}

void DrawSurface::drawGlyph (
    int32_t x, int32_t y,
    const uint8_t* d, uint8_t w, uint8_t h,
    uint16_t scaleX, uint16_t scaleY,
    const Color& color
) {
    uint16_t clipX, clipY, clipW, clipH;
    getClipRegion(&clipX, &clipY, &clipW, &clipH);

    const int32_t origScaledW = w * scaleX;
    const int32_t origScaledH = h * scaleY;

    int32_t clippedX = x;
    int32_t clippedY = y;
    int32_t scaledW = origScaledW;
    int32_t scaledH = origScaledH;
    if (!ClipRectToRectWH<int32_t>(clippedX, clippedY, scaledW, scaledH, clipX, clipY, clipW, clipH)) {
        // Fully outside clipping area -> skip it
        return;
    }

    uint8_t colorRaw[8];
    color.toBuffer(colorRaw, getNativePixelFormat(), getNativeByteOrder());

    const uint8_t byteW = (w+7) / 8;
    int32_t py = y;
    auto dPtr = d;

    if (clippedX == x  &&  clippedY == y  &&  scaledW == origScaledW  &&  scaledH == origScaledH) {
        [[likely]]
        // Fully inside clipping area -> perform fast drawing without coordinate checking

        for (uint_fast8_t oy = 0 ; oy < h ; oy++) {
            int32_t px = x;
            for (uint_fast8_t ox = 0 ; ox < w ; ox++) {
                if (dPtr[(ox>>3)] & (0x80 >> (ox&0x7))) {
                    // Draw single glyph pixel (possibly multiple display pixels if scaled)
                    for (int_fast32_t fy = py ; fy < py+scaleY ; fy++) {
                        for (int_fast32_t fx = px ; fx < px+scaleX ; fx++) {
                            drawPixelFastRaw(fx, fy, colorRaw);
                        }
                    }
                }
                px += scaleX;
            }
            py += scaleY;
            dPtr += byteW;
        }
    } else {
        [[unlikely]]
        // Partially clipped -> check each drawn pixel's bounds

        for (uint_fast8_t oy = 0 ; oy < h ; oy++) {
            int32_t px = x;
            for (uint_fast8_t ox = 0 ; ox < w ; ox++) {
                if (dPtr[(ox>>3)] & (0x80 >> (ox&0x7))) {
                    // Draw single glyph pixel (possibly multiple display pixels if scaled)
                    for (int_fast32_t fy = py ; fy < py+scaleY ; fy++) {
                        for (int_fast32_t fx = px ; fx < px+scaleX ; fx++) {
                            drawPixelRaw(fx, fy, colorRaw);
                        }
                    }
                }
                px += scaleX;
            }
            py += scaleY;
            dPtr += byteW;
        }
    }
}

void DrawSurface::drawTextCenteredTC(const Text& text, int32_t px, int32_t py)
{
    const int32_t pxStart = px;

    const std::string& str = text.getText();
    const Font& font = text.getFont();

    if (!font) {
        return;
    }

    const uint8_t glyphWidth = font.getGlyphWidth();
    const uint8_t glyphHeight = font.getGlyphHeight();
    const uint16_t scaleX = text.getScaleX();
    const uint16_t scaleY = text.getScaleY();
    const Color& color = text.getColor();

    const int32_t scaledGlyphWidth = static_cast<int32_t>(glyphWidth)*scaleX;
    const int32_t scaledGlyphHeight = static_cast<int32_t>(glyphHeight)*scaleY;

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
            drawGlyph(px, py, font.getGlyphBuffer(c), glyphWidth, glyphHeight, scaleX, scaleY, color);
            px += scaledGlyphWidth;
            cptr++;
        }

        cptr++;

        py += scaledGlyphHeight;
    } while (eolPtr != cptrEnd);
}


}

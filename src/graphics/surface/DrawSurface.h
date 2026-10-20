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

#pragma once

#include "../../Globals.h"

#include <string_view>

#include "../Bitmap.h"
#include "../Color.h"
#include "../Text.h"
#include "../../util/MathUtils.h"


namespace MINTGGGameEngine
{


/**
 * \brief Abstract base class for any surface that can be drawn on (e.g. screens or framebuffers).
 *
 * This class specifies the common interface for surfaces that the engine can draw on, and provides some useful
 * helper functions for implementing certain more complex drawing tasks (currently text and bitmaps).
 *
 * All drawing surfaces should support a clipping region: A region to which all drawing methods are constrained, unless
 * explicitly stated otherwise. All drawing functions have to handle coordinates that are outside the clipping region
 * or even outside the entire drawing surface correctly.
 *
 * Specific screen drivers will usually subclass the BufferedScreen or Screen class instead of using this class
 * directly.
 *
 * \see Screen
 * \see MemDrawSurface
 */
class DrawSurface
{
public:
    virtual ~DrawSurface() = default;

    /// \name Surface Properties
    ///@{

    /**
     * \brief Get the width of the drawing surface, in pixels.
     *
     * @return Width in pixels.
     */
    virtual uint16_t getWidth() const = 0;

    /**
     * \brief Get the height of the drawing surface, in pixels.
     *
     * @return Height in pixels.
     */
    virtual uint16_t getHeight() const = 0;

    size_t getSize() const { return static_cast<size_t>(getWidth()) * static_cast<size_t>(getHeight()); }

    float getCenterX() const { return static_cast<float>(getWidth()) * 0.5f; }
    float getCenterY() const { return static_cast<float>(getHeight()) * 0.5f; }
    Vec2 getCenter() const { return {getCenterX(), getCenterY()}; }

    virtual PixelFormat getNativePixelFormat() const = 0;

    virtual ByteOrder getNativeByteOrder() const = 0;

    ///@}


    /// \name Drawing Functions
    ///@{

    /**
     * \brief Fill the entire surface with a single color.
     *
     * \param color The color to fill with.
     * \param honorClipRegion true to fill only the current clipping region, false to fill the entire surface instead.
     */
    virtual void fill(const Color& color, bool honorClipRegion) = 0;

    /**
     * \brief Fills a single pixel with the given color.
     *
     * \param x The pixel's x coordinate.
     * \param y The pixel's y coordinate.
     * \param color The color to use.
     */
    virtual void drawPixel(int32_t x, int32_t y, const Color& color) = 0;

    /**
     * \brief Fills a single pixel with the given color, without checking if it's inside the clipping region.
     *
     * The caller is responsible for ensuring that the coordinates lie within the clipping region. If they don't,
     * behavior is undefined (could result in memory corruption or crashes!).
     *
     * \param x The pixel's x coordinate.
     * \param y The pixel's y coordinate.
     * \param color The color to use.
     */
    virtual void drawPixelFast(int32_t x, int32_t y, const Color& color) { drawPixel(x, y, color); }

    virtual void drawPixelRaw(int32_t x, int32_t y, const void* data) { drawPixelsRaw(0, 0, 1, 1, 1, 1, x, y, data); }

    virtual void drawPixelFastRaw(int32_t x, int32_t y, const void* data) { drawPixelRaw(x, y, data); }

    virtual void drawPixelsRaw (
        uint16_t srcX, uint16_t srcY,
        uint16_t srcW, uint16_t srcH,
        uint16_t blockW, uint16_t blockH,
        int32_t destX, int32_t destY,
        const void* data
        ) = 0;

    /**
     * \brief Draw a horizontal line, i.e. a line parallel to the X axis.
     *
     * \param x0 x coordinate of the line's starting point.
     * \param y0 y coordinate of the line's starting point.
     * \param w Width (length) of the line, in pixels.
     * \param color The color to use.
     */
    virtual void drawHLine(int32_t x0, int32_t y0, int32_t w, const Color& color) = 0;

    /**
     * \brief Draw a vertical line, i.e. a line parallel to the Y axis.
     *
     * \param x0 x coordinate of the line's starting point.
     * \param y0 y coordinate of the line's starting point.
     * \param h Height (length) of the line, in pixels.
     * \param color The color to use.
     */
    virtual void drawVLine(int32_t x0, int32_t y0, int32_t h, const Color& color) = 0;

    /**
     * \brief Draw an arbitrary line between two points.
     *
     * The exact algorithm used to determine which pixels to fill is implementation-defined.
     *
     * \param x0 x coordinate of the line's starting point.
     * \param y0 y coordinate of the line's starting point.
     * \param x1 x coordinate of the line's ending point.
     * \param y1 y coordinate of the line's ending point.
     * \param color The color to use.
     */
    virtual void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color) = 0;

    /**
     * \brief Draw an axis-aligned rectangle, either filled or just an outline.
     *
     * \param x x coordinate of one of the rectangle's corners (usually top-left)
     * \param y y coordinate of one of the rectangle's corners (usually top-left)
     * \param w Width of the rectangle in pixels. May be negative to invert the drawing direction.
     * \param h Height of the rectangle in pixels. May be negative to invert the drawing direction.
     * \param color The color to use.
     * \param filled true to fill the rectangle with the color, false to draw the outline only.
     */
    virtual void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled) = 0;

    /**
     * \brief Draw a circle, either filled or just an outline.
     *
     * The exact algorithm used to determine which pixels to fill is implementation-defined.
     *
     * \param cx Circle's center x coordinate.
     * \param cy Circle's center y coordinate.
     * \param r Circle's radius, in pixels.
     * \param color The color to use.
     * \param filled true to fill the rectangle with the color, false to draw the outline only.
     */
    virtual void drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled) = 0;

    /**
     * \brief Draw a bitmap.
     *
     * The bitmap can optionally be flipped prior to drawing.
     *
     * Subclasses can use the protected helper method drawBitmapHelper() to do most of the actual work.
     *
     * \param x x coordinate where the top-left pixel of the bitmap should be drawn.
     * \param y y coordinate where the top-left pixel of the bitmap should be drawn.
     * \param bitmap The bitmap to draw. Passing a null bitmap or a bitmap without color data is supported and will
     *      result in nothing being drawn.
     * \param flipDir The direction in which to flip the pixels of the bitmap prior to drawing.
     */
    virtual void drawBitmap (
        int32_t x, int32_t y,
        const Bitmap& bitmap,
        FlipDir flipDir,
        Rotation rotation,
        int16_t scaleX, int16_t scaleY
        ) = 0;

    /**
     * \brief Draw text.
     *
     * All rendering properties are taken from the Text object itself.
     *
     * Note that the coordinates passed to this function are those of the Text object's anchor position.
     *
     * Subclasses should not have to override this method, since it just calls drawPixel() internally.
     *
     * \param text The text to draw.
     * \param ox x coordinate at which the text is drawn.
     * \param oy y coordinate at which the text is drawn.
     */
    virtual void drawText(const Text& text, int32_t ox, int32_t oy);

    ///@}


    /// \name Clipping
    ///@{

    /**
     * \brief Set the region on the screen that all subsequent drawing will be limited to.
     *
     * Any pixels outside the clipping region will not be touched by any of the subsequently called drawing functions,
     * unless explicitly stated otherwise.
     *
     * Note that the coordinates passed to this function will themselves be clipped to ensure the clipping are lies
     * entirely within the valid drawing surface.
     *
     * By default, the clipping region is set to the entire surface.
     *
     * \param cx x coordinate of the top-left corner of the clipping region.
     * \param cy y coordinate of the top-left corner of the clipping region.
     * \param cw Width of the clipping region, in pixels.
     * \param ch Height of the clipping region, in pixels.
     * \see resetClipRegion()
     */
    virtual void setClipRegion(int16_t cx, int16_t cy, int16_t cw, int16_t ch) = 0;

    /**
     * \brief Get the currently used clipping region.
     *
     * The values returned are guaranteed to lie within the valid area of the drawing surface.
     *
     * \param cx x coordinate of the top-left corner of the clipping region.
     * \param cy y coordinate of the top-left corner of the clipping region.
     * \param cw Width of the clipping region, in pixels.
     * \param ch Height of the clipping region, in pixels.
     */
    virtual void getClipRegion(uint16_t* cx, uint16_t* cy, uint16_t* cw, uint16_t* ch) = 0;

    /**
     * \brief Set the clipping region to be the entire surface.
     *
     * \see setClipRegion()
     */
    void resetClipRegion();

    ///@}


    /// \name Retrieving pixel data
    ///@{

    /**
     * \brief Get the color of a single pixel on the surface.
     *
     * \param x The pixel's x coordinate.
     * \param y The pixel's y coordinate.
     * \return The pixel's current color.
     */
    virtual Color readPixel(int32_t x, int32_t y) = 0;

    ///@}


    /// \name Miscellaneous
    ///@{

    /**
     * \brief Save a screenshot of the entire drawing surface to a file.
     *
     * Currently, this will dump the entire surface data into a raw RGB565 file, without any kind of header or other
     * metadata. It may support exporting to BMP files in the future, based on the file extension.
     *
     * \param path The path to save the screenshot to.
     * \return true if successful, false otherwise.
     */
    virtual bool saveScreenshot(const std::string_view& path);

    ///@}

protected:
    /// \name Helpers for subclasses
    ///@{

    /**
     * \brief Draw a single glyph of text.
     *
     * \param x x coordinate where the top-left pixel of the glyph is drawn.
     * \param y y coordinate where the top-left pixel of the glyph is drawn.
     * \param d The raw glyph data (1 bit per glyph, padded to a full byte per line).
     * \param w Width of the glyph, in pixels.
     * \param h Height of the glyph, in pixels.
     * \param scale Scale factor. A scale factor of 1 draws the glyph as-is, a factor of 3 will draw each glyph
     *      pixel three times in both x and y directions.
     * \param color The color to use for drawing.
     */
    virtual void drawGlyph (
        int32_t x, int32_t y,
        const uint8_t* d, uint8_t w, uint8_t h,
        uint16_t scaleX, uint16_t scaleY,
        const Color& color
        );

    ///@}

private:
    template <bool forward>
    void drawTextLinear(const Text& text, int32_t px, int32_t py);

    void drawTextCenteredTC(const Text& text, int32_t px, int32_t py);
};



template <bool forward>
void DrawSurface::drawTextLinear(const Text& text, int32_t px, int32_t py)
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
            drawGlyph(px, py, font.getGlyphBuffer(c), glyphWidth, glyphHeight, scaleX, scaleY, color);
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

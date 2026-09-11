#pragma once

#include "../../Globals.h"

#include "../Bitmap.h"
#include "../Color.h"
#include "../Text.h"


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
    virtual void drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir) = 0;

    /**
     * \brief Draw text.
     *
     * All rendering properties are taken from the Text object itself.
     *
     * Note that the coordinates passed to this function are those of the Text object's anchor position.
     *
     * Subclasses should not have to override this method, since it just calls drawPixel() internally.
     *
     * @param text The text to draw.
     * @param ox x coordinate at which the text is drawn.
     * @param oy y coordinate at which the text is drawn.
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
    virtual void setClipRegion(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch) = 0;

    /**
     * \brief Get the currently used clipping region.
     *
     * The values returned are guaranteed to lie within the valid are of the drawing surface.
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
    virtual bool saveScreenshot(const char* path);

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
        uint16_t scale,
        const Color& color
        );

    /**
     * \brief Helper function for drawing a bitmap.
     *
     * This method does all the actual drawing, but uses functions passed as templates to draw individual pixels. It's
     * done this way to optimize drawing performance, allowing the compiler to inline the many individual drawing calls,
     * and to provide a potentially faster way for drawing a consecutive row of pixels.
     *
     * @tparam DrawPixelT The function to call for drawing a single pixel. It will be called with four arguments: First
     *      argument is the context passed to this method. Second and third argument are the x and y coordinate of the
     *      pixel. Fourth argument is the pixel's raw color (as uint16_t in RGB565 format). The pixel coordinates are
     *      guaranteed to be valid, so the function does not need to check them (and probably shouldn't for performance
     *      reasons).
     * @tparam DrawPixelsT The function to call for drawing a consecutive line of pixels, all with the same y
     *      coordinate. This must be provided in addition to DrawPixelT for performance reasons. It is called with five
     *      arguments: First argument is the context passed to this method. Second and third argument are the x and y
     *      coordinate of the first pixel to draw. Fourth argument is a pointer to the raw pixel color values (array
     *      of uint16_t values in RGB565 format). Fifth argument is the number of pixels to draw. The pixel coordinates
     *      are guaranteed to be valid, and all pixels are guaranteed to be consecutive within the same line.
     * @tparam ContextT An arbitrary user-defined value that is passed along to the drawPixel and drawPixels functions.
     *      This method doesn't use it in any way other than passing it along.
     * @param x x coordinate at which to draw the top-left pixel of the bitmap.
     * @param y y coordinate at which to draw the top-left pixel of the bitmap.
     * @param bitmap Bitmap to draw.
     * @param flipDir Direction in which to flip the bitmap's pixels prior to drawing.
     * @param drawPixel Function used to draw a single pixel.
     * @param drawPixels Function used to draw a consecutive line of pixels with the same y coordinate.
     * @param userPtr The user-defined context.
     */
    template <typename DrawPixelT, typename DrawPixelsT, typename ContextT>
    void drawBitmapHelper (
        int32_t x, int32_t y,
        const Bitmap& bitmap,
        FlipDir flipDir,
        DrawPixelT drawPixel,
        DrawPixelsT drawPixels,
        ContextT userPtr
        );

    ///@}

private:
    template <bool forward>
    void drawTextLinear(const Text& text, int32_t px, int32_t py);

    void drawTextCenteredTC(const Text& text, int32_t px, int32_t py);
};



template <typename DrawPixelT, typename DrawPixelsT, typename ContextT>
void DrawSurface::drawBitmapHelper (
    int32_t x, int32_t y,
    const Bitmap& bitmap,
    FlipDir flipDir,
    DrawPixelT drawPixel,
    DrawPixelsT drawPixels,
    ContextT context
) {
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

    uint16_t clipX, clipY, clipW, clipH;
    getClipRegion(&clipX, &clipY, &clipW, &clipH);

    if (x >= clipX+clipW  ||  y >= clipY+clipH) {
        // Bitmap is completely off-screen (high side)
        return;
    }
    if (x+w <= clipX  ||  y+h <= clipY) {
        // Bitmap is completely off-screen (low side)
        return;
    }

    if (x < clipX) {
        // Left side of bitmap is off-screen
        bxStart += (clipX-x)*bxStep;
        x = clipX;
    } else if (x+w > clipX+clipW) {
        // Right side of bitmap is off-screen
        bxEnd -= (x+w-clipX-clipW)*bxStep;
    }
    if (y < clipY) {
        // Top side of bitmap is off-screen
        byStart += (clipY-y)*byStep;
        y = clipY;
    } else if (y+h > clipY+clipH) {
        // Bottom side of bitmap is off-screen
        byEnd -= (y+h-clipY-clipH)*byStep;
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
    }}

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
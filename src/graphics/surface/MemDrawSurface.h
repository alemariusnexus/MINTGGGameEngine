#pragma once

#include "../../Globals.h"

#include <functional>

#include "DrawSurface.h"
#include "../../util/Util.h"

namespace MINTGGGameEngine
{


/**
 * \brief A drawing surface that draws onto a memory-based framebuffer.
 *
 * This implements all the basic drawing functions and is useful as a backend for implementing screens that use
 * double buffering.
 */
class MemDrawSurface : public MINTGGGameEngine::DrawSurface
{
public:
    typedef std::function<void(uint8_t*)> delete_fn_t;

private:
    struct DrawBitmapContext
    {
        uint16_t* buf;
        uint16_t width;
    };

    struct LineClipContext
    {
        float xMin;
        float yMin;
        float xMax;
        float yMax;
    };

public:
    /**
     * \brief Create a memory drawing surface with a user-provided buffer.
     *
     * Ownership of this buffer can be managed by passing a custom delete function that will be called when this
     * object is destroyed.
     *
     * Note that the caller is responsible for ensuring that the buffer fulfills all requirements that might be
     * necessary depending on what the buffer is used for (e.g. 16-bit aligned for RGB565 data, allocated in
     * DMA-capable memory for certain screen drivers, etc.).
     *
     * @param buffer The buffer to draw into.
     * @param width Width of the surface, in pixels.
     * @param height Height of the surface, in pixels.
     * @param del Function that is called with the buffer when this object is destroyed.
     * @param swapEndianness true to swap the endianness of all pixels drawn to and read from the buffer, false
     *      otherwise. Useful if the buffer is transferred directly to an external device that has a different
     *      endianness than the processor, e.g. using a DMA.
     */
    MemDrawSurface(uint8_t* buffer, uint16_t width, uint16_t height, const delete_fn_t& del, bool swapEndianness = false);

    /**
     * \brief Destroy this object.
     *
     * This will call the delete function passed to the constructor, which can optionally delete the buffer.
     */
    ~MemDrawSurface() override;


    /**
     * \brief Return a pointer to the internal buffer.
     *
     * @return Pointer to the buffer.
     */
    uint8_t* getBuffer() { return buf; }

    /**
     * \brief Return a pointer to the internal buffer.
     *
     * @return Pointer to the buffer.
     */
    const uint8_t* getBuffer() const { return buf; }

    uint16_t getWidth() const override;
    uint16_t getHeight() const override;

    void fill(const Color& color, bool honorClipRegion) override;
    void drawPixel(int32_t x, int32_t y, const Color& color) override;
    void drawHLine(int32_t x0, int32_t y0, int32_t w, const Color& color) override;
    void drawVLine(int32_t x0, int32_t y0, int32_t h, const Color& color) override;

    /**
     * \copydoc DrawSurface::drawLine()
     *
     * This class currently uses Bresenham's algorithm for line drawing, without any anti-aliasing.
     */
    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color) override;
    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled) override;

    /**
     * \copydoc DrawSurface::drawCircle()
     *
     * This class currently uses the midpoint circle algorithm, without any anti-aliasing.
     */
    void drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled) override;
    void drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir) override;

    void setClipRegion(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch) override;
    void getClipRegion(uint16_t* cx, uint16_t* cy, uint16_t* cw, uint16_t* ch) override;

    Color readPixel(int32_t x, int32_t y) override;

private:
    static void drawBitmapHelper_drawPixel(DrawBitmapContext* ctx, int32_t x, int32_t y, uint16_t c)
    {
        ctx->buf[y*ctx->width + x] = c;
    }
    static void drawBitmapHelper_drawPixelSwapped(DrawBitmapContext* ctx, int32_t x, int32_t y, uint16_t c)
    {
        ctx->buf[y*ctx->width + x] = SwapEndianness(c);
    }

    static void drawBitmapHelper_drawPixels(DrawBitmapContext* ctx, int32_t x, int32_t y, const uint16_t* c, int32_t w)
    {
        memcpy(ctx->buf + y*ctx->width + x, c, w*sizeof(uint16_t));
    }
    static void drawBitmapHelper_drawPixelsSwapped(DrawBitmapContext* ctx, int32_t x, int32_t y, const uint16_t* c, int32_t w)
    {
        uint16_t* buf = ctx->buf + y*ctx->width + x;
        while (w != 0) {
            *buf++ = SwapEndianness(*c++);
            w--;
        }
    }

    static int cohenSutherlandComputeOutCode(float x, float y, const LineClipContext& ctx);
    bool clipLine(float& x0, float& y0, float& x1, float& y1) const;

private:
    uint8_t* buf;
    uint16_t width;
    uint16_t height;
    delete_fn_t del;

    uint16_t clipX;
    uint16_t clipY;
    uint16_t clipW;
    uint16_t clipH;

    bool swapEndianness;
};

}

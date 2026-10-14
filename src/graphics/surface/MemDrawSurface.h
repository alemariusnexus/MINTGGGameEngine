#pragma once

#include "../../Globals.h"

#include <functional>

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   ifdef MINTGGGAMEENGINE_CHIP_ESP32P4
#       define MINTGGGAMEENGINE_MEMDRAWSURFACE_ESP_PPA_SUPPORTED
#endif
#endif

#ifdef MINTGGGAMEENGINE_MEMDRAWSURFACE_ESP_PPA_SUPPORTED
#   include <driver/ppa.h>
#endif

#include "../../platform/PlatformManager.h"
#include "../../util/Util.h"
#include "../BitmapRenderer.h"
#include "DrawSurface.h"

namespace MINTGGGameEngine
{


/**
 * \brief A drawing surface that draws onto a memory-based framebuffer.
 *
 * This implements all the basic drawing functions and is useful as a backend for implementing screens that use
 * double buffering.
 */
class MemDrawSurface : public DrawSurface
{
public:
    typedef std::function<void(void*)> delete_fn_t;

private:
    struct DrawBitmapContext
    {
        void* buf;
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
     * \param pixelFormat The pixel format for the buffer.
     * \param buffer The buffer to draw into.
     * \param width Width of the surface, in pixels.
     * \param height Height of the surface, in pixels.
     * \param del Function that is called with the buffer when this object is destroyed.
     */
    MemDrawSurface (
        PixelFormat pixelFormat,
        ByteOrder byteOrder,
        void* buffer,
        uint16_t width, uint16_t height,
        const delete_fn_t& del
        );

    /**
     * \brief Destroy this object.
     *
     * This will call the delete function passed to the constructor, which can optionally delete the buffer.
     */
    ~MemDrawSurface() override;

    void setBuffer(void* buffer, const delete_fn_t& del);

    /**
     * \brief Return a pointer to the internal buffer.
     *
     * @return Pointer to the buffer.
     */
    void* getBuffer() { return buf; }

    /**
     * \brief Return a pointer to the internal buffer.
     *
     * @return Pointer to the buffer.
     */
    const void* getBuffer() const { return buf; }

    uint16_t getWidth() const override;
    uint16_t getHeight() const override;

    PixelFormat getNativePixelFormat() const override;
    ByteOrder getNativeByteOrder() const override;

    void fill(const Color& color, bool honorClipRegion) override;

    void drawPixel(int32_t x, int32_t y, const Color& color) override;
    void drawPixelFast(int32_t x, int32_t y, const Color& color) override;
    void drawPixelRaw(int32_t x, int32_t y, const void* data) override;
    void drawPixelFastRaw(int32_t x, int32_t y, const void* data) override;
    void drawPixelsRaw (
        uint16_t srcX, uint16_t srcY,
        uint16_t srcW, uint16_t srcH,
        uint16_t blockW, uint16_t blockH,
        int32_t destX, int32_t destY,
        const void* data
        ) override;

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

    void drawBitmap (
        int32_t x, int32_t y,
        const Bitmap& bitmap,
        FlipDir flipDir,
        Rotation rotation,
        int16_t scaleX, int16_t scaleY
        ) override;

    void setClipRegion(int16_t cx, int16_t cy, int16_t cw, int16_t ch) override;
    void getClipRegion(uint16_t* cx, uint16_t* cy, uint16_t* cw, uint16_t* ch) override;

    Color readPixel(int32_t x, int32_t y) override;

    BitmapRenderer& getBitmapRenderer() { return bmpRenderer; }

private:
    static int cohenSutherlandComputeOutCode(float x, float y, const LineClipContext& ctx);
    bool clipLine(float& x0, float& y0, float& x1, float& y1) const;

#ifdef MINTGGGAMEENGINE_MEMDRAWSURFACE_ESP_PPA_SUPPORTED
    bool fillRectESPPPA(const Color& color, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
#endif

private:
    uint8_t* getBufferPtr(uint16_t x, uint16_t y)
            { return reinterpret_cast<uint8_t*>(buf) + (y*width + x)*bytesPerPixel; }
    const uint8_t* getBufferPtr(uint16_t x, uint16_t y) const
            { return reinterpret_cast<const uint8_t*>(buf) + (y*width + x)*bytesPerPixel; }

private:
    PixelFormat pixelFormat;
    ByteOrder byteOrder;
    uint8_t bytesPerPixel;

    void* buf;
    uint16_t width;
    uint16_t height;
    delete_fn_t deleteFn;

    uint16_t clipX;
    uint16_t clipY;
    uint16_t clipW;
    uint16_t clipH;

    BitmapRenderer bmpRenderer;

#ifdef MINTGGGAMEENGINE_MEMDRAWSURFACE_ESP_PPA_SUPPORTED
    ppa_client_handle_t ppaFillClient;
#endif
};

}

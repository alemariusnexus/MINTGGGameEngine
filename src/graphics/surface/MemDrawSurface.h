#pragma once

#include "../../Globals.h"

#include <functional>

#include "DrawSurface.h"
#include "util/Util.h"

namespace MINTGGGameEngine
{

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
    MemDrawSurface(uint8_t* buffer, uint16_t width, uint16_t height, const delete_fn_t& del, bool swapEndianness = false);
    ~MemDrawSurface() override;

    uint8_t* getBuffer() { return buf; }
    const uint8_t* getBuffer() const { return buf; }

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

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

#include "MemDrawSurface.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "../../platform/PlatformManager.h"
#include "../../util/Log.h"
#include "../../util/MathUtils.h"
#include "../../util/Util.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <esp_cache.h>
#endif


// TODO: Find a proper threshold. This was just a very rough guess
#define PPA_FILL_SIZE_THRESHOLD (80*1024)


LOG_USE_TAG("MemDrawSurface")


namespace MINTGGGameEngine
{


MemDrawSurface::MemDrawSurface (
    PixelFormat pixelFormat,
    ByteOrder byteOrder,
    void* buffer,
    uint16_t width, uint16_t height,
    const delete_fn_t& del
)
    : pixelFormat(pixelFormat), byteOrder(byteOrder),
      bytesPerPixel(CalcPixelFormatBytesPerPixel(pixelFormat)),
      buf(nullptr), width(width), height(height),
      clipX(0), clipY(0), clipW(width), clipH(height)
{
    setBuffer(buffer, del);

#ifdef MINTGGGAMEENGINE_MEMDRAWSURFACE_ESP_PPA_SUPPORTED
    ppaFillClient = nullptr;

    // Skip PPA initialization for small surfaces, to avoid wasting time here here.
    if (getSize() >= PPA_FILL_SIZE_THRESHOLD) {
        ppa_client_config_t ppaCfg = {
            .oper_type = PPA_OPERATION_FILL,
            .max_pending_trans_num = 1,
            .data_burst_length = PPA_DATA_BURST_LENGTH_128
        };
        esp_err_t err = ppa_register_client(&ppaCfg, &ppaFillClient);
        if (err != ESP_OK) {
            LogError("Error registering PPA fill client: %s", esp_err_to_name(err));
        }
    }
#endif
}

MemDrawSurface::~MemDrawSurface()
{
    if (deleteFn) {
        deleteFn(buf);
    }
    buf = nullptr;
}

void MemDrawSurface::setBuffer(void* buffer, const delete_fn_t& del)
{
    if (this->buf  &&  deleteFn) {
        deleteFn(buf);
    }

    // TODO: Check buffer alignment
    this->buf = buffer;
    deleteFn = del;
}

uint16_t MemDrawSurface::getWidth() const
{
    return width;
}

uint16_t MemDrawSurface::getHeight() const
{
    return height;
}

PixelFormat MemDrawSurface::getNativePixelFormat() const
{
    return pixelFormat;
}

ByteOrder MemDrawSurface::getNativeByteOrder() const
{
    return byteOrder;
}

void MemDrawSurface::fill(const Color& color, bool honorClipRegion)
{
    if (!buf) {
        return;
    }

    const auto sw = getWidth();

    uint16_t curClipX, curClipY, curClipW, curClipH;
    if (honorClipRegion) {
        curClipX = clipX;
        curClipY = clipY;
        curClipW = clipW;
        curClipH = clipH;
    } else {
        curClipX = 0;
        curClipY = 0;
        curClipW = sw;
        curClipH = getHeight();
    }

#ifdef MINTGGGAMEENGINE_MEMDRAWSURFACE_ESP_PPA_SUPPORTED
    if (fillRectESPPPA(color, curClipX, curClipY, curClipW, curClipH)) {
        return;
    }
#endif

    if (pixelFormat == PixelFormat::RGB565  ||  pixelFormat == PixelFormat::BGR565) {
        const uint16_t rawColor = FromNativeByteOrder(
            (pixelFormat == PixelFormat::RGB565) ? color.toRGB565() : color.toBGR565(), byteOrder);

        const uint16_t ex = curClipX + curClipW;
        const uint16_t ey = curClipY + curClipH;
        for (uint16_t y = curClipY ; y < ey ; y++) {
            uint16_t* buf16 = reinterpret_cast<uint16_t*>(buf) + y*sw + curClipX;
            for (uint16_t x = curClipX ; x < ex ; x++) {
                *buf16++ = rawColor;
            }
        }
    } else if (pixelFormat == PixelFormat::RGB888  ||  pixelFormat == PixelFormat::BGR888) {
        uint8_t r, g, b;
        color.toRGB888(r, g, b);

        const uint16_t ex = curClipX + curClipW;
        const uint16_t ey = curClipY + curClipH;

        if (pixelFormat == PixelFormat::RGB888) {
            for (uint16_t y = curClipY ; y < ey ; y++) {
                uint8_t* buf8 = reinterpret_cast<uint8_t*>(buf) + (y*sw + curClipX)*3;
                for (uint16_t x = curClipX ; x < ex ; x++) {
                    buf8[0] = r;
                    buf8[1] = g;
                    buf8[2] = b;
                    buf8 += 3;
                }
            }
        } else {
            for (uint16_t y = curClipY ; y < ey ; y++) {
                uint8_t* buf8 = reinterpret_cast<uint8_t*>(buf) + (y*sw + curClipX)*3;
                for (uint16_t x = curClipX ; x < ex ; x++) {
                    buf8[0] = b;
                    buf8[1] = g;
                    buf8[2] = r;
                    buf8 += 3;
                }
            }
        }
    } else if (pixelFormat == PixelFormat::RGBX8888  ||  pixelFormat == PixelFormat::XBGR8888) {
        uint8_t rawColor[4];
        color.toBuffer(rawColor, pixelFormat, byteOrder);

        const uint16_t ex = curClipX + curClipW;
        const uint16_t ey = curClipY + curClipH;

        for (uint16_t y = curClipY ; y < ey ; y++) {
            uint8_t* buf8 = reinterpret_cast<uint8_t*>(buf) + (y*sw + curClipX)*4;
            for (uint16_t x = curClipX ; x < ex ; x++) {
                memcpy(buf8, rawColor, 4);
                buf8 += 4;
            }
        }
    } else {
        assert(false);
    }
}

void MemDrawSurface::drawPixel(int32_t x, int32_t y, const Color& color)
{
    if (!buf) {
        return;
    }
    if (x < clipX  ||  y < clipY) {
        return;
    }
    if (x >= clipX+clipW  ||  y >= clipY+clipH) {
        return;
    }
    color.toBuffer(getBufferPtr(x, y), pixelFormat, byteOrder);
}

void MemDrawSurface::drawPixelFast(int32_t x, int32_t y, const Color& color)
{
    if (!buf) {
        return;
    }
    color.toBuffer(getBufferPtr(x, y), pixelFormat, byteOrder);
}

void MemDrawSurface::drawPixelRaw(int32_t x, int32_t y, const void* data)
{
    if (x < clipX  ||  x >= clipX+clipW  ||  y < clipY  ||  y >= clipY+clipH) {
        return;
    }
    memcpy(getBufferPtr(x, y), data, bytesPerPixel);
}

void MemDrawSurface::drawPixelFastRaw(int32_t x, int32_t y, const void* data)
{
    // A bit of micro-optimization for common sizes
    if (bytesPerPixel == 2) {
        *reinterpret_cast<uint16_t*>(getBufferPtr(x, y)) = *reinterpret_cast<const uint16_t*>(data);
    } else if (bytesPerPixel == 3) {
        memcpy(getBufferPtr(x, y), data, 3);
    } else {
        [[unlikely]]
        memcpy(getBufferPtr(x, y), data, bytesPerPixel);
    }
}

void MemDrawSurface::drawPixelsRaw (
    uint16_t srcX, uint16_t srcY,
    uint16_t srcW, uint16_t srcH,
    uint16_t blockW, uint16_t blockH,
    int32_t destX, int32_t destY,
    const void* data
) {
    BitmapRenderer::Transfer transfer = {
        .src = {
            .buffer = data,
            .alpha = nullptr,
            .width = srcW,
            .height = srcH,
            .blockOffsetX = srcX,
            .blockOffsetY = srcY,
            .blockWidth = blockW,
            .blockHeight = blockH,
            .pixelFormat = pixelFormat,
            .alphaStorage = AlphaStorage::Bit1, // Ignored
            .byteOrder = byteOrder
        },

        .dest = {
            .buffer = buf,
            .alpha = nullptr,
            .width = width,
            .height = height,
            .blockOffsetX = 0, // Calculated by prepareClippedTransfer()
            .blockOffsetY = 0, // Calculated by prepareClippedTransfer()
            .pixelFormat = pixelFormat,
            .alphaStorage = AlphaStorage::Bit1, // Ignored
            .byteOrder = byteOrder
        },

        .blendMode = BitmapRenderer::BlendMode::Opaque, // Ignored
        .alphaMode = BitmapRenderer::AlphaMode::KeepDestAlpha // Ignored
    };

    BitmapRenderer::TransformSimple transform = {
        .scaleX = 1,
        .scaleY = 1,

        .rotation = Rotation::Rot0,

        .mirror = FlipDir::None
    };

    bool visible = bmpRenderer.prepareClippedTransfer (
        transfer, transform,
        destX, destY,
        clipX, clipY, clipX+clipW, clipY+clipH
        );
    if (!visible) {
        return;
    }

    if (!bmpRenderer.checkValid(transfer, transform)) {
        LogError("Invalid BitmapRenderer configuration: %s", bmpRenderer.getErrorMessage());
        return;
    }

    bmpRenderer.render(transfer, transform);
}

/*void MemDrawSurface::copyPixelsRaw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const void* data)
{
    if (w == 1  &&  h == 1) {
        memcpy(getBufferPtr(x, y), data, bytesPerPixel);
        return;
    }

    if (false) {
        auto data8 = reinterpret_cast<uint8_t*>(const_cast<void*>(data));
        const size_t bytesPerLine = w*bytesPerPixel;
        while (h != 0) {
            memcpy(getBufferPtr(x, y), data, bytesPerLine);
            data8 += bytesPerLine;
            h--;
            y++;
        }
    }


    BitmapRenderer::Transfer transfer = {
        .src = {
            .buffer = data,
            .alpha = nullptr,
            .width = bitmap.getWidth(),
            .height = bitmap.getHeight(),
            .blockOffsetX = 0,
            .blockOffsetY = 0,
            .blockWidth = bitmap.getWidth(),
            .blockHeight = bitmap.getHeight(),
            .pixelFormat = bitmap.getPixelFormat(),
            .alphaStorage = bitmap.getAlphaStorage(),
            .byteOrder = bitmap.getByteOrder()
        },

        .dest = {
            .buffer = buf,
            .alpha = nullptr,
            .width = width,
            .height = height,
            .blockOffsetX = 0, // Calculated by prepareClippedTransfer()
            .blockOffsetY = 0, // Calculated by prepareClippedTransfer()
            .pixelFormat = pixelFormat,
            .alphaStorage = AlphaStorage::Bit1, // Ignored
            .byteOrder = byteOrder
        },

        .blendMode = BitmapRenderer::BlendMode::BinarySrcAlpha, // Ignored if bitmap has no alpha buffer
        .alphaMode = BitmapRenderer::AlphaMode::KeepDestAlpha // Ignored
    };

    BitmapRenderer::TransformSimple transform = {
        .scaleX = 1,
        .scaleY = 1,

        .rotation = Rotation::Rot0,

        .mirror = flipDir
    };

    bool visible = bmpRenderer.prepareClippedTransfer (
        transfer, transform,
        x, y,
        clipX, clipY, clipX+clipW, clipY+clipH
        );
    if (!visible) {
        return;
    }

    if (!bmpRenderer.checkValid(transfer, transform)) {
        LogError("Invalid BitmapRenderer configuration: %s", bmpRenderer.getErrorMessage());
        return;
    }

    bmpRenderer.render(transfer, transform);
}*/

void MemDrawSurface::drawHLine(int32_t x0, int32_t y0, int32_t w, const Color& color)
{
    if (!buf) {
        return;
    }

    // Handle negative width
    if (w < 0) {
        x0 += w;
        w = -w;
    }

    // Vertically out of frame
    if (y0 < clipY  ||  y0 >= clipY+clipH) {
        return;
    }

    // Clip low side
    if (x0 < clipX) {
        w -= (clipX-x0);
        x0 = clipX;
    }
    // Clip high side
    if (x0+w > clipX+clipW) {
        w = clipX+clipW-x0;
    }

    if (w <= 0) {
        return;
    }

    assert(x0 >= clipX);
    assert(x0+w <= clipX+clipW);
    assert(y0 >= clipY);
    assert(y0 < clipY+clipH);

    color.toBuffer(getBufferPtr(x0, y0), pixelFormat, byteOrder, w);
}

void MemDrawSurface::drawVLine(int32_t x0, int32_t y0, int32_t h, const Color& color)
{
    if (!buf) {
        return;
    }

    // Handle negative height
    if (h < 0) {
        y0 += h;
        h = -h;
    }

    // Horizontally out of frame
    if (x0 < clipX  ||  x0 >= clipX+clipW) {
        return;
    }

    // Clip low side
    if (y0 < clipY) {
        h -= (clipY-y0);
        y0 = clipY;
    }
    // Clip high side
    if (y0+h > clipY+clipH) {
        h = clipY+clipH-y0;
    }

    if (h <= 0) {
        return;
    }

    assert(x0 >= clipX);
    assert(x0 < clipX+clipW);
    assert(y0 >= clipY);
    assert(y0+h <= clipY+clipH);

    const auto sw = getWidth();

    if (pixelFormat == PixelFormat::RGB565  ||  pixelFormat == PixelFormat::BGR565) {
        const uint16_t rawColor = FromNativeByteOrder(
            (pixelFormat == PixelFormat::RGB565) ? color.toRGB565() : color.toBGR565(), byteOrder);

        auto buf16 = reinterpret_cast<uint16_t*>(getBufferPtr(x0, y0));
        const uint16_t* buf16End = buf16 + h*sw;
        while (buf16 != buf16End) {
            *buf16 = rawColor;
            buf16 += sw;
        }
    } else if (pixelFormat == PixelFormat::RGB888  ||  pixelFormat == PixelFormat::BGR888) {
        uint8_t r, g, b;
        color.toRGB888(r, g, b);

        auto buf8 = getBufferPtr(x0, y0);
        const auto buf8End = buf8 + h*sw*3;

        if (pixelFormat == PixelFormat::RGB888) {
            while (buf8 != buf8End) {
                buf8[0] = r;
                buf8[1] = g;
                buf8[2] = b;
                buf8 += sw*3;
            }
        } else {
            while (buf8 != buf8End) {
                buf8[0] = b;
                buf8[1] = g;
                buf8[2] = r;
                buf8 += sw*3;
            }
        }
    } else if (pixelFormat == PixelFormat::RGBX8888  ||  pixelFormat == PixelFormat::XBGR8888) {
        uint8_t rawColor[4];
        color.toBuffer(rawColor, pixelFormat, byteOrder);

        auto buf8 = getBufferPtr(x0, y0);
        const auto buf8End = buf8 + h*sw*4;

        while (buf8 != buf8End) {
            memcpy(buf8, rawColor, 4);
            buf8 += sw*4;
        }
    } else {
        assert(false);
    }
}

void MemDrawSurface::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color)
{
    if (!buf) {
        return;
    }

    if (x0 == x1) {
        drawVLine(x0, y0, y1-y0, color);
        return;
    }
    if (y0 == y1) {
        drawHLine(x0, y0, x1-x0, color);
        return;
    }


    // ********** CLIP LINE **********

    // Avoid float calculations if both points lie entirely within the clipping region -> trivial no-clipping case
    if (
            x0 < clipX  ||  x0 >= clipX+clipW
        ||  y0 < clipY  ||  y0 >= clipY+clipH
        ||  x1 < clipX  ||  x1 >= clipX+clipW
        ||  y1 < clipY  ||  y1 >= clipY+clipH
    ) {
        auto fx0 = static_cast<float>(x0);
        auto fy0 = static_cast<float>(y0);
        auto fx1 = static_cast<float>(x1);
        auto fy1 = static_cast<float>(y1);

        if (!clipLine(fx0, fy0, fx1, fy1)) {
            // Entirely outside of clip region
            return;
        }

        x0 = static_cast<int32_t>(std::round(fx0));
        y0 = static_cast<int32_t>(std::round(fy0));
        x1 = static_cast<int32_t>(std::round(fx1));
        y1 = static_cast<int32_t>(std::round(fy1));
    }


    // ********** BRESENHAM'S ALGORITHM **********

    uint8_t rawColor[8];
    color.toBuffer(rawColor, pixelFormat, byteOrder);

    const int32_t dx = std::abs(x1 - x0);
    const int32_t sx = x0 < x1 ? 1 : -1;
    const int32_t dy = -std::abs(y1 - y0);
    const int32_t sy = y0 < y1 ? 1 : -1;
    int32_t error = dx + dy;

    while (true) {
        memcpy(getBufferPtr(x0, y0), rawColor, bytesPerPixel);

        const int32_t e2 = 2 * error;
        if (e2 >= dy) {
            if (x0 == x1) {
                break;
            }
            error = error + dy;
            x0 = x0 + sx;
        }
        if (e2 <= dx) {
            if (y0 == y1) {
                break;
            }
            error = error + dx;
            y0 = y0 + sy;
        }
    }
}

void MemDrawSurface::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled)
{
    if (!buf) {
        return;
    }

    if (filled) {
        // Handle cases of negative width and height
        if (w < 0) {
            w = -w;
            x = x-w;
        }
        if (h < 0) {
            h = -h;
            y = y-h;
        }

        // Handle full rectangle out of bounds (towards high side)
        if (x >= clipX+clipW  ||  y >= clipY+clipH) {
            return;
        }
        // Handle full rectangle out of bounds (towards low side)
        if (x+w <= clipX  ||  y+h <= clipY) {
            return;
        }

        // Clip lowest corner to view area
        if (x < clipX) {
            w -= (clipX-x);
            x = clipX;
        }
        if (y < clipY) {
            h -= (clipY-y);
            y = clipY;
        }

        // Clip highest corner to view area
        if (x+w > clipX+clipW) {
            w = clipX+clipW - x;
        }
        if (y+h > clipY+clipH) {
            h = clipY+clipH - y;
        }

        if (w <= 0  ||  h <= 0) {
            return;
        }

        assert(x >= clipX);
        assert(y >= clipY);
        assert(w > 0);
        assert(h > 0);
        assert(x+w <= clipX+clipW);
        assert(y+h <= clipY+clipH);

#ifdef MINTGGGAMEENGINE_MEMDRAWSURFACE_ESP_PPA_SUPPORTED
        if (fillRectESPPPA(color, x, y, w, h)) {
            return;
        }
#endif

        const auto sw = getWidth();

        const uint16_t ey = y+h;
        const uint16_t ex = x+w;

        if (pixelFormat == PixelFormat::RGB565  ||  pixelFormat == PixelFormat::BGR565) {
            uint16_t rawColor = FromNativeByteOrder(
                    (pixelFormat == PixelFormat::RGB565) ? color.toRGB565() : color.toBGR565(), byteOrder);

            for (uint16_t py = y ; py < ey ; py++) {
                auto buf16 = reinterpret_cast<uint16_t*>(buf) + py*sw + x;
                for (uint16_t px = x ; px < ex ; px++) {
                    *buf16++ = rawColor;
                }
            }
        } else if (pixelFormat == PixelFormat::RGB888  ||  pixelFormat == PixelFormat::BGR888) {
            uint8_t r, g, b;
            color.toRGB888(r, g, b);

            if (pixelFormat == PixelFormat::RGB888) {
                for (uint16_t py = y ; py < ey ; py++) {
                    auto buf8 = reinterpret_cast<uint8_t*>(buf) + (py*sw + x)*3;
                    for (uint16_t px = x ; px < ex ; px++) {
                        buf8[0] = r;
                        buf8[1] = g;
                        buf8[2] = b;
                        buf8 += 3;
                    }
                }
            } else {
                for (uint16_t py = y ; py < ey ; py++) {
                    auto buf8 = reinterpret_cast<uint8_t*>(buf) + (py*sw + x)*3;
                    for (uint16_t px = x ; px < ex ; px++) {
                        buf8[0] = b;
                        buf8[1] = g;
                        buf8[2] = r;
                        buf8 += 3;
                    }
                }
            }
        } else if (pixelFormat == PixelFormat::RGBX8888  ||  pixelFormat == PixelFormat::XBGR8888) {
            uint8_t rawColor[4];
            color.toBuffer(rawColor, pixelFormat, byteOrder);

            for (uint16_t py = y ; py < ey ; py++) {
                auto buf8 = reinterpret_cast<uint8_t*>(buf) + (py*sw + x)*4;
                for (uint16_t px = x ; px < ex ; px++) {
                    memcpy(buf8, rawColor, 4);
                    buf8 += 4;
                }
            }
        } else {
            assert(false);
        }
    } else {
        // TODO: Check for off-by-one errors
        drawHLine(x, y, w, color);
        drawHLine(x, y+h, w, color);
        drawVLine(x, y, h, color);
        drawVLine(x+w, y, h, color);
    }
}

void MemDrawSurface::drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled)
{
    // This was adapted from HAGL. Apparently it's the Midpoint circle algorithm.

    if (!buf) {
        return;
    }

    if (filled) {
        int32_t x = 0;
        int32_t y = r;
        int32_t d = 3 - 2 * r;

        while (y >= x) {
            drawHLine(cx - x, cy + y, x * 2 + 1, color);
            drawHLine(cx - x, cy - y, x * 2 + 1, color);
            drawHLine(cx - y, cy + x, y * 2 + 1, color);
            drawHLine(cx - y, cy - x, y * 2 + 1, color);

            if (d <= 0) {
                d = d + 4 * x + 6;
                x++;
            } else {
                d = d + 4 * (x - y) + 10;
                x++;
                y--;
            }
        }
    } else {
        if (r == 0) {
            drawPixel(cx, cy, color);
            return;
        }

        int32_t x = 0;
        int32_t y = r;
        int32_t d = 3 - 2 * r;

        drawPixel(cx + x, cy + y, color);
        drawPixel(cx - x, cy + y, color);
        drawPixel(cx + x, cy - y, color);
        drawPixel(cx - x, cy - y, color);
        drawPixel(cx + y, cy + x, color);
        drawPixel(cx - y, cy + x, color);
        drawPixel(cx + y, cy - x, color);
        drawPixel(cx - y, cy - x, color);

        while (y >= x) {
            if (d > 0) {
                d = d + 4 * (x - y) + 10;
                y--;
                x++;
            } else {
                d = d + 4 * x + 6;
                x++;
            }

            drawPixel(cx + x, cy + y, color);
            drawPixel(cx - x, cy + y, color);
            drawPixel(cx + x, cy - y, color);
            drawPixel(cx - x, cy - y, color);
            drawPixel(cx + y, cy + x, color);
            drawPixel(cx - y, cy + x, color);
            drawPixel(cx + y, cy - x, color);
            drawPixel(cx - y, cy - x, color);
        }
    }
}

void MemDrawSurface::drawBitmap (
    int32_t x, int32_t y,
    const Bitmap& bitmap,
    FlipDir flipDir,
    Rotation rotation,
    int16_t scaleX, int16_t scaleY
) {
    if (!buf) {
        return;
    }
    if (!bitmap.getData()) {
        return;
    }

    BitmapRenderer::Transfer transfer = {
        .src = {
            .buffer = bitmap.getData(),
            .alpha = bitmap.getAlpha(),
            .width = bitmap.getWidth(),
            .height = bitmap.getHeight(),
            .blockOffsetX = 0,
            .blockOffsetY = 0,
            .blockWidth = bitmap.getWidth(),
            .blockHeight = bitmap.getHeight(),
            .pixelFormat = bitmap.getPixelFormat(),
            .alphaStorage = bitmap.getAlphaStorage(),
            .byteOrder = bitmap.getByteOrder()
        },

        .dest = {
            .buffer = buf,
            .alpha = nullptr,
            .width = width,
            .height = height,
            .blockOffsetX = 0, // Calculated by prepareClippedTransfer()
            .blockOffsetY = 0, // Calculated by prepareClippedTransfer()
            .pixelFormat = pixelFormat,
            .alphaStorage = AlphaStorage::Bit1, // Ignored
            .byteOrder = byteOrder
        },

        .blendMode = BitmapRenderer::BlendMode::BinarySrcAlpha, // Ignored if bitmap has no alpha buffer
        .alphaMode = BitmapRenderer::AlphaMode::KeepDestAlpha // Ignored
    };

    BitmapRenderer::TransformSimple transform = {
        .scaleX = scaleX,
        .scaleY = scaleY,

        .rotation = rotation,

        .mirror = flipDir
    };

    bool visible = bmpRenderer.prepareClippedTransfer (
        transfer, transform,
        x, y,
        clipX, clipY, clipX+clipW, clipY+clipH
        );
    if (!visible) {
        return;
    }

    if (!bmpRenderer.checkValid(transfer, transform)) {
        LogError("Invalid BitmapRenderer configuration: %s", bmpRenderer.getErrorMessage());
        return;
    }

    bmpRenderer.render(transfer, transform);
}

void MemDrawSurface::setClipRegion(int16_t cx, int16_t cy, int16_t cw, int16_t ch)
{
    const auto sw = getWidth();
    const auto sh = getHeight();

    bool intersects = ClipRectToRectWH<int16_t> (
        cx, cy, cw, ch,
        0, 0, static_cast<int16_t>(sw), static_cast<int16_t>(sh)
        );

    if (intersects) {
        clipX = cx;
        clipY = cy;
        clipW = cw;
        clipH = ch;
    } else {
        clipX = 0;
        clipY = 0;
        clipW = 0;
        clipH = 0;
    }
}

void MemDrawSurface::getClipRegion(uint16_t* cx, uint16_t* cy, uint16_t* cw, uint16_t* ch)
{
    if (cx) {
        *cx = clipX;
    }
    if (cy) {
        *cy = clipY;
    }
    if (cw) {
        *cw = clipW;
    }
    if (ch) {
        *ch = clipH;
    }
}

Color MemDrawSurface::readPixel(int32_t x, int32_t y)
{
    if (!buf) {
        return Color::BLACK;
    }
    if (x < 0  ||  y < 0) {
        return Color::BLACK;
    }
    const auto w = getWidth();
    const auto h = getHeight();
    if (x >= w  ||  y >= h) {
        return Color::BLACK;
    }
    return Color::fromBuffer(getBufferPtr(x, y), pixelFormat, byteOrder);
}



// ********** LINE CLIPPING **********

// The following is an implementation of the Cohen-Sutherland algorithm, adapted directly from code given on the
// English Wikipedia. It's the same principle used by HAGL.

constexpr int INSIDE = 0b0000;
constexpr int LEFT   = 0b0001;
constexpr int RIGHT  = 0b0010;
constexpr int BOTTOM = 0b0100;
constexpr int TOP    = 0b1000;

// Compute the bit code for a point (x, y) using the clip rectangle
// bounded diagonally by (xmin, ymin), and (xmax, ymax)
// ASSUME THAT xmax, xmin, ymax and ymin are global constants.
int MemDrawSurface::cohenSutherlandComputeOutCode(float x, float y, const LineClipContext& ctx)
{
	int code = INSIDE;          // initialised as being inside of clip window

	if (x < ctx.xMin)           // to the left of clip window
		code |= LEFT;
	else if (x > ctx.xMax)      // to the right of clip window
		code |= RIGHT;
	if (y < ctx.yMin)           // below the clip window
		code |= BOTTOM;
	else if (y > ctx.yMax)      // above the clip window
		code |= TOP;

	return code;
}

bool MemDrawSurface::clipLine(float& x0, float& y0, float& x1, float& y1) const
{
    const LineClipContext ctx = {
        .xMin = static_cast<float>(clipX),
        .yMin = static_cast<float>(clipY),
        .xMax = static_cast<float>(clipX+clipW-1),
        .yMax = static_cast<float>(clipY+clipH-1)
    };

	// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
	auto outcode0 = cohenSutherlandComputeOutCode(x0, y0, ctx);
	auto outcode1 = cohenSutherlandComputeOutCode(x1, y1, ctx);
	bool accept = false;

	while (true) {
		if (!(outcode0 | outcode1)) {
			// bitwise OR is 0: both points inside window; trivially accept and exit loop
			accept = true;
			break;
		} else if (outcode0 & outcode1) {
			// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
			// or BOTTOM), so both must be outside window; exit loop (accept is false)
			break;
		} else {
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge
			float x, y;

			// At least one endpoint is outside the clip rectangle; pick it.
			int outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

			// Now find the intersection point;
			// use formulas:
			//   slope = (y1 - y0) / (x1 - x0)
			//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
			//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
			// No need to worry about divide-by-zero because, in each case, the
			// outcode bit being tested guarantees the denominator is non-zero
			if (outcodeOut & TOP) {           // point is above the clip window
				x = x0 + (x1 - x0) * (ctx.yMax - y0) / (y1 - y0);
				y = ctx.yMax;
			} else if (outcodeOut & BOTTOM) { // point is below the clip window
				x = x0 + (x1 - x0) * (ctx.yMin - y0) / (y1 - y0);
				y = ctx.yMin;
			} else if (outcodeOut & RIGHT) {  // point is to the right of clip window
				y = y0 + (y1 - y0) * (ctx.xMax - x0) / (x1 - x0);
				x = ctx.xMax;
			} else {   // point is to the left of clip window
				y = y0 + (y1 - y0) * (ctx.xMin - x0) / (x1 - x0);
				x = ctx.xMin;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				x0 = x;
				y0 = y;
				outcode0 = cohenSutherlandComputeOutCode(x0, y0, ctx);
			} else {
				x1 = x;
				y1 = y;
				outcode1 = cohenSutherlandComputeOutCode(x1, y1, ctx);
			}
		}
	}

	return accept;
}

#ifdef MINTGGGAMEENGINE_MEMDRAWSURFACE_ESP_PPA_SUPPORTED

bool MemDrawSurface::fillRectESPPPA(const Color& color, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (!ppaFillClient) {
        return false;
    }

    if (    (pixelFormat == PixelFormat::RGB565  ||  pixelFormat == PixelFormat::BGR565)
        &&  byteOrder != ByteOrder::LittleEndian
    ) {
        // Only little endian is supported for formats where it actually matters
        return false;
    }

    // TODO: Find a proper heuristic
    const size_t fillSize = static_cast<size_t>(w) * static_cast<size_t>(h) * bytesPerPixel;
    if (fillSize < PPA_FILL_SIZE_THRESHOLD) {
        // CPU is probably faster (heuristic)
        return false;
    }

    const size_t bufSize = static_cast<uint32_t>(width) * static_cast<uint32_t>(height)
            * static_cast<uint32_t>(bytesPerPixel);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
    const size_t cacheAlign = esp_cache_get_line_size_by_addr(buf);
#else
    const size_t cacheAlign = 128;
#endif

    if (cacheAlign != 0) {
        // PPA requires output buffer and buffer size to be aligned to cache line size. esp_cache_msync() requires
        // the same.
        if (    reinterpret_cast<uintptr_t>(buf) % cacheAlign != 0
            ||  bufSize % cacheAlign != 0
        ) {
            return false;
        }

        // Sync cache to memory and invalidate it, to ensure the CPU doesn't retain stale data in cache, and that this
        // stale data doesn't later overwrite our fill.
        // TODO: We don't have to do this for the entire buffer. Just the filling window would be enough, but then
        //  we'd have to fiddle around with alignments again, so we're not doing that for now.
        esp_err_t err = esp_cache_msync(buf, bufSize, ESP_CACHE_MSYNC_FLAG_DIR_C2M | ESP_CACHE_MSYNC_FLAG_INVALIDATE);
        if (err != ESP_OK) {
            LogError("Error syncing cache for PPA filling: %s", esp_err_to_name(err));
            return false;
        }
    }

    ppa_fill_oper_config_t operCfg = {
        .out = {
            .buffer = buf,
            .buffer_size = bufSize,
            .pic_w = width,
            .pic_h = height,
            .block_offset_x = x,
            .block_offset_y = y,
            .fill_cm = PPA_FILL_COLOR_MODE_RGB888
        },
        .fill_block_w = w,
        .fill_block_h = h,
        .fill_argb_color = {
            .b = 0,
            .g = 0,
            .r = 0,
            .a = 0
        },
        .mode = PPA_TRANS_MODE_BLOCKING
    };

    switch (pixelFormat) {
    case PixelFormat::RGB565:
        operCfg.out.fill_cm = PPA_FILL_COLOR_MODE_RGB565;
        operCfg.fill_argb_color.b = color.getBlue();
        operCfg.fill_argb_color.g = color.getGreen();
        operCfg.fill_argb_color.r = color.getRed();
        operCfg.fill_argb_color.a = 255;
        break;
    case PixelFormat::BGR565:
        operCfg.out.fill_cm = PPA_FILL_COLOR_MODE_RGB565;
        operCfg.fill_argb_color.b = color.getRed();
        operCfg.fill_argb_color.g = color.getGreen();
        operCfg.fill_argb_color.r = color.getBlue();
        operCfg.fill_argb_color.a = 255;
        break;
    case PixelFormat::RGB888:
        operCfg.out.fill_cm = PPA_FILL_COLOR_MODE_RGB888;
        operCfg.fill_argb_color.b = color.getBlue();
        operCfg.fill_argb_color.g = color.getGreen();
        operCfg.fill_argb_color.r = color.getRed();
        operCfg.fill_argb_color.a = 255;
        break;
    case PixelFormat::BGR888:
        operCfg.out.fill_cm = PPA_FILL_COLOR_MODE_RGB888;
        operCfg.fill_argb_color.b = color.getRed();
        operCfg.fill_argb_color.g = color.getGreen();
        operCfg.fill_argb_color.r = color.getBlue();
        operCfg.fill_argb_color.a = 255;
        break;
    case PixelFormat::RGBX8888:
        // TODO: Check the order here
        operCfg.out.fill_cm = PPA_FILL_COLOR_MODE_ARGB8888;
        operCfg.fill_argb_color.b = color.getAlpha();
        operCfg.fill_argb_color.g = color.getBlue();
        operCfg.fill_argb_color.r = color.getGreen();
        operCfg.fill_argb_color.a = color.getRed();
        break;
    case PixelFormat::XBGR8888:
        // TODO: Check the order here
        operCfg.out.fill_cm = PPA_FILL_COLOR_MODE_ARGB8888;
        operCfg.fill_argb_color.b = color.getRed();
        operCfg.fill_argb_color.g = color.getGreen();
        operCfg.fill_argb_color.r = color.getBlue();
        operCfg.fill_argb_color.a = color.getAlpha();
        break;
    }

    esp_err_t err = ppa_do_fill(ppaFillClient, &operCfg);
    if (err != ESP_OK) {
        LogError("Error filling surface using PPA: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

#endif


}

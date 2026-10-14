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

#include "Screen.h"

#include "../surface/MemDrawSurface.h"


namespace MINTGGGameEngine
{

/**
 * \brief Abstract base class for screens that use double-buffering.
 *
 * Such a screen uses a MemDrawSurface internally, to which all drawing functions are delegated. A subclass then only
 * has to implement the missing functions from the Screen class itself to initialize the screen hardware, and to
 * transfer the framebuffer from memory to the screen.
 *
 * Most screens supported by the engine are derived from this class.
 *
 * \see Screen
 * \see MemDrawSurface
 */
class BufferedScreen : public Screen
{
public:
    /**
     * \brief Create a BufferedScreen from a user-provided buffer.
     *
     * See the MemDrawSurface constructor for details on the arguments, as they are simply forwarded to it.
     *
     * @param buffer
     * @param width
     * @param height
     * @param del
     * @param swapEndianness
     */
    BufferedScreen (
        PixelFormat pixelFormat,
        ByteOrder byteOrder,
        void* buffer,
        uint16_t width, uint16_t height,
        const MemDrawSurface::delete_fn_t& del
        );

    /**
     * \brief Destroy this BufferedScreen.
     */
    ~BufferedScreen() override;

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
    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color) override;

    void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled) override;

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

    /**
     * \brief Read a single pixel's color value.
     *
     * Reading is done from the framebuffer, **not** from the actual hardware screen. This means that reading is fast.
     *
     * @param x x coordinate of the pixel to read.
     * @param y y coordinate of the pixel to read.
     * @return The pixel's color.
     */
    Color readPixel(int32_t x, int32_t y) override;

    void drawText(const Text& text, int32_t ox, int32_t oy) override;

    bool saveScreenshot(const char* path) override;

    bool init() override;

    void shutdown() override;

    void commit() override;

    const MemDrawSurface& getBuffer() const { return fb; }
    MemDrawSurface& getBuffer() { return fb; }

protected:
    void setBuffer(void* buffer, const MemDrawSurface::delete_fn_t& del);

protected:
    MemDrawSurface fb;
};

}

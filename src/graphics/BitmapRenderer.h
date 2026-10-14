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

#include "../Globals.h"

#include <string>

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <esp_idf_version.h>
#   if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
#       ifdef MINTGGGAMEENGINE_CHIP_ESP32P4
#           define MINTGGGAMEENGINE_BITMAPRENDERER_ESP_ASYNCC_DMA2D_SUPPORTED
#       endif
#   endif
#endif

#ifdef MINTGGGAMEENGINE_BITMAPRENDERER_ESP_ASYNCC_DMA2D_SUPPORTED
#   include <esp_async_color_convert.h>
#endif


namespace MINTGGGameEngine
{

class BitmapRenderer
{
public:
    enum class BlendMode
    {
        Opaque,
        BinarySrcAlpha
    };

    enum class AlphaMode
    {
        KeepDestAlpha,
        CopySrcAlpha
    };

    struct Transfer
    {
        struct {
            const void* buffer;
            const void* alpha;
            uint16_t width;
            uint16_t height;
            uint16_t blockOffsetX;
            uint16_t blockOffsetY;
            uint16_t blockWidth;
            uint16_t blockHeight;
            PixelFormat pixelFormat;
            AlphaStorage alphaStorage;
            ByteOrder byteOrder;
        } src;

        struct {
            void* buffer;
            void* alpha;
            uint16_t width;
            uint16_t height;
            uint16_t blockOffsetX;
            uint16_t blockOffsetY;
            PixelFormat pixelFormat;
            AlphaStorage alphaStorage;
            ByteOrder byteOrder;
        } dest;

        BlendMode blendMode;
        AlphaMode alphaMode;
    };

    struct TransformSimple
    {
        int16_t scaleX;
        int16_t scaleY;

        Rotation rotation;

        FlipDir mirror;
    };

private:
    enum class CacheSyncDir
    {
        C2M,
        M2C
    };

public:
    BitmapRenderer();
    ~BitmapRenderer();

    bool prepareClippedTransfer (
        Transfer& transfer,
        const TransformSimple& transform,
        int32_t destBlockOffsetX, int32_t destBlockOffsetY,
        uint16_t destClipXS, uint16_t destClipYS, uint16_t destClipXE, uint16_t destClipYE
        );

    bool checkValid(const Transfer& transfer, const TransformSimple& transform);

    void render(Transfer& transfer, const TransformSimple& transform);

    bool hasError() const { return errmsg != nullptr; }
    const char* getErrorMessage() const { return errmsg; }

    bool getLastRenderWasHW() const { return lastRenderWasHW; }

private:
    void init();

    void resetError() { this->errmsg = nullptr; }
    bool setError(const char* msg) { this->errmsg = msg; return false; }

    template <
        // These MUST be chosen based on whether the corresponding buffer are actually present, and on whether or not we
        // actually want to do these operations. Just setting them to true as a general fallback is NOT allowed.
        bool WriteDestColor,            // MUST be false if either src.buffer or dest.buffer is null
        bool WriteDestAlpha,            // MUST be false if either src.alpha or dest.alpha is null
        bool TransparencyCheckEnabled,  // MUST be false if src.alpha is null
        int CommonBytesPerPixel,        // MUST be set correctly if FormatConvEnabled=false, otherwise is ignored

        // These can all be left as true, even if the corresponding features aren't actually needed for the transfer.
        // Choosing them more carefully can optimize performance though, because those features can be compiled out of
        // the resulting code.
        bool FormatConvEnabled,         // Large performance impact: Can skip color conversion per pixel, saving
                                        // multiple checks and calculations per pixel. If true, this is done even if
                                        // src and dest formats are identical (no runtime checks).
        bool ScaleEnabled,              // Medium performance impact: Can skip unnecessary single execution of 2D loop,
                                        // per pixel, plus some per-pixel calculations
        bool RotateEnabled,             // Minor performance impact: can skip one check per pixel
        bool MirrorEnabled              // Minor performance impact: can skip two checks per pixel
        >
    static void renderCPUGeneral(Transfer& transfer, const TransformSimple& transform);

    template <
        bool WriteDestColor,
        bool WriteDestAlpha,
        bool TransparencyCheckEnabled,

        bool FormatConvEnabled,
        bool ScaleEnabled,
        bool RotateEnabled,
        bool MirrorEnabled
        >
    static void renderCPUGeneral(Transfer& transfer, const TransformSimple& transform);

    void renderCopyStrided(Transfer& transfer, const TransformSimple& transform);

    void renderCopyContiguous(Transfer& transfer, const TransformSimple& transform);

#ifdef MINTGGGAMEENGINE_BITMAPRENDERER_ESP_ASYNCC_DMA2D_SUPPORTED
    bool renderCopyStridedESPAsyncCC(Transfer& transfer, const TransformSimple& transform);
#endif

private:
    const char* errmsg;

#ifdef MINTGGGAMEENGINE_BITMAPRENDERER_ESP_ASYNCC_DMA2D_SUPPORTED
    async_color_convert_handle_t asyncCCHandle;
#endif

    bool lastRenderWasHW;
};

}

#include "BitmapRenderer_Impl.h"

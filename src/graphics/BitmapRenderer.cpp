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

#include "BitmapRenderer.h"

#include <cassert>
#include <cstddef>

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <esp_cache.h>
#endif

#include "../platform/PlatformManager.h"
#include "../util/Log.h"
#include "../util/MathUtils.h"
#include "../util/Util.h"
#include "Color.h"

LOG_USE_TAG("BitmapRenderer")


// This is currently kept disabled. It kinda works, but it doesn't do any cache synchronization, which is very
// dangerous and can lead to errors. We could implement cache synchronization for it, but there are a few problems
// with that:
//
//      * Implementing it correctly isn't that easy, since esp_cache_msync() has alignment requirements. Just ignoring
//        them can lead to memory corruption, even with the ESP_CACHE_MSYNC_FLAG_UNALIGNED flag set (see ESP-IDF docs).
//        Because the AsyncCC implementation is usually used for strided copying, we will often have cases that aren't
//        aligned. In many such cases, ESP_CACHE_MSYNC_FLAG_UNALIGNED would be fine, but detecting when it's fine and
//        when it's not isn't that trivial.
//      * Since msync() expects a contiguous region of memory, we will have to sync more than we're actually copying,
//        or do many smaller msync() calls. That could be quite a lot of overhead.
//      * The AsyncCC implementation is already just marginally faster than the optimized CPU version, and all these
//        problems with proper cache synchronization may make it even slower.
//
// For those reasons, we'll keep it disabled for now.
//#define ENABLE_ASYNCCC_DMA2D_COPY


namespace MINTGGGameEngine
{

BitmapRenderer::BitmapRenderer()
    : errmsg(nullptr), lastRenderWasHW(false)
{
    init();
}

BitmapRenderer::~BitmapRenderer()
{
#ifdef MINTGGGAMEENGINE_BITMAPRENDERER_ESP_ASYNCC_DMA2D_SUPPORTED
    if (asyncCCHandle) {
        esp_async_color_convert_uninstall(asyncCCHandle);
        asyncCCHandle = nullptr;
    }
#endif
}

void BitmapRenderer::init()
{
#ifdef MINTGGGAMEENGINE_BITMAPRENDERER_ESP_ASYNCC_DMA2D_SUPPORTED
    asyncCCHandle = nullptr;

#ifdef ENABLE_ASYNCCC_DMA2D_COPY
    async_color_convert_config_t asyncCCCfg = {
        .backlog = 1,
        .dma_burst_size = 128,
        .intr_priority = 0
    };
    esp_err_t err = esp_async_color_convert_install_dma2d(&asyncCCCfg, &asyncCCHandle);
    if (err != ESP_OK) {
        LogError("Error installing AsyncCC via DMA2D: %s", esp_err_to_name(err));
    }
#endif
#endif
}

bool BitmapRenderer::prepareClippedTransfer (
        Transfer& transfer,
        const TransformSimple& transform,
        const int32_t destBlockOffsetX, const int32_t destBlockOffsetY,
        const uint16_t destClipXS, const uint16_t destClipYS, const uint16_t destClipXE, const uint16_t destClipYE
) {
    if (transform.scaleX == 0  ||  transform.scaleY == 0) {
        // Resulting bitmap is empty -> no transfer necessary
        return false;
    }

    assert(transform.scaleX >= 1  &&  transform.scaleY >= 1);

    const bool rotated90 = (transform.rotation == Rotation::Rot90  ||  transform.rotation == Rotation::Rot270);

    int_fast32_t destBlockWidth = transfer.src.blockWidth * transform.scaleX;
    int_fast32_t destBlockHeight = transfer.src.blockHeight * transform.scaleY;
    if (rotated90) {
        std::swap(destBlockWidth, destBlockHeight);
    }

    const int_fast32_t destBlockXE = destBlockOffsetX + destBlockWidth;
    const int_fast32_t destBlockYE = destBlockOffsetY + destBlockHeight;

    // Clip ((destBlockOffsetX, destBlockOffsetY), (destBlockXE, destBlockYE)) to clipping rect
    int_fast32_t clippedDestBlockXS = destBlockOffsetX;
    int_fast32_t clippedDestBlockYS = destBlockOffsetY;
    int_fast32_t clippedDestBlockXE = destBlockXE;
    int_fast32_t clippedDestBlockYE = destBlockYE;
    bool intersects = ClipRectToRect<int_fast32_t> (
        clippedDestBlockXS, clippedDestBlockYS, clippedDestBlockXE, clippedDestBlockYE,
        destClipXS, destClipYS, destClipXE, destClipYE
        );
    if (!intersects) {
        // Nothing left after clipping -> no transfer necessary
        return false;
    }

    assert(clippedDestBlockXS >= destBlockOffsetX  &&  clippedDestBlockXS >= destClipXS);
    assert(clippedDestBlockYS >= destBlockOffsetY  &&  clippedDestBlockYS >= destClipYS);
    assert(clippedDestBlockXE <= destBlockXE  &&  clippedDestBlockXE <= destClipXE);
    assert(clippedDestBlockYE <= destBlockYE  &&  clippedDestBlockYE <= destClipYE);

    if (    clippedDestBlockXS == destBlockOffsetX
        &&  clippedDestBlockYS == destBlockOffsetY
        &&  clippedDestBlockXE == destBlockXE
        &&  clippedDestBlockYE == destBlockYE
    ) {
        // No clipping necessary -> transfer was already ok
        transfer.dest.blockOffsetX = static_cast<uint16_t>(destBlockOffsetX);
        transfer.dest.blockOffsetY = static_cast<uint16_t>(destBlockOffsetY);
        return true;
    }

    // Calculate rotated scale factors
    auto rotScaleX = transform.scaleX;
    auto rotScaleY = transform.scaleY;
    if (rotated90) {
        std::swap(rotScaleX, rotScaleY);
    }

    // Calculate by how many (scaleX, scaleY) pixel blocks (NOT pixels) the dest block was shrunk by the clipping above.
    // We round up to full (scaleX, scaleY) blocks because the renderer doesn't support partially clipped pixels,
    // which would otherwise lead to buffer overruns.
    // TODO: Actually support partially clipped pixels now.
    const int_fast32_t destLeftBlockShrink = (clippedDestBlockXS - destBlockOffsetX + rotScaleX-1)
            / rotScaleX;
    const int_fast32_t destTopBlockShrink = (clippedDestBlockYS - destBlockOffsetY + rotScaleY-1)
            / rotScaleY;
    const int_fast32_t destRightBlockShrink = (destBlockXE - clippedDestBlockXE + rotScaleX-1)
            / rotScaleX;
    const int_fast32_t destBottomBlockShrink = (destBlockYE - clippedDestBlockYE + rotScaleY-1)
            / rotScaleY;

    // Now calculate the dest block shrink values in pixels
    const int_fast32_t destLeftShrink = destLeftBlockShrink * rotScaleX;
    const int_fast32_t destTopShrink = destTopBlockShrink * rotScaleY;

    // And calculate the src block shrink values in pixels. Note that if rotation is involved, we have to be careful
    // about which src sides correspond to which dest sides here.
    int_fast32_t srcLeftShrink, srcTopShrink, srcRightShrink, srcBottomShrink;
    switch (transform.rotation) {
    case Rotation::Rot0:
        srcLeftShrink = destLeftBlockShrink;
        srcTopShrink = destTopBlockShrink;
        srcRightShrink = destRightBlockShrink;
        srcBottomShrink = destBottomBlockShrink;
        break;
    case Rotation::Rot90:
        srcLeftShrink = destBottomBlockShrink;
        srcTopShrink = destLeftBlockShrink;
        srcRightShrink = destTopBlockShrink;
        srcBottomShrink = destRightBlockShrink;
        break;
    case Rotation::Rot180:
        srcLeftShrink = destRightBlockShrink;
        srcTopShrink = destBottomBlockShrink;
        srcRightShrink = destLeftBlockShrink;
        srcBottomShrink = destTopBlockShrink;
        break;
    case Rotation::Rot270:
        srcLeftShrink = destTopBlockShrink;
        srcTopShrink = destRightBlockShrink;
        srcRightShrink = destBottomBlockShrink;
        srcBottomShrink = destLeftBlockShrink;
        break;
    default:
        assert(false);
        return false;
    }

    // Shrink src block accordingly
    const uint16_t srcFinalBlockWidth = transfer.src.blockWidth - srcLeftShrink - srcRightShrink;
    const uint16_t srcFinalBlockHeight = transfer.src.blockHeight - srcTopShrink - srcBottomShrink;

    if (srcFinalBlockWidth == 0  ||  srcFinalBlockHeight == 0) {
        // A transfer did remain after clipping, but it contained only pixels that were partially clipped, which we
        // eliminated in the rounding step above -> again, there's no transfer necessary
        return false;
    }

    if ((transform.mirror & FlipDir::Horizontal) != 0) {
        std::swap(srcLeftShrink, srcRightShrink);
    }
    if ((transform.mirror & FlipDir::Vertical) != 0) {
        std::swap(srcTopShrink, srcBottomShrink);
    }

    transfer.src.blockWidth = srcFinalBlockWidth;
    transfer.src.blockHeight = srcFinalBlockHeight;

    transfer.src.blockOffsetX += srcLeftShrink;
    transfer.src.blockOffsetY += srcTopShrink;

    // Shrink dest block accordingly
    transfer.dest.blockOffsetX = destBlockOffsetX + destLeftShrink;
    transfer.dest.blockOffsetY = destBlockOffsetY + destTopShrink;

    return true;
}

bool BitmapRenderer::checkValid(const Transfer& transfer, const TransformSimple& transform)
{
    if ((transfer.src.buffer != nullptr) != (transfer.dest.buffer != nullptr)) {
        // Either both must be non-null (normal case), or both must be null (alpha only case)
        return setError("either both or neither buffers must be null");
    }
    if (    transfer.src.alphaStorage != AlphaStorage::Bit1
        ||  transfer.dest.alphaStorage != AlphaStorage::Bit1
    ) {
        return setError("unsupported alpha storage");
    }
    if (    transfer.src.blockOffsetX+transfer.src.blockWidth > transfer.src.width
        ||  transfer.src.blockOffsetY+transfer.src.blockHeight > transfer.src.height
    ) {
        return setError("invalid block coordinates");
    }
    if (transform.scaleX < 0  ||  transform.scaleY < 0) {
        return setError("downscaling not supported");
    }
    resetError();
    return true;
}

void BitmapRenderer::render(Transfer& transfer, const TransformSimple& transform)
{
    lastRenderWasHW = false;

    if (transform.scaleX == 0  ||  transform.scaleY == 0) {
        return;
    }

    const bool writeDestColor =
            transfer.src.buffer != nullptr
            && transfer.dest.buffer != nullptr;
    const bool writeDestAlpha =
            transfer.alphaMode == AlphaMode::CopySrcAlpha
            && transfer.src.alpha != nullptr
            && transfer.dest.alpha != nullptr;
    const bool transparencyCheckEnabled =
            writeDestColor
            && transfer.blendMode == BlendMode::BinarySrcAlpha
            && transfer.src.alpha != nullptr;

    const bool formatConvEnabled =
            writeDestColor
            &&  (
                (transfer.src.pixelFormat != transfer.dest.pixelFormat)
                || (transfer.src.byteOrder != transfer.dest.byteOrder)
            );
    const bool scaleEnabled = (transform.scaleX != 1  ||  transform.scaleY != 1);
    const bool rotateEnabled = (transform.rotation != Rotation::Rot0);
    const bool mirrorEnabled = (transform.mirror != FlipDir::None);

    if (!writeDestColor  &&  !writeDestAlpha) {
        // Absolutely nothing to do.
        return;
    }


    if (    !writeDestAlpha // We could still use an optimized version for the color part, but we don't for now
        &&  !transparencyCheckEnabled
        &&  !formatConvEnabled
        &&  !scaleEnabled  &&  !rotateEnabled // Vertical mirroring might actually be ok
    ) {
        // No alpha handling, format conversion, or complex coordinate transforms

        if (    transform.mirror == FlipDir::None
            &&  transfer.src.width == transfer.dest.width
            &&  transfer.src.blockWidth == transfer.src.width
            &&  transfer.src.blockOffsetX == 0
        ) {
            // All pixels of the block are contiguous in both src and dest -> use contiguous copy implementation
            renderCopyContiguous(transfer, transform);
            return;
        }

        if (transform.mirror == FlipDir::None  ||  transform.mirror == FlipDir::Vertical) {
            // No horizontal flipping -> use strided copy implementation
            renderCopyStrided(transfer, transform);
            return;
        }
    }


    // If we got here, there wasn't a specifically optimized implementation available. Fall back to the general-purpose
    // CPU implementation.

    // We COULD list the calls with all possible combinations of template parameter values, but since renderCPUGeneral()
    // is already quite a lot of code, this would duplicate that code more often than we probably want, especially for
    // embedded systems with limited flash. For example: The renderCPUGeneral() template currently takes 7 bool
    // parameters and one int parameter that currently supports 2 different values. That means that there are a total
    // of 2^7*2=256 possible combinations, and thus the code would be generated as many times. That's a lot of
    // instruction memory, and probably longer compilation times.
    // So instead we'll just list the combinations where the proper template parameter value is either critical to
    // correctness, or which are commonly seen in practice. The only downside is that for some rarer combinations, we
    // might not get the absolute optimal implementation.

    if (writeDestColor) {
        [[likely]]

        if (writeDestAlpha) {
            [[unlikely]]
            // Relatively uncommon. The framebuffer currently doesn't have alpha, so this is not used for the general
            // per-frame rendering. Often just done when loading new bitmaps or on some configuration change, so not
            // that critical for optimization.

            if (transparencyCheckEnabled) {
                [[unlikely]]
                // Very rare. Writing alpha values but also skipping transparent pixel color. Are you high?
                renderCPUGeneral<true, true, true,
                        true, true, true, true>(transfer, transform); // 1

            } else /* !transparencyCheckEnabled */ {
                [[likely]]
                // Somewhat common.
                if (formatConvEnabled) {
                    renderCPUGeneral<true, true, false,
                            true, true, true, true>(transfer, transform); // 1
                } else {
                    renderCPUGeneral<true, true, false,
                            false, true, true, true>(transfer, transform); // 2
                }
            }

        } else /* !writeDestAlpha */ {
            [[likely]]

            // Very common. This includes all the standard cases for writing to the framebuffer every frame, so
            // optimization is often more important.

            if (transparencyCheckEnabled) {
                // Very common. E.g. rendering transparent bitmaps.

                if (formatConvEnabled) {
                    [[unlikely]]
                    // Somewhat common. Often for format-converting bitmaps to a screen's native format. Usually not
                    // done on a per-frame basis, and format conversion is expensive anyway, so don't micro-optimize.
                    renderCPUGeneral<true, false, true,
                            true, true, true, true>(transfer, transform); // 1
                } else {
                    [[likely]]
                    // Very common. This is THE standard "render transparent bitmap into framebuffer" case, so we want
                    // to optimize it.
                    if (scaleEnabled) {
                        // Don't micro-optimize further. Scaling is the most expensive of the affine transforms, so
                        // the others shouldn't have much more impact.
                        renderCPUGeneral<true, false, true,
                                        false, true, true, true>(transfer, transform); // 2
                    } else /* !scaleEnabled */ {
                        if (rotateEnabled) {
                            if (mirrorEnabled) {
                                renderCPUGeneral<true, false, true,
                                        false, false, true, true>(transfer, transform); // 2
                            } else /* !mirrorEnabled */ {
                                renderCPUGeneral<true, false, true,
                                        false, false, true, false>(transfer, transform); // 2
                            }
                        } else /* !rotateEnabled */ {
                            if (mirrorEnabled) {
                                renderCPUGeneral<true, false, true,
                                        false, false, false, true>(transfer, transform); // 2
                            } else /* !mirrorEnabled */ {
                                renderCPUGeneral<true, false, true,
                                        false, false, false, false>(transfer, transform); // 2
                            }
                        }
                    }
                }
            } else /* !transparencyCheckEnabled */ {
                // Very common: Rendering opaque bitmaps.

                if (formatConvEnabled) {
                    [[unlikely]]
                    // Somewhat common. Often for format-converting bitmaps to a screen's native format. Usually not
                    // done on a per-frame basis, and format conversion is expensive anyway, so don't micro-optimize.
                    renderCPUGeneral<true, false, false,
                            true, true, true, true>(transfer, transform); // 1
                } else {
                    [[likely]]
                    // Very common. This is THE standard "render opaque bitmap into framebuffer" case, so we want
                    // to optimize it.
                    if (scaleEnabled) {
                        // Don't micro-optimize further. Scaling is the most expensive of the affine transforms, so
                        // the others shouldn't have much more impact.
                        renderCPUGeneral<true, false, false,
                                        false, true, true, true>(transfer, transform); // 2
                    } else /* !scaleEnabled */ {
                        if (rotateEnabled) {
                            if (mirrorEnabled) {
                                renderCPUGeneral<true, false, false,
                                        false, false, true, true>(transfer, transform); // 2
                            } else /* !mirrorEnabled */ {
                                renderCPUGeneral<true, false, false,
                                        false, false, true, false>(transfer, transform); // 2
                            }
                        } else /* !rotateEnabled */ {
                            if (mirrorEnabled) {
                                renderCPUGeneral<true, false, false,
                                        false, false, false, true>(transfer, transform); // 2
                            } else /* !mirrorEnabled */ {
                                renderCPUGeneral<true, false, false,
                                        false, false, false, false>(transfer, transform); // 2
                            }
                        }
                    }
                }
            }

        }

    } else /* !writeDestColor */ {
        [[unlikely]]
        // Only writing alpha? That's pretty rare in general -> no need for optimization

        assert(writeDestAlpha);
        assert(!formatConvEnabled);
        assert(!transparencyCheckEnabled);

        renderCPUGeneral<false, true, false,
                false, true, true, true>(transfer, transform);
    }
}

void BitmapRenderer::renderCopyStrided(Transfer& transfer, const TransformSimple& transform)
{
#ifdef MINTGGGAMEENGINE_BITMAPRENDERER_ESP_ASYNCC_DMA2D_SUPPORTED
    if (renderCopyStridedESPAsyncCC(transfer, transform)) {
        lastRenderWasHW = true;
        return;
    }
#endif

    bool moveForward;

    switch (transform.mirror) {
    case FlipDir::None:
        moveForward = true;
        break;
    case FlipDir::Vertical:
        moveForward = false;
        break;
    default:
        assert(false);
        return;
    }

    const int_fast8_t bytesPerPixel = CalcPixelFormatBytesPerPixel(transfer.src.pixelFormat);

    const ptrdiff_t srcLineIncrement = transfer.src.width * bytesPerPixel;
    const ptrdiff_t destLineIncrement = moveForward
            ? transfer.dest.width * bytesPerPixel
            : - transfer.dest.width * bytesPerPixel;
    const ptrdiff_t bytesPerBlockLine = transfer.src.blockWidth * bytesPerPixel;

    auto srcBufPtr = reinterpret_cast<uint8_t*>(const_cast<void*>(transfer.src.buffer))
            + (transfer.src.blockOffsetY*transfer.src.width + transfer.src.blockOffsetX) * bytesPerPixel;
    auto destBufPtr = reinterpret_cast<uint8_t*>(transfer.dest.buffer)
            + (moveForward
                ? (transfer.dest.blockOffsetY * transfer.dest.width
                    + transfer.dest.blockOffsetX) * bytesPerPixel
                : ((transfer.dest.blockOffsetY + transfer.src.blockHeight - 1) * transfer.dest.width
                    + transfer.dest.blockOffsetX) * bytesPerPixel);

    for (uint_fast16_t i = 0 ; i < transfer.src.blockHeight ; i++) {
        memcpy(destBufPtr, srcBufPtr, bytesPerBlockLine);
        srcBufPtr += srcLineIncrement;
        destBufPtr += destLineIncrement;
    }
}

void BitmapRenderer::renderCopyContiguous(Transfer& transfer, const TransformSimple& transform)
{
    const int_fast8_t bytesPerPixel = CalcPixelFormatBytesPerPixel(transfer.src.pixelFormat);

    auto srcBufPtr = reinterpret_cast<uint8_t*>(const_cast<void*>(transfer.src.buffer))
            + transfer.src.blockOffsetY * transfer.src.width * bytesPerPixel;
    auto destBufPtr = reinterpret_cast<uint8_t*>(transfer.dest.buffer)
            + transfer.dest.blockOffsetY * transfer.dest.width * bytesPerPixel;

    const size_t blockSize = transfer.src.blockWidth * transfer.src.blockHeight * bytesPerPixel;

    // Attempts to use various DMA hardware if feasible
    PlatformManager::getInstance().fastMemcpy(destBufPtr, srcBufPtr, blockSize, &lastRenderWasHW);
}

#ifdef MINTGGGAMEENGINE_BITMAPRENDERER_ESP_ASYNCC_DMA2D_SUPPORTED

bool BitmapRenderer::renderCopyStridedESPAsyncCC(Transfer& transfer, const TransformSimple& transform)
{
#ifdef ENABLE_ASYNCCC_DMA2D_COPY
    if (!asyncCCHandle) {
        return false;
    }
    if (transform.mirror != FlipDir::None) {
        return false;
    }

    const int_fast8_t bytesPerPixel = CalcPixelFormatBytesPerPixel(transfer.src.pixelFormat);
    const size_t copySizeBytes = static_cast<size_t>(transfer.src.blockWidth)
            * static_cast<size_t>(transfer.src.blockHeight) * bytesPerPixel;

    // TODO: Benchmark and adjust this value, or find better heuristics.
    if (copySizeBytes < 400*1024) {
        // AsyncCC is only viable for pretty large bitmaps. For some reason, there seems to ba a massive setup overhead.
        return false;
    }

    // The actual color format doesn't matter, since we're not doing an actual color conversion. It just needs to
    // have the right amount of bytes per pixel. Note also that AsyncCC doesn't support all color formats!
    esp_color_fourcc_t colorFormat;
    switch (CalcPixelFormatBytesPerPixel(transfer.src.pixelFormat)) {
    case 2:
        colorFormat = ESP_COLOR_FOURCC_RGB16;
        break;
    case 3:
        colorFormat = ESP_COLOR_FOURCC_RGB24;
        break;
    case 4:
        // Not supported by AsyncCC
        return false;
    default:
        return false;
    }

    async_color_convert_request_t ccReq = {
        .src_buffer = transfer.src.buffer,
        .src_stride = transfer.src.width,
        .src_height = transfer.src.height,
        .src_x = transfer.src.blockOffsetX,
        .src_y = transfer.src.blockOffsetY,

        .dst_buffer = transfer.dest.buffer,
        .dst_stride = transfer.dest.width,
        .dst_height = transfer.dest.height,
        .dst_x = transfer.dest.blockOffsetX,
        .dst_y = transfer.dest.blockOffsetY,

        .copy_width = transfer.src.blockWidth,
        .copy_height = transfer.src.blockHeight,

        .src_color_format = colorFormat,
        .dst_color_format = colorFormat
    };

    esp_err_t err = esp_color_convert_blocking(asyncCCHandle, &ccReq, -1);
    if (err != ESP_OK) {
        LogError("Error rendering bitmap using AsyncCC: %s", esp_err_to_name(err));
        return false;
    }

    return true;
#else
    return false;
#endif
}

#endif

}

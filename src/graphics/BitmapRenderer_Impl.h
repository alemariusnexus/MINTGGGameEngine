#pragma once

#include "../Globals.h"

#include "../util/MathUtils.h"
#include "../util/Util.h"
#include "Color.h"


namespace MINTGGGameEngine
{

template <
    bool WriteDestColor,
    bool WriteDestAlpha,
    bool TransparencyCheckEnabled,
    int CommonBytesPerPixel,

    bool FormatConvEnabled,
    bool ScaleEnabled,
    bool RotateEnabled,
    bool MirrorEnabled
    >
void BitmapRenderer::renderCPUGeneral(Transfer& transfer, const TransformSimple& transform)
{
    // The following code relies on some assumptions that the caller has to make sure to follow:
    //
    //  * The src and dest blocks defined by the transfer are fully inside the bounds of their buffers. No bounds
    //    checking nor clipping is performed. That's what prepareClippedTransfer() is for.
    //  * The template arguments were chosen sensibly. This is internal code - the caller should know what they're
    //    doing.
    //  * If WriteDestAlpha is enabled, the destination alpha buffer should not be uninitialized within the region that
    //    this method writes to. Leaving parts of it uninitialized leads to undefined behavior, though it should
    //    still work perfectly fine in "normal" situations (e.g. no especially weird hardware, no buffers in weird
    //    kinds of memory).
    //
    // Those aren't all the assumptions, but they're the ones that aren't that obvious at first glance.

    // The code below currently supports rendering a bitmap with any combination of the following:
    //
    //  * Offsets in the both src and dest
    //  * Positive integer scaling (if ScaleEnabled=true)
    //  * Mirroring
    //  * Rotation by 90, 180, or 270 degrees
    //  * Changing pixel format and/or byte order (if FormatConvEnabled=true)
    //  * Simple blending (BinarySrcAlpha if TransparencyCheckEnabled=true, else Opaque)
    //  * Writing destination alpha buffer (CopySrcAlpha if WriteDestAlpha=true, else KeepDestAlpha)

    constexpr bool UsesSrcAlpha = (WriteDestAlpha  ||  TransparencyCheckEnabled);

    const uint8_t srcBytesPerPixel = FormatConvEnabled
            ? CalcPixelFormatBytesPerPixel(transfer.src.pixelFormat)
            : CommonBytesPerPixel;
    const uint8_t destBytesPerPixel = FormatConvEnabled
            ? CalcPixelFormatBytesPerPixel(transfer.dest.pixelFormat)
            : CommonBytesPerPixel;

    const uint_fast16_t srcAlphaLineSize = (transfer.src.width+7) / 8;
    const uint_fast16_t destAlphaLineSize = (transfer.dest.width+7) / 8;

    const uint_fast32_t destBytesPerLine = transfer.dest.width * destBytesPerPixel;
    const uint_fast32_t srcBufEndOfLineIncrement = (transfer.src.width - transfer.src.blockWidth) * srcBytesPerPixel;
    const uint_fast32_t destBufBlockLineEndToStartIncrement
            = (transfer.dest.width - transform.scaleX) * destBytesPerPixel;

    auto srcBuf = reinterpret_cast<const uint8_t*>(transfer.src.buffer);
    auto srcBufPtr = WriteDestColor
            ? srcBuf + (transfer.src.blockOffsetY*transfer.src.width + transfer.src.blockOffsetX) * srcBytesPerPixel
            : nullptr;
    auto destBuf = reinterpret_cast<uint8_t*>(transfer.dest.buffer);

    auto srcAlpha = reinterpret_cast<const uint8_t*>(transfer.src.alpha);
    auto destAlpha = reinterpret_cast<uint8_t*>(transfer.dest.alpha);

    auto srcAlphaLineStartPtr = UsesSrcAlpha
            ? srcAlpha + transfer.src.blockOffsetY*srcAlphaLineSize
            : nullptr;

    // Loop over source pixels. Note that the loop coordinates are relative to the src BLOCK, not the src BUFFER (so
    // they always start at 0, not at blockOffsetX/Y). This makes mirroring and rotation easier to calculate
    for (uint_fast16_t srcBlockY = 0 ; srcBlockY < transfer.src.blockHeight ; srcBlockY++) {
        for (uint_fast16_t srcBlockX = 0 ; srcBlockX < transfer.src.blockWidth ; srcBlockX++) {
            const auto srcBufX = srcBlockX + transfer.src.blockOffsetX;

            // First we'll calculate the coordinates of the corresponding pixel, where the "destination pixel" can then
            // itself become an entire block of pixels once scaling is involved.
            auto destBlockX = srcBlockX;
            auto destBlockY = srcBlockY;

            // Step 1: Apply mirroring
            if constexpr (MirrorEnabled) {
                if ((transform.mirror & FlipDir::Horizontal) != 0) {
                    destBlockX = transfer.src.blockWidth - destBlockX - 1;
                }
                if ((transform.mirror & FlipDir::Vertical) != 0) {
                    destBlockY = transfer.src.blockHeight - destBlockY - 1;
                }
            }

            // Step 2: Apply rotation
            if constexpr (RotateEnabled) {
                switch (transform.rotation) {
                case Rotation::Rot0:
                    // Nothing to do
                    break;
                case Rotation::Rot90:
                    std::swap(destBlockX, destBlockY);
                    destBlockY = transfer.src.blockWidth - destBlockY - 1;
                    break;
                case Rotation::Rot180:
                    destBlockX = transfer.src.blockWidth - destBlockX - 1;
                    destBlockY = transfer.src.blockHeight - destBlockY - 1;
                    break;
                case Rotation::Rot270:
                    std::swap(destBlockX, destBlockY);
                    destBlockX = transfer.src.blockHeight - destBlockX - 1;
                    break;
                }
            }

            // Step 3: Transparency check
            const bool srcPixelIsOpaque = UsesSrcAlpha
                    ? srcAlphaLineStartPtr[srcBufX>>3] & (0x80 >> (srcBufX&7))
                    : true;

            // Step 4: Write dest color
            if constexpr (WriteDestColor) {
                if (!TransparencyCheckEnabled  ||  srcPixelIsOpaque) {
                    // Pixel is considered opaque for rendering (not necessarily for alpha copying) -> render it

                    if constexpr (ScaleEnabled) {
                        // Render single src pixel to every pixel in a (scaleX, scaleY) rect in dest
                        auto destBufPtr = destBuf
                                + (transfer.dest.blockOffsetY + destBlockY*transform.scaleY) * destBytesPerLine
                                + (transfer.dest.blockOffsetX + destBlockX*transform.scaleX) * destBytesPerPixel;
                        if constexpr (FormatConvEnabled) {
                            const Color pixelColor = Color::fromBuffer(srcBufPtr,
                                transfer.src.pixelFormat, transfer.src.byteOrder);
                            for (int_fast16_t sy = 0 ; sy < transform.scaleY ; sy++) {
                                pixelColor.toBuffer(destBufPtr, transfer.dest.pixelFormat, transfer.dest.byteOrder,
                                    transform.scaleX);
                                destBufPtr += destBytesPerLine;
                            }
                        } else { // !FormatConvEnabled
                            for (int_fast16_t sy = 0 ; sy < transform.scaleY ; sy++) {
                                for (int_fast16_t sx = 0 ; sx < transform.scaleX ; sx++) {
                                    memcpy(destBufPtr, srcBufPtr, CommonBytesPerPixel);
                                    destBufPtr += CommonBytesPerPixel;
                                }
                                destBufPtr += destBufBlockLineEndToStartIncrement;
                            }
                        }
                    } else { // !ScaleEnabled
                        // Render single src pixel to single dest pixel
                        auto destBufPtr = destBuf
                                + (transfer.dest.blockOffsetY+destBlockY) * destBytesPerLine
                                + (transfer.dest.blockOffsetX+destBlockX) * destBytesPerPixel;
                        if constexpr (FormatConvEnabled) {
                            Color::convertBufferToBuffer (
                                srcBufPtr, transfer.src.pixelFormat, transfer.src.byteOrder,
                                destBufPtr, transfer.dest.pixelFormat, transfer.dest.byteOrder
                                );
                        } else {
                            memcpy(destBufPtr, srcBufPtr, CommonBytesPerPixel);
                        }
                    }
                }
            }

            // Step 5: Write dest alpha (copying from source)
            // TODO: Maybe integrate this into the loops in step 4 above, so we don't have to loop twice.
            if constexpr (WriteDestAlpha) {
                const uint_fast16_t destBufBlockStartX = transfer.dest.blockOffsetX + destBlockX*transform.scaleX;
                auto destAlphaLineStartPtr = destAlpha
                    + (transfer.dest.blockOffsetY + destBlockY*transform.scaleY) * destAlphaLineSize;
                if (srcPixelIsOpaque) {
                    for (uint_fast16_t sy = 0 ; sy < transform.scaleY ; sy++) {
                        for (uint_fast16_t sx = 0 ; sx < transform.scaleX ; sx++) {
                            const auto destBufX = destBufBlockStartX + sx;
                            destAlphaLineStartPtr[destBufX >> 3] |= (0x80 >> (destBufX & 7));
                        }
                        destAlphaLineStartPtr += destAlphaLineSize;
                    }
                } else {
                    for (uint_fast16_t sy = 0 ; sy < transform.scaleY ; sy++) {
                        for (uint_fast16_t sx = 0 ; sx < transform.scaleX ; sx++) {
                            const auto destBufX = destBufBlockStartX + sx;
                            destAlphaLineStartPtr[destBufX >> 3] &= static_cast<uint8_t>(~(0x80 >> (destBufX & 7)));
                        }
                        destAlphaLineStartPtr += destAlphaLineSize;
                    }
                }
            }

            if constexpr (WriteDestColor) {
                srcBufPtr += srcBytesPerPixel;
            }
        }

        if constexpr (WriteDestColor) {
            srcBufPtr += srcBufEndOfLineIncrement;
        }
        if constexpr (UsesSrcAlpha) {
            srcAlphaLineStartPtr += srcAlphaLineSize;
        }
    }
}

template <
    bool WriteDestColor,
    bool WriteDestAlpha,
    bool TransparencyCheckEnabled,

    bool FormatConvEnabled,
    bool ScaleEnabled,
    bool RotateEnabled,
    bool MirrorEnabled
    >
void BitmapRenderer::renderCPUGeneral(Transfer& transfer, const TransformSimple& transform)
{
    if constexpr (FormatConvEnabled) {
        // The value for CommonBytesPerPixel doesn't matter in this case. We could pass something like -1, but that
        // would generate another template specialization wasting instruction memory, so we'll reuse one from below.
        renderCPUGeneral<WriteDestColor, WriteDestAlpha, TransparencyCheckEnabled, 2,
                    FormatConvEnabled, ScaleEnabled, RotateEnabled, MirrorEnabled>(transfer, transform);
    } else {
        const auto bytesPerPixel = CalcPixelFormatBytesPerPixel(transfer.src.pixelFormat);

        // NOTE: Currently we only support formats with 2, 3 or 4 bpp. It's possible that we want to add 1bpp in the
        //  future, but keep in mind that every template specialization here results in quite a lot of duplicate
        //  generated code, depending on the amount of template specializations in the render() method. So we'll
        //  deliberately keep it minimal for now.
        switch (bytesPerPixel) {
        case 2:
            renderCPUGeneral<WriteDestColor, WriteDestAlpha, TransparencyCheckEnabled, 2,
                    FormatConvEnabled, ScaleEnabled, RotateEnabled, MirrorEnabled>(transfer, transform);
            break;
        case 3:
            renderCPUGeneral<WriteDestColor, WriteDestAlpha, TransparencyCheckEnabled, 3,
                    FormatConvEnabled, ScaleEnabled, RotateEnabled, MirrorEnabled>(transfer, transform);
            break;
        case 4:
            renderCPUGeneral<WriteDestColor, WriteDestAlpha, TransparencyCheckEnabled, 4,
                    FormatConvEnabled, ScaleEnabled, RotateEnabled, MirrorEnabled>(transfer, transform);
            break;
        default:
            assert(false);
            return;
        }
    }
}

}

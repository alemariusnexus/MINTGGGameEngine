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

#include "Color.h"

#include <cassert>

#include "../util/Util.h"


namespace MINTGGGameEngine
{

const Color Color::BLACK        = {0, 0, 0};
const Color Color::WHITE        = {255, 255, 255};

const Color Color::RED          = {255, 0, 0};
const Color Color::GREEN        = {0, 255, 0};
const Color Color::BLUE         = {0, 0, 255};

const Color Color::YELLOW       = {255, 255, 0};
const Color Color::MAGENTA      = {255, 0, 255};
const Color Color::CYAN         = {0, 255, 255};


Color Color::fromRGB565(uint16_t rgb565)
{
    // Adapted from: https://stackoverflow.com/a/9069480
    return {
        static_cast<uint8_t>((static_cast<uint16_t>((rgb565 >> 11) & 0x1F)*527 + 23) >> 6),
        static_cast<uint8_t>((static_cast<uint16_t>((rgb565 >> 5) & 0x3F)*259 + 33) >> 6),
        static_cast<uint8_t>((static_cast<uint16_t>(rgb565 & 0x1F)*527 + 23) >> 6)
    };
}

Color Color::fromBGR565(uint16_t bgr565)
{
    // Adapted from: https://stackoverflow.com/a/9069480
    return {
        static_cast<uint8_t>((static_cast<uint16_t>(bgr565 & 0x1F)*527 + 23) >> 6),
        static_cast<uint8_t>((static_cast<uint16_t>((bgr565 >> 5) & 0x3F)*259 + 33) >> 6),
        static_cast<uint8_t>((static_cast<uint16_t>((bgr565 >> 11) & 0x1F)*527 + 23) >> 6)
    };
}

Color Color::fromBuffer(const void* buf, PixelFormat pixelFormat, ByteOrder byteOrder)
{
    const auto buf8 = reinterpret_cast<const uint8_t*>(buf);
    switch (pixelFormat) {
    case PixelFormat::RGB565:
        return fromRGB565(ToNativeByteOrder(*reinterpret_cast<const uint16_t*>(buf), byteOrder));
    case PixelFormat::BGR565:
        return fromBGR565(ToNativeByteOrder(*reinterpret_cast<const uint16_t*>(buf), byteOrder));
    case PixelFormat::RGB888:
        return {buf8[0], buf8[1], buf8[2]};
    case PixelFormat::BGR888:
        return {buf8[2], buf8[1], buf8[0]};
    case PixelFormat::RGBX8888:
        return {buf8[0], buf8[1], buf8[2], buf8[3]};
    case PixelFormat::XBGR8888:
        return {buf8[3], buf8[2], buf8[1], buf8[0]};
    default:
        assert(false);
        return {};
    }
}

void Color::convertBufferToBuffer (
        const void* fromBuf, PixelFormat fromPixelFormat, ByteOrder fromByteOrder,
        void* toBuf, PixelFormat toPixelFormat, ByteOrder toByteOrder,
        size_t numPixels
) {
    if (fromPixelFormat == toPixelFormat  &&  fromByteOrder == toByteOrder) {
        memcpy(toBuf, fromBuf, numPixels*CalcPixelFormatBytesPerPixel(fromPixelFormat));
        return;
    }

    auto fromBuf8 = reinterpret_cast<uint8_t*>(const_cast<void*>(fromBuf));
    auto toBuf8 = reinterpret_cast<uint8_t*>(toBuf);

    const auto fromBytesPerPixel = CalcPixelFormatBytesPerPixel(fromPixelFormat);
    const auto toBytesPerPixel = CalcPixelFormatBytesPerPixel(toPixelFormat);

    // TODO: Maybe optimize this
    while (numPixels-- != 0) {
        fromBuffer(fromBuf8, fromPixelFormat, fromByteOrder).toBuffer(toBuf8, toPixelFormat, toByteOrder);
        fromBuf8 += fromBytesPerPixel;
        toBuf8 += toBytesPerPixel;
    }
}

void Color::convertBufferToBuffer (
        const void* fromBuf, PixelFormat fromPixelFormat, ByteOrder fromByteOrder,
        void* toBuf, PixelFormat toPixelFormat, ByteOrder toByteOrder
) {
    if (fromPixelFormat == toPixelFormat  &&  fromByteOrder == toByteOrder) {
        memcpy(toBuf, fromBuf, CalcPixelFormatBytesPerPixel(fromPixelFormat));
        return;
    }

    auto fromBuf8 = reinterpret_cast<uint8_t*>(const_cast<void*>(fromBuf));
    auto toBuf8 = reinterpret_cast<uint8_t*>(toBuf);

    fromBuffer(fromBuf8, fromPixelFormat, fromByteOrder).toBuffer(toBuf8, toPixelFormat, toByteOrder);
}

size_t Color::toBuffer(void* buf, PixelFormat pixelFormat, ByteOrder byteOrder) const
{
    switch (pixelFormat) {
    case PixelFormat::RGB565:
        *reinterpret_cast<uint16_t*>(buf) = FromNativeByteOrder(toRGB565(), byteOrder);
        return 2;
    case PixelFormat::BGR565:
        *reinterpret_cast<uint16_t*>(buf) = FromNativeByteOrder(toBGR565(), byteOrder);
        return 2;
    case PixelFormat::RGB888: {
        auto buf8 = reinterpret_cast<uint8_t*>(buf);
        buf8[0] = r;
        buf8[1] = g;
        buf8[2] = b;
        return 3;
    }
    case PixelFormat::BGR888: {
        auto buf8 = reinterpret_cast<uint8_t*>(buf);
        buf8[0] = b;
        buf8[1] = g;
        buf8[2] = r;
        return 3;
    }
    case PixelFormat::RGBX8888: {
        auto buf8 = reinterpret_cast<uint8_t*>(buf);
        buf8[0] = r;
        buf8[1] = g;
        buf8[2] = b;
        buf8[3] = a;
        return 4;
    }
    case PixelFormat::XBGR8888: {
        auto buf8 = reinterpret_cast<uint8_t*>(buf);
        buf8[0] = a;
        buf8[1] = b;
        buf8[2] = g;
        buf8[3] = r;
        return 4;
    }
    default:
        assert(false);
        return 0;
    }
}

size_t Color::toBuffer(void* buf, PixelFormat pixelFormat, ByteOrder byteOrder, size_t numPixels) const
{
    switch (pixelFormat) {
    case PixelFormat::RGB565: {
        auto buf16 = reinterpret_cast<uint16_t*>(buf);
        const uint16_t raw = FromNativeByteOrder(toRGB565(), byteOrder);
        while (numPixels-- != 0) {
            *buf16++ = raw;
        }
        return 2;
    }
    case PixelFormat::BGR565: {
        auto buf16 = reinterpret_cast<uint16_t*>(buf);
        const uint16_t raw = FromNativeByteOrder(toBGR565(), byteOrder);
        while (numPixels-- != 0) {
            *buf16++ = raw;
        }
        return 2;
    }
    case PixelFormat::RGB888: {
        auto buf8 = reinterpret_cast<uint8_t*>(buf);
        while (numPixels-- != 0) {
            buf8[0] = r;
            buf8[1] = g;
            buf8[2] = b;
            buf8 += 3;
        }
        return 3;
    }
    case PixelFormat::BGR888: {
        auto buf8 = reinterpret_cast<uint8_t*>(buf);
        while (numPixels-- != 0) {
            buf8[0] = b;
            buf8[1] = g;
            buf8[2] = r;
            buf8 += 3;
        }
        return 3;
    }
    case PixelFormat::RGBX8888: {
        auto buf8 = reinterpret_cast<uint8_t*>(buf);
        while (numPixels-- != 0) {
            buf8[0] = r;
            buf8[1] = g;
            buf8[2] = b;
            buf8[3] = a;
            buf8 += 4;
        }
        return 4;
    }
    case PixelFormat::XBGR8888: {
        auto buf8 = reinterpret_cast<uint8_t*>(buf);
        while (numPixels-- != 0) {
            buf8[0] = a;
            buf8[1] = b;
            buf8[2] = g;
            buf8[3] = r;
            buf8 += 4;
        }
        return 4;
    }
    default:
        assert(false);
        return 0;
    }
}

}

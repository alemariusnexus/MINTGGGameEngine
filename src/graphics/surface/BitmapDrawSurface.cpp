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

#include "BitmapDrawSurface.h"


MINTGGGameEngine::BitmapDrawSurface::BitmapDrawSurface(Bitmap& bitmap)
    : MemDrawSurface (
        bitmap.getPixelFormat(),
        bitmap.getByteOrder(),
        bitmap.getData(),
        bitmap.getWidth(), bitmap.getHeight(),
        {}
        ),
      bmp(bitmap)
{
}

MINTGGGameEngine::BitmapDrawSurface::BitmapDrawSurface (
    uint16_t width, uint16_t height,
    bool haveAlpha, uint8_t alpha,
    PixelFormat pixelFormat,
    AlphaStorage alphaStorage,
    ByteOrder byteOrder
)   : MemDrawSurface (
        pixelFormat,
        byteOrder,
        nullptr,
        width, height,
        {}
        )
{
    bmp = Bitmap(width, height, true, haveAlpha, pixelFormat, alphaStorage, byteOrder);
    bmp.fillAlpha(alpha);
    setBuffer(bmp.getData(), {});
}

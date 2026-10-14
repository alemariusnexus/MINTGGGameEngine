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

#include "../../Globals.h"

#include "MemDrawSurface.h"


namespace MINTGGGameEngine
{

class BitmapDrawSurface : public MemDrawSurface
{
public:
    BitmapDrawSurface(Bitmap& bitmap);
    BitmapDrawSurface (
        uint16_t width, uint16_t height,
        bool haveAlpha = false, uint8_t alpha = 0xFF,
        PixelFormat pixelFormat = PixelFormat::RGB565,
        AlphaStorage alphaStorage = AlphaStorage::Bit1,
        ByteOrder byteOrder = ByteOrder::Native
        );

    Bitmap getBitmap() const { return bmp; }

private:
    Bitmap bmp;
};

}

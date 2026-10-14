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

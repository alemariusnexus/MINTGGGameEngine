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

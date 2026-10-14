#pragma once

#include "../Globals.h"

#include "../storage/Reader.h"
#include "surface/DrawSurface.h"
#include "Bitmap.h"


namespace MINTGGGameEngine
{


class ImageLoader
{
private:
    enum
    {
        BMPLoadFlagAlphaFromColor    = 0x01
    };

public:
    ImageLoader();
    ImageLoader(DrawSurface& surfaceForFormat);

    void setPixelFormat(PixelFormat format) { pixelFormat = format; }
    void setAlphaStorage(AlphaStorage storage) { alphaStorage = storage; }
    void setByteOrder(ByteOrder order) { byteOrder = order; }

    void setOffset(uint16_t x, uint16_t y) { offsetX = x; offsetY = y; }
    void setMaxSize(uint16_t w, uint16_t h) { maxWidth = w; maxHeight = h; }

    void setScaleFactor(float scaleX, float scaleY) { scaleFactorX = scaleX; scaleFactorY = scaleY; }
    float getScaleFactorX() const { return scaleFactorX; }
    float getScaleFactorY() const { return scaleFactorY; }

    void setColorAllocFlags(uint32_t flags) { colorAllocFlags = flags; }
    uint32_t getColorAllocFlags() const { return colorAllocFlags; }

    PixelFormat getPixelFormat() const { return pixelFormat; }
    AlphaStorage getAlphaStorage() const { return alphaStorage; }
    ByteOrder getByteOrder() const { return byteOrder; }

    uint16_t getOffsetX() const { return offsetX; }
    uint16_t getOffsetY() const { return offsetY; }
    uint16_t getMaxWidth() const { return maxWidth; }
    uint16_t getMaxHeight() const { return maxHeight; }

    const char* getErrorMessage() const { return errmsg; }

    Bitmap loadBitmapBMP(Reader& reader);
    Bitmap loadBitmapBMP(const std::string_view& path);

    Bitmap loadBitmapBMPAlphaOnly(Reader& reader);
    Bitmap loadBitmapBMPAlphaOnly(const std::string_view& path);

    Bitmap loadBitmapBMPSeparateAlpha(Reader& rgbReader, Reader& alphaReader);
    Bitmap loadBitmapBMPSeparateAlpha (
        const std::string_view& rgbPath, const std::string_view& alphaPath);

private:
    bool loadBMPRaw (
        const std::string_view& path,
        void** outRgb, void** outAlpha,
        uint16_t* outWidth, uint16_t* outHeight,
        int flags
        );
    bool loadBMPRaw (
        Reader& reader,
        void** outRgb, void** outAlpha,
        uint16_t* outWidth, uint16_t* outHeight,
        int flags
        );

    bool setError(const char* msg) { errmsg = msg; return false; }

private:
    PixelFormat pixelFormat;
    AlphaStorage alphaStorage;
    ByteOrder byteOrder;
    uint16_t offsetX;
    uint16_t offsetY;
    uint16_t maxWidth;
    uint16_t maxHeight;
    float scaleFactorX;
    float scaleFactorY;
    const char* errmsg;

    uint32_t colorAllocFlags;
};


}

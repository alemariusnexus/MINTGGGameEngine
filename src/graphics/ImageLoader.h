#pragma once

#include "../Globals.h"

#include "../storage/BufferedReader.h"
#include "Bitmap.h"


namespace MINTGGGameEngine
{


class ImageLoader
{
private:
    enum
    {
        BMPLoadFlagMaskFromColor    = 0x01
    };

public:
    ImageLoader();

    void setOffset(uint16_t x, uint16_t y) { offsetX = x; offsetY = y; }
    void setMaxSize(uint16_t w, uint16_t h) { maxWidth = w; maxHeight = h; }

    uint16_t getOffsetX() const { return offsetX; }
    uint16_t getOffsetY() const { return offsetY; }
    uint16_t getMaxWidth() const { return maxWidth; }
    uint16_t getMaxHeight() const { return maxHeight; }

    const char* getErrorMessage() const { return errmsg; }

    Bitmap loadBitmapBMP(const char* path);
    Bitmap loadBitmapBMPSeparateMask(const char* rgbPath, const char* maskPath);

private:
    bool loadBMPRaw565 (
        const char* path,
        uint16_t** outRgb, uint8_t** outMask,
        uint16_t* outWidth, uint16_t* outHeight,
        int flags
        );
    bool loadBMPRaw565 (
        BufferedReader& reader,
        uint16_t** outRgb, uint8_t** outMask,
        uint16_t* outWidth, uint16_t* outHeight,
        int flags
        );

    bool setError(const char* errmsg) { this->errmsg = errmsg; return false; }

private:
    uint16_t offsetX;
    uint16_t offsetY;
    uint16_t maxWidth;
    uint16_t maxHeight;
    const char* errmsg;
};


}

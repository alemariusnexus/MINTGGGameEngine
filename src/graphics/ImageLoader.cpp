#include "ImageLoader.h"

#include "../storage/BufferedReader.h"
#include "../storage/FileReader.h"
#include "../util/Log.h"
#include "../util/Util.h"


namespace MINTGGGameEngine
{


struct BMPFileHeader
{
    char signature[2]; // e.g. "BM"
    uint32_t fileSize;
    uint8_t unused1[4];
    uint32_t dataOffset; // From beginning of file to BMP data
} __attribute__((packed));

struct BMPInfoHeader
{
    uint32_t infoHeaderSize; // Should be 40 for BITMAPINFOHEADER
    uint32_t width; // in pixels
    uint32_t height; // in pixels
    uint16_t planes;
    uint16_t bitsPerPixel; // We support only 24 and 32 here
    uint32_t compression; // We ignore this and hope for the best
    uint32_t imageSize;
    uint32_t xPixelsPerMeter;
    uint32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t importantColors;
} __attribute__((packed));



ImageLoader::ImageLoader()
    : pixelFormat(PixelFormat::RGB565),
      alphaStorage(AlphaStorage::Bit1),
      byteOrder(ByteOrder::Native),
      offsetX(0), offsetY(0), maxWidth(UINT16_MAX), maxHeight(UINT16_MAX),
      scaleFactorX(1), scaleFactorY(1),
      errmsg(nullptr),
      colorAllocFlags(0)
{
}

ImageLoader::ImageLoader(DrawSurface& surfaceForFormat)
    : pixelFormat(surfaceForFormat.getNativePixelFormat()),
      alphaStorage(AlphaStorage::Bit1),
      byteOrder(surfaceForFormat.getNativeByteOrder()),
      offsetX(0), offsetY(0), maxWidth(UINT16_MAX), maxHeight(UINT16_MAX),
      scaleFactorX(1), scaleFactorY(1),
      errmsg(nullptr),
      colorAllocFlags(0)
{
}

Bitmap ImageLoader::loadBitmapBMP(Reader& reader)
{
    void* rgb = nullptr;
    void* mask = nullptr;
    uint16_t w, h;
    if (!loadBMPRaw(reader, &rgb, &mask, &w, &h, 0)) {
        return {};
    }
    return Bitmap::takeOwnership(w, h, rgb, mask, pixelFormat, alphaStorage, byteOrder).scaled(scaleFactorX, scaleFactorY);
}

Bitmap ImageLoader::loadBitmapBMP(const std::string_view& path)
{
    void* rgb = nullptr;
    void* mask = nullptr;
    uint16_t w, h;
    if (!loadBMPRaw(path, &rgb, &mask, &w, &h, 0)) {
        return {};
    }
    return Bitmap::takeOwnership(w, h, rgb, mask, pixelFormat, alphaStorage, byteOrder).scaled(scaleFactorX, scaleFactorY);
}

Bitmap ImageLoader::loadBitmapBMPAlphaOnly(Reader& reader)
{
    void* alpha = nullptr;
    uint16_t w, h;
    if (!loadBMPRaw(reader, nullptr, &alpha, &w, &h, BMPLoadFlagAlphaFromColor)) {
        return {};
    }
    return Bitmap::takeOwnership(w, h, nullptr, alpha, pixelFormat, alphaStorage, byteOrder).scaled(scaleFactorX, scaleFactorY);
}

Bitmap ImageLoader::loadBitmapBMPAlphaOnly(const std::string_view& path)
{
    void* alpha = nullptr;
    uint16_t w, h;
    if (!loadBMPRaw(path, nullptr, &alpha, &w, &h, BMPLoadFlagAlphaFromColor)) {
        return {};
    }
    return Bitmap::takeOwnership(w, h, nullptr, alpha, pixelFormat, alphaStorage, byteOrder).scaled(scaleFactorX, scaleFactorY);
}

Bitmap ImageLoader::loadBitmapBMPSeparateAlpha(Reader& rgbReader, Reader& maskReader)
{
    void* rgb = nullptr;
    uint16_t w, h;
    if (!loadBMPRaw(rgbReader, &rgb, nullptr, &w, &h, 0)) {
        return {};
    }

    void* alpha = nullptr;
    uint16_t wm, hm;
    if (!loadBMPRaw(maskReader, nullptr, &alpha, &wm, &hm, BMPLoadFlagAlphaFromColor)) {
        free(rgb);
        return {};
    }

    if (wm != w  ||  hm != h) {
        free(rgb);
        free(alpha);
        setError("alpha dimensions mismatch");
        return {};
    }

    return Bitmap::takeOwnership(w, h, rgb, alpha, pixelFormat, alphaStorage, byteOrder).scaled(scaleFactorX, scaleFactorY);
}

Bitmap ImageLoader::loadBitmapBMPSeparateAlpha (
    const std::string_view& rgbPath,
    const std::string_view& alphaPath
) {
    void* rgb = nullptr;
    uint16_t w, h;
    if (!loadBMPRaw(rgbPath, &rgb, nullptr, &w, &h, 0)) {
        return {};
    }

    void* mask = nullptr;
    uint16_t wm, hm;
    if (!loadBMPRaw(alphaPath, nullptr, &mask, &wm, &hm, BMPLoadFlagAlphaFromColor)) {
        free(rgb);
        return {};
    }

    if (wm != w  ||  hm != h) {
        free(rgb);
        free(mask);
        setError("alpha dimensions mismatch");
        return {};
    }

    return Bitmap::takeOwnership(w, h, rgb, mask, pixelFormat, alphaStorage, byteOrder).scaled(scaleFactorX, scaleFactorY);
}

bool ImageLoader::loadBMPRaw (
    const std::string_view& path,
    void** outRgb, void** outAlpha,
    uint16_t* outWidth, uint16_t* outHeight,
    int flags
) {
    char fbuf[512];
    FileReader freader{File(path)};
    if (!freader.open(&errmsg)) {
        return false;
    }
    BufferedReader reader(freader, fbuf, sizeof(fbuf));
    return loadBMPRaw(reader, outRgb, outAlpha, outWidth, outHeight, flags);
}

bool ImageLoader::loadBMPRaw (
    Reader& reader,
    void** outRgb, void** outAlpha,
    uint16_t* outWidth, uint16_t* outHeight,
    int flags
) {
    //timer_mstick_t t1 = TimerGetTickcountMs();

    ssize_t bmpOrigin = reader.tell();
    if (bmpOrigin < 0) {
        return setError("file position not available");
    }

    //timer_mstick_t t2 = TimerGetTickcountMs();

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    size_t numRead;
    numRead = reader.read(&fileHeader, sizeof(fileHeader));
    if (numRead != sizeof(fileHeader)) {
        return setError("premature end of file header");
    }

    if (fileHeader.signature[0] != 'B'  ||  fileHeader.signature[1] != 'M') {
        return setError("invalid BMP signature");
    }

    numRead = reader.read(&infoHeader, sizeof(infoHeader));
    if (numRead != sizeof(infoHeader)) {
        return setError("premature end of info header");
    }

    if (infoHeader.infoHeaderSize < 40) {
        return setError("unsupported info header");
    }

    if (infoHeader.bitsPerPixel != 24  &&  infoHeader.bitsPerPixel != 32) {
        return setError("unsupported bits per pixel");
    }

    if (infoHeader.width > UINT16_MAX  ||  infoHeader.height > UINT16_MAX) {
        return setError("image too large");
    }

    //timer_mstick_t t3 = TimerGetTickcountMs();

    const uint16_t sx = std::min(offsetX, static_cast<uint16_t>(infoHeader.width));
    const uint16_t sy = std::min(offsetY, static_cast<uint16_t>(infoHeader.height));
    const uint16_t w = std::min(maxWidth, static_cast<uint16_t>(infoHeader.width-sx));
    const uint16_t h = std::min(maxHeight, static_cast<uint16_t>(infoHeader.height-sy));

    if (outWidth) {
        *outWidth = w;
    }
    if (outHeight) {
        *outHeight = h;
    }

    const uint16_t ey = offsetY+h-1;

    const bool loadAlpha =
        outAlpha
        &&  (   infoHeader.bitsPerPixel == 32
                ||  ((flags & BMPLoadFlagAlphaFromColor) != 0)
            )
        ;

    void* rgb = nullptr;
    uint8_t* alpha = nullptr;
    bool rgbOwned = false;
    bool maskOwned = false;
    const size_t maskBytesPerLine = Bitmap::calcAlphaBytesPerLine(w);
    const auto outBytesPerPixel = CalcPixelFormatBytesPerPixel(pixelFormat);

    assert(alphaStorage == AlphaStorage::Bit1);

    if (outRgb) {
        if (*outRgb) {
            rgb = *outRgb;
        } else {
            rgb = Bitmap::allocDataBuffer(w, h, outBytesPerPixel, colorAllocFlags);
            if (!rgb) {
                return setError("rgb allocation failed");
            }
            rgbOwned = true;
        }
    }
    if (loadAlpha) {
        if (*outAlpha) {
            alpha = static_cast<uint8_t*>(*outAlpha);
        } else {
            alpha = static_cast<uint8_t*>(malloc(maskBytesPerLine * h * sizeof(uint8_t)));
            if (!alpha) {
                if (rgbOwned) free(rgb);
                return setError("alpha allocation failed");
            }
            maskOwned = true;
        }
        memset(alpha, 0, maskBytesPerLine*h*sizeof(uint8_t));
    }

    if (w != 0  &&  h != 0) {
        const uint8_t inBytesPerPixel = infoHeader.bitsPerPixel/8;

        // NOTE: The BMP files this was tested with store pixel data bottom-up, in BGR(A) order.

        // Size of data for one full line in BMP file
        ssize_t lineSize = static_cast<ssize_t>(infoHeader.width*inBytesPerPixel);
        if (lineSize%4 != 0) {
            // Lines are padded to multiples of 4 bytes
            lineSize += 4 - (lineSize%4);
        }

        const size_t interLineSkipSize = lineSize - w*inBytesPerPixel;

        //timer_mstick_t t4 = TimerGetTickcountMs();

        bool seekOk = reader.seek (
            bmpOrigin + fileHeader.dataOffset                               // Start of data block
                    + (infoHeader.height-ey-1)*lineSize                     // Bottom lines until `ey`
                    + sx*inBytesPerPixel,
            File::SeekSet
            );
        if (!seekOk) {
            return setError("error seeking data");
        }

        //timer_mstick_t t5 = TimerGetTickcountMs();

        if (infoHeader.bitsPerPixel == 24) {
            uint8_t bgr[3];
            for (uint16_t outY = h-1 ; outY != UINT16_MAX ; outY--) { // Assumes integer underflow, which IS well-defined
                const size_t maskLineOffset = outY*maskBytesPerLine;
                uint8_t* rgbPtr = rgb ? static_cast<uint8_t*>(rgb) + outY*w*outBytesPerPixel : nullptr;
                for (uint16_t outX = 0 ; outX < w ; outX++) {
                    numRead = reader.read(bgr, 3);
                    if (numRead != 3) {
                        if (rgbOwned) free(rgb);
                        if (maskOwned) free(alpha);
                        return setError("premature end of data");
                    }
                    if (rgb) {
                        switch (pixelFormat) {
                        case PixelFormat::RGB565:
                            *reinterpret_cast<uint16_t*>(rgbPtr) = FromNativeByteOrder<uint16_t>(
                                    ((bgr[2] >> 3) << 11)       // R
                                        | ((bgr[1] >> 2) << 5)  // G
                                        | (bgr[0] >> 3),        // B
                                    byteOrder);
                            break;
                        case PixelFormat::BGR565:
                            *reinterpret_cast<uint16_t*>(rgbPtr) = FromNativeByteOrder<uint16_t>(
                                    ((bgr[0] >> 3) << 11)       // R
                                        | ((bgr[1] >> 2) << 5)  // G
                                        | (bgr[2] >> 3),        // B
                                    byteOrder);
                            break;
                        case PixelFormat::RGB888:
                            rgbPtr[0] = bgr[2];
                            rgbPtr[1] = bgr[1];
                            rgbPtr[2] = bgr[0];
                            break;
                        case PixelFormat::BGR888:
                            rgbPtr[0] = bgr[0];
                            rgbPtr[1] = bgr[1];
                            rgbPtr[2] = bgr[2];
                            break;
                        case PixelFormat::RGBX8888:
                            rgbPtr[0] = bgr[2];
                            rgbPtr[1] = bgr[1];
                            rgbPtr[2] = bgr[0];
                            rgbPtr[3] = 255;
                            break;
                        case PixelFormat::XBGR8888:
                            rgbPtr[0] = 255;
                            rgbPtr[1] = bgr[0];
                            rgbPtr[2] = bgr[1];
                            rgbPtr[3] = bgr[2];
                            break;
                        default:
                            assert(false);
                        }
                        rgbPtr += outBytesPerPixel;
                    }
                    if (loadAlpha) {
                        uint16_t rgbSum = static_cast<uint16_t>(bgr[0])
                            + static_cast<uint16_t>(bgr[1])
                            + static_cast<uint16_t>(bgr[2]);
                        if (rgbSum > 127*3) {
                            alpha[maskLineOffset + (outX>>3)] |= (0x80 >> (outX&7));
                        }
                    }
                }
                if (outY != 0) { // Important, otherwise seek might fail at end of file!
                    if (!reader.seek(interLineSkipSize, File::SeekCur)) {
                        if (rgbOwned) free(rgb);
                        if (maskOwned) free(alpha);
                        return setError("error seeking data");
                    }
                }
            }
        } else if (infoHeader.bitsPerPixel == 32) {
            uint8_t bgra[4];
            for (uint16_t outY = h-1 ; outY != UINT16_MAX ; outY--) { // Assumes integer underflow, which IS well-defined
                const size_t maskLineOffset = outY*maskBytesPerLine;
                uint8_t* rgbPtr = rgb ? static_cast<uint8_t*>(rgb) + outY*w*outBytesPerPixel : nullptr;
                for (uint16_t outX = 0 ; outX < w ; outX++) {
                    numRead = reader.read(bgra, 4);
                    if (numRead != 4) {
                        if (rgbOwned) free(rgb);
                        if (maskOwned) free(alpha);
                        return setError("premature end of data");
                    }
                    if (rgb) {
                        switch (pixelFormat) {
                        case PixelFormat::RGB565:
                            *reinterpret_cast<uint16_t*>(rgbPtr) = FromNativeByteOrder<uint16_t>(
                                    ((bgra[2] >> 3) << 11)      // R
                                        | ((bgra[1] >> 2) << 5) // G
                                        | (bgra[0] >> 3),       // B
                                    byteOrder);
                            break;
                        case PixelFormat::BGR565:
                            *reinterpret_cast<uint16_t*>(rgbPtr) = FromNativeByteOrder<uint16_t>(
                                    ((bgra[0] >> 3) << 11)      // R
                                        | ((bgra[1] >> 2) << 5) // G
                                        | (bgra[2] >> 3),       // B
                                    byteOrder);
                            break;
                        case PixelFormat::RGB888:
                            rgbPtr[0] = bgra[2];
                            rgbPtr[1] = bgra[1];
                            rgbPtr[2] = bgra[0];
                            break;
                        case PixelFormat::BGR888:
                            rgbPtr[0] = bgra[0];
                            rgbPtr[1] = bgra[1];
                            rgbPtr[2] = bgra[2];
                            break;
                        case PixelFormat::RGBX8888:
                            rgbPtr[0] = bgra[2];
                            rgbPtr[1] = bgra[1];
                            rgbPtr[2] = bgra[0];
                            rgbPtr[3] = 255;
                            break;
                        case PixelFormat::XBGR8888:
                            rgbPtr[0] = 255;
                            rgbPtr[1] = bgra[0];
                            rgbPtr[2] = bgra[1];
                            rgbPtr[3] = bgra[2];
                            break;
                        default:
                            assert(false);
                        }
                        rgbPtr += outBytesPerPixel;
                    }
                    if (loadAlpha) {
                        if ((flags & BMPLoadFlagAlphaFromColor) != 0) {
                            uint16_t rgbSum = static_cast<uint16_t>(bgra[0])
                                + static_cast<uint16_t>(bgra[1])
                                + static_cast<uint16_t>(bgra[2]);
                            if (rgbSum > 127*3) {
                                alpha[maskLineOffset + (outX>>3)] |= (0x80 >> (outX&7));
                            }
                        } else {
                            if (bgra[3] > 127) {
                                alpha[maskLineOffset + (outX>>3)] |= (0x80 >> (outX&7));
                            }
                        }
                    }
                }
                if (outY != 0) { // Important, otherwise seek might fail at end of file!
                    if (!reader.seek(interLineSkipSize, File::SeekCur)) {
                        if (rgbOwned) free(rgb);
                        if (maskOwned) free(alpha);
                        return setError("error seeking data");
                    }
                }
            }
        }

        //timer_mstick_t t6 = TimerGetTickcountMs();

        /*LogInfo("Timing   -   t1-t2: %u, t2-t3: %u, t3:t4: %u, t4-t5: %u, t5-t6: %u",
            (unsigned int) (t2-t1),
            (unsigned int) (t3-t2),
            (unsigned int) (t4-t3),
            (unsigned int) (t5-t4),
            (unsigned int) (t6-t5)
            );*/
    }

    if (outRgb  &&  !*outRgb) {
        *outRgb = rgb;
    }
    if (outAlpha  &&  !*outAlpha) {
        *outAlpha = alpha;
    }

    return true;
}


}

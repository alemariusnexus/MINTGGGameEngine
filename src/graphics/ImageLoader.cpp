#include "ImageLoader.h"

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
    : offsetX(0), offsetY(0), maxWidth(UINT16_MAX), maxHeight(UINT16_MAX)
{
}

Bitmap ImageLoader::loadBitmapBMP(const char* path)
{
    uint16_t* rgb = nullptr;
    uint8_t* mask = nullptr;
    uint16_t w, h;
    if (!loadBMPRaw565(path, &rgb, &mask, &w, &h, 0)) {
        return {};
    }
    return Bitmap::takeOwnership(w, h, rgb, mask);
}

Bitmap ImageLoader::loadBitmapBMPSeparateMask(const char* rgbPath, const char* maskPath)
{
    uint16_t* rgb = nullptr;
    uint16_t w, h;
    if (!loadBMPRaw565(rgbPath, &rgb, nullptr, &w, &h, 0)) {
        return {};
    }

    uint8_t* mask = nullptr;
    uint16_t wm, hm;
    if (!loadBMPRaw565(rgbPath, nullptr, &mask, &wm, &hm, BMPLoadFlagMaskFromColor)) {
        free(rgb);
        return {};
    }

    if (wm != w  ||  hm != h) {
        free(rgb);
        free(mask);
        setError("mask dimensions mismatch");
        return {};
    }

    return Bitmap::takeOwnership(w, h, rgb, mask);
}

bool ImageLoader::loadBMPRaw565 (
    const char* path,
    uint16_t** outRgb, uint8_t** outMask,
    uint16_t* outWidth, uint16_t* outHeight,
    int flags
) {
    char fbuf[512];
    BufferedReader reader(File(path), fbuf, sizeof(fbuf));
    return loadBMPRaw565(reader, outRgb, outMask, outWidth, outHeight, flags);
}

bool ImageLoader::loadBMPRaw565 (
    BufferedReader& reader,
    uint16_t** outRgb, uint8_t** outMask,
    uint16_t* outWidth, uint16_t* outHeight,
    int flags
) {
    timer_mstick_t t1 = TimerGetTickcountMs();

    if (!reader.isOpen()) {
        if (!reader.open(&errmsg)) {
            return false;
        }
    }

    ssize_t bmpOrigin = reader.tell();
    if (bmpOrigin < 0) {
        return setError("file position not available");
    }

    timer_mstick_t t2 = TimerGetTickcountMs();

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

    timer_mstick_t t3 = TimerGetTickcountMs();

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

    const bool loadMask =
        outMask
        &&  (   infoHeader.bitsPerPixel == 32
                ||  ((flags & BMPLoadFlagMaskFromColor) != 0)
            )
        ;

    uint16_t* rgb = nullptr;
    uint8_t* mask = nullptr;
    bool rgbOwned = false;
    bool maskOwned = false;
    const size_t maskBytesPerLine = Bitmap::calcMaskBytesPerLine(w);

    if (outRgb) {
        if (*outRgb) {
            rgb = *outRgb;
        } else {
            rgb = static_cast<uint16_t*>(malloc(w * h * sizeof(uint16_t)));
            if (!rgb) {
                return setError("rgb allocation failed");
            }
            rgbOwned = true;
        }
    }
    if (loadMask) {
        if (*outMask) {
            mask = *outMask;
        } else {
            mask = static_cast<uint8_t*>(malloc(maskBytesPerLine * h * sizeof(uint8_t)));
            if (!mask) {
                if (rgbOwned) free(rgb);
                return setError("mask allocation failed");
            }
            maskOwned = true;
        }
        memset(mask, 0, maskBytesPerLine*h*sizeof(uint8_t));
    }

    if (w != 0  &&  h != 0) {
        const uint8_t bytesPerPixel = infoHeader.bitsPerPixel/8;

        // NOTE: The BMP files this was tested with store pixel data bottom-up, in BGR(A) order.

        // Size of data for one full line in BMP file
        ssize_t lineSize = static_cast<ssize_t>(infoHeader.width*bytesPerPixel);
        if (lineSize%4 != 0) {
            // Lines are padded to multiples of 4 bytes
            lineSize += 4 - (lineSize%4);
        }

        const size_t interLineSkipSize = lineSize - w*bytesPerPixel;

        timer_mstick_t t4 = TimerGetTickcountMs();

        bool seekOk = reader.seek (
            bmpOrigin + fileHeader.dataOffset                               // Start of data block
            + (infoHeader.height-ey-1)*lineSize                             // Bottom lines until `ey`
            + sx*bytesPerPixel,
            File::SeekSet
            );
        if (!seekOk) {
            return setError("error seeking data");
        }

        timer_mstick_t t5 = TimerGetTickcountMs();

        if (infoHeader.bitsPerPixel == 24) {
            uint8_t bgr[3];
            for (uint16_t outY = h-1 ; outY != UINT16_MAX ; outY--) { // Assumes integer underflow, which IS well-defined
                const size_t maskLineOffset = outY*maskBytesPerLine;
                for (uint16_t outX = 0 ; outX < w ; outX++) {
                    numRead = reader.read(bgr, 3);
                    if (numRead != 3) {
                        if (rgbOwned) free(rgb);
                        if (maskOwned) free(mask);
                        return setError("premature end of data");
                    }
                    if (rgb) {
                        rgb[outY*w + outX] =
                            ((bgr[2] >> 3) << 11)   // R
                            | ((bgr[1] >> 2) << 5)  // G
                            | (bgr[0] >> 3);        // B
                    }
                    if (loadMask) {
                        uint16_t rgbSum = static_cast<uint16_t>(bgr[0])
                            + static_cast<uint16_t>(bgr[1])
                            + static_cast<uint16_t>(bgr[2]);
                        if (rgbSum > 127*3) {
                            mask[maskLineOffset + (outX>>3)] |= (0x80 >> (outX&7));
                        }
                    }
                }
                if (outY != 0) { // Important, otherwise seek might fail at end of file!
                    if (!reader.seek(interLineSkipSize, File::SeekCur)) {
                        if (rgbOwned) free(rgb);
                        if (maskOwned) free(mask);
                        return setError("error seeking data");
                    }
                }
            }
        } else if (infoHeader.bitsPerPixel == 32) {
            uint8_t bgra[4];
            for (uint16_t outY = h-1 ; outY != UINT16_MAX ; outY--) { // Assumes integer underflow, which IS well-defined
                const size_t maskLineOffset = outY*maskBytesPerLine;
                for (uint16_t outX = 0 ; outX < w ; outX++) {
                    numRead = reader.read(bgra, 4);
                    if (numRead != 4) {
                        if (rgbOwned) free(rgb);
                        if (maskOwned) free(mask);
                        return setError("premature end of data");
                    }
                    if (rgb) {
                        rgb[outY*w + outX] =
                            ((bgra[2] >> 3) << 11)  // R
                            | ((bgra[1] >> 2) << 5) // G
                            | (bgra[0] >> 3);       // B
                    }
                    if (loadMask) {
                        if ((flags & BMPLoadFlagMaskFromColor) != 0) {
                            uint16_t rgbSum = static_cast<uint16_t>(bgra[0])
                                + static_cast<uint16_t>(bgra[1])
                                + static_cast<uint16_t>(bgra[2]);
                            if (rgbSum > 127*3) {
                                mask[maskLineOffset + (outX>>3)] |= (0x80 >> (outX&7));
                            }
                        } else {
                            if (bgra[3] > 127) {
                                mask[maskLineOffset + (outX>>3)] |= (0x80 >> (outX&7));
                            }
                        }
                    }
                }
                if (outY != 0) { // Important, otherwise seek might fail at end of file!
                    if (!reader.seek(interLineSkipSize, File::SeekCur)) {
                        if (rgbOwned) free(rgb);
                        if (maskOwned) free(mask);
                        return setError("error seeking data");
                    }
                }
            }
        }

        timer_mstick_t t6 = TimerGetTickcountMs();

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
    if (outMask  &&  !*outMask) {
        *outMask = mask;
    }

    return true;
}


}

#include "Bitmap.h"

#include <algorithm>
#include <new>


namespace MINTGGGameEngine
{


// Used as data and/or mask buffer for bitmaps with zero width or height, to avoid using nullptr.
static uint8_t BitmapDummyBuffer;


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


Bitmap Bitmap::loadBMP (
    File& file,
    uint16_t ox, uint16_t oy,
    uint16_t w, uint16_t h,
    const char** outErrmsg
) {
    ssize_t bmpOrigin = file.tell();
    if (bmpOrigin < 0) {
        if (outErrmsg) *outErrmsg = "error getting file position";
        return Bitmap();
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    
    size_t numRead;
    numRead = file.read(&fileHeader, sizeof(fileHeader));
    if (numRead != sizeof(fileHeader)) {
        if (outErrmsg) *outErrmsg = "premature end of file header";
        return Bitmap();
    }
    
    if (fileHeader.signature[0] != 'B'  ||  fileHeader.signature[1] != 'M') {
        if (outErrmsg) *outErrmsg = "invalid BMP signature";
        return Bitmap();
    }
    
    numRead = file.read(&infoHeader, sizeof(infoHeader));
    if (numRead != sizeof(infoHeader)) {
        if (outErrmsg) *outErrmsg = "premature end of info header";
        return Bitmap();
    }
    
    if (infoHeader.infoHeaderSize < 40) {
        if (outErrmsg) *outErrmsg = "unsupported info header";
        return Bitmap();
    }
    
    if (infoHeader.bitsPerPixel != 24  &&  infoHeader.bitsPerPixel != 32) {
        if (outErrmsg) *outErrmsg = "unsupported bits per pixel";
        return Bitmap();
    }

    if (infoHeader.width > UINT16_MAX  ||  infoHeader.height > UINT16_MAX) {
        if (outErrmsg) *outErrmsg = "image too large";
        return Bitmap();
    }

    const uint16_t sx = std::min(ox, static_cast<uint16_t>(infoHeader.width));
    const uint16_t sy = std::min(oy, static_cast<uint16_t>(infoHeader.height));
    w = std::min(w, static_cast<uint16_t>(infoHeader.width-sx));
    h = std::min(h, static_cast<uint16_t>(infoHeader.height-sy));

    if (w == 0  ||  h == 0) {
        // Special case: empty image (will not be handled gracefully by the code below due to integer underflow)
        if (infoHeader.bitsPerPixel == 24) {
            return Bitmap(w, h, reinterpret_cast<const uint16_t*>(&BitmapDummyBuffer));
        } else if (infoHeader.bitsPerPixel == 32) {
            return Bitmap(w, h, reinterpret_cast<const uint16_t*>(&BitmapDummyBuffer), &BitmapDummyBuffer);
        }
    }

    const uint16_t ey = oy+h-1;

    auto data16 = static_cast<uint16_t*>(malloc(w * h * sizeof(uint16_t)));
    if (!data16) {
        // Probably not enough RAM for image (too large)
        if (outErrmsg) *outErrmsg = "allocation failed";
        return Bitmap();
    }
    uint8_t* mask = nullptr;

    const uint8_t bytesPerPixel = infoHeader.bitsPerPixel/8;

    // NOTE: The BMP files this was tested with store pixel data bottom-up, in BGR(A) order.

    ssize_t fileOffset = static_cast<ssize_t> (
        bmpOrigin + fileHeader.dataOffset                               // Start of data block
        + (infoHeader.height-ey-1)*infoHeader.width*bytesPerPixel       // Bottom lines until `ey`
        + sx*bytesPerPixel                                              // Start of bottom line until `sx`;
        );

    // Size of data for one full line in BMP file
    ssize_t lineSize = static_cast<ssize_t>(infoHeader.width*bytesPerPixel);
    if (lineSize%4 != 0) {
        // Lines are padded to multiples of 4 bytes
        lineSize += 4 - (lineSize%4);
    }
    
    if (infoHeader.bitsPerPixel == 24) {
        uint8_t bgr[3];
        for (uint16_t outY = h-1 ; outY != UINT16_MAX ; outY--) { // Assumes integer underflow, which IS well-defined
            if (!file.seek(fileOffset)) {
                if (outErrmsg) *outErrmsg = "failed to seek data";
                free(data16);
                return Bitmap();
            }
            for (uint16_t outX = 0 ; outX < w ; outX++) {
                numRead = file.read(bgr, 3);
                if (numRead != 3) {
                    if (outErrmsg) *outErrmsg = "premature end of data";
                    free(data16);
                    return Bitmap();
                }
                data16[outY*w + outX] =
                    ((bgr[2] >> 3) << 11)   // R
                    | ((bgr[1] >> 2) << 5)  // G
                    | (bgr[0] >> 3);        // B
            }
            fileOffset += lineSize; // Advance one full line
        }
    } else if (infoHeader.bitsPerPixel == 32) {
        uint16_t maskByteW = (w+7) / 8;
        mask = static_cast<uint8_t*>(malloc(maskByteW * h * sizeof(uint8_t)));
        if (!mask) {
            // Probably not enough RAM for image (too large)
            free(data16);
            if (outErrmsg) *outErrmsg = "allocation failed";
            return Bitmap();
        }
        memset(mask, 0, maskByteW*h*sizeof(uint8_t));
        uint8_t bgra[4];
        for (uint16_t outY = h-1 ; outY != UINT16_MAX ; outY--) { // Assumes integer underflow, which IS well-defined
            if (!file.seek(fileOffset)) {
                if (outErrmsg) *outErrmsg = "failed to seek data";
                free(data16);
                return Bitmap();
            }
            for (uint16_t outX = 0 ; outX < w ; outX++) {
                numRead = file.read(bgra, 4);
                if (numRead != 4) {
                    if (outErrmsg) *outErrmsg = "premature end of data";
                    free(data16);
                    free(mask);
                    return Bitmap();
                }
                data16[outY*w + outX] =
                    ((bgra[2] >> 3) << 11)  // R
                    | ((bgra[1] >> 2) << 5) // G
                    | (bgra[0] >> 3);       // B
                if (bgra[3] > 127) {
                    mask[outY*maskByteW + (outX>>3)] |= (0x80 >> (outX&7));
                }
            }
            fileOffset += lineSize; // Advance one full line
        }
    }
    
    return Bitmap::takeOwnership(infoHeader.width, infoHeader.height, data16, mask);
}

Bitmap Bitmap::loadBMP (
    const char* path,
    uint16_t ox, uint16_t oy,
    uint16_t w, uint16_t h,
    const char** outErrmsg
) {
    File file(path);
    if (!file.open()) {
        if (outErrmsg) {
            *outErrmsg = "error opening file";
        }
        return Bitmap();
    }
    Bitmap bmp = loadBMP(file, ox, oy, w, h, outErrmsg);
    file.close();
    return bmp;
}

Bitmap Bitmap::scaled(int16_t factor) const
{
    if (!d) {
        return Bitmap();
    }
    
    if (factor == 0) {
        return Bitmap();
    } else if (factor == 1  ||  factor == -1) {
        return *this;
    }
    
    if (factor > 0) {
        // Scale up
        uint16_t ufactor = factor;
        uint16_t nw = d->w * ufactor;
        uint16_t nh = d->h * ufactor;
        uint16_t maskByteW = (d->w+7) / 8;
        uint16_t newMaskByteW = (nw+7) / 8;
        auto nd = static_cast<uint16_t*>(malloc(nw*nh*sizeof(uint16_t)));
        if (!nd) {
            return Bitmap();
        }
        auto nm = static_cast<uint8_t*>(d->m ? malloc(newMaskByteW*nh*sizeof(uint8_t)) : nullptr);
        if (nm) {
            memset(nm, 0, newMaskByteW*nh);
        } else if (d->m) {
            free(nd);
            return Bitmap();
        }
        for (uint16_t y = 0 ; y < d->h ; y++) {
            for (uint16_t x = 0 ; x < d->w ; x++) {
                uint16_t pix = d->d[y*d->w + x];
                bool msk = nm ? ((d->m[y*maskByteW + (x>>3)] << (x&7)) & 0x80) : false;
                
                for (uint16_t ny = y*ufactor ; ny < (y+1)*ufactor ; ny++) {
                    for (uint16_t nx = x*ufactor ; nx < (x+1)*ufactor ; nx++) {
                        nd[ny*nw + nx] = pix;
                        if (msk) {
                            nm[ny*newMaskByteW + (nx>>3)] |= (0x80 >> (nx&7));
                        }
                    }
                }
            }
        }
        return Bitmap::takeOwnership(nw, nh, nd, nm);
    } else {
        // Scale down
        uint16_t ufactor = -factor;
        // TODO: Implement
        return Bitmap();
    }
}

}

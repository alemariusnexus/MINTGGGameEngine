#include "Bitmap.h"


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


Bitmap Bitmap::loadBMP(File& file, const char** outErrmsg)
{
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    
    size_t numRead;
    numRead = file.read(reinterpret_cast<uint8_t*>(&fileHeader), sizeof(fileHeader));
    if (numRead != sizeof(fileHeader)) {
        if (outErrmsg) *outErrmsg = "premature end of file header";
        return Bitmap();
    }
    
    if (fileHeader.signature[0] != 'B'  ||  fileHeader.signature[1] != 'M') {
        if (outErrmsg) *outErrmsg = "invalid BMP signature";
        return Bitmap();
    }
    
    numRead = file.read(reinterpret_cast<uint8_t*>(&infoHeader), sizeof(infoHeader));
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
    
    if (infoHeader.width > 256  ||  infoHeader.height > 256) {
        if (outErrmsg) *outErrmsg = "image too large";
        return Bitmap();
    }
    
    if (!file.seek(fileHeader.dataOffset)) {
        if (outErrmsg) *outErrmsg = "failed to seek data";
        return Bitmap();
    }
    
    size_t dataSize = infoHeader.width*infoHeader.height*infoHeader.bitsPerPixel/8;
    
    uint16_t* data16 = new uint16_t[infoHeader.width*infoHeader.height];
    uint8_t* mask = nullptr;
    
    if (infoHeader.bitsPerPixel == 24) {
        // NOTE: The BMP files this was tested with store pixel data bottom-up,
        // with BGR order.
        uint8_t bgr[3];
        for (uint16_t y = 0 ; y < infoHeader.height ; y++) {
            for (uint16_t x = 0 ; x < infoHeader.width ; x++) {
                numRead = file.read(bgr, 3);
                if (numRead != 3) {
                    if (outErrmsg) *outErrmsg = "premature end of data";
                    delete[] data16;
                    return Bitmap();
                }
                data16[(infoHeader.height-y-1)*infoHeader.width + x] =
                    ((bgr[2] >> 3) << 11)   // R
                    | ((bgr[1] >> 2) << 5)  // G
                    | (bgr[0] >> 3);        // B
            }
            if ((infoHeader.width*3) % 4 != 0) {
                // Lines are padded to multipls of 4 bytes
                file.read(bgr, 4 - ((infoHeader.width*3) % 4));
            }
        }
    } else if (infoHeader.bitsPerPixel == 32) {
        // NOTE: The BMP files this was tested with store pixel data bottom-up,
        // with BGRA order.
        uint16_t maskByteW = (infoHeader.width+7) / 8;
        mask = new uint8_t[maskByteW*infoHeader.height];
        memset(mask, 0, maskByteW*infoHeader.height);
        uint8_t bgra[4];
        for (uint16_t y = 0 ; y < infoHeader.height ; y++) {
            for (uint16_t x = 0 ; x < infoHeader.width ; x++) {
                numRead = file.read(bgra, 4);
                if (numRead != 4) {
                    if (outErrmsg) *outErrmsg = "premature end of data";
                    delete[] data16;
                    delete[] mask;
                    return Bitmap();
                }
                data16[(infoHeader.height-y-1)*infoHeader.width + x] =
                    ((bgra[2] >> 3) << 11)  // R
                    | ((bgra[1] >> 2) << 5) // G
                    | (bgra[0] >> 3);       // B
                if (bgra[3] > 127) {
                    mask[(infoHeader.height-y-1)*maskByteW + (x>>3)] |= (0x80 >> (x&7));
                }
            }
        }
    }
    
    return Bitmap::takeOwnership(infoHeader.width, infoHeader.height, data16, mask);
}

Bitmap Bitmap::loadBMP(const char* file, const char** outErrmsg)
{
    File f = SD.open(file);
    if (!f) {
        if (outErrmsg) *outErrmsg = "failed to open file";
        return Bitmap();
    }
    Bitmap bitmap = Bitmap::loadBMP(f, outErrmsg);
    f.close();
    return bitmap;
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
        uint16_t* nd = new uint16_t[nw*nh];
        uint8_t* nm = d->m ? new uint8_t[newMaskByteW*nh] : nullptr;
        if (nm) {
            memset(nm, 0, newMaskByteW*nh);
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

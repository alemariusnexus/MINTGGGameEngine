#include "Bitmap.h"

#include <algorithm>

#include "../storage/BufferedReader.h"
#include "../util/Log.h"
#include "../util/Util.h"
#include "ImageLoader.h"


namespace MINTGGGameEngine
{


Bitmap Bitmap::loadBMP (
    const char* path,
    uint16_t ox, uint16_t oy,
    uint16_t w, uint16_t h,
    const char** outErrmsg
) {
    ImageLoader il;
    il.setOffset(ox, oy);
    il.setMaxSize(w, h);
    Bitmap bmp = il.loadBitmapBMP(path);
    if (!bmp) {
        if (outErrmsg) *outErrmsg = il.getErrorMessage();
    }
    return bmp;
}

Bitmap Bitmap::createPlaceholder(uint16_t w, uint16_t h)
{
    uint16_t* data = static_cast<uint16_t*>(malloc(w*h*sizeof(uint16_t)));

    const uint16_t evenColor = 0xF81F;
    const uint16_t oddColor = 0x07FF;

    for (uint16_t y = 0 ; y < h ; y++) {
        for (uint16_t x = 0 ; x < w ; x++) {
            data[y*w + x] = (y+x)%2 == 0 ? evenColor : oddColor;
        }
    }

    return takeOwnership(w, h, data);
}

size_t Bitmap::getMemoryUsage() const
{
    size_t memUsage = 0;
    if (d) {
        memUsage += sizeof(Data);
        if (d->d) {
            memUsage += d->w*d->h*sizeof(uint16_t);
        }
        if (d->m) {
            uint16_t maskByteW = (d->w+7)/8;
            memUsage += maskByteW*d->h*sizeof(uint8_t);
        }
    }
    return memUsage;
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

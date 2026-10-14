#include "Bitmap.h"

#include <algorithm>
#include <cmath>

#include "../platform/PlatformManager.h"
#include "../storage/BufferedReader.h"
#include "../util/Log.h"
#include "../util/Util.h"
#include "surface/BitmapDrawSurface.h"
#include "BitmapRenderer.h"
#include "ImageLoader.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <esp_heap_caps.h>
#endif

LOG_USE_TAG("Bitmap")


namespace MINTGGGameEngine
{

Bitmap Bitmap::fromStaticData (
    DrawSurface& surfaceForFormat,
    uint16_t w, uint16_t h,
    const void* d, const void* a,
    PixelFormat pixelFormat,
    AlphaStorage alphaStorage,
    ByteOrder byteOrder
) {
    Bitmap staticBmp = fromStaticData(w, h, d, a, pixelFormat, alphaStorage, byteOrder);

    if (    surfaceForFormat.getNativePixelFormat() != pixelFormat
        ||  surfaceForFormat.getNativeByteOrder() != byteOrder
    ) {
        // Format mismatch -> convert from static buffer
        staticBmp = staticBmp.convertFor(surfaceForFormat);
    }

    return staticBmp;
}

Bitmap Bitmap::loadBMP (
    const std::string_view& path,
    float scaleFactorX, float scaleFactorY,
    uint16_t ox, uint16_t oy,
    uint16_t w, uint16_t h,
    const char** outErrmsg
) {
    ImageLoader il;
    il.setOffset(ox, oy);
    il.setMaxSize(w, h);
    il.setScaleFactor(scaleFactorX, scaleFactorY);
    Bitmap bmp = il.loadBitmapBMP(path);
    if (!bmp) {
        if (outErrmsg) {
            *outErrmsg = il.getErrorMessage();
        } else {
            LogError("Error loading bitmap '%s': %s", path.data(), il.getErrorMessage());
        }
    }
    return bmp;
}

Bitmap Bitmap::loadBMP (
    DrawSurface& surfaceForFormat,
    const std::string_view& path,
    float scaleFactorX, float scaleFactorY,
    uint16_t ox, uint16_t oy,
    uint16_t w, uint16_t h,
    const char** outErrmsg
) {
    ImageLoader il(surfaceForFormat);
    il.setOffset(ox, oy);
    il.setMaxSize(w, h);
    il.setScaleFactor(scaleFactorX, scaleFactorY);
    Bitmap bmp = il.loadBitmapBMP(path);
    if (!bmp) {
        if (outErrmsg) {
            *outErrmsg = il.getErrorMessage();
        } else {
            LogError("Error loading bitmap '%s': %s", path.data(), il.getErrorMessage());
        }
    }
    return bmp;
}

Bitmap Bitmap::loadBMPAlphaOnly (
    const std::string_view& path,
    float scaleFactorX, float scaleFactorY,
    uint16_t ox, uint16_t oy,
    uint16_t w, uint16_t h,
    const char** outErrmsg
) {
    ImageLoader il;
    il.setOffset(ox, oy);
    il.setMaxSize(w, h);
    il.setScaleFactor(scaleFactorX, scaleFactorY);
    Bitmap bmp = il.loadBitmapBMPAlphaOnly(path);
    if (!bmp) {
        if (outErrmsg) {
            *outErrmsg = il.getErrorMessage();
        } else {
            LogError("Error loading bitmap '%s': %s", path.data(), il.getErrorMessage());
        }
    }
    return bmp;
}

Bitmap Bitmap::createPlaceholder(uint16_t w, uint16_t h)
{
    //uint16_t* data = static_cast<uint16_t*>(malloc(w*h*sizeof(uint16_t)));
    auto data = static_cast<uint16_t*>(allocDataBuffer(w, h, PixelFormat::RGB565));

    const uint16_t evenColor = 0xF81F;
    const uint16_t oddColor = 0x07FF;

    for (uint16_t y = 0 ; y < h ; y++) {
        for (uint16_t x = 0 ; x < w ; x++) {
            data[y*w + x] = (y+x)%2 == 0 ? evenColor : oddColor;
        }
    }

    return takeOwnership(w, h, data);
}

void* Bitmap::allocDataBuffer (
    uint16_t width, uint16_t height,
    uint8_t bytesPerPixel,
    uint32_t flags
) {
    const size_t size = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(bytesPerPixel);
    return PlatformManager::getInstance().allocDataBuffer (
        size,
        flags | PlatformManager::ALLOC_FLAG_BITMAP | PlatformManager::ALLOC_FLAG_UNALIGNED_ACCESS
        );
}


Bitmap::Bitmap (
        uint16_t w, uint16_t h,
        bool haveData, bool haveAlpha,
        PixelFormat pixelFormat,
        AlphaStorage alphaStorage,
        ByteOrder byteOrder
) : d(std::make_shared<Data>(
        w, h,
        haveData ? allocDataBuffer(w, h, pixelFormat) : nullptr,
        haveAlpha ? malloc(calcAlphaBytesPerLine(w)*h*sizeof(uint8_t)) : nullptr,
        pixelFormat, alphaStorage, byteOrder,
        true
        ))
{
    assert(alphaStorage == AlphaStorage::Bit1);
}

size_t Bitmap::getMemoryUsage() const
{
    size_t memUsage = 0;
    if (d) {
        memUsage += sizeof(Data);
        if (d->d) {
            memUsage += d->w*d->h*d->bytesPerPixel;
        }
        if (d->a) {
            assert(d->alphaStorage == AlphaStorage::Bit1);
            uint16_t maskByteW = (d->w+7)/8;
            memUsage += maskByteW*d->h*sizeof(uint8_t);
        }
    }
    return memUsage;
}

Color Bitmap::getPixel(uint16_t x, uint16_t y) const
{
    if (!d  ||  !d->d) {
        return {};
    }
    return Color::fromBuffer (
        getData(x, y),
        d->pixelFormat,
        d->byteOrder
        );
}

void Bitmap::setPixel(uint16_t x, uint16_t y, const Color& color)
{
    if (!d  ||  !d->d) return;
    color.toBuffer(getData(x, y), d->pixelFormat, d->byteOrder);
}

void Bitmap::fill(const Color& color)
{
    if (!d  ||  !d->d) return;
    BitmapDrawSurface surface(*this);
    surface.fill(color, false);
}

void Bitmap::fillAlpha(uint8_t alpha)
{
    if (!d  ||  !d->a) return;
    assert(d->alphaStorage == AlphaStorage::Bit1);
    const size_t alphaBufSize = calcAlphaBytesPerLine(d->w) * d->h * sizeof(uint8_t);
    if (alpha) {
        memset(d->a, 0xFF, alphaBufSize);
    } else {
        memset(d->a, 0x00, alphaBufSize);
    }
}

Bitmap Bitmap::transformSimple (
        PixelFormat toPixelFormat, AlphaStorage toAlphaStorage, ByteOrder toByteOrder,
        int16_t scaleX, int16_t scaleY,
        Rotation rotation,
        FlipDir mirror
) const {
    if (!d) {
        return {};
    }

    if (scaleX < 0  ||  scaleY < 0) {
        // Not supported yet
        return {};
    }

    Bitmap resultBmp (
        scaleX * d->w, scaleY * d->h,
        d->d ? true : false,
        d->a ? true : false,
        toPixelFormat, toAlphaStorage, toByteOrder
        );

    BitmapRenderer::Transfer transfer = {
        .src = {
            .buffer = d->d,
            .alpha = d->a,
            .width = d->w,
            .height = d->h,
            .blockOffsetX = 0,
            .blockOffsetY = 0,
            .blockWidth = d->w,
            .blockHeight = d->h,
            .pixelFormat = d->pixelFormat,
            .alphaStorage = d->alphaStorage,
            .byteOrder = d->byteOrder
        },

        .dest = {
            .buffer = resultBmp.getData(),
            .alpha = resultBmp.getAlpha(),
            .width = resultBmp.getWidth(),
            .height = resultBmp.getHeight(),
            .blockOffsetX = 0,
            .blockOffsetY = 0,
            .pixelFormat = toPixelFormat,
            .alphaStorage = toAlphaStorage,
            .byteOrder = toByteOrder
        },

        .blendMode = BitmapRenderer::BlendMode::Opaque,
        .alphaMode = BitmapRenderer::AlphaMode::CopySrcAlpha
    };

    BitmapRenderer::TransformSimple transform = {
        .scaleX = scaleX,
        .scaleY = scaleY,

        .rotation = rotation,

        .mirror = mirror
    };

    BitmapRenderer renderer;

    if (!renderer.checkValid(transfer, transform)) {
        LogError("Error transforming bitmap: %s", renderer.getErrorMessage());
        return {};
    }

    renderer.render(transfer, transform);

    return resultBmp;
}

Bitmap Bitmap::convertFormat(PixelFormat toPixelFormat, AlphaStorage toAlphaStorage, ByteOrder toByteOrder) const
{
    if (!d) {
        return {};
    }

    if (d->pixelFormat == toPixelFormat  &&  d->alphaStorage == toAlphaStorage  &&  d->byteOrder == toByteOrder) {
        // Same format -> nothing to do
        return *this;
    }

    if (d->pixelFormat == toPixelFormat  &&  d->alphaStorage == toAlphaStorage) {
        if (d->pixelFormat != PixelFormat::RGB565  &&  d->pixelFormat != PixelFormat::RGB565) {
            // Byte order doesn't apply -> nothing to do
            // TODO: We can't do this, because the new bitmap still has to have its byteOrder flag changed to the
            //  new one, even if it makes no technical difference. I don't think we can avoid at least a full data
            //  copy here. For now we'll actually do a transform.
            //return *this;
        }
    }

    return transformSimple (
        toPixelFormat, toAlphaStorage, toByteOrder,
        1, 1,
        Rotation::Rot0,
        FlipDir::None
        );
}

Bitmap Bitmap::convertFor(DrawSurface& surface) const
{
    if (!d) {
        return {};
    }
    return convertFormat(surface.getNativePixelFormat(), d->alphaStorage, surface.getNativeByteOrder());
}

Bitmap Bitmap::scaled(float scaleX, float scaleY) const
{
    if (!d) {
        return {};
    }

    const auto iscaleX = static_cast<int16_t>(std::round(scaleX));
    const auto iscaleY = static_cast<int16_t>(std::round(scaleY));

    if (iscaleX == 0  ||  iscaleY == 0) {
        return {};
    } else if (std::abs(iscaleX) == 1  &&  std::abs(iscaleY) == 1) {
        return *this;
    }

    return transformSimple (
        d->pixelFormat, d->alphaStorage, d->byteOrder,
        iscaleX, iscaleY,
        Rotation::Rot0,
        FlipDir::None
        );
}

Bitmap Bitmap::rotate(Rotation rotation) const
{
    if (!d) {
        return {};
    }

    if (rotation == Rotation::Rot0) {
        return *this;
    }

    return transformSimple (
        d->pixelFormat, d->alphaStorage, d->byteOrder,
        1, 1,
        rotation,
        FlipDir::None
        );
}

Bitmap Bitmap::mirror(FlipDir mirror) const
{
    if (!d) {
        return {};
    }

    if (mirror == FlipDir::None) {
        return *this;
    }

    return transformSimple (
        d->pixelFormat, d->alphaStorage, d->byteOrder,
        1, 1,
        Rotation::Rot0,
        mirror
        );
}

}

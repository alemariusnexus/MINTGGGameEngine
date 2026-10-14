/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "../Globals.h"
#include "../storage/File.h"

#include "../util/Vec2.h"
#include "../util/Log.h"
#include "../util/Util.h"
#include "Color.h"

#include <memory>


namespace MINTGGGameEngine
{

class DrawSurface;


/**
 * \brief Represents a two-dimensional rectangular array of color pixel values.
 *
 * This class currently stores colors in RGB565 format, from top to bottom. It
 * can also store a separate bit mask to define fully-transparent pixels (but
 * not partially-transparent ones).
 *
 * This class uses shared pointers, so copying is cheap. Note however that
 * bitmaps can take up a lot of RAM, which is a scarce resource on most
 * microcontrollers.
 */
class Bitmap
{
private:
    struct Data
    {
        Data (
            uint16_t w, uint16_t h,
            void* d, void* a,
            PixelFormat pixelFormat, AlphaStorage alphaStorage, ByteOrder byteOrder,
            bool own
            ) : w(w), h(h),
                d(d), a(a),
                pixelFormat(pixelFormat), alphaStorage(alphaStorage), byteOrder(byteOrder),
                bytesPerPixel(CalcPixelFormatBytesPerPixel(pixelFormat)),
                own(own)
        {
        }
        ~Data() { if (own) { free(d); free(a); } }
        
        uint16_t w;
        uint16_t h;

        void* d;
        void* a;

        PixelFormat pixelFormat;
        AlphaStorage alphaStorage;
        ByteOrder byteOrder;

        uint8_t bytesPerPixel;

        bool own;
    };

public:
    /**
     * \brief Create a bitmap from the given raw color and bitmask data,
     *      without copying.
     * 
     * The new bitmap takes ownership of all the data, and will delete it when
     * the bitmap itself is deleted. It is assumed that the data was created
     * using the C function malloc(). The data must remain valid throughout the
     * lifetime of the bitmap object (and all its shallow copies).
     *
     * \param w The width in pixels.
     * \param h The height in pixels.
     * \param d The raw color data.
     * \param m The bit mask, or null if none is used.
     * \return The new bitmap.
     */
    static Bitmap takeOwnership (
        uint16_t w, uint16_t h,
        void* d, void* a = nullptr,
        PixelFormat pixelFormat = PixelFormat::RGB565,
        AlphaStorage alphaStorage = AlphaStorage::Bit1,
        ByteOrder byteOrder = ByteOrder::Native
    ) { return {w, h, d, a, pixelFormat, alphaStorage, byteOrder, true}; }

    static Bitmap fromStaticData (
        uint16_t w, uint16_t h,
        const void* d, const void* a = nullptr,
        PixelFormat pixelFormat = PixelFormat::RGB565,
        AlphaStorage alphaStorage = AlphaStorage::Bit1,
        ByteOrder byteOrder = ByteOrder::Native
    ) { return {w, h, d, a, pixelFormat, alphaStorage, byteOrder}; }

    static Bitmap fromStaticData (
        DrawSurface& surfaceForFormat,
        uint16_t w, uint16_t h,
        const void* d, const void* a = nullptr,
        PixelFormat pixelFormat = PixelFormat::RGB565,
        AlphaStorage alphaStorage = AlphaStorage::Bit1,
        ByteOrder byteOrder = ByteOrder::Native
    );

    /**
     * \brief Create a bitmap by copying the given raw color and bitmask data.
     * 
     * A copy is made of all data, so the original data is not required to stay
     * valid after this call.
     *
     * \param w The width in pixels.
     * \param h The height in pixels.
     * \param d The raw color data.
     * \param m The bit mask, or null if none is used.
     * \return The new bitmap.
     */
    static Bitmap copyFrom (
        uint16_t w, uint16_t h,
        const void* d, const void* a = nullptr,
        PixelFormat pixelFormat = PixelFormat::RGB565,
        AlphaStorage alphaStorage = AlphaStorage::Bit1,
        ByteOrder byteOrder = ByteOrder::Native
    ) {
        auto cd = allocDataBuffer(w, h, pixelFormat);
        if (!cd) {
            return {};
        }
        const auto bytesPerPixel = CalcPixelFormatBytesPerPixel(pixelFormat);
        memcpy(cd, d, w*h*bytesPerPixel);
        uint8_t* cm = nullptr;
        if (a) {
            assert(alphaStorage == AlphaStorage::Bit1);
            uint16_t maskByteW = (w+7)/8;
            cm = static_cast<uint8_t*>(malloc(maskByteW*h*sizeof(uint8_t)));
            if (!cm) {
                free(cd);
                return {};
            }
            memcpy(cm, a, maskByteW*h);
        }
        return {w, h, cd, cm, pixelFormat, alphaStorage, byteOrder, true};
    }
    
    /**
     * \brief Create a bitmap from a BMP file at the given File object.
     *
     * \param in file The file path to load the BMP data from.
     * \param out outErrmsg Pointer to an error message, in case the loading
     *      fails. Can be NULL if no error message if required.
     * \return The loaded bitmap.
     * \see loadBMP(File&, const char**)
     */
    static Bitmap loadBMP (
        const std::string_view& path,
        float scaleFactorX = 1, float scaleFactorY = 1,
        uint16_t ox = 0, uint16_t oy = 0,
        uint16_t w = UINT16_MAX, uint16_t h = UINT16_MAX,
        const char** outErrmsg = nullptr
        );

    static Bitmap loadBMP (
        DrawSurface& surfaceForFormat,
        const std::string_view& path,
        float scaleFactorX = 1, float scaleFactorY = 1,
        uint16_t ox = 0, uint16_t oy = 0,
        uint16_t w = UINT16_MAX, uint16_t h = UINT16_MAX,
        const char** outErrmsg = nullptr
        );

    static Bitmap loadBMPAlphaOnly (
        const std::string_view& path,
        float scaleFactor = 1, float scaleFactorY = 1,
        uint16_t ox = 0, uint16_t oy = 0,
        uint16_t w = UINT16_MAX, uint16_t h = UINT16_MAX,
        const char** outErrmsg = nullptr
        );

    /**
     * \brief Create a simple placeholder bitmap of the given size.
     *
     * @param w The width in pixels.
     * @param h The height in pixels.
     * @return The placeholder bitmap.
     */
    static Bitmap createPlaceholder(uint16_t w, uint16_t h);

    /**
     * \brief Calculate number of bytes used for a single line in the mask.
     *
     * @param w The width in pixels.
     * @return Number of bytes per line, including padding.
     */
    static size_t calcAlphaBytesPerLine(uint16_t w) { return (w+7)/8; }

    static void* allocDataBuffer (
        uint16_t width, uint16_t height,
        uint8_t bytesPerPixel,
        uint32_t flags = 0
        );

    static void* allocDataBuffer (
        uint16_t width, uint16_t height,
        PixelFormat pixelFormat,
        uint32_t flags = 0
        )
        { return allocDataBuffer(width, height, CalcPixelFormatBytesPerPixel(pixelFormat), flags); }
    
public:
    /**
     * \brief Create a bitmap from the given raw color and alpha data,
     *      without copying.
     *
     * The raw data is not copied, and no ownership is taken. The data must
     * remain valid for the entire lifetime of the Bitmap (and its shallow
     * copies). This constructor is primarily useful for creating bitmaps from
     * global static data.
     *
     * \param w The width in pixels.
     * \param h The height in pixels.
     * \param d The raw RGB565 data.
     * \param m The bit mask, or null if none is used.
     * \see takeOwnership()
     * \see copyFrom()
     * \see loadBMP()
     */
    Bitmap (
        uint16_t w, uint16_t h,
        const void* d, const void* a = nullptr,
        PixelFormat pixelFormat = PixelFormat::RGB565,
        AlphaStorage alphaStorage = AlphaStorage::Bit1,
        ByteOrder byteOrder = ByteOrder::Native
        ) : d(std::make_shared<Data>(
            w, h,
            const_cast<void*>(d), const_cast<void*>(a),
            pixelFormat, alphaStorage, byteOrder,
            false
            )) {}

    /**
     * \brief Create a bitmap with uninitialized data, to be filled later.
     *
     * A data and mask buffer with the correct size are allocated automatically if requested.
     * Note that neither buffers are initialized, so the initial data is undefined!
     *
     * @param w The width in pixels.
     * @param h The height in pixels.
     * @param haveData true to create the bitmap with color data (RGB565), false otherwise.
     * @param haveAlpha true to create the bitmap with alpha data, false otherwise.
     */
    Bitmap (
        uint16_t w, uint16_t h,
        bool haveData = true, bool haveAlpha = false,
        PixelFormat pixelFormat = PixelFormat::RGB565,
        AlphaStorage alphaStorage = AlphaStorage::Bit1,
        ByteOrder byteOrder = ByteOrder::Native
        );
    
    /**
     * \brief Create an invalid bitmap.
     */
    Bitmap() : d() {}
    
    /**
     * \brief Copy constructor, creating a shallow copy (reference sharing).
     */
    Bitmap(const Bitmap& o) : d(o.d) {}
    
    
    /// \name Bitmap Properties
    ///@{
    
    /**
     * \brief Return the bitmap width, in pixels.
     */
    uint16_t getWidth() const { return d ? d->w : 0; }
    
    /**
     * \brief Return the bitmap height, in pixels.
     */
    uint16_t getHeight() const { return d ? d->h : 0; }

    Vec2 getSize() const { return d ? Vec2(d->w, d->h) : Vec2(); }
    
    /**
     * \brief Return the raw data buffer.
     */
    const void* getData() const { return d ? d->d : nullptr; }
    void* getData() { return d ? d->d : nullptr; }

    const void* getData(uint16_t x, uint16_t y) const
            { return d && d->d ? reinterpret_cast<const uint8_t*>(d->d) + (y*d->w + x)*d->bytesPerPixel : nullptr; }
    void* getData(uint16_t x, uint16_t y) { return const_cast<void*>(const_cast<const Bitmap*>(this)->getData()); }
    
    /**
     * \brief Return the raw bit mask.
     */
    const void* getAlpha() const { return d ? d->a : nullptr; }
    void* getAlpha() { return d ? d->a : nullptr; }

    PixelFormat getPixelFormat() const { return d ? d->pixelFormat : PixelFormat::RGB565; }

    AlphaStorage getAlphaStorage() const { return d ? d->alphaStorage : AlphaStorage::Bit1; }

    ByteOrder getByteOrder() const { return d ? d->byteOrder : ByteOrder::Native; }

    /**
     * \brief Estimate number of bytes in RAM used by the bitmap.
     *
     * @return Estimate byte size.
     */
    size_t getMemoryUsage() const;
    
    ///@}


    /// \name Pixel Access
    ///@{

    /**
     * \brief Returns the color of a single pixel.
     *
     * @param x x coordinate.
     * @param y y coordinate.
     * @return The pixel's color.
     */
    Color getPixel(uint16_t x, uint16_t y) const;

    /**
     * \brief Returns the mask value of a single pixel.
     *
     * @param x x coordinate.
     * @param y y coordinate.
     * @return true if the mask is set, false otherwise.
     */
    uint8_t getPixelAlpha(uint16_t x, uint16_t y) const
    {
        if (!d  ||  !d->a) return 0;
        switch (d->alphaStorage) {
        case AlphaStorage::Bit1: {
            uint16_t maskByteW = (d->w+7)/8;
            return (reinterpret_cast<uint8_t*>(d->a)[y*maskByteW + (x>>3)] & (0x80 >> (x&7))) != 0
                ? 0xFF : 0x00;
        }
        default:
            assert(false);
            return 0x00;
        }
    }

    /**
     * \brief Set the color of a single pixel.
     *
     * @param x x coordinate.
     * @param y y coordinate.
     * @param color Pixel color.
     */
    void setPixel(uint16_t x, uint16_t y, const Color& color);

    /**
     * \brief Set the mask value of a single pixel.
     *
     * @param x x coordinate.
     * @param y y coordinate.
     * @param set true to set the mask, false to clear it.
     */
    void setPixelAlpha(uint16_t x, uint16_t y, uint8_t alpha) const
    {
        if (!d  ||  !d->a) return;
        switch (d->alphaStorage) {
        case AlphaStorage::Bit1: {
            uint16_t maskByteW = (d->w+7)/8;
            if (alpha) {
                reinterpret_cast<uint8_t*>(d->a)[y*maskByteW + (x>>3)] |= (0x80 >> (x&7));
            } else {
                reinterpret_cast<uint8_t*>(d->a)[y*maskByteW + (x>>3)] &= ~(0x80 >> (x&7));
            }
            break;
        }
        default:
            assert(false);
        }
    }

    ///@}
    
    
    /// \name Bitmap Manipulation
    ///@{

    void fill(const Color& color);

    void fillAlpha(uint8_t alpha);

    Bitmap transformSimple (
        PixelFormat toPixelFormat, AlphaStorage toAlphaStorage, ByteOrder toByteOrder,
        int16_t scaleX, int16_t scaleY,
        Rotation rotation,
        FlipDir mirror
        ) const;

    Bitmap convertFormat(PixelFormat toPixelFormat, AlphaStorage toAlphaStorage, ByteOrder toByteOrder) const;

    Bitmap convertFor(DrawSurface& surface) const;
    
    /**
     * \brief Return a scaled-up or scaled-down version of this bitmap.
     *
     * A scale factor of 1 or -1 yields means no changes in scale. Higher positive values scale up the image by that
     * factor. Negative values scale down the image by the absolute value of the factor.
     *
     * Currently, scaling is only supported by integer multiples, and downscaling
     * isn't supported at all. Fractional values will be rounded to the nearest
     * integer. This may change in the future.
     *
     * \param scaleX Scale along the x axis.
     * \param scaleY Scale along the y axis.
     * \return The scaled bitmap.
     */
    Bitmap scaled(float scaleX, float scaleY) const;

    Bitmap rotate(Rotation rotation) const;

    Bitmap mirror(FlipDir mirror) const;
    
    ///@}
    
    
    /// \name Operators
    ///@{

    Bitmap& operator=(const Bitmap& other) { d = other.d; return *this; }

    /**
     * \brief Check if the bitmap is valid.
     */
    operator bool() const { return (bool) d; }
    
    bool operator==(const Bitmap& other) const { return d == other.d; }
    bool operator!=(const Bitmap& other) const { return d != other.d; }
    
    ///@}

private:
    Bitmap (
        uint16_t w, uint16_t h,
        void* d, void* m,
        PixelFormat pixelFormat, AlphaStorage alphaStorage, ByteOrder byteOrder,
        bool own)
        : d(std::make_shared<Data>(
            w, h,
            d, m,
            pixelFormat, alphaStorage, byteOrder,
            own)) {}

private:
    std::shared_ptr<Data> d;
};

}

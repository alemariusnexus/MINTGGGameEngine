#pragma once

#include "../Globals.h"
#include "../storage/File.h"

#include "../util/Log.h"
#include "Color.h"

#include <memory>


namespace MINTGGGameEngine
{

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
        Data(uint16_t w, uint16_t h, uint16_t* d, uint8_t* m, bool own) : w(w), h(h), d(d), m(m), own(own) {}
        ~Data() { if (own) { free(d); free(m); } }
        
        uint16_t w;
        uint16_t h;
        uint16_t* d;
        uint8_t* m;
        bool own;
    };

public:
    /**
     * \brief Create a bitmap from the given raw RGB565 and bitmask data,
     *      without copying.
     * 
     * The new bitmap takes ownership of all the data, and will delete it when
     * the bitmap itself is deleted. It is assumed that the data was created
     * using the C function malloc(). The data must remain valid throughout the
     * lifetime of the bitmap object (and all its shallow copies).
     *
     * \param w The width in pixels.
     * \param h The height in pixels.
     * \param d The raw RGB565 data.
     * \param m The bit mask, or null if none is used.
     * \return The new bitmap.
     */
    static Bitmap takeOwnership(uint16_t w, uint16_t h, uint16_t* d, uint8_t* m = nullptr)
            { return Bitmap(w, h, d, m, true); }

    /**
     * \brief Create a bitmap by copying the given raw RGB565 and bitmask data.
     * 
     * A copy is made of all data, so the original data is not required to stay
     * valid after this call.
     *
     * \param w The width in pixels.
     * \param h The height in pixels.
     * \param d The raw RGB565 data.
     * \param m The bit mask, or null if none is used.
     * \return The new bitmap.
     */
    static Bitmap copyFrom(uint16_t w, uint16_t h, const uint16_t* d, const uint8_t* m = nullptr)
    {
        auto cd = static_cast<uint16_t*>(malloc(w*h*sizeof(uint16_t)));
        if (!cd) {
            return Bitmap();
        }
        uint8_t* cm = nullptr;
        memcpy(cd, d, w*h*sizeof(uint16_t));
        if (m) {
            uint16_t maskByteW = (w+7)/8;
            cm = static_cast<uint8_t*>(malloc(maskByteW*h*sizeof(uint8_t)));
            if (!cm) {
                free(cd);
                return Bitmap();
            }
            memcpy(cm, m, maskByteW*h);
        }
        return Bitmap(w, h, cd, cm, true);
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
        const char* path,
        uint16_t ox = 0, uint16_t oy = 0,
        uint16_t w = UINT16_MAX, uint16_t h = UINT16_MAX,
        const char** outErrmsg = nullptr
        );

    static size_t calcMaskBytesPerLine(uint16_t w) { return (w+7)/8; }
    
public:
    /**
     * \brief Create a bitmap from the given raw RGB565 and bitmask data,
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
    Bitmap(uint16_t w, uint16_t h, const uint16_t* d, const uint8_t* m = nullptr)
            : d(std::make_shared<Data>(w, h, const_cast<uint16_t*>(d), const_cast<uint8_t*>(m), false)) {}

    /**
     * \brief Create a bitmap with uninitialized data, to be filled later.
     *
     * A data and mask buffer with the correct size are allocated automatically if requested.
     * Note that neither buffers are initialized, so the initial data is undefined!
     *
     * @param w The width in pixels.
     * @param h The height in pixels.
     * @param haveData true to create the bitmap with color data (RGB565), false otherwise.
     * @param haveMask true to create the bitmap with mask data, false otherwise.
     */
    Bitmap(uint16_t w, uint16_t h, bool haveData = true, bool haveMask = false)
            : d(std::make_shared<Data>(w, h,
                static_cast<uint16_t*>(malloc(w*h*sizeof(uint16_t))),
                static_cast<uint8_t*>(malloc(calcMaskBytesPerLine(w)*h*sizeof(uint8_t))),
                true
                )) {}
    
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
    
    /**
     * \brief Return the raw RGB565 data.
     */
    const uint16_t* getData() const { return d ? d->d : nullptr; }
    
    /**
     * \brief Return the raw bit mask.
     */
    const uint8_t* getMask() const { return d ? d->m : nullptr; }

    size_t getMemoryUsage() const;
    
    ///@}


    /// \name Pixel Access
    ///@{

    uint16_t getPixelRaw(uint16_t x, uint16_t y) const
            { return d  &&  d->d ? d->d[y*d->w+x] : 0; }

    Color getPixel(uint16_t x, uint16_t y) const { return Color(getPixelRaw(x, y)); }

    bool getMaskPixel(uint16_t x, uint16_t y) const
    {
        if (!d  ||  !d->m) return false;
        uint16_t maskByteW = (d->w+7)/8;
        return (d->m[y*maskByteW + (x>>3)] & (0x80 >> (x&7))) != 0;
    }

    void setPixel(uint16_t x, uint16_t y, const Color& color)
    {
        if (!d  ||  !d->d) return;
        d->d[x*d->w+x] = color.toRGB565();
    }

    void setMaskPixel(uint16_t x, uint16_t y, bool set) const
    {
        if (!d  ||  !d->m) return;
        uint16_t maskByteW = (d->w+7)/8;
        if (set) {
            d->m[y*maskByteW + (x>>3)] |= (0x80 >> (x&7));
        } else {
            d->m[y*maskByteW + (x>>3)] &= ~(0x80 >> (x&7));
        }
        //return (d->m[y*maskByteW + (x>>3)] & (0x80 >> (x&7))) != 0;
    }

    ///@}
    
    
    /// \name Bitmap Manipulation
    ///@{
    
    /**
     * \brief Return a scaled-up or scaled-down version of this bitmap.
     *
     * Only scaling by integer multiples is supported. Currently, downscaling
     * is not supported at all and will simply return the original bitmap
     * unchanged.
     *
     * \param factor The scale factor. A factor of 1 or -1 yields the original
     *      image. Higher values scale up the image. Negative values scale down
     *      the image by the absolute value of the factor.
     * \return The scaled bitmap.
     */
    Bitmap scaled(int16_t factor) const;
    
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
    Bitmap(uint16_t w, uint16_t h, uint16_t* d, uint8_t* m, bool own)
        : d(std::make_shared<Data>(w, h, d, m, own)) {}

private:
    std::shared_ptr<Data> d;
};

}

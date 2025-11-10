#pragma once

#include "Globals.h"

#include <memory>

#include <SD.h>


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
        Data(uint16_t w, uint16_t h, const uint16_t* d, const uint8_t* m, bool own) : w(w), h(h), d(d), m(m), own(own) {}
        ~Data() { if (own) { delete[] d; delete[] m; } }
        
        uint16_t w;
        uint16_t h;
        const uint16_t* d;
        const uint8_t* m;
        bool own;
    };

public:
    /**
     * \brief Create a bitmap from the given raw RGB565 and bitmask data,
     *      without copying.
     * 
     * The new bitmap takes ownership of all the data, and will delete it when
     * the bitmap itself is deleted. It is assumed that the data was created
     * using the C++ operator new[]. The data must remain valid throughout the
     * lifetime of the bitmap object (and all its shallow copies).
     *
     * \param w The width in pixels.
     * \param h The height in pixels.
     * \param d The raw RGB565 data.
     * \param m The bit mask, or null if none is used.
     * \return The new bitmap.
     */
    static Bitmap takeOwnership(uint16_t w, uint16_t h, const uint16_t* d, const uint8_t* m = nullptr)
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
        uint16_t* cd = new uint16_t[w*h];
        uint8_t* cm = nullptr;
        memcpy(cd, d, w*h*sizeof(uint16_t));
        if (m) {
            uint16_t maskByteW = (w+7)/8;
            cm = new uint8_t[maskByteW*h];
            memcpy(cm, m, maskByteW*h);
        }
        return Bitmap(w, h, cd, cm, true);
    }
    
    
    /**
     * \brief Create a bitmap from a BMP file at the given File object.
     *
     * Note that only a very limited subset of BMP images is supported.
     * Currently, only BGR888 and BGRA8888 formats can be loaded, even though
     * the resulting bitmap will be in RGB565 format.
     * This was tested on images exported by GIMP 2.10.32.
     *
     * \param in file The file to load the BMP data from.
     * \param out outErrmsg Pointer to an error message, in case the loading
     *      fails. Can be NULL if no error message if required.
     * \return The loaded bitmap.
     */
    static Bitmap loadBMP(File& file, const char** outErrmsg = nullptr);
    
    /**
     * \brief Create a bitmap from a BMP file at the given File object.
     *
     * \param in file The file path to load the BMP data from.
     * \param out outErrmsg Pointer to an error message, in case the loading
     *      fails. Can be NULL if no error message if required.
     * \return The loaded bitmap.
     * \see loadBMP(File&, const char**)
     */
    static Bitmap loadBMP(const char* file, const char** outErrmsg = nullptr);
    
public:
    /**
     * \brief Create a bitmap from the given raw RGB565 and bitmask data,
     *      without copying.
     *
     * This is equivalent to calling takeOwnership().
     *
     * \see takeOwnership().
     */
    Bitmap(uint16_t w, uint16_t h, const uint16_t* d, const uint8_t* m = nullptr)
            : d(std::make_shared<Data>(w, h, d, m, false)) {}
    
    /**
     * \brief Create an invalid bitmap.
     */
    Bitmap() : d() {}
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
    Bitmap(uint16_t w, uint16_t h, const uint16_t* d, const uint8_t* m, bool own)
        : d(std::make_shared<Data>(w, h, d, m, own)) {}

private:
    std::shared_ptr<Data> d;
};

}

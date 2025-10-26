#pragma once

#include "Globals.h"

#include <memory>

#include <SD.h>


namespace MINTGGGameEngine
{

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
    static Bitmap takeOwnership(uint16_t w, uint16_t h, const uint16_t* d, const uint8_t* m = nullptr)
            { return Bitmap(w, h, d, m, true); }
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
    static Bitmap loadBMP(File& file, const char** outErrmsg = nullptr);
    static Bitmap loadBMP(const char* file, const char** outErrmsg = nullptr);
    
public:
    Bitmap(uint16_t w, uint16_t h, const uint16_t* d, const uint8_t* m = nullptr)
            : d(std::make_shared<Data>(w, h, d, m, false)) {}
    Bitmap() : d() {}
    Bitmap(const Bitmap& o) : d(o.d) {}
    
    uint16_t getWidth() const { return d ? d->w : 0; }
    uint16_t getHeight() const { return d ? d->h : 0; }
    const uint16_t* getData() const { return d ? d->d : nullptr; }
    const uint8_t* getMask() const { return d ? d->m : nullptr; }
    
    Bitmap scaled(int16_t factor) const;

    Bitmap& operator=(const Bitmap& other) { d = other.d; return *this; }

    operator bool() const { return (bool) d; }
    
    bool operator==(const Bitmap& other) const { return d == other.d; }
    bool operator!=(const Bitmap& other) const { return d != other.d; }

private:
    Bitmap(uint16_t w, uint16_t h, const uint16_t* d, const uint8_t* m, bool own)
        : d(std::make_shared<Data>(w, h, d, m, own)) {}

private:
    std::shared_ptr<Data> d;
};

}

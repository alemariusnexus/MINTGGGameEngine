#pragma once

#include "../Globals.h"

#include "File.h"


namespace MINTGGGameEngine
{


class Reader
{
public:
    virtual size_t read(void* out, size_t size) = 0;
    virtual size_t skip(size_t size) = 0;
    virtual bool seek(ssize_t offset, File::SeekMode mode = File::SeekSet) = 0;
    virtual ssize_t tell() = 0;
};


}
#pragma once

#include "../Globals.h"

#include "Reader.h"


namespace MINTGGGameEngine
{


class MemReader : public Reader
{
public:
    MemReader(const void* buf, size_t bufSize);
    virtual ~MemReader();

    size_t read(void* out, size_t size) override;
    size_t skip(size_t size) override;
    bool seek(ssize_t offset, File::SeekMode mode = File::SeekSet) override;
    ssize_t tell() override;

private:
    const uint8_t* buf;
    size_t bufSize;
    size_t offset;
};


}
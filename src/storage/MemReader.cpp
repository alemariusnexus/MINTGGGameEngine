#include "MemReader.h"

#include <algorithm>
#include <cassert>


namespace MINTGGGameEngine
{


MemReader::MemReader(const void* buf, size_t bufSize)
    : buf(reinterpret_cast<const uint8_t*>(buf)), bufSize(bufSize), offset(0)
{
}

MemReader::~MemReader()
{
}

size_t MemReader::read(void* out, size_t size)
{
    size = std::min(size, bufSize-offset);
    memcpy(out, buf+offset, size);
    offset += size;
    return size;
}

size_t MemReader::skip(size_t size)
{
    size = std::min(size, bufSize-offset);
    offset += size;
    return size;
}

bool MemReader::seek(ssize_t offset, File::SeekMode mode)
{
    if (mode == File::SeekSet) {
        if (offset < 0  ||  offset > bufSize) {
            return false;
        }
        this->offset = static_cast<size_t>(offset);
        return true;
    } else if (mode == File::SeekCur) {
        return seek(this->offset + offset, File::SeekSet);
    } else {
        assert(false);
        return false;
    }
}

ssize_t MemReader::tell()
{
    return static_cast<ssize_t>(offset);
}


}

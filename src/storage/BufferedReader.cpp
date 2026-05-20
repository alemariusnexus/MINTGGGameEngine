#include "BufferedReader.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>

#include "util/Log.h"


namespace MINTGGGameEngine
{


BufferedReader::BufferedReader(Reader& reader, void* buf, size_t bufSize)
    : reader(&reader), buf(reinterpret_cast<uint8_t*>(buf)),
      bufSize(bufSize), bufOwned(false), bufOffset(0), bufNumLeft(0)
{
}

BufferedReader::BufferedReader(Reader& reader, size_t bufSize)
    : reader(&reader), buf(bufSize != 0 ? static_cast<uint8_t*>(malloc(bufSize)) : nullptr),
      bufSize(bufSize), bufOwned(true), bufOffset(0), bufNumLeft(0)
{
}

BufferedReader::BufferedReader(Reader& reader)
    : BufferedReader(reader, 128)
{
}

BufferedReader::~BufferedReader()
{
    if (bufOwned) {
        free(buf);
    }
}

size_t BufferedReader::read(void* out, size_t size)
{
    if (!out  ||  size == 0) {
        return 0;
    }

    uint8_t* out8 = reinterpret_cast<uint8_t*>(out);

    if (!this->buf) {
        return readDirect(out8, size);
    }

    // First: Read as much as possible from buffer
    size_t numRead = readFromBuffer(out8, size);
    out8 += numRead;
    size -= numRead;

    // Read full blocks directly into output
    if (size >= this->bufSize) {
        size_t blockReadSize = (size / this->bufSize) * this->bufSize;
        size_t numReadDirect = readDirect(out8, blockReadSize);
        numRead += numReadDirect;
        if (numReadDirect < blockReadSize) {
            return numRead;
        }
        out8 += numReadDirect;
        size -= numReadDirect;
    }

    // Read last part into buffer
    if (size != 0  &&  size <= this->bufSize) {
        numRead += readThroughBuffer(out8, size);
    }

    return numRead;
}

size_t BufferedReader::skip(size_t size)
{
    if (size == 0) {
        return 0;
    }

    if (!this->buf) {
        return reader->skip(size);
    }

    size_t numSkipped;

    if (size < this->bufNumLeft) {
        // Skip only part of buffer
        numSkipped = size;
        this->bufOffset += size;
        this->bufNumLeft -= size;
    } else if (size == this->bufNumLeft) {
        // Skip exactly the buffer
        numSkipped = size;
        this->bufOffset = 0;
        this->bufNumLeft = 0;
    } else {
        // Skip the buffer, and more data from the file
        numSkipped = this->bufNumLeft;
        this->bufOffset = 0;
        this->bufNumLeft = 0;

        numSkipped += reader->skip(size-numSkipped);
    }

    return numSkipped;
}

bool BufferedReader::seek(ssize_t offset, File::SeekMode mode)
{
    if (mode == File::SeekSet) {
        this->bufOffset = 0;
        this->bufNumLeft = 0;
        return reader->seek(offset, File::SeekSet);
    } else if (mode == File::SeekCur) {
        if (offset >= 0  &&  offset <= this->bufNumLeft) {
            // Seek destination within buffer
            if (offset == this->bufNumLeft) {
                // Skip exactly the buffer
                this->bufOffset = 0;
                this->bufNumLeft = 0;
            } else {
                // Skip part of the buffer
                this->bufOffset += offset;
                this->bufNumLeft -= offset;
            }
            return true;
        } else {
            // Seek destination not within buffer -> drop buffer and seek normally
            bool seekOk = reader->seek(offset-this->bufNumLeft, File::SeekCur);
            this->bufOffset = 0;
            this->bufNumLeft = 0;
            return seekOk;
        }
    }
    return false;
}

ssize_t BufferedReader::tell()
{
    return reader->tell() - this->bufNumLeft;
}

size_t BufferedReader::readFromBuffer(uint8_t* out, size_t size)
{
    size_t numRead;
    if (this->buf) {
        if (size < this->bufNumLeft) {
            // Buffer contains more than was requested -> read part of it
            numRead = size;
            memcpy(out, this->buf+this->bufOffset, numRead);
            this->bufOffset += size;
            this->bufNumLeft -= size;
        } else {
            // Entire buffer is requested (and potentially more) -> copy all and reset buffer
            numRead = this->bufNumLeft;
            memcpy(out, this->buf+this->bufOffset, numRead);
            this->bufOffset = 0;
            this->bufNumLeft = 0;
        }
    } else {
        numRead = 0;
    }
    return numRead;
}

size_t BufferedReader::readDirect(uint8_t* out, size_t size)
{
    size_t numRead = 0;
    size_t numReadSingle;
    do {
        numReadSingle = reader->read(out, size);
        out += numReadSingle;
        size -= numReadSingle;
        numRead += numReadSingle;
    } while (size != 0  &&  numReadSingle != 0);

    return numRead;
}

size_t BufferedReader::readThroughBuffer(uint8_t* out, size_t size)
{
    // For larger sizes, readDirect() should have been used first!
    assert(size <= this->bufSize);

    // Buffer should have been emptied first
    assert(this->bufOffset == 0);
    assert(this->bufNumLeft == 0);

    size_t numRead = readDirect(this->buf, this->bufSize);

    if (numRead <= size) {
        // All data read goes immediately to the output (e.g. at end of file)
        memcpy(out, this->buf, numRead);
    } else {
        // Only first part goes into the output, rest is read-ahead
        memcpy(out, this->buf, size);
        this->bufOffset += size;
        this->bufNumLeft = numRead-size;
        numRead = size;
    }

    return numRead;
}


}

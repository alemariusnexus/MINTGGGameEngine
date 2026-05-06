#pragma once

#include "../Globals.h"

#include <string>

#include "File.h"


namespace MINTGGGameEngine
{


class BufferedReader
{
public:
    BufferedReader(const File& file, void* buf, size_t bufSize);
    BufferedReader(const char* path, void* buf, size_t bufSize);
    BufferedReader(const File& file, size_t bufSize);
    BufferedReader(const char* path, size_t bufSize);
    BufferedReader(const File& file);
    BufferedReader(const char* path);
    ~BufferedReader();

    bool open(const char** outErrmsg = nullptr);
    void close();
    bool isOpen() const;

    size_t read(void* out, size_t size);
    size_t skip(size_t size);
    bool seek(ssize_t offset, File::SeekMode mode = File::SeekSet);
    ssize_t tell();

private:
    size_t readFromBuffer(uint8_t* out, size_t size);
    size_t readDirect(uint8_t* out, size_t size);
    size_t readThroughBuffer(uint8_t* out, size_t size);

private:
    File file;

    uint8_t* buf;
    size_t bufSize;
    bool bufOwned;

    size_t bufOffset;
    size_t bufNumLeft;
};


}

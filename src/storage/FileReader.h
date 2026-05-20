#pragma once

#include "../Globals.h"

#include <string>

#include "File.h"
#include "Reader.h"


namespace MINTGGGameEngine
{


class FileReader : public Reader
{
public:
    FileReader(const File& file);
    FileReader(const std::string_view& path);
    virtual ~FileReader();

    bool open(const char** outErrmsg = nullptr);
    void close();
    bool isOpen() const;

    size_t read(void* out, size_t size) override;
    size_t skip(size_t size) override;
    bool seek(ssize_t offset, File::SeekMode mode = File::SeekSet) override;
    ssize_t tell() override;

private:
    File file;
};


}
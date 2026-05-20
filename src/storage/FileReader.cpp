#include "FileReader.h"


namespace MINTGGGameEngine
{


FileReader::FileReader(const File& file)
    : file(file)
{
}

FileReader::FileReader(const std::string_view& path)
    : file(File(path))
{
}

FileReader::~FileReader()
{
    close();
}

bool FileReader::open(const char** outErrmsg)
{
    return file.open(File::ReadOnly, outErrmsg);
}

void FileReader::close()
{
    file.close();
}

bool FileReader::isOpen() const
{
    return file.isOpen();
}

size_t FileReader::read(void* out, size_t size)
{
    if (!file.isOpen()) {
        return 0;
    }
    return file.read(out, size);
}

size_t FileReader::skip(size_t size)
{
    if (!file.isOpen()) {
        return 0;
    }
    return file.skip(size);
}

bool FileReader::seek(ssize_t offset, File::SeekMode mode)
{
    if (!file.isOpen()) {
        return false;
    }
    return file.seek(offset, mode);
}

ssize_t FileReader::tell()
{
    if (!file.isOpen()) {
        return -1;
    }
    return file.tell();
}


}

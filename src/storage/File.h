#pragma once

#include "../Globals.h"

#include <cstdlib>
#include <string>
#include <vector>

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
#include <SD.h>
#endif


namespace MINTGGGameEngine
{


class File
{
public:
    enum OpenMode
    {
        ReadOnly,
        WriteOnly,
        AppendOnly
    };

public:
    File(const File& other);
    File(const std::string& path);
    File(const File& parent, const std::string& child);
    ~File();

    std::string getPath() const { return path; }

    bool exists() const;
    bool isDirectory() const;
    bool isRegularFile() const;
    size_t getSize() const;

    std::vector<File> listChildren(bool recursive = false) const;

    bool mkdir();
    bool remove();

    bool open(OpenMode mode = ReadOnly);
    void close();
    bool isOpen() const;

    bool flush();
    size_t read(void* data, size_t size);
    size_t write(const void* data, size_t size);
    int printf(const char* format, ...);
    bool seek(ssize_t offset);
    ssize_t tell();

    std::string getTextContent();
    bool setTextContent(const std::string& content);

private:
    void normalizePath();

    bool listChildren(std::vector<File>& res, bool recursive) const;

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    bool openSDFile(::File* outFile) const;
#endif

private:
    std::string path;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    FILE* fhandle;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    ::File fhandle;
#endif
};


}

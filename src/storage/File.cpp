#include "File.h"

#ifndef MINTGGGAMEENGINE_PORT_ARDUINO
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#include <cstdarg>
#include <cstdio>


namespace MINTGGGameEngine
{


File::File(const File& other)
    : path(other.path)
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
      , fhandle(nullptr)
#endif
{
    normalizePath();
}

File::File(const std::string& path)
    : path(path)
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
      , fhandle(nullptr)
#endif
{
}

File::File(const File& parent, const std::string& child)
    : path(parent.getPath().append("/").append(child))
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
      , fhandle(nullptr)
#endif
{
    normalizePath();
}

File::~File()
{
    close();
}

void File::normalizePath()
{
    if (path.ends_with("/")) {
        path.pop_back();
    }
}

bool File::exists() const
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct stat st;
    return stat(path.c_str(), &st) == 0;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    return SD.exists(path.c_str());
#else
    return false;
#endif
}

bool File::isDirectory() const
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    File f = SD.open(path.c_str());
    if (!f) {
        return false;
    }
    bool isdir = f.isDirectory();
    f.close();
    return isdir;
#else
    return false;
#endif
}

bool File::isRegularFile() const
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode);
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    return !isDirectory();
#else
    return false;
#endif
}

size_t File::getSize() const
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return 0;
    }
    return static_cast<size_t>(st.st_size);
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    File f = SD.open(path.c_str());
    if (!f) {
        return false;
    }
    auto size = f.size();
    f.close();
    return static_cast<size_t>(size);
#endif
}

std::vector<File> File::listChildren(bool recursive) const
{
    std::vector<File> results;
    listChildren(results, recursive);
    return results;
}

void File::listChildren(std::vector<File>& res, bool recursive) const
{
    size_t oldSize = res.size();

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    DIR* dir = opendir(path.c_str());
    while (struct dirent* ent = readdir(dir)) {
        res.emplace_back(*this, ent->d_name);
    }
    closedir(dir);
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    File dir = SD.open(path.c_str());
    File entry;
    while (entry = dir.openNextFile()) {
        res.emplace_back(*this, entry.name());
        entry.close();
    }
    dir.close();
#endif

    if (recursive) {
        size_t newSize = res.size();
        for (size_t i = oldSize ; i < newSize ; i++) {
            res[i].listChildren(res, true);
        }
    }
}

bool File::mkdir()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    return ::mkdir(path.c_str(), 0) == 0;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    return SD.mkdir(path.c_str()) != 0;
#endif
}

bool File::remove()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    return ::remove(path.c_str()) == 0;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    return SD.remove(path.c_str()) != 0;
#endif
}

bool File::open(OpenMode mode)
{
    close();

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    const char* smode;
    switch (mode) {
    case ReadOnly:
        smode = "r";
        break;
    case WriteOnly:
        smode = "w";
        break;
    case AppendOnly:
        smode = "a";
        break;
    default:
        return false;
    }
    fhandle = fopen(path.c_str(), smode);
    if (!fhandle) {
        return false;
    }
    return true;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    switch (mode) {
    case ReadOnly:
        fhandle = SD.open(path.c_str(), FILE_READ);
        break;
    case WriteOnly:
        if (!remove()) {
            return false;
        }
        fhandle = SD.open(path.c_str(), FILE_WRITE);
        break;
    case AppendOnly:
        fhandle = SD.open(path.c_str(), FILE_WRITE);
        break;
    default:
        return false;
    }
    if (!fhandle) {
        return false;
    }
    return true;
#else
    return false;
#endif
}

void File::close()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (fhandle) {
        fclose(fhandle);
        fhandle = nullptr;
    }
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    if (fhandle) {
        fhandle.close();
        fhandle = File();
    }
#endif
}

bool File::isOpen() const
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    return fhandle != nullptr;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    return (bool) fhandle;
#else
    return false;
#endif
}

bool File::flush()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (!fhandle) {
        return false;
    }
    return fflush(fhandle) == 0;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    if (!fhandle) {
        return false;
    }
    fhandle.flush();
    return true;
#else
    return false;
#endif
}

size_t File::read(void* data, size_t size)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (!fhandle) {
        return 0;
    }
    return fread(data, 1, size, fhandle);
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    if (!fhandle) {
        return 0;
    }
    auto res = fhandle.read(data, size);
    if (res < 0) {
        return 0;
    }
    return static_cast<size_t>(res);
#else
    return 0;
#endif
}

size_t File::write(const void* data, size_t size)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (!fhandle) {
        return 0;
    }
    return fwrite(data, 1, size, fhandle);
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    if (!fhandle) {
        return 0;
    }
    return fhandle.write(data, size);
#else
    return 0;
#endif
}

int File::printf(const char* format, ...)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (!fhandle) {
        return -1;
    }

    va_list args;
    va_start(args, format);
    int res = vfprintf(fhandle, format, args);
    va_end(args);
    return res;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    // TODO: Implement
    return 0;
#else
    return 0;
#endif
}

bool File::seek(ssize_t offset)
{
#ifdef ESP_PLATFORM
    if (!fhandle) {
        return false;
    }
    return fseek(fhandle, offset, SEEK_SET) == 0;
#else
    return false;
#endif
}

ssize_t File::tell()
{
#ifdef ESP_PLATFORM
    if (!fhandle) {
        return -1;
    }
    return ftell(fhandle);
#else
    return -1;
#endif
}

std::string File::getTextContent()
{
    if (!isOpen()) {
        if (!open(ReadOnly)) {
            return {};
        }
    }
    std::string content;
    char buf[512];
    size_t numRead;
    while ((numRead = read(buf, sizeof(buf))) != 0) {
        content.append(buf, numRead);
    }
    close();
    return content;
}

bool File::setTextContent(const std::string& content)
{
    if (!isOpen()) {
        if (!open(WriteOnly)) {
            return false;
        }
    }
    if (write(content.c_str(), content.length()) != content.length()) {
        return false;
    }
    close();
    return true;
}


}

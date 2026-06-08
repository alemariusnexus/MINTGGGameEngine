#include "File.h"

#ifndef MINTGGGAMEENGINE_PORT_ARDUINO
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#include <algorithm>
#include <cstdarg>
#include <cstdio>

#include "StorageEngine.h"
#include "util/Log.h"


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

File::File(const std::string_view& path)
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
    while (path.ends_with("/")) {
        path.pop_back();
    }
}

bool File::exists() const
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct stat st;
    return stat(path.c_str(), &st) == 0;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    std::string relPath;
    if (StorageEngine::getInstance().checkSDFilePath(path, &relPath)) {
        return SD.exists(relPath.c_str());
    }
    return false;
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
    ::File f;
    if (!openSDFile(&f)) {
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
    ::File f;
    if (!openSDFile(&f)) {
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
    if (!listChildren(results, recursive)) {
        return {};
    }
    return results;
}

bool File::listChildren(std::vector<File>& res, bool recursive) const
{
    size_t oldSize = res.size();

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    DIR* dir = opendir(path.c_str());
    while (struct dirent* ent = readdir(dir)) {
        res.emplace_back(*this, ent->d_name);
    }
    closedir(dir);
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    ::File dir;
    if (!openSDFile(&dir)) {
        return false;
    }
    ::File entry;
    while (entry = dir.openNextFile()) {
        res.emplace_back(*this, entry.name());
        entry.close();
    }
    dir.close();
#endif

    if (recursive) {
        size_t newSize = res.size();
        for (size_t i = oldSize ; i < newSize ; i++) {
            if (!res[i].listChildren(res, true)) {
                return false;
            }
        }
    }

    return true;
}

bool File::mkdir()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    return ::mkdir(path.c_str(), 0) == 0;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    std::string relPath;
    if (StorageEngine::getInstance().checkSDFilePath(path, &relPath)) {
        return SD.mkdir(path.c_str()) != 0;
    }
    return false;
#endif
}

bool File::remove()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    return ::remove(path.c_str()) == 0;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    std::string relPath;
    if (StorageEngine::getInstance().checkSDFilePath(path, &relPath)) {
        return SD.remove(path.c_str()) != 0;
    }
    return false;
#endif
}

bool File::open(OpenMode mode, const char** outErrmsg)
{
    close();

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    const char* smode;
    switch (mode) {
    case ReadOnly:
        smode = "rb";
        break;
    case WriteOnly:
        smode = "wb";
        break;
    case AppendOnly:
        smode = "ab";
        break;
    default:
        return false;
    }
    fhandle = fopen(path.c_str(), smode);
    if (!fhandle) {
        if (outErrmsg) *outErrmsg = strerror(errno);
        return false;
    }
    return true;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    // Arduino does not specify data type of mode parameter, so we won't assume...
    std::string relPath;
    if (!StorageEngine::getInstance().checkSDFilePath(path, &relPath)) {
        return false;
    }
    switch (mode) {
    case ReadOnly:
        fhandle = SD.open(relPath.c_str(), FILE_READ);
        break;
    case WriteOnly:
        if (!remove()) {
            return false;
        }
        fhandle = SD.open(relPath.c_str(), FILE_WRITE);
        break;
    case AppendOnly:
        fhandle = SD.open(relPath.c_str(), FILE_WRITE);
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
        fhandle = ::File();
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
    auto res = fhandle.read(reinterpret_cast<uint8_t*>(data), size);
    if (res < 0) {
        return 0;
    }
    return static_cast<size_t>(res);
#else
    return 0;
#endif
}

size_t File::readAll(void* data, size_t size)
{
    size_t numRead = 0;
    while (size != 0) {
        size_t numReadLocal = read(reinterpret_cast<uint8_t*>(data)+numRead, size);
        if (numReadLocal == 0) {
            break;
        }
        size -= numReadLocal;
        numRead += numReadLocal;
    }
    return numRead;
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
    return fhandle.write(reinterpret_cast<const uint8_t*>(data), size);
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
    // Adapted from esp32-arduino's Print::vprintf()
    char stackBuf[128];
    char* dataBuf = stackBuf;
    va_list args;
    va_start(args, format);
    va_list args2;
    va_copy(args2, args);
    int len = vsnprintf(dataBuf, sizeof(stackBuf), format, args);
    va_end(args);
    if (len < 0) {
        va_end(args2);
        return len;
    } else if (len >= sizeof(stackBuf)) {
        dataBuf = new char[len+1];
        if (!dataBuf) {
            va_end(args2);
            return -1;
        }
        len = vsnprintf(dataBuf, len+1, format, args2);
        va_end(args2);
    }
    size_t numWritten = write(dataBuf, len);
    if (dataBuf != stackBuf) {
        delete[] dataBuf;
    }
    return static_cast<int>(numWritten);
#else
    return 0;
#endif
}

size_t File::skip(size_t size)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (!fhandle) {
        return false;
    }
    char dummy[64];
    size_t numRead;
    do {
        numRead = fread(dummy, 1, std::min(sizeof(dummy), size), fhandle);
        size -= numRead;
    } while (size != 0);
    /*if (fseek(fhandle, size, SEEK_CUR) == 0) {
        return size;
    }*/
    return size;
#else
    // TODO: Implement
    return 0;
#endif
}

bool File::seek(ssize_t offset, SeekMode mode)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (!fhandle) {
        return false;
    }
    int origin;
    switch (mode) {
    case SeekSet:
        origin = SEEK_SET;
        break;
    case SeekCur:
        origin = SEEK_CUR;
        break;
    case SeekEnd:
        origin = SEEK_END;
        break;
    default:
        return false;
    }
    return fseek(fhandle, offset, origin) == 0;
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    if (!fhandle) {
        return false;
    }
    if (mode == SeekSet) {
        return fhandle.seek(offset) != 0;
    } else if (mode == SeekCur) {
        ssize_t cpos = static_cast<ssize_t>(fhandle.position());
        return fhandle.seek(cpos+offset) != 0;
    } else if (mode == SeekEnd) {
        ssize_t size = static_cast<ssize_t>(fhandle.size());
        return fhandle.seek(size+offset) != 0;
    }
#else
    return false;
#endif
}

ssize_t File::tell()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (!fhandle) {
        return -1;
    }
    return ftell(fhandle);
#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    if (!fhandle) {
        return -1;
    }
    return static_cast<ssize_t>(fhandle.position());
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
    char buf[128];
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

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
bool File::openSDFile(::File* outFile) const
{
    std::string relPath;
    if (!StorageEngine::getInstance().checkSDFilePath(path, &relPath)) {
        return false;
    }
    *outFile = SD.open(relPath.c_str());
    if (!*outFile) {
        return false;
    }
    return true;
}
#endif


}

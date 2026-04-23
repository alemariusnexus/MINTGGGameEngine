#include "File.h"

#ifdef ESP_PLATFORM
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#include <cstdarg>
#include <cstdio>

#include "esp_private/startup_internal.h"


namespace MINTGGGameEngine
{


File::File(const File& other)
    : path(other.path)
#ifdef ESP_PLATFORM
      , fhandle(nullptr)
#endif
{
    normalizePath();
}

File::File(const std::string& path)
    : path(path)
#ifdef ESP_PLATFORM
      , fhandle(nullptr)
#endif
{
}

File::File(const File& parent, const std::string& child)
    : path(parent.getPath().append("/").append(child))
#ifdef ESP_PLATFORM
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
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

bool File::isDirectory() const
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

bool File::isRegularFile() const
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode);
}

size_t File::getSize() const
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return 0;
    }
    return static_cast<size_t>(st.st_size);
}

std::vector<File> File::listChildren(bool recursive) const
{
    std::vector<File> results;
    listChildren(results, recursive);
    return results;
}

void File::listChildren(std::vector<File>& res, bool recursive) const
{
    DIR* dir = opendir(path.c_str());
    while (struct dirent* ent = readdir(dir)) {
        res.emplace_back(*this, ent->d_name);
    }
    closedir(dir);
}

bool File::mkdir()
{
    return ::mkdir(path.c_str(), 0) == 0;
}

bool File::remove()
{
    return ::remove(path.c_str()) == 0;
}

bool File::open(OpenMode mode)
{
    close();

#ifdef ESP_PLATFORM
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
#else
    return false;
#endif
}

void File::close()
{
#ifdef ESP_PLATFORM
    if (fhandle) {
        fclose(fhandle);
        fhandle = nullptr;
    }
#endif
}

bool File::isOpen() const
{
#ifdef ESP_PLATFORM
    return fhandle != nullptr;
#else
    return false;
#endif
}

bool File::flush()
{
    if (!fhandle) {
        return false;
    }
    return fflush(fhandle) == 0;
}

size_t File::read(void* data, size_t size)
{
    if (!fhandle) {
        return 0;
    }
    return fread(data, 1, size, fhandle);
}

size_t File::write(const void* data, size_t size)
{
    if (!fhandle) {
        return 0;
    }
    return fwrite(data, 1, size, fhandle);
}

int File::printf(const char* format, ...)
{
    if (!fhandle) {
        return -1;
    }

    va_list args;
    va_start(args, format);
    int res = vfprintf(fhandle, format, args);
    va_end(args);
    return res;
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

/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "File.h"

#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#ifdef _WIN32
#   include <io.h>
#endif

#include <algorithm>
#include <cstdarg>
#include <cstdio>

#include "StorageEngine.h"


// TODO: Provide proper Windows implementation, without relying on MinGW compat modules.


namespace MINTGGGameEngine
{


File::File(const File& other)
    : path(other.path)
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
      , fhandle(nullptr)
#endif
{
    normalizePath();
}

File::File(const std::string_view& path)
    : path(path)
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
      , fhandle(nullptr)
#endif
{
}

File::File(const File& parent, const std::string& child)
    : path(parent.getPath().append("/").append(child))
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
      , fhandle(nullptr)
#endif
{
    normalizePath();
}

File::~File()
{
    close();
}

std::string File::resolvePath(const std::string& path)
{
    return StorageEngine::getInstance().resolvePath(path);
}

void File::normalizePath()
{
    while (path.ends_with("/")) {
        path.pop_back();
    }
}

bool File::exists() const
{
    const std::string rpath = resolvePath(path);
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    struct stat st;
    return stat(rpath.c_str(), &st) == 0;
#else
    return false;
#endif
}

bool File::isDirectory() const
{
    const std::string rpath = resolvePath(path);
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    struct stat st;
    if (stat(rpath.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
#else
    return false;
#endif
}

bool File::isRegularFile() const
{
    const std::string rpath = resolvePath(path);
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    struct stat st;
    if (stat(rpath.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode);
#else
    return false;
#endif
}

size_t File::getSize() const
{
    const std::string rpath = resolvePath(path);
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    struct stat st;
    if (stat(rpath.c_str(), &st) != 0) {
        return 0;
    }
    return static_cast<size_t>(st.st_size);
#else
    return 0;
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

    const std::string rpath = resolvePath(path);
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    DIR* dir = opendir(rpath.c_str());
    while (struct dirent* ent = readdir(dir)) {
        res.emplace_back(*this, ent->d_name);
    }
    closedir(dir);
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
    const std::string rpath = resolvePath(path);
#ifdef _WIN32
    return ::mkdir(rpath.c_str()) == 0;
#elif defined(MINTGGGAMEENGINE_FILE_IMPL_POSIX)
    return ::mkdir(rpath.c_str(), 0) == 0;
#endif
}

bool File::remove()
{
    const std::string rpath = resolvePath(path);
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    return ::remove(rpath.c_str()) == 0;
#else
    return false;
#endif
}

bool File::open(OpenMode mode, const char** outErrmsg)
{
    close();

    const std::string rpath = resolvePath(path);

#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
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
    fhandle = fopen(rpath.c_str(), smode);
    if (!fhandle) {
        if (outErrmsg) *outErrmsg = strerror(errno);
        return false;
    }
    return true;
#else
    return false;
#endif
}

void File::close()
{
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    if (fhandle) {
        fclose(fhandle);
        fhandle = nullptr;
    }
#endif
}

bool File::isOpen() const
{
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    return fhandle != nullptr;
#else
    return false;
#endif
}

bool File::flush()
{
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    if (!fhandle) {
        return false;
    }
    return fflush(fhandle) == 0;
#else
    return false;
#endif
}

size_t File::read(void* data, size_t size)
{
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    if (!fhandle) {
        return 0;
    }
    return fread(data, 1, size, fhandle);
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
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    if (!fhandle) {
        return 0;
    }
    return fwrite(data, 1, size, fhandle);
#else
    return 0;
#endif
}

int File::printf(const char* format, ...)
{
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
    if (!fhandle) {
        return -1;
    }

    va_list args;
    va_start(args, format);
    int res = vfprintf(fhandle, format, args);
    va_end(args);
    return res;
#else
    return 0;
#endif
}

size_t File::skip(size_t size)
{
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
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
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
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
#else
    return false;
#endif
}

ssize_t File::tell()
{
#ifdef MINTGGGAMEENGINE_FILE_IMPL_POSIX
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


}

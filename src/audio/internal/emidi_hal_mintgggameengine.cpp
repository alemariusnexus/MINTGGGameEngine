#include "../../Globals.h"

#include <cstring>

#include "../../storage/BufferedReader.h"
#include "../../storage/FileReader.h"
#include "../../storage/Reader.h"
#include "../../util/Log.h"

LOG_USE_TAG("emidi_hal_mintgggameengine")

using namespace MINTGGGameEngine;


extern "C" {

#include "../../3rdparty/eMIDI/src/hal/emidi_hal.h"



FILE* eMidi_fopen(const char* pPathName, const char* pMode)
{
    if (strcmp(pMode, "r") != 0  &&  strcmp(pMode, "rb") != 0) {
        return nullptr;
    }

    FileReader* fr = new FileReader(pPathName);
    const char* errmsg;
    bool ok = fr->open(&errmsg);
    if (!ok) {
        LogError("Error opening MIDI file for eMIDI: %s", errmsg);
        delete fr;
        return nullptr;
    }

    return reinterpret_cast<FILE*>(fr);
}

int eMidi_fclose(FILE* pStream)
{
    delete reinterpret_cast<FileReader*>(pStream);
    return 0;
}

long eMidi_ftell(FILE* pStream)
{
    FileReader* fr = reinterpret_cast<FileReader*>(pStream);
    return static_cast<long>(fr->tell());
}

int eMidi_fseek(FILE* pStream, long offset, int whence)
{
    FileReader* fr = reinterpret_cast<FileReader*>(pStream);
    MINTGGGameEngine::File::SeekMode seekMode = MINTGGGameEngine::File::SeekSet;
    switch (whence) {
    case SEEK_CUR: seekMode = MINTGGGameEngine::File::SeekCur; break;
    case SEEK_SET: seekMode = MINTGGGameEngine::File::SeekSet; break;
    case SEEK_END: seekMode = MINTGGGameEngine::File::SeekEnd; break;
    }
    return fr->seek(static_cast<ssize_t>(offset), seekMode) ? 0 : 1;
}

size_t eMidi_fread(void* p, size_t size, size_t nmemb, FILE* pStream)
{
    FileReader* fr = reinterpret_cast<FileReader*>(pStream);
    return fr->read(p, size*nmemb);
}

size_t eMidi_fwrite(const void* p, size_t size, size_t nmemb, FILE* pStream)
{
    // TODO: Implement if we want MIDI file writing at some point
    return 0;
}

int eMidi_printf(const char* pFormat, ...)
{
    // TODO: Implement maybe. Not actually needed right now
    return 0;
}

}

#pragma once

#include "../Globals.h"

#include "../storage/Reader.h"
#include "AudioClip.h"


namespace MINTGGGameEngine
{


class MIDILoader
{
public:
    MIDILoader();

    void setTranspose(int8_t halftones) { transposeHalftones = halftones; }

    AudioClip loadMIDIFile(const std::string_view& path);

    bool isLoadSuccessful() const { return loadSuccessful; }
    const std::string& getErrorMessage() const { return errmsg; }

private:
    void setError(const std::string_view& errmsg);

    uint16_t midiNoteToFrequency(uint8_t note) const;
    uint32_t midiTimeToClipTime(uint32_t time) const;

private:
    bool loadSuccessful;
    std::string errmsg;
    int8_t transposeHalftones;
};


}

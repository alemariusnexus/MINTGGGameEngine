#pragma once

#include "../Globals.h"

#include "../storage/Reader.h"
#include "AudioClip.h"


namespace MINTGGGameEngine
{


/**
 * \brief Class for loading AudioClips from simple MIDI files.
 *
 * Currently, only a specific subset of MIDI files are supported.
 *
 * For one, since the AudioEngine only supports playback of a single tone at a
 * time, MIDI files with harmony will not work. The loader attempts to still
 * load usable audio when there are overlapping notes, but the overlap will
 * not be reproduced.
 *
 * Secondly, only MIDI type 0 files are currently supported, which contain
 * only a single track.
 */
class MIDILoader
{
public:
    MIDILoader();

    /**
     * \brief Set amount of halftones to transpose all notes for subsequently loaded
     *  AudioClips.
     *
     * The default transpose is 0, which keeps the original MIDI file notes.
     *
     * @param halftones Number of halftones to transpose. Positive values
     *  transpose up, negative values transpose down.
     */
    void setTranspose(int8_t halftones) { transposeHalftones = halftones; }

    /**
     * \brief Load a MIDI file at the given path.
     *
     * @param path Path to MIDI file.
     * @return The loaded AudioClip.
     */
    AudioClip loadMIDIFile(const std::string_view& path);

    /**
     * \brief Check whether the last MIDI load was successful.
     *
     * If loading failed, an error message can be retrieved with getErrorMessage().
     *
     * @return true if successful, false otherwise.
     * @see getErrorMessage()
     */
    bool isLoadSuccessful() const { return loadSuccessful; }

    /**
     * \brief Return the last reported error message.
     *
     * @return The last error message, or an empty string is no error occurred.
     * @see isLoadSuccessful()
     */
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

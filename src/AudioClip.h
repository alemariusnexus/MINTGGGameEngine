#pragma once

#include "Globals.h"

#include <memory>
#include <vector>


namespace MINTGGGameEngine
{

/**
 * \brief Frequency values in Hz for common piano notes in equal temperament
 *      tuning.
 *
 * The english notation for note names is used here, and frequencies are
 * reounded to the nearest integer value.
 * 
 * A small letter s in the note name indicates the sharp version of the note.
 * For flat notes, the enharmonic sharp equivalent must be used (example:
 * NOTE_Fs4 is F sharp 4, alias G flat 4, at ~370Hz).
 */
enum Note
{
    NOTE_A0  = 28,
    NOTE_As0 = 29,
    NOTE_B0  = 31,

    NOTE_C1  = 33,
    NOTE_Cs1 = 35,
    NOTE_D1  = 37,
    NOTE_Ds1 = 39,
    NOTE_E1  = 41,
    NOTE_F1  = 44,
    NOTE_Fs1 = 46,
    NOTE_G1  = 49,
    NOTE_Gs1 = 52,
    NOTE_A1  = 55,
    NOTE_As1 = 58,
    NOTE_B1  = 62,

    NOTE_C2  = 65,
    NOTE_Cs2 = 69,
    NOTE_D2  = 73,
    NOTE_Ds2 = 78,
    NOTE_E2  = 82,
    NOTE_F2  = 87,
    NOTE_Fs2 = 92,
    NOTE_G2  = 98,
    NOTE_Gs2 = 104,
    NOTE_A2  = 110,
    NOTE_As2 = 117,
    NOTE_B2  = 123,

    NOTE_C3  = 131,
    NOTE_Cs3 = 139,
    NOTE_D3  = 147,
    NOTE_Ds3 = 156,
    NOTE_E3  = 165,
    NOTE_F3  = 175,
    NOTE_Fs3 = 185,
    NOTE_G3  = 196,
    NOTE_Gs3 = 208,
    NOTE_A3  = 220,
    NOTE_As3 = 233,
    NOTE_B3  = 247,

    NOTE_C4  = 262,
    NOTE_Cs4 = 277,
    NOTE_D4  = 294,
    NOTE_Ds4 = 311,
    NOTE_E4  = 330,
    NOTE_F4  = 349,
    NOTE_Fs4 = 370,
    NOTE_G4  = 392,
    NOTE_Gs4 = 415,
    NOTE_A4  = 440,
    NOTE_As4 = 466,
    NOTE_B4  = 494,

    NOTE_C5  = 523,
    NOTE_Cs5 = 554,
    NOTE_D5  = 587,
    NOTE_Ds5 = 622,
    NOTE_E5  = 659,
    NOTE_F5  = 698,
    NOTE_Fs5 = 740,
    NOTE_G5  = 784,
    NOTE_Gs5 = 831,
    NOTE_A5  = 880,
    NOTE_As5 = 932,
    NOTE_B5  = 988,

    NOTE_C6  = 1047,
    NOTE_Cs6 = 1109,
    NOTE_D6  = 1175,
    NOTE_Ds6 = 1245,
    NOTE_E6  = 1319,
    NOTE_F6  = 1397,
    NOTE_Fs6 = 1480,
    NOTE_G6  = 1568,
    NOTE_Gs6 = 1661,
    NOTE_A6  = 1760,
    NOTE_As6 = 1865,
    NOTE_B6  = 1976,

    NOTE_C7  = 2093,
    NOTE_Cs7 = 2217,
    NOTE_D7  = 2349,
    NOTE_Ds7 = 2489,
    NOTE_E7  = 2637,
    NOTE_F7  = 2794,
    NOTE_Fs7 = 2960,
    NOTE_G7  = 3136,
    NOTE_Gs7 = 3322,
    NOTE_A7  = 3520,
    NOTE_As7 = 3729,
    NOTE_B7  = 3951,

    NOTE_C8  = 4186
};


/**
 * \brief An simple audio clip.
 *
 * Audio clips can be played through the AudioEngine. They can be single-shot
 * or looped. See AudioEngine for details.
 *
 * A clip is a sequence of atoms (i.e. notes or pauses). Each Atom is defined
 * by its frequency and duration.
 *
 * Time is measured in so called base units. The actual real time duration of
 * this base unit is defined by the clip's tempo: The tempo is the number of
 * base units per minute of real time. This allows speeding up or slowing down
 * a clip simply by changing its tempo on-the-fly, without having to change
 * individual atoms.
 * It is up to the user to define what a base unit corresponds to. It usually
 * corresponds to a certain note value in sheet music (e.g. a quarter note, or
 * an eighth note). Example: If the base unit is chosen to be a quarter note,
 * the tempo defines the number of quarter notes in a minute.
 * Note that the base unit can't be subdivided (note durations in this class
 * are integer values), so the base unit must be chosen to be the lowest common
 * denominator of all durations encountered in the clip.
 *
 * This class assumes that only one note (or none in case of pauses) is playing
 * at any given time, i.e. it does not support harmony.
 *
 * \see AudioEngine
 */
class AudioClip
{
public:
    /**
     * \brief A single note (or pause) in the clip.
     */
    struct Atom
    {
        Atom(uint16_t freq, uint16_t duration, uint32_t timestamp) : freq(freq), duration(duration), timestamp(timestamp) {}
        Atom(uint16_t duration, uint32_t timestamp) : freq(0), duration(duration), timestamp(timestamp) {}
        
        bool operator==(const Atom& o) const { return timestamp == o.timestamp; }
        bool operator!=(const Atom& o) const { return !(*this == o); }
        bool operator<(const Atom& o) const { return timestamp < o.timestamp; }
        bool operator<=(const Atom& o) const { return timestamp <= o.timestamp; }
        bool operator>(const Atom& o) const { return timestamp > o.timestamp; }
        bool operator>=(const Atom& o) const { return timestamp >= o.timestamp; }
        
        uint16_t freq; ///< Frequency (in Hz), or 0 for a pause
        uint16_t duration; ///< Note duration (in base units)
        uint32_t timestamp; ///< Time offset from start of clip (in base units)
    };
    struct Data
    {
        Data() : tempo(1.0f), noteEndReleaseDuration(0.0f) {}
        std::vector<Atom> atoms;
        float tempo; // Number of base units per minute
        float noteEndReleaseDuration; // Percent of base unit
    };
    
public:
    /**
     * \brief Create an empty audio clip.
     */
    AudioClip() : d(std::make_shared<Data>()) {}
    AudioClip(const AudioClip& other) : d(other.d) {}
    
    
    /// \name Atoms (Notes and Pauses)
    ///@{
    
    /**
     * \brief Add a single note to the end of the clip.
     *
     * \param freq The note's frequency. See Note enum for predefined values.
     * \param duration The note's duration, in base units.
     */
    void note(uint16_t freq, uint16_t duration) { newAtom(freq, duration); }
    
    /**
     * \brief Add a single pause (i.e. silence) to the end of the clip.
     *
     * \param duration The pause's duration, in base units.
     */
    void pause(uint16_t duration) { newAtom(0, duration); }
    
    /**
     * \brief Remove all notes and pauses from the clip.
     */
    void clear();
    
    /**
     * \brief Return all atoms (notes and pauses) of the clip, in chronological
     *      order.
     */
    const std::vector<Atom>& getAtoms() const { return d->atoms; }
    
    ///@}
    
    
    /// \name Clip Properties
    ///@{
    
    /**
     * \brief Get the clip's tempo, i.e. how many base units per minute.
     */
    float getTempo() const { return d->tempo; }
    
    /**
     * \brief Get the clip's tempo in reference to the given base unit.
     */
    float getTempo(uint16_t unit) const;
    
    /**
     * \brief Set the tempo of the clip, i.e. how many base units per minute.
     */
    void setTempo(float tempo) { d->tempo = tempo; }
    
    /**
     * \brief Set the tempo of the clip in reference to a given unit, i.e. how
     *      many such units per minute.
     */
    void setTempo(uint16_t unit, float tempo);
    
    /**
     * \brief Get the duration for which to release a note prior to its end.
     *
     * Currently, this is a value between 0 and 1. 0.1 means that the final 10%
     * of each note's duration are actually silent. A value of 0 would play the
     * notes legato.
     */
    float getNoteEndReleaseDuration() const { return d->noteEndReleaseDuration; }
    
    /**
     * \brief Set the duration for which to release a note prior to its end.
     *
     * \see getNoteEndReleaseDurtion()
     */
    void setNoteEndReleaseDuration(float noteEndReleaseDuration) { d->noteEndReleaseDuration = noteEndReleaseDuration; }
    
    /**
     * \brief Get the length of the clip, in base units.
     */
    uint32_t getLength() const;
    
    ///@}
    
    
    /// \name Playback Utilities
    ///@{
    
    /**
     * \brief Convert real time since clip start to base units since clip start.
     *
     * \param timeMs Real time offset from clip start, in milliseconds.
     * \return Base unit offset from clip start, including fractional units.
     */
    float realTimeToBaseUnits(float timeMs) const;
    
    /**
     * \brief Get information about playback at a given point in the clip.
     *
     * This returns the atom (i.e. note or pause) that is active at that given
     * time, as well as the time within that note.
     *
     * \param in pos The offset from clip start, in base units. It is valid to
     *      provide a position past the end of the clip, in which case it will
     *      wrap back around to the clip start (useful e.g. for looping clips).
     * \param out Atom that is currently playing at the given playback position.
     * \param out outTimeWithinAtom The offset from the start of the currently
     *      playing atom at the given playback position, in base units.
     * \return The actual playback position (in case of wrap-around).
     */
    float getPlaybackPosition(float pos, Atom** outAtom, float* outTimeWithinAtom) const;
    
    ///@}
    
    
    /// \name Operators
    ///@{
    
    AudioClip& operator=(const AudioClip& other) { d = other.d; return *this; }
    
    bool operator==(const AudioClip& other) const { return d == other.d; }
    bool operator!=(const AudioClip& other) const { return d != other.d; }
    
    ///@}

private:
    void newAtom(uint16_t freq, uint16_t duration);

private:
    std::shared_ptr<Data> d;
};

}

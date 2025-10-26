#pragma once

#include "Globals.h"

#include <memory>
#include <vector>


namespace MINTGGGameEngine
{

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


class AudioClip
{
public:
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
        
        uint16_t freq;
        uint16_t duration; // In base units (arbitrary, length defined by tempo, e.g. sixteenth notes)
        uint32_t timestamp; // Offset from start of clip, in base units
    };
    struct Data
    {
        Data() : tempo(1.0f), noteEndReleaseDuration(0.0f) {}
        std::vector<Atom> atoms;
        float tempo; // Number of base units per minute
        float noteEndReleaseDuration; // Percent of base unit
    };
    
public:
    AudioClip() : d(std::make_shared<Data>()) {}
    AudioClip(const AudioClip& other) : d(other.d) {}
    
    AudioClip& operator=(const AudioClip& other) { d = other.d; return *this; }
    
    bool operator==(const AudioClip& other) const { return d == other.d; }
    bool operator!=(const AudioClip& other) const { return d != other.d; }
    
    void note(uint16_t freq, uint16_t duration) { newAtom(freq, duration); }
    void pause(uint16_t duration) { newAtom(0, duration); }
    void clear();
    
    const std::vector<Atom>& getAtoms() const { return d->atoms; }
    float getTempo() const { return d->tempo; }
    float getTempo(uint16_t unit) const;
    float getNoteEndReleaseDuration() const { return d->noteEndReleaseDuration; }
    
    void setTempo(float tempo) { d->tempo = tempo; }
    void setTempo(uint16_t unit, float tempo);
    void setNoteEndReleaseDuration(float noteEndReleaseDuration) { d->noteEndReleaseDuration = noteEndReleaseDuration; }
    
    uint32_t getLength() const;
    
    float realTimeToBaseUnits(float timeMs) const;
    
    float getPlaybackPosition(float pos, Atom** outAtom, float* outTimeWithinAtom) const;

private:
    void newAtom(uint16_t freq, uint16_t duration);

private:
    std::shared_ptr<Data> d;
};

}

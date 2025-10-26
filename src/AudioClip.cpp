#include "AudioClip.h"

#include <algorithm>


namespace MINTGGGameEngine
{

void AudioClip::newAtom(uint16_t freq, uint16_t duration)
{
    uint32_t timestamp = 0;
    if (!d->atoms.empty()) {
        const Atom& last = d->atoms.back();
        timestamp = last.timestamp + last.duration;
    }
    d->atoms.push_back(Atom(freq, duration, timestamp));
}

void AudioClip::clear()
{
    d->atoms.clear();
}

void AudioClip::setTempo(uint16_t unit, float tempo)
{
    setTempo(unit*tempo);
}

float AudioClip::getTempo(uint16_t unit) const
{
    return unit == 0 ? 0.0f : getTempo() / unit;
}

uint32_t AudioClip::getLength() const
{
    return d->atoms.empty() ? 0 : d->atoms.back().timestamp + d->atoms.back().duration;
}

float AudioClip::realTimeToBaseUnits(float timeMs) const
{
    // tempo = baseUnits / minutes
    // baseUnits = tempo * minutes
    return d->tempo * (timeMs / 60000.0f);
}

float AudioClip::getPlaybackPosition(float pos, Atom** outAtom, float* outTimeWithinAtom) const
{
    if (d->atoms.empty()) {
        *outAtom = nullptr;
        *outTimeWithinAtom = 0;
        return 0.0f;
    }
    pos = fmod(pos, getLength());
    auto next = std::upper_bound(d->atoms.begin(), d->atoms.end(), pos, [](float pos, const Atom& atom) {
        return pos < atom.timestamp;
    });
    if (next == d->atoms.begin()) {
        *outAtom = &d->atoms.front();
        *outTimeWithinAtom = 0.0f;
        return 0.0f;
    }
    next--;
    *outAtom = &*next;
    *outTimeWithinAtom = pos - next->timestamp;
    return pos;
}

}

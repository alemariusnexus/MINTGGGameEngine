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

#include "AudioClip.h"

#include <cmath>

#include <algorithm>

#include "MIDILoader.h"
#include "../util/Log.h"


namespace MINTGGGameEngine
{

AudioClip AudioClip::loadMIDI (
    const std::string_view& path,
    int8_t transposeHalftones,
    std::string* outErrmsg
) {
    MIDILoader loader;
    loader.setTranspose(transposeHalftones);
    AudioClip clip = loader.loadMIDIFile(path);
    if (outErrmsg) {
        if (loader.isLoadSuccessful()) {
            *outErrmsg = {};
        } else {
            *outErrmsg = loader.getErrorMessage();
        }
    }
    return clip;
}

void AudioClip::newAtom(uint16_t freq, uint16_t duration, uint32_t timestamp)
{
    d->atoms.push_back(Atom(freq, duration, timestamp));
}

void AudioClip::newAtom(uint16_t freq, uint16_t duration)
{
    uint32_t timestamp = 0;
    if (!d->atoms.empty()) {
        const Atom& last = d->atoms.back();
        timestamp = last.timestamp + last.duration;
    }
    newAtom(freq, duration, timestamp);
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
        *outTimeWithinAtom = -1.0f;
        return pos;
    }
    next--;
    *outAtom = &*next;
    *outTimeWithinAtom = pos - next->timestamp;
    return pos;
}

}

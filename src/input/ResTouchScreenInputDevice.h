/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "../Globals.h"

#include "../platform/ResTouchScreen.h"
#include "TouchInputDevice.h"


namespace MINTGGGameEngine
{

class ResTouchScreenInputDevice : public TouchInputDevice
{
public:
    ResTouchScreenInputDevice (
        gpionum_t xmPin, gpionum_t xpPin, gpionum_t ymPin, gpionum_t ypPin,
        uint16_t width, uint16_t height
        );

    void setDebounceTime(uint32_t timeMs) { debounceTimeMs = timeMs; }
    uint32_t getDebounceTime() const { return debounceTimeMs; }

    bool setup();

    void tick() override;

private:
    void sample();
    bool debounce();

private:
    ResTouchScreen touchScreen;

    uint16_t width, height;

    uint32_t debounceTimeMs;

    bool preDebounceActive;
    timer_mstick_t debounceStartTime;

    bool active;
    uint16_t curX, curY;
};

}

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

#include "ResTouchScreenInputDevice.h"

#include <cmath>

#include "../util/Log.h"

LOG_USE_TAG("ResTouchScreenInputDevice")


namespace MINTGGGameEngine
{

ResTouchScreenInputDevice::ResTouchScreenInputDevice (
    gpionum_t xmPin, gpionum_t xpPin, gpionum_t ymPin, gpionum_t ypPin,
    uint16_t width, uint16_t height
)   : touchScreen(xmPin, xpPin, ymPin, ypPin), width(width), height(height), debounceTimeMs(0),
      preDebounceActive(false), debounceStartTime(0), active(false), curX(0), curY(0)
{
}

bool ResTouchScreenInputDevice::setup()
{
    return touchScreen.setup();
}

void ResTouchScreenInputDevice::tick()
{
    sample();
}

void ResTouchScreenInputDevice::sample()
{
    const auto nowUs = TimerGetTickcountUs();

    Vec2 rpos;
    bool touched = touchScreen.read(&rpos);
    rpos.setY(1.0f - rpos.y());

    const auto lastX = curX;
    const auto lastY = curY;

    const auto px = static_cast<uint16_t>(std::round(rpos.x() * width));
    const auto py = static_cast<uint16_t>(std::round(rpos.y() * height));

    if (touched) {
        curX = px;
        curY = py;
    }
    if (preDebounceActive != touched) {
        // Pre-debounced state changed from last sample -> start debouncing over again
        preDebounceActive = touched;
        debounceStartTime = static_cast<timer_mstick_t>(nowUs / 1000);
    }

    if (debounce()) {
        // Debounced touch begin or end already reported
        return;
    }

    if (active) {
        if (curX != lastX  ||  curY != lastY) {
            // Touch moved
            TouchInputDevice::TouchDataPoint data = {
                .id = 1,
                .flags = TouchInputDevice::TOUCH_DATA_FLAGS_ACTIVE,
                .px = curX,
                .py = curY,
                .pressure = 0,
                .timestamp = nowUs
            };
            reportUpdate(data);
        }
    }
}

bool ResTouchScreenInputDevice::debounce()
{
    if (preDebounceActive == active) {
        return false;
    }

    auto now = TimerGetTickcountMs();
    if (now >= debounceStartTime + debounceTimeMs) {
        // Debounced touch begin or end

        active = preDebounceActive;

        TouchInputDevice::TouchDataPoint data = {
            .id = 1,
            .flags = 0,
            .px = curX,
            .py = curY,
            .pressure = 0,
            .timestamp = static_cast<timer_ustick_t>(debounceStartTime) * 1000
        };
        if (active) {
            data.flags |= TouchInputDevice::TOUCH_DATA_FLAGS_ACTIVE;
        }
        reportUpdate(data);

        debounceStartTime = 0;

        return true;
    }

    return false;
}

}

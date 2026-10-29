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

#include "../util/Util.h"


namespace MINTGGGameEngine
{

class InputEngine;

class TouchInputDevice
{
    friend class InputEngine;

public:
    typedef int64_t touchpoint_id_t;

    enum TouchDataFlags
    {
        TOUCH_DATA_FLAGS_ACTIVE = 0x01
    };

    struct TouchDataPoint
    {
        touchpoint_id_t id;
        uint8_t flags;
        uint16_t px;
        uint16_t py;
        uint16_t pressure;
        timer_ustick_t timestamp; // [us]
    };

public:
    TouchInputDevice();
    virtual ~TouchInputDevice() = default;

    virtual void tick() {}

    void reportUpdate(const TouchDataPoint& data);

private:
    void registerInputEngine(InputEngine* engine);

private:
    InputEngine* inputEngine;
};

}

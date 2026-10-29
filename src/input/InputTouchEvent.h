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
#include "../util/Vec2.h"
#include "TouchInputDevice.h"

namespace MINTGGGameEngine
{

typedef int64_t touch_id_t;

class InputTouchEvent
{
public:
    enum class Type
    {
        Begin,
        Move,
        End
    };

public:
    InputTouchEvent (
        touch_id_t id,
        TouchInputDevice* dev,
        Type type,
        uint16_t px, uint16_t py,
        timer_ustick_t timestamp
        );

    touch_id_t getID() const { return id; }

    TouchInputDevice* getDevice() const { return dev; }

    Type getType() const { return type; }
    bool isBegin() const { return type == Type::Begin; }
    bool isMove() const { return type == Type::Move; }
    bool isEnd() const { return type == Type::End; }

    uint16_t getX() const { return px; }
    uint16_t getY() const { return py; }
    Vec2 getPosition() const { return {static_cast<float>(px), static_cast<float>(py)}; }

    timer_ustick_t getTimestamp() const { return timestamp; }

private:
    touch_id_t id;

    TouchInputDevice* dev;

    Type type;
    uint16_t px;
    uint16_t py;
    timer_ustick_t timestamp; // [us]
};

}

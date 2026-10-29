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

#include "InputTouchEvent.h"

namespace MINTGGGameEngine
{

InputTouchEvent::InputTouchEvent (
    touch_id_t id,
    TouchInputDevice* dev,
    Type type,
    uint16_t px, uint16_t py,
    timer_ustick_t timestamp
) : id(id), dev(dev), type(type), px(px), py(py), timestamp(timestamp)
{
}

}

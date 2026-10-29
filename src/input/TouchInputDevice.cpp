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

#include "TouchInputDevice.h"

#include "InputEngine.h"

namespace MINTGGGameEngine
{

TouchInputDevice::TouchInputDevice()
    : inputEngine(nullptr)
{
}

void TouchInputDevice::reportUpdate(const TouchDataPoint& data)
{
    if (inputEngine) {
        inputEngine->injectTouchInputUpdate(this, data);
    }
}

void TouchInputDevice::registerInputEngine(InputEngine* engine)
{
    inputEngine = engine;
}

}

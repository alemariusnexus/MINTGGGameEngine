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

#include "../util/Vec3.h"


namespace MINTGGGameEngine
{

class ResTouchScreen
{
public:
    /**
     *
     * \param xmPin X- pin. Must be an analog pin!
     * \param xpPin X+ pin. Can be a digital pin.
     * \param ymPin Y- pin. Can be a digital pin.
     * \param ypPin Y+ pin. Must be an analog pin!
     */
    ResTouchScreen(gpionum_t xmPin, gpionum_t xpPin, gpionum_t ymPin, gpionum_t ypPin);

    bool setup();

    bool read(Vec2* position);

private:
    bool checkTouched(uint16_t ioVoltageMv);
    float readRatioX(uint16_t ioVoltageMv);
    float readRatioY(uint16_t ioVoltageMv);
    float readPressure(float ratioX, uint16_t ioVoltageMv);

private:
    gpionum_t xmPin, xpPin;
    gpionum_t ymPin, ypPin;
};

}

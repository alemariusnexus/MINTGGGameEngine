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

#pragma once

#include "../Globals.h"

#include "GPIODevice.h"


namespace MINTGGGameEngine
{


/**
 * \brief A GPIODevice that uses the microcontroller's internal GPIO pins.
 *
 * The calls of this class mostly just redirect to standard Arduino functions like
 * pinMode(), digitalWrite() and digitalRead().
 *
 * Use getInstance() to get the single global object of this class.
 */
class GPIODeviceNative : public GPIODevice
{
public:
	static GPIODeviceNative& getInstance();

public:
	bool setPinMode(gpionum_t pin, uint8_t mode) override;

    uint8_t readPin(gpionum_t pin) override;
	void writePin(gpionum_t pin, uint8_t val) override;
};


}

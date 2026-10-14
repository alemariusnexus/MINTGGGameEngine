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

#include "GPIODevice.h"


namespace MINTGGGameEngine
{


void GPIODevice::readPins(const gpionum_t* pins, uint8_t* values, size_t numPins)
{
	for (size_t i = 0 ; i < numPins ; i++) {
		values[i] = readPin(pins[i]);
	}
}


}

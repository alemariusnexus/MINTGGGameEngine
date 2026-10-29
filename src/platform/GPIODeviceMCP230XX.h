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
#include "MCP230XXDevice.h"


namespace MINTGGGameEngine
{


/**
 * \brief A GPIODevice that interfaces to an external MCP230XX I2C IO expander.
 *
 * Don't forget to call begin() at startup!
 *
 * For reading multiple pins at once, it is recommended to use readPins(), since
 * calling readPin() multiple times results in separate I2C messages being sent
 * for each pin.
 *
 * Note that the MCP230XX supports pullups, but no pulldowns, so pin modes of
 * PULLDOWN or INPUT_PULLDOWN will not work.
 */
class GPIODeviceMCP230XX : public GPIODevice
{
public:
    GPIODeviceMCP230XX(I2CBusMaster* master, uint8_t widthBytes, uint32_t clockFreq = 400000, uint8_t i2cAddr = 0x20);
	GPIODeviceMCP230XX(uint8_t widthBytes, uint32_t clockFreq = 400000, uint8_t i2cAddr = 0x20);
	
	/**
	 * \brief Initializes the MCP2300X.
	 */
	void init();

	bool setPinMode(gpionum_t pin, uint8_t mode) override;

    uint8_t readPin(gpionum_t pin) override;
	void writePin(gpionum_t pin, uint8_t val) override;
	
	void readPins(const gpionum_t* pins, uint8_t* values, size_t numPins) override;
	
private:
	MCP230XXDevice mcp;
};


}

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

#include "GPIODeviceMCP2300X.h"

#include "../util/Log.h"
#include "../util/Util.h"


namespace MINTGGGameEngine
{


GPIODeviceMCP2300X::GPIODeviceMCP2300X(gpionum_t sclPin, gpionum_t sdaPin, uint32_t clockFreq, uint8_t i2cAddr)
    : mcp(sclPin, sdaPin, clockFreq, i2cAddr)
{
}

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO

GPIODeviceMCP2300X::GPIODeviceMCP2300X(TwoWire& bus, uint8_t i2cAddr)
    : mcp(bus, i2cAddr)
{
}

#endif

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF

GPIODeviceMCP2300X::GPIODeviceMCP2300X(i2c_master_bus_handle_t bus, i2c_master_dev_handle_t dev)
    : mcp(bus, dev)
{
}

#endif

void GPIODeviceMCP2300X::begin()
{
}

bool GPIODeviceMCP2300X::setPinMode(gpionum_t pin, uint8_t mode)
{
    if (pin < 0) {
        return false;
    }

    bool output, puEnabled, pdEnabled;
    ExtractArduinoPinMode(mode, &output, &puEnabled, &pdEnabled);

    if (pdEnabled) {
        // MCP2300X doesn't support pulldowns
        return false;
    }

    uint8_t oldIODir;
    uint8_t oldPUState;

    if (!mcp.getIODirection(&oldIODir)) {
        return false;
    }
    if (!mcp.getPullupEnabled(&oldPUState)) {
        return false;
    }

    uint8_t newIODir = oldIODir;
    uint8_t newPUState = oldPUState;

    if (output) {
        newIODir &= ~(1 << pin);
    } else {
        newIODir |= (1 << pin);
    }

    if (puEnabled) {
        newPUState |= (1 << pin);
    } else {
        newPUState &= ~(1 << pin);
    }

    if (newIODir != oldIODir) {
        if (!mcp.setIODirection(newIODir)) {
            return false;
        }
    }
    if (newPUState != oldPUState) {
        if (!mcp.setPullupEnabled(newPUState)) {
            return false;
        }
    }

	return true;
}

uint8_t GPIODeviceMCP2300X::readPin(gpionum_t pin)
{
    if (pin < 0) {
        return 0;
    }
    uint8_t state;
    if (!mcp.readPins(&state)) {
        return 0;
    }
	return (state & (1 << pin)) != 0;
}

void GPIODeviceMCP2300X::writePin(gpionum_t pin, uint8_t val)
{
    if (pin < 0) {
        return;
    }
    uint8_t newState;
    if (!mcp.readPins(&newState)) {
        return;
    }
	if (val) {
	    newState |= (1 << pin);
	} else {
	    newState &= ~(1 << pin);
	}
    mcp.writePins(newState);
}

void GPIODeviceMCP2300X::readPins(const gpionum_t* pins, uint8_t* values, size_t numPins)
{
    uint8_t val8;
    if (!mcp.readPins(&val8)) {
        return;
    }
	for (size_t i = 0 ; i < numPins ; i++) {
		if (pins[i] >= 0  &&  pins[i] < 8) {
			values[i] = ((val8 >> pins[i]) & 0x01) ? HIGH : LOW;
		} else {
			values[i] = LOW;
		}
	}
}


}

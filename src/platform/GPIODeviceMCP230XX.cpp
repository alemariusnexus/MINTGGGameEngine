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

#include "GPIODeviceMCP230XX.h"

#include <cassert>

#include "../util/Log.h"
#include "../util/Util.h"
#include "HardwareManager.h"


namespace MINTGGGameEngine
{


GPIODeviceMCP230XX::GPIODeviceMCP230XX(I2CBusMaster* master, uint8_t widthBytes, uint32_t clockFreq, uint8_t i2cAddr)
    : mcp(master, widthBytes, clockFreq, i2cAddr)
{
}

GPIODeviceMCP230XX::GPIODeviceMCP230XX(uint8_t widthBytes, uint32_t clockFreq, uint8_t i2cAddr)
    : mcp(widthBytes, clockFreq, i2cAddr)
{
}

void GPIODeviceMCP230XX::init()
{
    mcp.init();
}

bool GPIODeviceMCP230XX::setPinMode(gpionum_t pin, uint8_t mode)
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

    const uint8_t portIdx = pin / 8;
    pin %= 8;

    uint8_t oldIODir;
    uint8_t oldPUState;

    if (!mcp.getIODirection(portIdx, &oldIODir)) {
        return false;
    }
    if (!mcp.getPullupEnabled(portIdx, &oldPUState)) {
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
        if (!mcp.setIODirection(portIdx, newIODir)) {
            return false;
        }
    }
    if (newPUState != oldPUState) {
        if (!mcp.setPullupEnabled(portIdx, newPUState)) {
            return false;
        }
    }

	return true;
}

uint8_t GPIODeviceMCP230XX::readPin(gpionum_t pin)
{
    if (pin < 0) {
        return 0;
    }
    const uint8_t portIdx = pin / 8;
    pin %= 8;
    uint8_t state;
    if (!mcp.readPins(portIdx, &state)) {
        return 0;
    }
	return (state & (1 << pin)) != 0;
}

void GPIODeviceMCP230XX::writePin(gpionum_t pin, uint8_t val)
{
    if (pin < 0) {
        return;
    }
    const uint8_t portIdx = pin / 8;
    pin %= 8;
    uint8_t newState;
    if (!mcp.readPins(portIdx, &newState)) {
        return;
    }
	if (val) {
	    newState |= (1 << pin);
	} else {
	    newState &= ~(1 << pin);
	}
    mcp.writePins(portIdx, newState);
}

void GPIODeviceMCP230XX::readPins(const gpionum_t* pins, uint8_t* values, size_t numPins)
{
    for (size_t i = 0 ; i < numPins ; i++) {
        values[i] = LOW;
    }

    for (uint8_t portIdx = 0 ; portIdx < mcp.getWidthBytes() ; portIdx++) {
        bool portRequested = false;

        // Check if any pin on this port is requested
        for (size_t i = 0 ; i < numPins ; i++) {
            if (pins[i] >= 0  &&  pins[i]/8 == portIdx) {
                portRequested = true;
                break;
            }
        }

        if (portRequested) {
            uint8_t val8;
            if (!mcp.readPins(portIdx, &val8)) {
                return;
            }

            for (size_t i = 0 ; i < numPins ; i++) {
                if (pins[i] >= 0  &&  pins[i]/8 == portIdx) {
                    const uint8_t localPinIdx = pins[i] % 8;
                    values[i] = ((val8 >> localPinIdx) & 0x01) ? HIGH : LOW;
                }
            }
        }
    }
}


}

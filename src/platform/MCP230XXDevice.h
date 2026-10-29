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

#include "i2c/I2CBusDevice.h"
#include "i2c/I2CBusMaster.h"


namespace MINTGGGameEngine
{


class MCP230XXDevice {
public:
    MCP230XXDevice(I2CBusMaster* master, uint8_t widthBytes, uint32_t clockFreq = 400000, uint8_t i2cAddr = 0x20);
    MCP230XXDevice(uint8_t widthBytes, uint32_t clockFreq = 400000, uint8_t i2cAddr = 0x20);

    void init();

    uint8_t getWidthBytes() const { return widthBytes; }

    bool setIODirection(uint8_t portIdx, uint8_t ioDir);
    bool getIODirection(uint8_t portIdx, uint8_t* ioDir);

    bool setPullupEnabled(uint8_t portIdx, uint8_t puEn);
    bool getPullupEnabled(uint8_t portIdx, uint8_t* puEn);

    bool readPins(uint8_t portIdx, uint8_t* pinStates);
    bool writePins(uint8_t portIdx, uint8_t pinStates);

private:
    bool readRegister(uint8_t portIdx, uint8_t addr, uint8_t* value);
    bool writeRegister(uint8_t portIdx, uint8_t addr, uint8_t value);

private:
    I2CBusMaster* master;
    I2CBusDevice* dev;
    uint8_t widthBytes;
    uint32_t clockFreq;
    uint8_t i2cAddr;
};


}

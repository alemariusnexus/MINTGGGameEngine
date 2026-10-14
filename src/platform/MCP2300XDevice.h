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

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
#   include <Wire.h>
#endif

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <driver/i2c_master.h>
#endif


namespace MINTGGGameEngine
{


class MCP2300XDevice {
public:
    MCP2300XDevice(gpionum_t sclPin, gpionum_t sdaPin, uint32_t clockFreq = 400000, uint8_t i2cAddr = 0x20);
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    MCP2300XDevice(TwoWire& bus, uint8_t i2cAddr = 0x20);
#endif
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    MCP2300XDevice(i2c_master_bus_handle_t bus, i2c_master_dev_handle_t dev);
#endif

    bool setIODirection(uint8_t ioDir);
    bool getIODirection(uint8_t* ioDir);

    bool setPullupEnabled(uint8_t puEn);
    bool getPullupEnabled(uint8_t* puEn);

    bool readPins(uint8_t* pinStates);
    bool writePins(uint8_t pinStates);

private:
    void init();

    bool readRegister(uint8_t addr, uint8_t* value);
    bool writeRegister(uint8_t addr, uint8_t value);

private:
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    struct
    {
        TwoWire* i2cBus;
        uint8_t i2cAddr;
    } arduino;
#endif
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct
    {
        i2c_master_bus_handle_t i2cBus;
        i2c_master_dev_handle_t i2cDev;
    } espidf;
#endif
};


}

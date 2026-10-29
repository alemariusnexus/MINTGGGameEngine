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

#include "../../Globals.h"

#include <list>

#include "I2CBusDevice.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <driver/i2c_master.h>
#endif


namespace MINTGGGameEngine
{

class I2CBusMaster
{
public:
    I2CBusMaster(gpionum_t sclPin, gpionum_t sdaPin);
    ~I2CBusMaster();

    bool init();

    bool isInitialized() const { return initialized; }

    bool isValid() const { return isInitialized()  &&  sclPin >= 0  &&  sdaPin >= 0; }

    I2CBusDevice* addDevice(uint16_t address, uint32_t clockFreqHz, int flags = 0);
    void removeDevice(I2CBusDevice* dev);

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    i2c_master_bus_handle_t getESPIDFBusHandle() const { return espidf.i2cBus; }
#endif

private:
    gpionum_t sclPin;
    gpionum_t sdaPin;

    bool initialized;

    std::list<I2CBusDevice*> devices;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct {
        i2c_master_bus_handle_t i2cBus;
    } espidf;
#endif
};

}

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

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <driver/spi_master.h>
#endif

namespace MINTGGGameEngine
{

class SPIBusMaster;

class SPIBusDevice
{
    friend class SPIBusMaster;

public:
    enum Flags
    {
    };

public:
    ~SPIBusDevice();

    SPIBusMaster* getMaster() { return master; }
    const SPIBusMaster* getMaster() const { return master; }

    bool isValid() const;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    spi_device_handle_t getESPDeviceHandle() const { return espidf.dev; }
#endif

    bool transmit(const uint8_t* data, size_t dataSize);

    std::string getLastErrorMessage() const { return errmsg; }

private:
    SPIBusDevice(SPIBusMaster* master, gpionum_t csPin, uint32_t clockFreqHz, int flags);

    bool init();

    void setError(const std::string& msg) { errmsg = msg; }
    void resetError() { errmsg = {}; }

private:
    SPIBusMaster* master;

    gpionum_t csPin;
    uint32_t clockFreqHz;
    int flags;

    std::string errmsg;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct {
        spi_device_handle_t dev;
    } espidf;
#endif
};

}

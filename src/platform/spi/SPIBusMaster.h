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

#include "SPIBusDevice.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <driver/spi_master.h>
#endif

namespace MINTGGGameEngine
{

class SPIBusMaster
{
public:
    SPIBusMaster (
        gpionum_t misoPin, gpionum_t mosiPin, gpionum_t sckPin,
        size_t maxTransferSize
        );
    ~SPIBusMaster();

    bool init();

    bool isInitialized() const { return initialized; }

    bool isValid() const { return isInitialized()  &&  (sckPin >= 0); }

    size_t getMaxTransferSize() const { return maxTransferSize; }

    SPIBusDevice* addDevice(gpionum_t csPin, uint32_t clockFreqHz, int flags = 0);
    void removeDevice(SPIBusDevice* dev);

    std::string getLastErrorMessage() const { return errmsg; }

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    void setESPIDFHost(spi_host_device_t host) { espidf.host = host; }
    spi_host_device_t getESPIDFHost() const { return espidf.host; }
#endif

private:
    void setError(const std::string& msg) { errmsg = msg; }
    void resetError() { errmsg = {}; }

private:
    gpionum_t misoPin;
    gpionum_t mosiPin;
    gpionum_t sckPin;

    size_t maxTransferSize;

    bool initialized;

    std::list<SPIBusDevice*> devices;

    std::string errmsg;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct {
        spi_host_device_t host;
    } espidf;
#endif
};

}

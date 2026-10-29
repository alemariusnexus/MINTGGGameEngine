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

#include "SPIBusMaster.h"

#include <limits>

#include "../../util/Log.h"

LOG_USE_TAG("SPIBusMaster")


namespace MINTGGGameEngine
{

SPIBusMaster::SPIBusMaster (
    gpionum_t misoPin, gpionum_t mosiPin, gpionum_t sckPin,
    size_t maxTransferSize
)   : misoPin(misoPin), mosiPin(mosiPin), sckPin(sckPin), maxTransferSize(maxTransferSize), initialized(false)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (maxTransferSize > std::numeric_limits<int>::max()) {
        maxTransferSize = std::numeric_limits<int>::max();
    }
    espidf.host = SPI2_HOST;
#endif
}

SPIBusMaster::~SPIBusMaster()
{
    while (!devices.empty()) {
        removeDevice(devices.front());
    }
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (espidf.host) {
        spi_bus_free(espidf.host);
    }
#endif
}

bool SPIBusMaster::init()
{
    if (isInitialized()) {
        return true;
    }
    if (!isValid()) {
        return true;
    }

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    spi_bus_config_t busCfg = {
        .mosi_io_num = mosiPin,
        .miso_io_num = misoPin,
        .sclk_io_num = sckPin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = static_cast<int>(maxTransferSize),
        .flags = 0
    };

    esp_err_t res = spi_bus_initialize(espidf.host, &busCfg, SPI_DMA_CH_AUTO);
    if (res != ESP_OK) {
        LogError("Error initializing SPI bus: %s", esp_err_to_name(res));
        return false;
    }
#endif

    initialized = true;

    return true;
}

SPIBusDevice* SPIBusMaster::addDevice(gpionum_t csPin, uint32_t clockFreqHz, int flags)
{
    if (!isValid()) {
        return nullptr;
    }

    auto dev = new SPIBusDevice(this, csPin, clockFreqHz, flags);
    if (!dev->init()) {
        delete dev;
        return nullptr;
    }
    devices.push_back(dev);
    return dev;
}

void SPIBusMaster::removeDevice(SPIBusDevice* dev)
{
    if (!isValid()) {
        return;
    }

    devices.remove(dev);
    delete dev;
}

}

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

#include "SPIBusDevice.h"

#include "../../util/Log.h"
#include "SPIBusMaster.h"

LOG_USE_TAG("SPIBusDevice")


namespace MINTGGGameEngine
{

SPIBusDevice::SPIBusDevice(SPIBusMaster* master, gpionum_t csPin, uint32_t clockFreqHz, int flags)
    : master(master), csPin(csPin), clockFreqHz(clockFreqHz), flags(flags)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    espidf.dev = nullptr;
#endif
}

SPIBusDevice::~SPIBusDevice()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (espidf.dev) {
        spi_bus_remove_device(espidf.dev);
    }
#endif
}

bool SPIBusDevice::isValid() const
{
    return master->isValid();
}

bool SPIBusDevice::init()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    uint32_t devFlags = 0;
    if ((flags & FLAG_ESPIDF_NO_DUMMY) != 0) {
        devFlags |= SPI_DEVICE_NO_DUMMY;
    }
    spi_device_interface_config_t devCfg = {
        .mode = 0,
        .clock_speed_hz = static_cast<int>(clockFreqHz),
        .spics_io_num = csPin,
        .flags = devFlags,
        .queue_size = 8
    };
    esp_err_t res = spi_bus_add_device(master->getESPIDFHost(), &devCfg, &espidf.dev);
    if (res != ESP_OK) {
        setError(esp_err_to_name(res));
        return false;
    }
#endif

    resetError();
    return true;
}

bool SPIBusDevice::transmit(const uint8_t* data, size_t dataSize)
{
    if (!isValid()) {
        return false;
    }

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    spi_transaction_t trans = {
        .flags = 0,
        .length = dataSize * sizeof(uint8_t),
        .tx_buffer = data,
        .rx_buffer = nullptr
    };

    esp_err_t res = spi_device_polling_transmit(espidf.dev, &trans);
    if (res != ESP_OK) {
        setError(esp_err_to_name(res));
        return false;
    }

    resetError();
    return true;
#else
    setError("not supported on this platform");
    return false;
#endif
}

}

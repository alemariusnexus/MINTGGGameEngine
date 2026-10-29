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

#include "I2CBusMaster.h"

#include "../../util/Log.h"

LOG_USE_TAG("I2CBusMaster")


namespace MINTGGGameEngine
{

I2CBusMaster::I2CBusMaster(gpionum_t sclPin, gpionum_t sdaPin)
    : sclPin(sclPin), sdaPin(sdaPin), initialized(false)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    espidf.i2cBus = nullptr;
#endif
}

I2CBusMaster::~I2CBusMaster()
{
    while (!devices.empty()) {
        removeDevice(devices.front());
    }
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (espidf.i2cBus) {
        i2c_del_master_bus(espidf.i2cBus);
    }
#endif
}

bool I2CBusMaster::init()
{
    if (isInitialized()) {
        return true;
    }
    if (sclPin < 0  ||  sdaPin < 0) {
        return true;
    }

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    i2c_master_bus_config_t busCfg = {
        .i2c_port = -1,
        .sda_io_num = static_cast<gpio_num_t>(sdaPin),
        .scl_io_num = static_cast<gpio_num_t>(sclPin),
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7
    };
    esp_err_t err = i2c_new_master_bus(&busCfg, &espidf.i2cBus);
    if (err != ESP_OK) {
        LogError("Error setting up I2C master: %s", esp_err_to_name(err));
        return false;
    }
#endif

    initialized = true;

    return true;
}

I2CBusDevice* I2CBusMaster::addDevice(uint16_t address, uint32_t clockFreqHz, int flags)
{
    if (!isValid()) {
        return nullptr;
    }

    auto dev = new I2CBusDevice(this, address, clockFreqHz, flags);
    if (!dev->init()) {
        delete dev;
        return nullptr;
    }
    devices.push_back(dev);
    return dev;
}

void I2CBusMaster::removeDevice(I2CBusDevice* dev)
{
    if (!isValid()) {
        return;
    }

    devices.remove(dev);
    delete dev;
}

}

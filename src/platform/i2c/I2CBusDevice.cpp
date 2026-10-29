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

#include "I2CBusDevice.h"

#include "../../util/Log.h"
#include "I2CBusMaster.h"

LOG_USE_TAG("I2CBusDevice")


namespace MINTGGGameEngine
{

I2CBusDevice::I2CBusDevice(I2CBusMaster* master, uint16_t addr, uint32_t clockFreqHz, int flags)
    : master(master), address(addr), clockFreqHz(clockFreqHz), flags(flags)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    espidf.i2cDev = nullptr;
    espidf.lastErrCode = ESP_OK;
#endif
}

I2CBusDevice::~I2CBusDevice()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    if (espidf.i2cDev) {
        i2c_master_bus_rm_device(espidf.i2cDev);
    }
#endif
}

bool I2CBusDevice::init()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    i2c_device_config_t devCfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = clockFreqHz
    };
    if ((flags & FLAG_DISABLE_ACK_CHECK) != 0) {
        devCfg.flags.disable_ack_check = 1;
    }

    espidf.lastErrCode = i2c_master_bus_add_device(master->getESPIDFBusHandle(), &devCfg, &espidf.i2cDev);
    if (espidf.lastErrCode != ESP_OK) {
        LogError("Error setting up I2C device: %s", esp_err_to_name(espidf.lastErrCode));
        return false;
    }
#endif

    return true;
}

bool I2CBusDevice::isValid() const
{
    return master->isValid();
}

bool I2CBusDevice::transmit(const uint8_t* data, size_t dataSize, int timeoutMs)
{
    if (!isValid()) {
        return false;
    }
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    espidf.lastErrCode = i2c_master_transmit(espidf.i2cDev, data, dataSize, timeoutMs);
    if (espidf.lastErrCode != ESP_OK) {
        LogError("Error transmitting data: %s", esp_err_to_name(espidf.lastErrCode));
        return false;
    }
    return true;
#else
    return false;
#endif
}

bool I2CBusDevice::receive(uint8_t* data, size_t dataSize, int timeoutMs)
{
    if (!isValid()) {
        return false;
    }
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    espidf.lastErrCode = i2c_master_receive(espidf.i2cDev, data, dataSize, timeoutMs);
    if (espidf.lastErrCode != ESP_OK) {
        LogError("Error receiving data: %s", esp_err_to_name(espidf.lastErrCode));
        return false;
    }
    return true;
#else
    return false;
#endif
}

bool I2CBusDevice::transmitAndReceive(const uint8_t* txData, size_t txSize, uint8_t* rxData, size_t rxSize, int timeoutMs)
{
    if (!isValid()) {
        return false;
    }
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    espidf.lastErrCode = i2c_master_transmit_receive(espidf.i2cDev, txData, txSize, rxData, rxSize, timeoutMs);
    if (espidf.lastErrCode != ESP_OK) {
        LogError("Error transmitting and receiving data: %s", esp_err_to_name(espidf.lastErrCode));
        return false;
    }
    return true;
#else
    return false;
#endif
}

}

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
#   include <driver/i2c_master.h>
#endif


namespace MINTGGGameEngine
{

class I2CBusMaster;

class I2CBusDevice
{
    friend class I2CBusMaster;

public:
    enum Flags
    {
        FLAG_DISABLE_ACK_CHECK      = 0x01
    };

public:
    ~I2CBusDevice();

    I2CBusMaster* getMaster() { return master; }
    const I2CBusMaster* getMaster() const { return master; }

    bool isValid() const;

    bool transmit(const uint8_t* data, size_t dataSize, int timeoutMs);

    bool receive(uint8_t* data, size_t dataSize, int timeoutMs);

    bool transmitAndReceive(const uint8_t* txData, size_t txSize, uint8_t* rxData, size_t rxSize, int timeoutMs);

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    i2c_master_dev_handle_t getESPIDFDevHandle() const { return espidf.i2cDev; }
    esp_err_t getESPLastErrorCode() const { return espidf.lastErrCode; }
#endif

private:
    I2CBusDevice(I2CBusMaster* master, uint16_t addr, uint32_t clockFreqHz, int flags);

    bool init();

private:
    I2CBusMaster* master;

    uint16_t address;
    uint32_t clockFreqHz;
    int flags;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct {
        i2c_master_dev_handle_t i2cDev;
        esp_err_t lastErrCode;
    } espidf;
#endif
};

}

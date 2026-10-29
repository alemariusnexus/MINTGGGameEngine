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

#include "../Globals.h"

#include "i2c/I2CBusMaster.h"
#include "spi/SPIBusMaster.h"


namespace MINTGGGameEngine
{

typedef uint8_t i2cbus_id_t;
typedef uint8_t spibus_id_t;


class HardwareManager
{
public:
    struct I2CBusConfig
    {
        gpionum_t sclPin;
        gpionum_t sdaPin;
    };

    struct SPIBusConfig
    {
        gpionum_t misoPin;
        gpionum_t mosiPin;
        gpionum_t sckPin;
        size_t maxTransferSize;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
        struct {
            spi_host_device_t host;
        } espidf;
#endif
    };

private:
    struct I2CBusData
    {
        I2CBusConfig cfg;
        I2CBusMaster* master;
    };

    struct SPIBusData
    {
        SPIBusConfig cfg;
        SPIBusMaster* master;
    };

public:
    static HardwareManager& instance();

public:
    bool begin();
    void shutdown();

    I2CBusMaster* getI2CBus(i2cbus_id_t id = MGG_BUS_I2C_DEFAULT);
    SPIBusMaster* getSPIBus(spibus_id_t id = MGG_BUS_SPI_DEFAULT);

private:
    HardwareManager();


    bool initI2C();
    void shutdownI2C();

    bool initI2CBus(i2cbus_id_t id, const I2CBusConfig& cfg);
    void shutdownI2CBus(i2cbus_id_t id);


    bool initSPI();
    void shutdownSPI();

    bool initSPIBus(spibus_id_t id, const SPIBusConfig& cfg);
    void shutdownSPIBus(spibus_id_t id);

private:
    I2CBusData i2cBusData[MGG_BUS_I2C_MAX_BUSES];
    SPIBusData spiBusData[MGG_BUS_SPI_MAX_BUSES];
};

}

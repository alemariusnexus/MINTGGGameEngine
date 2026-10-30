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

#include "HardwareManager.h"

#include "../util/Log.h"
#include "i2c/I2CBusMaster.h"

LOG_USE_TAG("HardwareManager")


namespace MINTGGGameEngine
{

HardwareManager& HardwareManager::instance()
{
    static HardwareManager inst;
    return inst;
}

HardwareManager::HardwareManager()
{
}

bool HardwareManager::begin()
{
    if (!initI2C()) {
        return false;
    }
    if (!initSPI()) {
        return false;
    }

    return true;
}

void HardwareManager::shutdown()
{
    shutdownSPI();
    shutdownI2C();
}

bool HardwareManager::initI2C()
{
    return  initI2CBus(MGG_BUS_I2C_0, {.sclPin = MGG_BUS_I2C_0_PIN_SCL, .sdaPin = MGG_BUS_I2C_0_PIN_SDA})
        &&  initI2CBus(MGG_BUS_I2C_1, {.sclPin = MGG_BUS_I2C_0_PIN_SCL, .sdaPin = MGG_BUS_I2C_0_PIN_SDA})
        &&  initI2CBus(MGG_BUS_I2C_2, {.sclPin = MGG_BUS_I2C_0_PIN_SCL, .sdaPin = MGG_BUS_I2C_0_PIN_SDA})
        &&  initI2CBus(MGG_BUS_I2C_3, {.sclPin = MGG_BUS_I2C_0_PIN_SCL, .sdaPin = MGG_BUS_I2C_0_PIN_SDA});
}

void HardwareManager::shutdownI2C()
{
    shutdownI2CBus(MGG_BUS_I2C_3);
    shutdownI2CBus(MGG_BUS_I2C_2);
    shutdownI2CBus(MGG_BUS_I2C_1);
    shutdownI2CBus(MGG_BUS_I2C_0);
}

bool HardwareManager::initI2CBus(i2cbus_id_t id, const I2CBusConfig& cfg)
{
    i2cBusData[id].cfg = cfg;
    i2cBusData[id].master = new I2CBusMaster(cfg.sclPin, cfg.sdaPin);
    i2cBusData[id].master->init();
    if (i2cBusData[id].master->isValid()) {
        LogInfo("Initialized BUS_I2C_%u (scl: %d, sda: %d)", id, cfg.sclPin, cfg.sdaPin);
    }
    return true;
}

void HardwareManager::shutdownI2CBus(i2cbus_id_t id)
{
    delete i2cBusData[id].master;
    i2cBusData[id].master = nullptr;
}

bool HardwareManager::initSPI()
{
    return  initSPIBus(MGG_BUS_SPI_0,   {   .misoPin = MGG_BUS_SPI_0_PIN_MISO, .mosiPin = MGG_BUS_SPI_0_PIN_MOSI, .sckPin = MGG_BUS_SPI_0_PIN_SCK,
                                            .maxTransferSize = MGG_BUS_SPI_0_PIN_MAX_TRF_SIZE
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
                                            , .espidf = {.host = MGG_BUS_SPI_0_ESP_HOST}
#endif
                                        })

        &&  initSPIBus(MGG_BUS_SPI_1,   {   .misoPin = MGG_BUS_SPI_1_PIN_MISO, .mosiPin = MGG_BUS_SPI_1_PIN_MOSI, .sckPin = MGG_BUS_SPI_1_PIN_SCK,
                                            .maxTransferSize = MGG_BUS_SPI_1_PIN_MAX_TRF_SIZE
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
                                            , .espidf = {.host = MGG_BUS_SPI_1_ESP_HOST}
#endif
                                        })

        &&  initSPIBus(MGG_BUS_SPI_2,   {   .misoPin = MGG_BUS_SPI_2_PIN_MISO, .mosiPin = MGG_BUS_SPI_2_PIN_MOSI, .sckPin = MGG_BUS_SPI_2_PIN_SCK,
                                            .maxTransferSize = MGG_BUS_SPI_2_PIN_MAX_TRF_SIZE
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
                                            , .espidf = {.host = MGG_BUS_SPI_2_ESP_HOST}
#endif
                                        })

        &&  initSPIBus(MGG_BUS_SPI_3,   {   .misoPin = MGG_BUS_SPI_3_PIN_MISO, .mosiPin = MGG_BUS_SPI_3_PIN_MOSI, .sckPin = MGG_BUS_SPI_3_PIN_SCK,
                                            .maxTransferSize = MGG_BUS_SPI_3_PIN_MAX_TRF_SIZE
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
                                            , .espidf = {.host = MGG_BUS_SPI_3_ESP_HOST}
#endif
                                        });
}

void HardwareManager::shutdownSPI()
{
    shutdownI2CBus(MGG_BUS_SPI_3);
    shutdownI2CBus(MGG_BUS_SPI_2);
    shutdownI2CBus(MGG_BUS_SPI_1);
    shutdownI2CBus(MGG_BUS_SPI_0);
}

bool HardwareManager::initSPIBus(spibus_id_t id, const SPIBusConfig& cfg)
{
    spiBusData[id].cfg = cfg;
    spiBusData[id].master = new SPIBusMaster(cfg.misoPin, cfg.mosiPin, cfg.sckPin, cfg.maxTransferSize);
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    spiBusData[id].master->setESPIDFHost(cfg.espidf.host);
#endif
    if (!spiBusData[id].master->init()) {
        LogError("Error initializing BUS_SPI_%u: %s", id, spiBusData[id].master->getLastErrorMessage().c_str());
        return false;
    }
    if (spiBusData[id].master->isValid()) {
        LogInfo("Initialized BUS_SPI_%u (miso: %d, mosi: %d, sck: %d, maxTrfSize: %llu)",
            id,
            cfg.misoPin, cfg.mosiPin, cfg.sckPin, cfg.maxTransferSize
            );
    }
    return true;
}

void HardwareManager::shutdownSPIBus(spibus_id_t id)
{
    delete spiBusData[id].master;
    spiBusData[id].master = nullptr;
}

I2CBusMaster* HardwareManager::getI2CBus(i2cbus_id_t id)
{
    return i2cBusData[id].master;
}

SPIBusMaster* HardwareManager::getSPIBus(spibus_id_t id)
{
    return spiBusData[id].master;
}

}

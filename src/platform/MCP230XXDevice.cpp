/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "MCP230XXDevice.h"

#include "../util/Log.h"
#include "HardwareManager.h"


LOG_USE_TAG("MCP2300XDevice")


#define MCP230XX_REG_ADDR_IODIR     0x00
#define MCP230XX_REG_ADDR_IPOL      0x01
#define MCP230XX_REG_ADDR_GPINTEN   0x02
#define MCP230XX_REG_ADDR_DEFVAL    0x03
#define MCP230XX_REG_ADDR_INTCON    0x04
#define MCP230XX_REG_ADDR_IOCON     0x05
#define MCP230XX_REG_ADDR_GPPU      0x06
#define MCP230XX_REG_ADDR_INTF      0x07
#define MCP230XX_REG_ADDR_INTCAP    0x08
#define MCP230XX_REG_ADDR_GPIO      0x09
#define MCP230XX_REG_ADDR_OLAT      0x0A


namespace MINTGGGameEngine
{



MCP230XXDevice::MCP230XXDevice(I2CBusMaster* master, uint8_t widthBytes, uint32_t clockFreq, uint8_t i2cAddr)
    : master(master), dev(nullptr), widthBytes(widthBytes), clockFreq(clockFreq), i2cAddr(i2cAddr)
{
}

MCP230XXDevice::MCP230XXDevice(uint8_t widthBytes, uint32_t clockFreq, uint8_t i2cAddr)
    : master(nullptr), dev(nullptr), widthBytes(widthBytes), clockFreq(clockFreq), i2cAddr(i2cAddr)
{
}

void MCP230XXDevice::init()
{
    if (!master) {
        master = HardwareManager::instance().getI2CBus();
    }
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    dev = master->addDevice(i2cAddr, clockFreq);
    if (!dev) {
        LogError("Error initializing MCP230XX: error adding I2C device.");
    }
#endif
}

bool MCP230XXDevice::setIODirection(uint8_t portIdx, uint8_t ioDir)
{
    return writeRegister(portIdx, MCP230XX_REG_ADDR_IODIR, ioDir);
}

bool MCP230XXDevice::getIODirection(uint8_t portIdx, uint8_t* ioDir)
{
    return readRegister(portIdx, MCP230XX_REG_ADDR_IODIR, ioDir);
}

bool MCP230XXDevice::setPullupEnabled(uint8_t portIdx, uint8_t puEn)
{
    return writeRegister(portIdx, MCP230XX_REG_ADDR_GPPU, puEn);
}

bool MCP230XXDevice::getPullupEnabled(uint8_t portIdx, uint8_t* puEn)
{
    return readRegister(portIdx, MCP230XX_REG_ADDR_GPPU, puEn);
}

bool MCP230XXDevice::writePins(uint8_t portIdx, uint8_t pinStates)
{
    return writeRegister(portIdx, MCP230XX_REG_ADDR_OLAT, pinStates);
}

bool MCP230XXDevice::readPins(uint8_t portIdx, uint8_t* pinStates)
{
    return readRegister(portIdx, MCP230XX_REG_ADDR_GPIO, pinStates);
}

bool MCP230XXDevice::readRegister(uint8_t portIdx, uint8_t addr, uint8_t* value)
{
    if (!dev) {
        return false;
    }
    addr = addr*widthBytes + portIdx;
    return dev->transmitAndReceive(&addr, 1, value, 1, -1);
}

bool MCP230XXDevice::writeRegister(uint8_t portIdx, uint8_t addr, uint8_t value)
{
    if (!dev) {
        return false;
    }
    const uint8_t writeBuf[] = {static_cast<uint8_t>(addr*widthBytes + portIdx), value};
    return dev->transmit(writeBuf, sizeof(writeBuf), -1);
}


}

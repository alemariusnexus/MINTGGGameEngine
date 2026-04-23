#include "MCP2300XDevice.h"

#include "../util/Log.h"


LOG_USE_TAG("MCP2300XDevice")


#define MCP2300X_REG_ADDR_IODIR     0x00
#define MCP2300X_REG_ADDR_IPOL      0x01
#define MCP2300X_REG_ADDR_GPINTEN   0x02
#define MCP2300X_REG_ADDR_DEFVAL    0x03
#define MCP2300X_REG_ADDR_INTCON    0x04
#define MCP2300X_REG_ADDR_IOCON     0x05
#define MCP2300X_REG_ADDR_GPPU      0x06
#define MCP2300X_REG_ADDR_INTF      0x07
#define MCP2300X_REG_ADDR_INTCAP    0x08
#define MCP2300X_REG_ADDR_GPIO      0x09
#define MCP2300X_REG_ADDR_OLAT      0x0A


namespace MINTGGGameEngine
{


#ifdef ARDUINO
    // TODO: Implement
#elif defined(ESP_PLATFORM)
MCP2300XDevice::MCP2300XDevice(i2c_master_bus_handle_t bus, i2c_master_dev_handle_t dev)
    : i2cBus(bus), i2cDev(dev)
{
}

MCP2300XDevice::MCP2300XDevice(gpio_num_t sclPin, gpio_num_t sdaPin, uint32_t clockFreq, uint8_t i2cAddr)
{
    i2c_master_bus_config_t busCfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = sdaPin,
        .scl_io_num = sclPin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7
    };
    if (i2c_new_master_bus(&busCfg, &i2cBus) != ESP_OK) {
        LogError(TAG, "Error setting up I2C master.");
        return;
    }

    i2c_device_config_t devCfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = i2cAddr,
        .scl_speed_hz = clockFreq
    };

    if (i2c_master_bus_add_device(i2cBus, &devCfg, &i2cDev) != ESP_OK) {
        LogError(TAG, "Error setting up MCP2300X.");
        return;
    }
}
#endif

bool MCP2300XDevice::setIODirection(uint8_t ioDir)
{
    return writeRegister(MCP2300X_REG_ADDR_IODIR, ioDir);
}

bool MCP2300XDevice::getIODirection(uint8_t* ioDir)
{
    return readRegister(MCP2300X_REG_ADDR_IODIR, ioDir);
}

bool MCP2300XDevice::setPullupEnabled(uint8_t puEn)
{
    return writeRegister(MCP2300X_REG_ADDR_GPPU, puEn);
}

bool MCP2300XDevice::getPullupEnabled(uint8_t* puEn)
{
    return readRegister(MCP2300X_REG_ADDR_GPPU, puEn);
}

bool MCP2300XDevice::writePins(uint8_t pinStates)
{
    return writeRegister(MCP2300X_REG_ADDR_OLAT, pinStates);
}

bool MCP2300XDevice::readPins(uint8_t* pinStates)
{
    return readRegister(MCP2300X_REG_ADDR_GPIO, pinStates);
}

bool MCP2300XDevice::readRegister(uint8_t addr, uint8_t* value)
{
#ifdef ARDUINO
    // TODO: Implement
    return false;
#elif defined(ESP_PLATFORM)
    if (i2c_master_transmit_receive(i2cDev, &addr, 1, value, 1, -1) != ESP_OK) {
        return false;
    }
    return true;
#endif
}

bool MCP2300XDevice::writeRegister(uint8_t addr, uint8_t value)
{
#ifdef ARDUINO
    // TODO: Implement
    return false;
#elif defined(ESP_PLATFORM)
    uint8_t writeBuf[] = {addr, value};
    if (i2c_master_transmit(i2cDev, writeBuf, sizeof(writeBuf), -1) != ESP_OK) {
        return false;
    }
    return true;
#endif
}
}

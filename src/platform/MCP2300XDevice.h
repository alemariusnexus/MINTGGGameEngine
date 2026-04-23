#pragma once

#ifdef ARDUINO
// TODO: Implement
#else
#   include <driver/i2c_master.h>
#endif


namespace MINTGGGameEngine
{


class MCP2300XDevice {
public:
#ifdef ARDUINO
    // TODO: Implement
#elif defined(ESP_PLATFORM)
    MCP2300XDevice(i2c_master_bus_handle_t bus, i2c_master_dev_handle_t dev);
    MCP2300XDevice(gpio_num_t sclPin, gpio_num_t sdaPin, uint32_t clockFreq = 400000, uint8_t i2cAddr = 0x20);
#endif

    bool setIODirection(uint8_t ioDir);
    bool getIODirection(uint8_t* ioDir);

    bool setPullupEnabled(uint8_t puEn);
    bool getPullupEnabled(uint8_t* puEn);

    bool readPins(uint8_t* pinStates);
    bool writePins(uint8_t pinStates);

private:
    bool readRegister(uint8_t addr, uint8_t* value);
    bool writeRegister(uint8_t addr, uint8_t value);

private:
#ifdef ARDUINO

#elif defined(ESP_PLATFORM)
    i2c_master_bus_handle_t i2cBus;
    i2c_master_dev_handle_t i2cDev;
#endif
};


}

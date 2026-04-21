#pragma once

#include "Globals.h"

#include <unordered_map>

#ifdef ARDUINO
#include <Wire.h>
#endif

#include "GPIODevice.h"
#include "MCP2300XDevice.h"


namespace MINTGGGameEngine
{


/**
 * \brief A GPIODevice that interfaces to an external MCP2300X I2C IO expander.
 *
 * Don't forget to call begin() at startup!
 *
 * For reading multiple pins at one, it is recommended to use readPins(), since
 * calling readPin() multiple times results in separate I2C messages being sent
 * for each pin.
 *
 * Note that the MCP2300X supports pullups, but no pulldowns, so pin modes of
 * PULLDOWN or INPUT_PULLDOWN will not work.
 */
class GPIODeviceMCP2300X : public GPIODevice
{
public:
#ifdef ARDUINO
	/**
	 * \brief Create a new MCP2300X instance.
	 *
	 * \param i2cAddr The I2C slave address of the device. 0x20 is the default for when
	 * 		the ADDR pins of the MCP2300X are all connected to GND.
	 * \param wire The Arduino Wire instance to use for the I2C bus.
	 */
	GPIODeviceMCP2300X(uint8_t i2cAddr = 0x20, TwoWire* wire = &Wire);
#elif defined(ESP_PLATFORM)
    GPIODeviceMCP2300X(gpio_num_t sclPin, gpio_num_t sdaPin, uint32_t clockFreq = 400000, uint8_t i2cAddr = 0x20);
    GPIODeviceMCP2300X(i2c_master_bus_handle_t bus, i2c_master_dev_handle_t dev);
#endif
	
	/**
	 * \brief Initializes the MCP2300X.
	 */
	void begin();

	bool setPinMode(unsigned int pin, uint8_t mode) override;

    uint8_t readPin(unsigned int pin) override;
	void writePin(unsigned int pin, uint8_t val) override;
	
	void readPins(const unsigned int* pins, uint8_t* values, size_t numPins) override;
	
private:
	MCP2300XDevice mcp;
};


}

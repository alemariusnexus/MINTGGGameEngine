#include "GPIODeviceMCP2300X.h"

#include "Log.h"
#include "Util.h"


namespace MINTGGGameEngine
{


#ifdef ARDUINO

GPIODeviceMCP2300X::GPIODeviceMCP2300X(uint8_t i2cAddr, TwoWire* wire)
		: mcp(i2cAddr, wire)
{
}

#elif defined(ESP_PLATFORM)

GPIODeviceMCP2300X::GPIODeviceMCP2300X(gpio_num_t sclPin, gpio_num_t sdaPin, uint32_t clockFreq, uint8_t i2cAddr)
    : mcp(sclPin, sdaPin, clockFreq, i2cAddr)
{
}

GPIODeviceMCP2300X::GPIODeviceMCP2300X(i2c_master_bus_handle_t bus, i2c_master_dev_handle_t dev)
    : mcp(bus, dev)
{
}

#endif

void GPIODeviceMCP2300X::begin()
{
}

bool GPIODeviceMCP2300X::setPinMode(unsigned int pin, uint8_t mode)
{
    bool output, puEnabled, pdEnabled;
    ExtractArduinoPinMode(mode, &output, &puEnabled, &pdEnabled);

    if (pdEnabled) {
        // MCP2300X doesn't support pulldowns
        return false;
    }

    uint8_t oldIODir;
    uint8_t oldPUState;

    if (!mcp.getIODirection(&oldIODir)) {
        return false;
    }
    if (!mcp.getPullupEnabled(&oldPUState)) {
        return false;
    }

    uint8_t newIODir = oldIODir;
    uint8_t newPUState = oldPUState;

    if (output) {
        newIODir &= ~(1 << pin);
    } else {
        newIODir |= (1 << pin);
    }

    if (puEnabled) {
        newPUState |= (1 << pin);
    } else {
        newPUState &= ~(1 << pin);
    }

    if (newIODir != oldIODir) {
        if (!mcp.setIODirection(newIODir)) {
            return false;
        }
    }
    if (newPUState != oldPUState) {
        if (!mcp.setPullupEnabled(newPUState)) {
            return false;
        }
    }

	return true;
}

uint8_t GPIODeviceMCP2300X::readPin(unsigned int pin)
{
    uint8_t state;
    if (!mcp.readPins(&state)) {
        return 0;
    }
	return (state & (1 << pin)) != 0;
}

void GPIODeviceMCP2300X::writePin(unsigned int pin, uint8_t val)
{
    uint8_t newState;
    if (!mcp.readPins(&newState)) {
        return;
    }
	if (val) {
	    newState |= (1 << pin);
	} else {
	    newState &= ~(1 << pin);
	}
    mcp.writePins(newState);
}

void GPIODeviceMCP2300X::readPins(const unsigned int* pins, uint8_t* values, size_t numPins)
{
    uint8_t val8;
    if (!mcp.readPins(&val8)) {
        return;
    }
	for (size_t i = 0 ; i < numPins ; i++) {
		if (pins[i] < 8) {
			values[i] = ((val8 >> pins[i]) & 0x01) ? HIGH : LOW;
		} else {
			values[i] = LOW;
		}
	}
}


}

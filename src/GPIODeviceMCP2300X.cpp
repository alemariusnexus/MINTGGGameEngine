#include "GPIODeviceMCP2300X.h"


namespace MINTGGGameEngine
{


GPIODeviceMCP2300X::GPIODeviceMCP2300X(uint8_t i2cAddr, TwoWire* wire)
		: mcp(i2cAddr, wire)
{
}

void GPIODeviceMCP2300X::begin()
{
	mcp.begin(false);
}

bool GPIODeviceMCP2300X::setPinMode(unsigned int pin, uint8_t mode)
{
	if (!mcp.pinMode1(pin, mode)) {
		return false;
	}
	
	// Whose idea was it not to include pullup handling in pinMode1()???
	if (mode == INPUT_PULLDOWN  ||  mode == PULLDOWN) {
		// MCP2300X doesn't support pulldowns
		return false;
	} else if (mode == INPUT_PULLUP  ||  mode == PULLUP) {
		return mcp.setPullup(pin, true);
	} else {
		return mcp.setPullup(pin, false);
	}
}

uint8_t GPIODeviceMCP2300X::readPin(unsigned int pin)
{
	return mcp.read1(pin);
}

void GPIODeviceMCP2300X::writePin(unsigned int pin, uint8_t val)
{
	mcp.write1(pin, val);
}

void GPIODeviceMCP2300X::readPins(const unsigned int* pins, uint8_t* values, size_t numPins)
{
	uint8_t val8 = mcp.read8();
	for (size_t i = 0 ; i < numPins ; i++) {
		if (pins[i] < 8) {
			values[i] = ((val8 >> pins[i]) & 0x01) ? HIGH : LOW;
		} else {
			values[i] = false;
		}
	}
}


}

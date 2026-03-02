#include "GPIODeviceNative.h"


namespace MINTGGGameEngine
{


GPIODeviceNative& GPIODeviceNative::getInstance()
{
	static GPIODeviceNative inst;
	return inst;
}


bool GPIODeviceNative::setPinMode(unsigned int pin, uint8_t mode)
{
	::pinMode(static_cast<uint8_t>(pin), mode);
	return true;
}

uint8_t GPIODeviceNative::readPin(unsigned int pin)
{
	return ::digitalRead(pin);
}

void GPIODeviceNative::writePin(unsigned int pin, uint8_t val)
{
	::digitalWrite(pin, val);
}


}

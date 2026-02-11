#include "GPIODevice.h"


namespace MINTGGGameEngine
{


void GPIODevice::readPins(const unsigned int* pins, uint8_t* values, size_t numPins)
{
	for (size_t i = 0 ; i < numPins ; i++) {
		values[i] = readPin(pins[i]);
	}
}


}

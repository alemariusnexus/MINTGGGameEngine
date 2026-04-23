#pragma once

#include "../Globals.h"

#include "GPIODevice.h"


namespace MINTGGGameEngine
{


/**
 * \brief A GPIODevice that uses the microcontroller's internal GPIO pins.
 *
 * The calls of this class mostly just redirect to standard Arduino functions like
 * pinMode(), digitalWrite() and digitalRead().
 *
 * Use getInstance() to get the single global object of this class.
 */
class GPIODeviceNative : public GPIODevice
{
public:
	static GPIODeviceNative& getInstance();

public:
	bool setPinMode(unsigned int pin, uint8_t mode) override;

    uint8_t readPin(unsigned int pin) override;
	void writePin(unsigned int pin, uint8_t val) override;
};


}

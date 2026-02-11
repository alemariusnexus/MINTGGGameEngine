#pragma once

#include "Globals.h"


namespace MINTGGGameEngine
{


/**
 * \brief Represents a device that is capable of controlling a set of
 *		General Purpose Input/Output (GPIO) pins.
 *
 * This is the base class for such devices, and can be used to write code that
 * works for any such type of device, whether built into the microcontroller or
 * external (e.g. an IO expander).
 *
 * Currently, two subclasses exist:
 *
 *	* GPIODeviceNative: Handles the built-in GPIO pins of the microcontroller.
 *	* GPIODeviceMCP2300X: Handles GPIOs of an external MCP2300X IO expander.
 *
 * \see GPIODeviceNative
 * \see GPIODeviceMCP2300X
 */
class GPIODevice
{
public:
	/**
	 * \brief Set the mode of a single pin, similar to Arduino's pinMode().
	 *
	 * Note that not all subclasses may implement all pin modes.
	 *
	 * \param pin The GPIO pin number.
	 * \param mode The pin mode. Takes the same values as Arduino's pinMode() function.
	 * \return true if successful, false otherwise (e.g. if the mode isn't supported).
	 */
	virtual bool setPinMode(unsigned int pin, uint8_t mode) = 0;
	
	/**
	 * \brief An alias for setPinMode(), for Arduino compatibility.
	 *
	 * \see setPinMode()
	 */
	bool pinMode(unsigned int pin, uint8_t mode) { return setPinMode(pin, mode); }

	
	/**
	 * \brief Read the digital value of a single pin, similar to Arduino's digitalRead().
	 *
	 * For reading multiple pin values of the same device, for some subclasses
	 * (especially external IO expanders) readPins() can be much faster.
	 *
	 * \param pin The GPIO pin number.
	 * \return HIGH or LOW, depending on the pin's state.
	 * \see readPins()
	 */
    virtual uint8_t readPin(unsigned int pin) = 0;
	
	/**
	 * \brief An alias for readPin(), for Arduino compatibility.
	 *
	 * \see readPin()
	 */
	uint8_t digitalRead(unsigned int pin) { return readPin(pin); }
	
	/**
	 * \brief Set the digital value of a single pin, similar to Arduino's digitalWrite().
	 *
	 * \param pin The GPIO pin number.
	 * \param val The new pin state, HIGH or LOW.
	 */
	virtual void writePin(unsigned int pin, uint8_t val) = 0;
	
	/**
	 * \brief An alias for writePin(), for Arduino compatibility.
	 *
	 * \see writePin()
	 */
	void digitalWrite(unsigned int pin, uint8_t value) { return writePin(pin, value); }
	
	
	/**
	 * \brief Read the digital value of multiple GPIO pins.
	 *
	 * This can be faster than calling readPin() repeatedly for external devices that
	 * support setting multiple pins in one combined request.
	 *
	 * \param pins Array of GPIO pin numbers.
	 * \param values Array of pin values that were read (output parameter).
	 * \param numPins Number of pins to read (size of the arrays)
	 */
	virtual void readPins(const unsigned int* pins, uint8_t* values, size_t numPins);
};


}

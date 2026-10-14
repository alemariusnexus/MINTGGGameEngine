#include "GPIODeviceNative.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <driver/gpio.h>
#endif

#include "../util/Util.h"


namespace MINTGGGameEngine
{


GPIODeviceNative& GPIODeviceNative::getInstance()
{
	static GPIODeviceNative inst;
	return inst;
}


bool GPIODeviceNative::setPinMode(gpionum_t pin, uint8_t mode)
{
    if (pin < 0) {
        return false;
    }
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
	::pinMode(static_cast<uint8_t>(pin), mode);
    return true;
#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)
    bool output, puEnabled, pdEnabled;
    ExtractArduinoPinMode(mode, &output, &puEnabled, &pdEnabled);
    if (output) {
        gpio_set_direction(static_cast<gpio_num_t>(pin), GPIO_MODE_OUTPUT);
    } else {
        gpio_set_direction(static_cast<gpio_num_t>(pin), GPIO_MODE_INPUT);
    }
    if (puEnabled) {
        if (pdEnabled) {
            gpio_set_pull_mode(static_cast<gpio_num_t>(pin), GPIO_PULLUP_PULLDOWN);
        } else {
            gpio_set_pull_mode(static_cast<gpio_num_t>(pin), GPIO_PULLUP_ONLY);
        }
    } else if (pdEnabled) {
        gpio_set_pull_mode(static_cast<gpio_num_t>(pin), GPIO_PULLDOWN_ONLY);
    } else {
        gpio_set_pull_mode(static_cast<gpio_num_t>(pin), GPIO_FLOATING);
    }
    return true;
#else
    return false;
#endif
}

uint8_t GPIODeviceNative::readPin(gpionum_t pin)
{
    if (pin < 0) {
        return 0;
    }
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
	return ::digitalRead(pin);
#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)
    return gpio_get_level(static_cast<gpio_num_t>(pin));
#else
    return 0;
#endif
}

void GPIODeviceNative::writePin(gpionum_t pin, uint8_t val)
{
    if (pin < 0) {
        return;
    }
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    ::digitalWrite(pin, val);
#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)
	gpio_set_level(static_cast<gpio_num_t>(pin), val);
#else
    // Nothing to do (not supported)
#endif
}


}

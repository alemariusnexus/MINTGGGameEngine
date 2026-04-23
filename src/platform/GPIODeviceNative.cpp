#include "GPIODeviceNative.h"

#include <driver/gpio.h>

#include "../util/Util.h"


namespace MINTGGGameEngine
{


GPIODeviceNative& GPIODeviceNative::getInstance()
{
	static GPIODeviceNative inst;
	return inst;
}


bool GPIODeviceNative::setPinMode(unsigned int pin, uint8_t mode)
{
#ifdef ARDUINO
	::pinMode(static_cast<uint8_t>(pin), mode);
    return true;
#elif defined(ESP_PLATFORM)
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
#endif
}

uint8_t GPIODeviceNative::readPin(unsigned int pin)
{
#ifdef ARDUINO
	return ::digitalRead(pin);
#elif defined(ESP_PLATFORM)
    return gpio_get_level(static_cast<gpio_num_t>(pin));
#endif
}

void GPIODeviceNative::writePin(unsigned int pin, uint8_t val)
{
#ifdef ARDUINO
    ::digitalWrite(pin, val);
#elif defined(ESP_PLATFORM)
	gpio_set_level(static_cast<gpio_num_t>(pin), val);
#endif
}


}

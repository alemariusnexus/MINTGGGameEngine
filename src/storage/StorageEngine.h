#pragma once

#include "../Globals.h"

#include <string>

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#include <hal/spi_types.h>
#endif


namespace MINTGGGameEngine
{


class StorageEngine
{
public:
    StorageEngine();

    bool begin();

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    bool mountSDCard (
        const char* mountPoint = "/sdcard",
        spi_host_device_t spiHost = SPI2_HOST,
        int csPin = -1,
        uint32_t clkFreq = 20000000
        );
//#elif defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    bool mountSDCard (
        const char* mountPoint = "/sdcard"
        );

    bool checkSDFilePath(const std::string& path, std::string* outRelPath);
#endif

private:
    std::string sdMountPath;
};


}

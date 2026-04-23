#pragma once
#include "hal/spi_types.h"


namespace MINTGGGameEngine
{


class StorageEngine
{
public:
    StorageEngine();

    bool begin();

    bool mountSDCard (
        const char* mountPoint = "/sdcard",
        spi_host_device_t spiHost = SPI2_HOST,
        int csPin = -1,
        uint32_t clkFreq = 20000000
        );
};


}

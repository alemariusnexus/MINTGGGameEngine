#pragma once

#include "../../../Globals.h"

#include <driver/spi_master.h>

#include "AbstractMIPIScreen.h"


namespace MINTGGGameEngine
{

/**
 * \brief Driver for an ST7735-based screen.
 *
 * This class currently assumes that the ST7735 is connected using the SPI interface. The ST7735 supports other
 * interfaces, but they are not handled by this class.
 * A hardware SPI peripheral is used to communicate with the screen.
 */
class ScreenST7735 : public AbstractMIPIScreen
{
public:
    struct Config
    {
        uint16_t width;
        uint16_t height;

        uint16_t offsetX;
        uint16_t offsetY;

        uint8_t* staticBuffer;

        bool mirrorX;
        bool mirrorY;
        bool swapXY;
        bool bgrColors;

        uint32_t clockFreqHz;

        spi_host_device_t spiHost;
        size_t spiMaxTransferSize;

        struct
        {
            gpionum_t cs;
            gpionum_t dc;
            gpionum_t rst;
        } pins;
    };

public:
    static void initConfig(Config& cfg);

public:
    ScreenST7735(const Config& cfg);
    virtual ~ScreenST7735();
    
    void commit() override;

protected:
    bool mipiInitBus() override;
    bool mipiWriteCommandPlusData(uint8_t command, const uint8_t* data, size_t length) override;

    bool doHWReset() override;

private:
    Config cfg;

    spi_device_handle_t spiDev;
};

}

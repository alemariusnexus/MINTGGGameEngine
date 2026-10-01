#pragma once

#include "../../../Globals.h"

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <driver/spi_master.h>
#   include <esp_lcd_panel_io.h>
#   include <esp_lcd_io_i80.h>
#endif

#include "AbstractMIPIScreen.h"


namespace MINTGGGameEngine
{

/**
 * \brief Driver for an ILI9341-based screen.
 *
 * This class currently assumes that the ILI9341 is connected using the Intel 8080 parallel interface with 8 DAT lines
 * and 16 bits per pixel. The ILI9341 supports other interfaces and configurations, but they are not handled by this
 * class.
 * A hardware peripheral is used to communicate with the display if available. Note that different ESP32 chips may use
 * different peripherals for this that may be faster or slower (some chips have a dedicated LCD controller peripheral,
 * while some abuse a special mode in their I2S peripheral. This is hidden behind a driver abstraction of ESP-IDF).
 */
class ScreenILI9341 : public AbstractMIPIScreen
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

        struct
        {
            gpionum_t cs;
            gpionum_t dc;
            gpionum_t rst;
            gpionum_t wr;
            gpionum_t rd;
            gpionum_t dat[8];
        } pins;
    };

public:
    static void initConfig(Config& cfg);

public:
    ScreenILI9341(const Config& cfg);
    virtual ~ScreenILI9341();
    
    void commit() override;

protected:
    bool mipiInitBus() override;
    bool mipiWriteCommandPlusData(uint8_t command, const uint8_t* data, size_t length) override;

    bool doHWReset() override;

private:
    Config cfg;

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    esp_lcd_i80_bus_handle_t busHandle;
    esp_lcd_panel_io_handle_t ioHandle;
#endif
};

}

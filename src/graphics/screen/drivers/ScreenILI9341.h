#pragma once

#include "../../../Globals.h"

#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_io_i80.h>

#include "AbstractMIPIScreen.h"


namespace MINTGGGameEngine
{

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
            int cs;
            int dc;
            int rst;
            int wr;
            int rd;
            int dat[8];
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

    esp_lcd_i80_bus_handle_t busHandle;
    esp_lcd_panel_io_handle_t ioHandle;
};

}

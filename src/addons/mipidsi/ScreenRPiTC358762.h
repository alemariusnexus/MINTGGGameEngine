#pragma once

#include "../../Globals.h"


#if MGG_ENABLE_COPYLEFT_ADDONS

#include "../../core/Engine.h"
#include "../../graphics/screen/drivers/mipidsi/AbstractMIPIDSIScreen.h"

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
#   include <esp_lcd_io_i2c.h>
#   include <esp_lcd_mipi_dsi.h>
#   include <esp_ldo_regulator.h>
#   include <hal/mipi_dsi_hal.h>
#endif


namespace MINTGGGameEngine
{

class ScreenRPiTC358762 : public AbstractMIPIDSIScreen
{
public:
    struct Config
    {
        AbstractMIPIDSIScreen::Config dsi;
    };

public:
    static void initConfig(Config& cfg);

public:
    ScreenRPiTC358762(const Config& cfg);

protected:
    bool initPanel(uint8_t numFramebufs) override;
    void shutdownPanel() override;

    PlatformData& getPlatformData() override { return plfData; }

private:
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    bool initLDO();
    void shutdownLDO();

    bool initI2C();
    void shutdownI2C();

    bool initDSIDBI();
    void shutdownDSIDBI();

    bool initLCDDriver(uint8_t numFramebufs);
    void shutdownLCDDriver();

    esp_err_t lcdPanelReset(esp_lcd_panel_t* panel);
    esp_err_t lcdPanelInit(esp_lcd_panel_t* panel);
    esp_err_t lcdPanelDel(esp_lcd_panel_t* panel);
    esp_err_t lcdPanelDispOnOff(esp_lcd_panel_t* panel, bool onOff);

    esp_err_t attinyWrite(uint8_t addr, uint8_t value);
    esp_err_t attinyRead(uint8_t addr, uint8_t* value);
    void bridgeWrite(uint16_t addr, uint32_t value);
#endif

private:
    Config cfg;

    PlatformData plfData;

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    esp_ldo_channel_handle_t ldoChanHandle;
    i2c_master_bus_handle_t i2cBusHandle;
    i2c_master_dev_handle_t i2cAttinyHandle;
    esp_lcd_dsi_bus_handle_t dsiBusHandle;
    esp_lcd_panel_io_handle_t panelIOHandle;
    int i2cAttinyTimeout;
    uint8_t dbiVirtualChannel;

    mipi_dsi_hal_context_t* dsiHALCtx;

    esp_err_t (*panelOrigInit)(esp_lcd_panel_t*);
    esp_err_t (*panelOrigDel)(esp_lcd_panel_t*);
#endif
};

} // MINTGGGameEngine

#endif

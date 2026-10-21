/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "../../../../Globals.h"

#include "AbstractMIPIDSIScreen.h"

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
#   include <driver/i2c_master.h>
#   include <esp_lcd_io_i2c.h>
#   include <esp_lcd_mipi_dsi.h>
#   include <esp_ldo_regulator.h>
#   include "../../../../3rdparty/esp_lcd_st7701/esp_lcd_st7701.h"
#endif


namespace MINTGGGameEngine
{

class ScreenST7701 : public AbstractMIPIDSIScreen
{
public:
    struct Config
    {
        AbstractMIPIDSIScreen::Config dsi;
    };

public:
    static void initConfig(Config& cfg, bool useMainScreenConfig = false);

public:
    ScreenST7701(const Config& cfg);

protected:
    bool initPanel(uint8_t numFramebufs) override;
    void shutdownPanel() override;

    PlatformData& getPlatformData() override { return plfData; }

    virtual bool initCustomPanelHW() = 0;
    virtual void shutdownCustomPanelHW() = 0;

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    virtual size_t getCustomPanelInitSequence(const st7701_lcd_init_cmd_t** outInitCmds) = 0;
#endif

    bool supportsHardwareTransforms(bool mirrorX, bool mirrorY, bool swapXY) const override;

private:
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    bool initLDO();
    void shutdownLDO();

    bool initDSIDBI();
    void shutdownDSIDBI();

    bool initLCDDriver(uint8_t numFramebufs);
    void shutdownLCDDriver();
#endif

private:
    Config cfg;

    PlatformData plfData;

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    esp_ldo_channel_handle_t ldoChanHandle;
    esp_lcd_dsi_bus_handle_t dsiBusHandle;
    esp_lcd_panel_io_handle_t panelIOHandle;
#endif
};

} // MINTGGGameEngine

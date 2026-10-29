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

#include "../../../../platform/i2c/I2CBusDevice.h"
#include "ScreenST7701.h"


namespace MINTGGGameEngine
{

class ScreenWaveshare4_3_DSI_TOUCH_A : public ScreenST7701
{
public:
    struct Config
    {
        ScreenST7701::Config st7701;
        I2CBusMaster* i2cBusMaster;
    };

public:
    static void initConfig(Config& cfg, bool useMainScreenConfig = false);

public:
    ScreenWaveshare4_3_DSI_TOUCH_A(const Config& cfg);

    std::string getModelID() override { return "waveshare_4_3_dsi_touch_a"; }

protected:
    bool initCustomPanelHW() override;
    void shutdownCustomPanelHW() override;

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    size_t getCustomPanelInitSequence(const st7701_lcd_init_cmd_t** outInitCmds) override;
#endif

private:
    Config cfg;

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    I2CBusDevice* i2cDev;
#endif
};

}

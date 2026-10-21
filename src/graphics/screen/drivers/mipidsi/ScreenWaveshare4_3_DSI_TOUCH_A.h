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

#include "ScreenST7701.h"

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
#   include <driver/i2c_master.h>
#endif


namespace MINTGGGameEngine
{

class ScreenWaveshare4_3_DSI_TOUCH_A : public ScreenST7701
{
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
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    i2c_master_bus_handle_t i2cBusHandle;
    i2c_master_dev_handle_t i2cDevHandle;
#endif
};

}

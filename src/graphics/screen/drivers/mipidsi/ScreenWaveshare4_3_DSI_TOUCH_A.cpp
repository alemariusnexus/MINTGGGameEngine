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

#include "ScreenWaveshare4_3_DSI_TOUCH_A.h"

#include "../../../../util/Log.h"

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
#   include "../../../../3rdparty/esp_lcd_st7701/esp_lcd_st7701.h"
#endif

LOG_USE_TAG("ScreenWaveshare4_3_DSI_TOUCH_A")

namespace MINTGGGameEngine
{

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
static const st7701_lcd_init_cmd_t DSIInitSequence[] = {
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x13}, 5, 0},
    {0xEF, (uint8_t[]){0x08}, 1, 0},
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x10}, 5, 0},
    {0xC0, (uint8_t[]){0x63, 0x00}, 2, 0},
    {0xC1, (uint8_t[]){0x0D, 0x02}, 2, 0},
    {0xC2, (uint8_t[]){0x17, 0x08}, 2, 0},
    {0xCC, (uint8_t[]){0x10}, 1, 0},
    {0xB0, (uint8_t[]){0x40, 0xC9, 0x94, 0x0E, 0x10, 0x05, 0x0B, 0x09, 0x08, 0x26, 0x04, 0x52, 0x10, 0x69, 0x6B, 0x69}, 16, 0},

    {0xB1, (uint8_t[]){0x40, 0xD2, 0x98, 0x0C, 0x92, 0x07, 0x09, 0x08, 0x07, 0x25, 0x02, 0x0E, 0x0C, 0x6E, 0x78, 0x55}, 16, 0},

    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x11}, 5, 0},
    {0xB0, (uint8_t[]){0x5D}, 1, 0},
    {0xB1, (uint8_t[]){0x4E}, 1, 0},
    {0xB2, (uint8_t[]){0x87}, 1, 0},
    {0xB3, (uint8_t[]){0x80}, 1, 0},
    {0xB5, (uint8_t[]){0x4E}, 1, 0},
    {0xB7, (uint8_t[]){0x85}, 1, 0},
    {0xB8, (uint8_t[]){0x21}, 1, 0},
    {0xB9, (uint8_t[]){0x10, 0x1F}, 2, 0},
    {0xBB, (uint8_t[]){0x03}, 1, 0},
    {0xBC, (uint8_t[]){0x00}, 1, 0},

    {0xC1, (uint8_t[]){0x78}, 1, 0},
    {0xC2, (uint8_t[]){0x78}, 1, 0},
    {0xD0, (uint8_t[]){0x88}, 1, 0},
    {0xE0, (uint8_t[]){0x00, 0x3A, 0x02}, 3, 0},
    {0xE1, (uint8_t[]){0x04, 0xA0, 0x00, 0xA0, 0x05, 0xA0, 0x00, 0xA0, 0x00, 0x40, 0x40}, 11, 0},

    {0xE2, (uint8_t[]){0x30, 0x00, 0x40, 0x40, 0x32, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00}, 13, 0},

    {0xE3, (uint8_t[]){0x00, 0x00, 0x33, 0x33}, 4, 0},
    {0xE4, (uint8_t[]){0x44, 0x44}, 2, 0},
    {0xE5, (uint8_t[]){0x09, 0x2E, 0xA0, 0xA0, 0x0B, 0x30, 0xA0, 0xA0, 0x05, 0x2A, 0xA0, 0xA0, 0x07, 0x2C, 0xA0, 0xA0}, 16, 0},

    {0xE6, (uint8_t[]){0x00, 0x00, 0x33, 0x33}, 4, 0},
    {0xE7, (uint8_t[]){0x44, 0x44}, 2, 0},
    {0xE8, (uint8_t[]){0x08, 0x2D, 0xA0, 0xA0, 0x0A, 0x2F, 0xA0, 0xA0, 0x04, 0x29, 0xA0, 0xA0, 0x06, 0x2B, 0xA0, 0xA0}, 16, 0},

    {0xEB, (uint8_t[]){0x00, 0x00, 0x4E, 0x4E, 0x00, 0x00, 0x00}, 7, 0},
    {0xEC, (uint8_t[]){0x08, 0x01}, 2, 0},

    {0xED, (uint8_t[]){0xB0, 0x2B, 0x98, 0xA4, 0x56, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0x65, 0x4A, 0x89, 0xB2, 0x0B}, 16, 0},

    {0xEF, (uint8_t[]){0x08, 0x08, 0x08, 0x45, 0x3F, 0x54}, 6, 0},
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x00}, 5, 0},
    {0x11, (uint8_t[]){0x00}, 0, 120},
    {0x29, (uint8_t[]){0x00}, 0, 0},
};
#endif


ScreenWaveshare4_3_DSI_TOUCH_A::ScreenWaveshare4_3_DSI_TOUCH_A(const Config& cfg)
    : ScreenST7701(cfg)
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
      , i2cBusHandle(nullptr), i2cDevHandle(nullptr)
#endif
{
}

bool ScreenWaveshare4_3_DSI_TOUCH_A::initCustomPanelHW()
{
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    i2c_master_bus_config_t i2cConfig = {
        .i2c_port = -1,
        .sda_io_num = static_cast<gpio_num_t>(7),
        .scl_io_num = static_cast<gpio_num_t>(8),
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .trans_queue_depth = 0,
        .flags {
            .enable_internal_pullup = true,
            .allow_pd = false
        }
    };
    esp_err_t err = i2c_new_master_bus(&i2cConfig, &i2cBusHandle);
    if (err != ESP_OK) {
        LogError("Error creating I2C master bus: %s", esp_err_to_name(err));
        return false;
    }

    i2c_device_config_t devCfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x45,
        .scl_speed_hz = 100000,
        .scl_wait_us = 0
    };
    err = i2c_master_bus_add_device(i2cBusHandle, &devCfg, &i2cDevHandle);
    if (err != ESP_OK) {
        LogError("Error adding I2C device: %s", esp_err_to_name(err));
        return false;
    }

    const uint8_t i2cInitSeq[][2] = {
        {0x95, 0x11},
        {0x95, 0x17},
        {0x96, 0x00},
        {0x96, 0xFF}
    };

    for (const uint8_t* initCmd : i2cInitSeq) {
        err = i2c_master_transmit(i2cDevHandle, initCmd, 2, 100);
        if (err != ESP_OK) {
            LogError("Error sending I2C command: %s", esp_err_to_name(err));
            return false;
        }
    }
#endif

    return true;
}

void ScreenWaveshare4_3_DSI_TOUCH_A::shutdownCustomPanelHW()
{
#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
    if (i2cDevHandle) {
        i2c_master_bus_rm_device(i2cDevHandle);
        i2cDevHandle = nullptr;
    }
    if (i2cBusHandle) {
        i2c_del_master_bus(i2cBusHandle);
        i2cBusHandle = nullptr;
    }
#endif
}

#ifdef MINTGGGAMEENGINE_ABSTRACTMIPIDSISCREEN_LCD_ESP_DSI_SUPPORTED
size_t ScreenWaveshare4_3_DSI_TOUCH_A::getCustomPanelInitSequence(const st7701_lcd_init_cmd_t** outInitCmds)
{
    *outInitCmds = DSIInitSequence;
    return sizeof(DSIInitSequence) / sizeof(st7701_lcd_init_cmd_t);
}
#endif

}

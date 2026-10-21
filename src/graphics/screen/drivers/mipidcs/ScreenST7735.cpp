/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "ScreenST7735.h"

#include <algorithm>

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <driver/gpio.h>
#endif

#include "../../../../util/Util.h"


LOG_USE_TAG("ScreenST7735")


namespace MINTGGGameEngine
{


void ScreenST7735::initConfig(Config& cfg, bool useMainScreenConfig)
{
    cfg.engine = nullptr;

    cfg.width = 160;
    cfg.height = 128;

    cfg.offsetX = 0;
    cfg.offsetY = 0;

    cfg.staticBuffer = nullptr;

    cfg.mirrorX = true;
    cfg.mirrorY = false;
    cfg.swapXY = true;
    cfg.bgrColors = false;

    cfg.clockFreqHz = 40000000;

    cfg.pins.cs = -1;
    cfg.pins.dc = -1;
    cfg.pins.rst = -1;

    if (!useMainScreenConfig) {
        return;
    }

#ifdef MGG_SCREEN_MAIN_WIDTH
    cfg.width = MGG_SCREEN_MAIN_WIDTH;
#endif
#ifdef MGG_SCREEN_MAIN_HEIGHT
    cfg.height = MGG_SCREEN_MAIN_HEIGHT;
#endif

#ifdef MGG_SCREEN_MAIN_OFFSET_X
    cfg.offsetX = MGG_SCREEN_MAIN_OFFSET_X;
#endif
#ifdef MGG_SCREEN_MAIN_OFFSET_Y
    cfg.offsetY = MGG_SCREEN_MAIN_OFFSET_Y;
#endif

#ifdef MGG_SCREEN_MAIN_MIRROR_X
    cfg.mirrorX = MGG_SCREEN_MAIN_MIRROR_X;
#endif
#ifdef MGG_SCREEN_MAIN_MIRROR_Y
    cfg.mirrorY = MGG_SCREEN_MAIN_MIRROR_Y;
#endif
#ifdef MGG_SCREEN_MAIN_SWAP_XY
    cfg.swapXY = MGG_SCREEN_MAIN_SWAP_XY;
#endif
#ifdef MGG_SCREEN_MAIN_PIXEL_FORMAT
    assert (
            MGG_SCREEN_MAIN_PIXEL_FORMAT == PixelFormat::RGB565
        ||  MGG_SCREEN_MAIN_PIXEL_FORMAT == PixelFormat::BGR565);
    if constexpr (MGG_SCREEN_MAIN_PIXEL_FORMAT == PixelFormat::RGB565) {
        cfg.bgrColors = false;
    } else {
        cfg.bgrColors = true;
    }
#endif

#ifdef MGG_SCREEN_MAIN_SPI_CLOCK
    cfg.clockFreqHz = MGG_SCREEN_MAIN_SPI_CLOCK;
#endif

#ifdef MGG_SCREEN_MAIN_PIN_CS
    cfg.pins.cs = MGG_SCREEN_MAIN_PIN_CS;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_DC
    cfg.pins.dc = MGG_SCREEN_MAIN_PIN_DC;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_RST
    cfg.pins.rst = MGG_SCREEN_MAIN_PIN_RST;
#endif
}


ScreenST7735::ScreenST7735(const Config& cfg)
    : AbstractMIPIDCSScreen (
        cfg.bgrColors ? PixelFormat::BGR565 : PixelFormat::RGB565,
        ByteOrder::BigEndian,
        cfg.width, cfg.height,
        cfg.offsetX, cfg.offsetY,
        cfg.mirrorX, cfg.mirrorY, cfg.swapXY,
        false,
        cfg.staticBuffer
        ),
      cfg(cfg)
{
}

ScreenST7735::~ScreenST7735()
{
    // TODO: Cleanup bus
}

bool ScreenST7735::init()
{
    LogInfo (
        "Initializing ST7735 screen (%ux%u+%u+%u, mirrorX: %c, mirrorY: %c, swapXY: %c, bgr: %c, clock: %u, "
        "cs: %d, dc: %d, rst: %d)",
        cfg.width, cfg.height, cfg.offsetX, cfg.offsetY,
        cfg.mirrorX ? '1' : '0', cfg.mirrorY ? '1' : '0', cfg.swapXY ? '1' : '0', cfg.bgrColors ? '1' : '0',
        cfg.clockFreqHz,
        cfg.pins.cs, cfg.pins.dc, cfg.pins.rst
        );

    if (!AbstractMIPIDCSScreen::init()) {
        return false;
    }

    return true;
}

void ScreenST7735::commit()
{
    BufferedScreen::commit();

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    const auto w = getWidth();
    const auto h = getHeight();

    if (w == 0  ||  h == 0) {
        return;
    }

    if (!mipiSetAddress(0, 0, w-1, h-1)) {
        return;
    }
    mipiWriteCommandPlusData(MIPI_DCS_WRITE_MEMORY_START, fb.getBuffer(), w * h * sizeof(uint16_t));
#endif
}

bool ScreenST7735::mipiInitBus()
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    if (cfg.pins.cs >= 0) {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.cs,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        esp_err_t res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring CS pin: %s", esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.cs), 1);
    }

    {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.dc,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        esp_err_t res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring DC pin: %s", esp_err_to_name(res));
            return false;
        }
    }

    if (cfg.pins.rst >= 0) {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.rst,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        esp_err_t res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring RST pin: %s", esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.rst), 1);
    }

    spi_device_interface_config_t devCfg = {
        .mode = 0,
        .clock_speed_hz = static_cast<int>(cfg.clockFreqHz),
        .spics_io_num = cfg.pins.cs,
        .flags = SPI_DEVICE_NO_DUMMY,
        .queue_size = 8
    };
    esp_err_t res = spi_bus_add_device(cfg.engine->getESPSPIHostDevice(), &devCfg, &spiDev);
    if (res != ESP_OK) {
        LogError("Error adding SPI device: %s", esp_err_to_name(res));
        return false;
    }

    return true;
#else
    return false;
#endif
}

bool ScreenST7735::mipiWriteCommandPlusData(uint8_t command, const void* data, size_t length)
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    spi_transaction_t transaction = {
        .flags = SPI_TRANS_USE_TXDATA,
        .length = 8,
        .tx_data = {command},
    };

    // Set DC -> command
    gpio_set_level(static_cast<gpio_num_t>(cfg.pins.dc), 0);
    esp_err_t res = spi_device_polling_transmit(spiDev, &transaction);
    if (res != ESP_OK) {
        LogError("Error sending MIPI command %02X: %s", command, esp_err_to_name(res));
        return false;
    }

    const size_t maxTransferSize = cfg.engine->getESPSPIMaxTransferSize();

    if (data  &&  length != 0) {
        // Set DC -> data
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.dc), 1);
        for (size_t i = 0; i < length; i += maxTransferSize) {
            size_t chunk = std::min(maxTransferSize, length - i);

            spi_transaction_t datTransaction = {
                .flags = 0,
                .length = chunk * 8,
                .tx_buffer = reinterpret_cast<const uint8_t*>(data) + i,
                .rx_buffer = nullptr
            };

            res = spi_device_polling_transmit(spiDev, &datTransaction);
            if (res != ESP_OK) {
                LogError("Error sending MIPI command %02X data: %s", command, esp_err_to_name(res));
                return false;
            }
        }
    }

    return true;
#else
    return false;
#endif
}

bool ScreenST7735::doHWReset()
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    if (cfg.pins.rst < 0) {
        return false;
    }

    gpio_set_level(static_cast<gpio_num_t>(cfg.pins.rst), 0);
    DelayTaskMs(100);
    gpio_set_level(static_cast<gpio_num_t>(cfg.pins.rst), 1);
    DelayTaskMs(100);

    return true;
#else
    return false;
#endif
}


}

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

#include "ScreenILI9341.h"

#include <algorithm>

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <driver/gpio.h>
#   include <hal/lcd_hal.h>
#   include <soc/gpio_sig_map.h>
#   if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
#       include <hal/lcd_periph.h>
#   else
#       include <soc/lcd_periph.h>
#   endif
#endif

#include "../../../../util/Util.h"


LOG_USE_TAG("ScreenILI9341")


#ifndef MINTGGGAMEENGINE_CHIP_ESP32C3
#   define LCD_I80_DRIVER_SUPPORTED
#endif

#define LCD_PERIPH_MAX_TRANSFER_BYTES   (10*320*2+1)


namespace MINTGGGameEngine
{

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
// HACK: This must mirror the start of the definition of esp_lcd_i80_bus_t. We need it to obtain the bus ID for
//  reconfiguring pins when switching between LCD interface and touch with multiplexed pins. We do a very basic sanity
//  check on this in mipiInitBus(), and it might need updating for other ESP-IDF versions.
struct priv_esp_lcd_i80_bus_t {
    int bus_id;
    portMUX_TYPE spinlock;
    lcd_hal_context_t hal;
    size_t bus_width;
};
#endif


void ScreenILI9341::initConfig(Config& cfg, bool useMainScreenConfig)
{
    cfg.engine = nullptr;

    cfg.width = 320;
    cfg.height = 240;

    cfg.offsetX = 0;
    cfg.offsetY = 0;

    cfg.staticBuffer = nullptr;

    cfg.mirrorX = false;
    cfg.mirrorY = false;
    cfg.swapXY = true;
    cfg.bgrColors = true;

    cfg.clockFreqHz = 10000000;

    cfg.pins.cs     = -1;
    cfg.pins.dc     = -1;
    cfg.pins.rst    = -1;
    cfg.pins.wr     = -1;
    cfg.pins.rd     = -1;
    cfg.pins.dat[0] = -1;
    cfg.pins.dat[1] = -1;
    cfg.pins.dat[2] = -1;
    cfg.pins.dat[3] = -1;
    cfg.pins.dat[4] = -1;
    cfg.pins.dat[5] = -1;
    cfg.pins.dat[6] = -1;
    cfg.pins.dat[7] = -1;
    cfg.pins.touchXm = -1;
    cfg.pins.touchXp = -1;
    cfg.pins.touchYm = -1;
    cfg.pins.touchYp = -1;

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

#ifdef MGG_SCREEN_MAIN_PIXEL_CLOCK
    cfg.clockFreqHz = MGG_SCREEN_MAIN_PIXEL_CLOCK;
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
#ifdef MGG_SCREEN_MAIN_PIN_WR
    cfg.pins.wr = MGG_SCREEN_MAIN_PIN_WR;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_RD
    cfg.pins.rd = MGG_SCREEN_MAIN_PIN_RD;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_DAT0
    cfg.pins.dat[0] = MGG_SCREEN_MAIN_PIN_DAT0;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_DAT1
    cfg.pins.dat[1] = MGG_SCREEN_MAIN_PIN_DAT1;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_DAT2
    cfg.pins.dat[2] = MGG_SCREEN_MAIN_PIN_DAT2;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_DAT3
    cfg.pins.dat[3] = MGG_SCREEN_MAIN_PIN_DAT3;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_DAT4
    cfg.pins.dat[4] = MGG_SCREEN_MAIN_PIN_DAT4;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_DAT5
    cfg.pins.dat[5] = MGG_SCREEN_MAIN_PIN_DAT5;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_DAT6
    cfg.pins.dat[6] = MGG_SCREEN_MAIN_PIN_DAT6;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_DAT7
    cfg.pins.dat[7] = MGG_SCREEN_MAIN_PIN_DAT7;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_TOUCH_XM
    cfg.pins.touchXm = MGG_SCREEN_MAIN_PIN_TOUCH_XM;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_TOUCH_XP
    cfg.pins.touchXp = MGG_SCREEN_MAIN_PIN_TOUCH_XP;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_TOUCH_YM
    cfg.pins.touchYm = MGG_SCREEN_MAIN_PIN_TOUCH_YM;
#endif
#ifdef MGG_SCREEN_MAIN_PIN_TOUCH_YP
    cfg.pins.touchYp = MGG_SCREEN_MAIN_PIN_TOUCH_YP;
#endif
}


ScreenILI9341::ScreenILI9341(const Config& cfg)
    : AbstractMIPIDCSScreen(
        cfg.bgrColors ? PixelFormat::BGR565 : PixelFormat::RGB565,
        ByteOrder::BigEndian,
        cfg.width, cfg.height,
        cfg.offsetX, cfg.offsetY,
        cfg.mirrorX, cfg.mirrorY, cfg.swapXY,
        true,
        cfg.staticBuffer
        ),
      cfg(cfg), touchDev(nullptr)
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
      , ifaceMtx(nullptr), touchTaskShutdownRequested(false)
#endif
{
}

ScreenILI9341::~ScreenILI9341()
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    if (isTouchEnabled()) {
        touchTaskShutdownRequested = true;
        while (touchTaskShutdownRequested) {
            vTaskDelay(1);
        }

        vSemaphoreDelete(ifaceMtx);

        delete touchDev;
    }
#endif

    // TODO: Cleanup bus
}

bool ScreenILI9341::init()
{
    LogInfo (
        "Initializing ILI9341 screen (%ux%u+%u+%u, mirrorX: %c, mirrorY: %c, swapXY: %c, bgr: %c, clock: %u, "
        "cs: %d, dc: %d, rst: %d, wr: %d, rd: %d, dat: %d, %d, %d, %d, %d, %d, %d, %d, tx-: %d, tx+: %d, ty-: %d, ty+: %d)",
        cfg.width, cfg.height, cfg.offsetX, cfg.offsetY,
        cfg.mirrorX ? '1' : '0', cfg.mirrorY ? '1' : '0', cfg.swapXY ? '1' : '0', cfg.bgrColors ? '1' : '0',
        cfg.clockFreqHz,
        cfg.pins.cs, cfg.pins.dc, cfg.pins.rst, cfg.pins.wr, cfg.pins.rd,
        cfg.pins.dat[0], cfg.pins.dat[1], cfg.pins.dat[2], cfg.pins.dat[3],
        cfg.pins.dat[4], cfg.pins.dat[5], cfg.pins.dat[6], cfg.pins.dat[7],
        cfg.pins.touchXm, cfg.pins.touchXp, cfg.pins.touchYm, cfg.pins.touchYp
        );

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    mipiWriteSem = xSemaphoreCreateBinary();
#endif

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    if (isTouchEnabled()) {
        // TODO: Some code here relies on the touch pins being multiplexed with exactly these four LCD pins. That's
        //  the case for the module I'm testing with, but not necessarily for all other similar modules. We could and
        //  make this work in other cases as well. It shouldn't be too hard, I'm just too lazy to do it right now.
        if (    cfg.pins.touchXm != cfg.pins.cs
            ||  cfg.pins.touchXp != cfg.pins.dat[1]
            ||  cfg.pins.touchYm != cfg.pins.dat[0]
            ||  cfg.pins.touchYp != cfg.pins.dc
        ) {
            LogError("Touch pins are currently assumed to be multiplexed with very specific LCD pins!");
            return false;
        }

        touchDev = new ResTouchScreenInputDevice (
            cfg.pins.touchXm,
            cfg.pins.touchXp,
            cfg.pins.touchYm,
            cfg.pins.touchYp,
            cfg.width,
            cfg.height
            );
        touchDev->setDebounceTime(15);
        touchDev->setup();

        ifaceMtx = xSemaphoreCreateMutex();

        xSemaphoreTake(ifaceMtx, portMAX_DELAY);

        LogInfo("Starting ILI9341Touch task...");
        auto res = xTaskCreate (
            [](void* arg) { static_cast<ScreenILI9341*>(arg)->touchTaskMain(); },
            "ILI9341Touch",
            1024,
            this,
            1,
            &touchTask
            );
        if (res != pdPASS) {
            LogError("Error creating ILI9341Touch task!");
            xSemaphoreGive(ifaceMtx);
            return false;
        }
    }
#endif

    if (!AbstractMIPIDCSScreen::init()) {
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
        if (ifaceMtx) {
            xSemaphoreGive(ifaceMtx);
        }
#endif
        return false;
    }

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    if (ifaceMtx) {
        xSemaphoreGive(ifaceMtx);
    }
#endif

    return true;
}

void ScreenILI9341::commit()
{
    BufferedScreen::commit();

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    auto w = getWidth();
    auto h = getHeight();

    if (w == 0  ||  h == 0) {
        return;
    }

    const size_t singleLineSize = w * sizeof(uint16_t);
    const size_t maxLinesPerTransfer = LCD_PERIPH_MAX_TRANSFER_BYTES / singleLineSize;

    uint16_t x1 = 0;
    uint16_t y1 = 0;

    auto buffer = reinterpret_cast<uint8_t*>(fb.getBuffer());

    while (h != 0) {
        size_t singleTrfH = std::min(static_cast<size_t>(h), maxLinesPerTransfer);
        const uint16_t x2 = x1 + w - 1;
        const uint16_t y2 = y1 + singleTrfH - 1;
        const size_t size = w * singleTrfH * sizeof(uint16_t);

        if (ifaceMtx) {
            xSemaphoreTake(ifaceMtx, portMAX_DELAY);
        }

        switchInterface(false);

        if (!mipiSetAddress(x1, y1, x2, y2)) {
            if (ifaceMtx) {
                xSemaphoreGive(ifaceMtx);
            }
            return;
        }

        if (!mipiWriteCommandPlusData(MIPI_DCS_WRITE_MEMORY_START, buffer, size)) {
            if (ifaceMtx) {
                xSemaphoreGive(ifaceMtx);
            }
            return;
        }

        if (ifaceMtx) {
            xSemaphoreGive(ifaceMtx);
        }

        buffer += size;
        y1 += singleTrfH;
        h -= singleTrfH;
    }
#endif
}

TouchInputDevice* ScreenILI9341::getTouchInputDevice()
{
    if (touchDev) {
        return touchDev;
    }
    return AbstractMIPIDCSScreen::getTouchInputDevice();
}

bool ScreenILI9341::isTouchEnabled() const
{
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    return true;
#else
    return cfg.pins.touchXm >= 0  &&  cfg.pins.touchXp >= 0  &&  cfg.pins.touchYm >= 0  &&  cfg.pins.touchYp >= 0;
#endif
}

bool ScreenILI9341::mipiInitBus()
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
#ifdef LCD_I80_DRIVER_SUPPORTED
    // ********** GPIO SETUP **********

    esp_err_t res;

    if (cfg.pins.cs >= 0) {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.cs,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        res = gpio_config(&gpioCfg);
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
        res = gpio_config(&gpioCfg);
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
        res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring RST pin: %s", esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.rst), 1);
    }

    {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.wr,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring WR pin: %s", esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.wr), 1);
    }

    if (cfg.pins.rd >= 0) {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << cfg.pins.rd,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring RD pin: %s", esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(cfg.pins.rd), 1);
    }

    unsigned int datPinIdx = 0;
    for (int datPin : cfg.pins.dat) {
        gpio_config_t gpioCfg = {
            .pin_bit_mask = 1ull << datPin,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        res = gpio_config(&gpioCfg);
        if (res != ESP_OK) {
            LogError("Error configuring DAT[%u] pin: %s", datPinIdx, esp_err_to_name(res));
            return false;
        }
        gpio_set_level(static_cast<gpio_num_t>(datPin), 1);
        datPinIdx++;
    }


    // ********** LCD DRIVER SETUP **********

    esp_lcd_i80_bus_config_t busCfg = {
        .dc_gpio_num = static_cast<gpio_num_t>(cfg.pins.dc),
        .wr_gpio_num = static_cast<gpio_num_t>(cfg.pins.wr),
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .data_gpio_nums = {
            static_cast<gpio_num_t>(cfg.pins.dat[0]),
            static_cast<gpio_num_t>(cfg.pins.dat[1]),
            static_cast<gpio_num_t>(cfg.pins.dat[2]),
            static_cast<gpio_num_t>(cfg.pins.dat[3]),
            static_cast<gpio_num_t>(cfg.pins.dat[4]),
            static_cast<gpio_num_t>(cfg.pins.dat[5]),
            static_cast<gpio_num_t>(cfg.pins.dat[6]),
            static_cast<gpio_num_t>(cfg.pins.dat[7])
        },
        .bus_width = 8,
        .max_transfer_bytes = LCD_PERIPH_MAX_TRANSFER_BYTES,
        .dma_burst_size = 64
    };
    res = esp_lcd_new_i80_bus(&busCfg, &busHandle);
    if (res != ESP_OK) {
        LogError("Error creating LCD I80 bus: %s", esp_err_to_name(res));
        return false;
    }

    auto privBusHandle = reinterpret_cast<priv_esp_lcd_i80_bus_t*>(busHandle);
    if (privBusHandle->bus_id < 0  ||  privBusHandle->bus_width != 8) {
        LogError("Error: priv_esp_lcd_i80_bus_t seems to be out of sync with ESP-IDF (hack!) and needs updating.");
        return false;
    }

    esp_lcd_panel_io_i80_config_t ioCfg = {
        .cs_gpio_num = static_cast<gpio_num_t>(cfg.pins.cs),
        .pclk_hz = cfg.clockFreqHz,
        .trans_queue_depth = 10,
        .on_color_trans_done = [](esp_lcd_panel_io_handle_t ioHandle, esp_lcd_panel_io_event_data_t* edata, void* udata) {
            return static_cast<ScreenILI9341*>(udata)->onColorTransDone(ioHandle, edata);
        },
        .user_ctx = this,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1
        }
    };
    res = esp_lcd_new_panel_io_i80(busHandle, &ioCfg, &ioHandle);
    if (res != ESP_OK) {
        LogError("Error creating I80 IO handle: %s", esp_err_to_name(res));
        return false;
    }

    return true;
#else
    // We could bit-bang the protocol, but that's too slow to give a smooth frame experience at the screen sizes this
    // chip is typically used for. The chips without the I80 driver also probably don't even have enough pins for the
    // parallel interface.
    LogError("ILI9341 with parallel interface is not supported on this chip!");
    return false;
#endif
#else
    return false;
#endif
}

bool ScreenILI9341::mipiWriteCommandPlusData(uint8_t command, const void* data, size_t length)
{
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    esp_err_t res;
    if (length <= 1) {
        res = esp_lcd_panel_io_tx_param(ioHandle, command, data, length);
    } else {
        res = esp_lcd_panel_io_tx_color(ioHandle, command, data, length);
        if (res == ESP_OK) {
            if (xSemaphoreTake(mipiWriteSem, portMAX_DELAY) != pdTRUE) {
                return false;
            }
        }
    }
    if (res != ESP_OK) {
        LogError("Error sending MIPI command %02X: %s", command, esp_err_to_name(res));
        return false;
    }
    return true;
#else
    return false;
#endif
}

bool ScreenILI9341::doHWReset()
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


#ifndef MINTGGGAMEENGINE_PORT_DESKTOP

bool ScreenILI9341::onColorTransDone(esp_lcd_panel_io_handle_t ioHandle, esp_lcd_panel_io_event_data_t* edata)
{
    BaseType_t hpTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(mipiWriteSem, &hpTaskWoken);
    return hpTaskWoken == pdTRUE;
}

void ScreenILI9341::touchTaskMain()
{
    while (!touchTaskShutdownRequested) {
        xSemaphoreTake(ifaceMtx, portMAX_DELAY);

        switchInterface(true);
        sampleTouch();

        xSemaphoreGive(ifaceMtx);

        vTaskDelay(3);
    }

    touchTaskShutdownRequested = false;

    vTaskDelete(nullptr);
}

void ScreenILI9341::switchInterface(bool touch)
{
    if (!isTouchEnabled()) {
        return;
    }

    if (touch) {
#ifdef LCD_I80_DRIVER_SUPPORTED
        esp_rom_gpio_connect_out_signal(cfg.pins.touchXm, SIG_GPIO_OUT_IDX, false, false);
        esp_rom_gpio_connect_out_signal(cfg.pins.touchXp, SIG_GPIO_OUT_IDX, false, false);
        esp_rom_gpio_connect_out_signal(cfg.pins.touchYm, SIG_GPIO_OUT_IDX, false, false);
        esp_rom_gpio_connect_out_signal(cfg.pins.touchYp, SIG_GPIO_OUT_IDX, false, false);
#endif

        gpio_set_direction(static_cast<gpio_num_t>(cfg.pins.touchXm), GPIO_MODE_INPUT);
        gpio_set_direction(static_cast<gpio_num_t>(cfg.pins.touchXp), GPIO_MODE_INPUT);
        gpio_set_direction(static_cast<gpio_num_t>(cfg.pins.touchYm), GPIO_MODE_INPUT);
        gpio_set_direction(static_cast<gpio_num_t>(cfg.pins.touchYp), GPIO_MODE_INPUT);
    } else {
        gpio_set_direction(static_cast<gpio_num_t>(cfg.pins.touchXm), GPIO_MODE_OUTPUT);
        gpio_set_direction(static_cast<gpio_num_t>(cfg.pins.touchXp), GPIO_MODE_OUTPUT);
        gpio_set_direction(static_cast<gpio_num_t>(cfg.pins.touchYm), GPIO_MODE_OUTPUT);
        gpio_set_direction(static_cast<gpio_num_t>(cfg.pins.touchYp), GPIO_MODE_OUTPUT);

#ifdef LCD_I80_DRIVER_SUPPORTED
        // HACK: This seems to be the best way to get the bus ID right now...
        auto busID = reinterpret_cast<priv_esp_lcd_i80_bus_t*>(busHandle)->bus_id;

        // This is necessary for the I80 driver. It mirrors what lcd_i80_bus_configure_gpio() does internally. That's
        // more implementation details than I'd like to have here, but seems like the best option right now.
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
        esp_rom_gpio_connect_out_signal(cfg.pins.touchXm, soc_lcd_i80_signals[busID].cs_sig, false, false);
        esp_rom_gpio_connect_out_signal(cfg.pins.touchXp, soc_lcd_i80_signals[busID].data_sigs[1], false, false);
        esp_rom_gpio_connect_out_signal(cfg.pins.touchYm, soc_lcd_i80_signals[busID].data_sigs[0], false, false);
        esp_rom_gpio_connect_out_signal(cfg.pins.touchYp, soc_lcd_i80_signals[busID].dc_sig, false, false);
#else
        esp_rom_gpio_connect_out_signal(cfg.pins.touchXm, lcd_periph_i80_signals.buses[busID].cs_sig, false, false);
        esp_rom_gpio_connect_out_signal(cfg.pins.touchXp, lcd_periph_i80_signals.buses[busID].data_sigs[1], false, false);
        esp_rom_gpio_connect_out_signal(cfg.pins.touchYm, lcd_periph_i80_signals.buses[busID].data_sigs[0], false, false);
        esp_rom_gpio_connect_out_signal(cfg.pins.touchYp, lcd_periph_i80_signals.buses[busID].dc_sig, false, false);
#endif
#endif
    }
}

void ScreenILI9341::sampleTouch()
{
    touchDev->tick();
}

#endif


}

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

#pragma once

#include "../../../../Globals.h"

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
#   include "freertos/FreeRTOS.h"
#   include "freertos/semphr.h"
#   include "freertos/task.h"
#   include <driver/spi_master.h>
#   include <esp_lcd_panel_io.h>
#   include <esp_lcd_io_i80.h>
#endif

#include "../../../../core/Engine.h"
#include "../../../../input/ResTouchScreenInputDevice.h"
#include "AbstractMIPIDCSScreen.h"


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
class ScreenILI9341 : public AbstractMIPIDCSScreen
{
public:
    struct Config
    {
        Engine* engine;

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

            gpionum_t touchXm; // Must be an ADC pin
            gpionum_t touchXp;
            gpionum_t touchYm;
            gpionum_t touchYp; // Must be an ADC pin
        } pins;
    };

public:
    static void initConfig(Config& cfg, bool useMainScreenConfig = false);

public:
    ScreenILI9341(const Config& cfg);
    virtual ~ScreenILI9341();

    bool init() override;
    
    void commit() override;

    std::string getModelID() override { return "ili9341"; }

    TouchInputDevice* getTouchInputDevice() override;

    bool isTouchEnabled() const;

protected:
    bool mipiInitBus() override;
    bool mipiWriteCommandPlusData(uint8_t command, const void* data, size_t length) override;

    bool doHWReset() override;

private:
#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    bool onColorTransDone(esp_lcd_panel_io_handle_t ioHandle, esp_lcd_panel_io_event_data_t* edata);

    void touchTaskMain();
    void switchInterface(bool touch);
    void sampleTouch();
#endif

private:
    Config cfg;

    ResTouchScreenInputDevice* touchDev;

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    esp_lcd_i80_bus_handle_t busHandle;
    esp_lcd_panel_io_handle_t ioHandle;
    SemaphoreHandle_t mipiWriteSem;

    SemaphoreHandle_t ifaceMtx;
    TaskHandle_t touchTask;
    volatile bool touchTaskShutdownRequested;
#endif
};

}

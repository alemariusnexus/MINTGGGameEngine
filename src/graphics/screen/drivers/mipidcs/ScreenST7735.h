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
#include "../../../../core/Engine.h"

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <driver/spi_master.h>
#endif

#include "AbstractMIPIDCSScreen.h"


namespace MINTGGGameEngine
{

/**
 * \brief Driver for an ST7735-based screen.
 *
 * This class currently assumes that the ST7735 is connected using the SPI interface. The ST7735 supports other
 * interfaces, but they are not handled by this class.
 * A hardware SPI peripheral is used to communicate with the screen.
 */
class ScreenST7735 : public AbstractMIPIDCSScreen
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
        } pins;
    };

public:
    static void initConfig(Config& cfg, bool useMainScreenConfig = false);

public:
    ScreenST7735(const Config& cfg);
    virtual ~ScreenST7735();

    bool init() override;
    
    void commit() override;

    std::string getModelID() override { return "st7735"; }

protected:
    bool mipiInitBus() override;
    bool mipiWriteCommandPlusData(uint8_t command, const void* data, size_t length) override;

    bool doHWReset() override;

private:
    Config cfg;

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    spi_device_handle_t spiDev;
#endif
};

}

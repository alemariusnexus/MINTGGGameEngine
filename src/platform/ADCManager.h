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

#include "../Globals.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#include <esp_adc/adc_oneshot.h>
#endif

#include <unordered_map>


namespace MINTGGGameEngine
{


class ADCManager
{
private:
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    struct ADCUnitConfig
    {
        adc_oneshot_unit_handle_t adcHandle;
    };
    struct ADCPinConfig
    {
        adc_oneshot_unit_handle_t adcHandle;
        adc_channel_t adcChannel;
    };
#endif

public:
    static ADCManager& getInstance();

    bool setupAnalogPin(gpionum_t pin);
    void destroyAnalogPin(gpionum_t pin);
    uint32_t readRaw(gpionum_t pin);

    uint32_t getMaxRawValue();

private:
    ADCManager();

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    adc_oneshot_unit_handle_t setupADCInst(adc_unit_t unit);
#endif

private:
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    std::unordered_map<adc_unit_t, ADCUnitConfig> adcUnitConfigs;
    std::unordered_map<gpionum_t, ADCPinConfig> adcPinConfigs;
#endif
};


}

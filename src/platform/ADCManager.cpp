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

#include "ADCManager.h"

#include <algorithm>

#include "../util/Log.h"

LOG_USE_TAG("ADCManager")


namespace MINTGGGameEngine
{


#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
static const adc_unit_t ADCUnitsESPIDF[] = {
    ADC_UNIT_1

    // Quote from ESP-IDF documentation for ESP32C3 specifically:
    // "ADC2 oneshot mode is no longer supported, due to hardware limitations"
#ifndef MINTGGGAMEENGINE_CHIP_ESP32C3
    , ADC_UNIT_2
#endif
};
#endif


ADCManager& ADCManager::getInstance()
{
    static ADCManager inst;
    return inst;
}

bool ADCManager::begin()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    LogInfo("Calibrating ADCs...");

    adc_cali_scheme_ver_t caliSchemes;
    esp_err_t err = adc_cali_check_scheme(&caliSchemes);
    if (err != ESP_OK) {
        LogError("Error checking calibration scheme: %s", esp_err_to_name(err));
        return false;
    }

    for (const adc_unit_t adcUnit : ADCUnitsESPIDF) {
        setupADCInst(adcUnit);

        ADCUnitConfig& adcUnitCfg = adcUnitConfigs[adcUnit];

#ifdef ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        if ((caliSchemes & ADC_CALI_SCHEME_VER_CURVE_FITTING) != 0) {
            adc_cali_curve_fitting_config_t caliCfg = {
                .unit_id = adcUnit,
                .atten = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_12
            };
            err = adc_cali_create_scheme_curve_fitting(&caliCfg, &adcUnitCfg.caliHandle);
            if (err != ESP_OK) {
                LogError("Error calibrating ADC: %s", esp_err_to_name(err));
                return false;
            }
        }
#endif

#ifdef ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        if ((caliSchemes & ADC_CALI_SCHEME_VER_LINE_FITTING) != 0) {
            adc_cali_line_fitting_config_t caliCfg = {
                .unit_id = adcUnit,
                .atten = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_12
            };
            err = adc_cali_create_scheme_line_fitting(&caliCfg, &adcUnitCfg.caliHandle);
            if (err != ESP_OK) {
                LogError("Error calibrating ADC: %s", esp_err_to_name(err));
                return false;
            }
        }
#endif
    }
#endif

    return true;
}

void ADCManager::shutdown()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    adc_cali_scheme_ver_t caliSchemes;
    esp_err_t err = adc_cali_check_scheme(&caliSchemes);
    if (err != ESP_OK) {
        LogError("Error checking calibration scheme: %s", esp_err_to_name(err));
        return;
    }

    for (const adc_unit_t adcUnit : ADCUnitsESPIDF) {
        auto it = adcUnitConfigs.find(adcUnit);
        if (it == adcUnitConfigs.end()) {
            continue;
        }
        ADCUnitConfig& adcUnitCfg = it->second;

#ifdef ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        if ((caliSchemes & ADC_CALI_SCHEME_VER_CURVE_FITTING) != 0) {
            adc_cali_delete_scheme_curve_fitting(adcUnitCfg.caliHandle);
        }
#endif

#ifdef ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        if ((caliSchemes & ADC_CALI_SCHEME_VER_LINE_FITTING) != 0) {
            adc_cali_delete_scheme_line_fitting(adcUnitCfg.caliHandle);
        }
#endif

        adc_oneshot_del_unit(adcUnitCfg.adcHandle);
    }
#endif
}

bool ADCManager::setupAnalogPin(gpionum_t pin)
{
    if (pin < 0) {
        return false;
    }
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    auto it = adcPinConfigs.find(pin);
    if (it != adcPinConfigs.end()) {
        return true;
    }

    adc_unit_t adcUnit;
    adc_channel_t adcChannel;
    if (adc_oneshot_io_to_channel(pin, &adcUnit, &adcChannel) != ESP_OK) {
        return false;
    }

    adc_oneshot_unit_handle_t adcUnitHandle = setupADCInst(adcUnit);

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12
    };
    if (adc_oneshot_config_channel(adcUnitHandle, adcChannel, &config) != ESP_OK) {
        return false;
    }

    ADCPinConfig pinCfg;
    pinCfg.adcHandle = adcUnitHandle;
    pinCfg.adcChannel = adcChannel;
    adcPinConfigs[pin] = pinCfg;
#else
    return false;
#endif

    return true;
}

void ADCManager::destroyAnalogPin(gpionum_t pin)
{
    // Nothing to do
}

uint32_t ADCManager::readRaw(gpionum_t pin)
{
    if (pin < 0) {
        return 0;
    }
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    auto it = adcPinConfigs.find(pin);
    if (it == adcPinConfigs.end()) {
        return 0;
    }

    int rawValue;
    if (adc_oneshot_read(it->second.adcHandle, it->second.adcChannel, &rawValue) != ESP_OK) {
        return 0;
    }
    return rawValue;
#else
    return 0;
#endif
}

uint16_t ADCManager::readMv(gpionum_t pin)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    adc_unit_t adcUnit;
    adc_channel_t adcChannel;
    if (adc_oneshot_io_to_channel(pin, &adcUnit, &adcChannel) != ESP_OK) {
        return 0;
    }

    auto it = adcPinConfigs.find(pin);
    if (it == adcPinConfigs.end()) {
        return 0;
    }

    int rawValue;
    if (adc_oneshot_read(it->second.adcHandle, it->second.adcChannel, &rawValue) != ESP_OK) {
        return 0;
    }

    const ADCUnitConfig& unitCfg = adcUnitConfigs[adcUnit];

    int voltageMv;
    esp_err_t err = adc_cali_raw_to_voltage(unitCfg.caliHandle, static_cast<int>(rawValue), &voltageMv);
    if (err != ESP_OK) {
        LogError("Error converting raw value to voltage: %s", esp_err_to_name(err));
        return 0;
    }

    return voltageMv >= 0 ? static_cast<uint16_t>(voltageMv) : 0;
#else
    return 0;
#endif
}

float ADCManager::readIOVoltageRatio(gpionum_t pin, uint16_t ioVoltageMv)
{
    const auto voltageMv = readMv(pin);
    if (ioVoltageMv == 0) {
        return -1.0f;
    }
    return std::min(std::max(static_cast<float>(voltageMv) / static_cast<float>(ioVoltageMv), 0.0f), 1.0f);
}

float ADCManager::readIOVoltageRatio(gpionum_t pin)
{
    return readIOVoltageRatio(pin, readIOVoltageMv());
}


#ifdef MINTGGGAMEENGINE_PORT_ESPIDF

adc_oneshot_unit_handle_t ADCManager::setupADCInst(adc_unit_t unit)
{
    auto it = adcUnitConfigs.find(unit);
    if (it != adcUnitConfigs.end()) {
        return it->second.adcHandle;
    }

    ADCUnitConfig unitCfg;
    unitCfg.caliHandle = nullptr;
    adc_oneshot_unit_init_cfg_t cfg = {
        .unit_id = unit,
        .clk_src = static_cast<adc_oneshot_clk_src_t>(0),
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    if (adc_oneshot_new_unit(&cfg, &unitCfg.adcHandle) != ESP_OK) {
        return nullptr;
    }

    adcUnitConfigs[unit] = unitCfg;

    return unitCfg.adcHandle;
}

#endif

uint16_t ADCManager::convertRawToMv(gpionum_t pin, uint32_t rawValue)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    adc_unit_t adcUnit;
    adc_channel_t adcChannel;
    if (adc_oneshot_io_to_channel(pin, &adcUnit, &adcChannel) != ESP_OK) {
        return 0;
    }

    const ADCUnitConfig& unitCfg = adcUnitConfigs[adcUnit];

    int voltageMv;
    esp_err_t err = adc_cali_raw_to_voltage(unitCfg.caliHandle, static_cast<int>(rawValue), &voltageMv);
    if (err != ESP_OK) {
        LogError("Error converting raw value to voltage: %s", esp_err_to_name(err));
        return 0;
    }

    return voltageMv >= 0 ? static_cast<uint16_t>(voltageMv) : 0;
#else
    return 0;
#endif
}

uint32_t ADCManager::getMaxRawValue()
{
    // TODO: Neither Arduino nor ESP-IDF seem to have a way to query this. For now we'll
    //  assume 12 bit width. Find a better way!
    return 4095;
}

uint16_t ADCManager::readIOVoltageMv()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    // TODO: Support measuring it from an external ADC pin if available
    return 3300;
#else
    return 0;
#endif
}


}

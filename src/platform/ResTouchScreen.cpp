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

#include "ResTouchScreen.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <driver/gpio.h>
#endif

#include "../util/Log.h"
#include "../util/Util.h"
#include "ADCManager.h"

LOG_USE_TAG("ResTouchScreen")


namespace MINTGGGameEngine
{

ResTouchScreen::ResTouchScreen(gpionum_t xmPin, gpionum_t xpPin, gpionum_t ymPin, gpionum_t ypPin)
    : xmPin(xmPin), xpPin(xpPin), ymPin(ymPin), ypPin(ypPin)
{
}

bool ResTouchScreen::setup()
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    gpio_config_t gpioCfg = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    for (gpionum_t pin : {xmPin, xpPin, ymPin, ypPin}) {
        gpioCfg.pin_bit_mask = 1ull << pin;
        esp_err_t err = gpio_config(&gpioCfg);
        if (err != ESP_OK) {
            LogError("Error configuring pin %d: %s", pin, esp_err_to_name(err));
            return false;
        }
    }

    auto& adcMgr = ADCManager::getInstance();

    if (!adcMgr.setupAnalogPin(xmPin)) {
        LogError("Error setting up analog pin for X-");
        return false;
    }
    if (!adcMgr.setupAnalogPin(ypPin)) {
        LogError("Error setting up analog pin for Y+");
        return false;
    }

    return true;
#else
    return false;
#endif
}

bool ResTouchScreen::read(Vec2* position)
{
    ADCManager& adcMgr = ADCManager::getInstance();
    const uint16_t ioVoltageMv = adcMgr.readIOVoltageMv();
    const bool touched = checkTouched(ioVoltageMv);
    if (!touched) {
        return false;
    }
    if (position) {
        const float rx = readRatioX(ioVoltageMv);
        const float ry = readRatioY(ioVoltageMv);
        position->set(rx, ry);
        //const float pressure = readPressure(rx, ioVoltageMv);
    }
    return checkTouched(ioVoltageMv);
}

bool ResTouchScreen::checkTouched(uint16_t ioVoltageMv)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    // Leave X+/Y-/X+ floating
    gpio_set_direction(static_cast<gpio_num_t>(xpPin), GPIO_MODE_INPUT);
    gpio_set_direction(static_cast<gpio_num_t>(ymPin), GPIO_MODE_INPUT);
    gpio_set_direction(static_cast<gpio_num_t>(ypPin), GPIO_MODE_INPUT);

    // Drive X- to GND
    gpio_set_level(static_cast<gpio_num_t>(xmPin), 0);
    gpio_set_direction(static_cast<gpio_num_t>(xmPin), GPIO_MODE_OUTPUT);

    // Enable weak pull-up on Y+
    gpio_set_pull_mode(static_cast<gpio_num_t>(ypPin), GPIO_PULLUP_ONLY);

    // Wait for voltage to settle
    TimerDelaySpinUs(5);

    // Measure voltge at Y+
    const float ratio = ADCManager::getInstance().readIOVoltageRatio(ypPin, ioVoltageMv);

    // Disable weak pull-up on Y+
    gpio_set_pull_mode(static_cast<gpio_num_t>(ypPin), GPIO_FLOATING);

    return ratio < 0.5f;
#else
    return false;
#endif
}

float ResTouchScreen::readRatioX(uint16_t ioVoltageMv)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    // Leave Y-/Y+ floating
    gpio_set_direction(static_cast<gpio_num_t>(ymPin), GPIO_MODE_INPUT);
    gpio_set_direction(static_cast<gpio_num_t>(ypPin), GPIO_MODE_INPUT);

    // Actively drive X-/X+
    gpio_set_level(static_cast<gpio_num_t>(xmPin), 0);
    gpio_set_level(static_cast<gpio_num_t>(xpPin), 1);
    gpio_set_direction(static_cast<gpio_num_t>(xmPin), GPIO_MODE_OUTPUT);
    gpio_set_direction(static_cast<gpio_num_t>(xpPin), GPIO_MODE_OUTPUT);

    // Wait for voltage to settle
    TimerDelaySpinUs(5);

    // Measure voltage at Y+
    const float ratio = ADCManager::getInstance().readIOVoltageRatio(ypPin, ioVoltageMv);

    // Release X-/X+ again
    gpio_set_direction(static_cast<gpio_num_t>(xmPin), GPIO_MODE_INPUT);
    gpio_set_direction(static_cast<gpio_num_t>(xpPin), GPIO_MODE_INPUT);

    return ratio;
#else
    return 0;
#endif
}

float ResTouchScreen::readRatioY(uint16_t ioVoltageMv)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    // Leave X-/X+ floating
    gpio_set_direction(static_cast<gpio_num_t>(xmPin), GPIO_MODE_INPUT);
    gpio_set_direction(static_cast<gpio_num_t>(xpPin), GPIO_MODE_INPUT);

    // Actively drive Y-/Y+
    gpio_set_level(static_cast<gpio_num_t>(ymPin), 0);
    gpio_set_level(static_cast<gpio_num_t>(ypPin), 1);
    gpio_set_direction(static_cast<gpio_num_t>(ymPin), GPIO_MODE_OUTPUT);
    gpio_set_direction(static_cast<gpio_num_t>(ypPin), GPIO_MODE_OUTPUT);

    // Wait for voltage to settle
    TimerDelaySpinUs(5);

    // Measure voltage at X-
    const float ratio = ADCManager::getInstance().readIOVoltageRatio(xmPin, ioVoltageMv);

    // Release Y-/Y+ again
    gpio_set_direction(static_cast<gpio_num_t>(ymPin), GPIO_MODE_INPUT);
    gpio_set_direction(static_cast<gpio_num_t>(ypPin), GPIO_MODE_INPUT);

    return ratio;
#else
    return 0;
#endif
}

float ResTouchScreen::readPressure(float ratioX, uint16_t ioVoltageMv)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    // Based on Microchip's 4-wire measurement method:
    // https://onlinedocs.microchip.com/oxy/GUID-51CE727C-126C-4670-A2BA-189781F25924-en-US-3/GUID-E58FA210-3DE6-4507-A1E0-A405E40F4802.html

    // Leave X-/Y+ floating
    gpio_set_direction(static_cast<gpio_num_t>(xmPin), GPIO_MODE_INPUT);
    gpio_set_direction(static_cast<gpio_num_t>(ypPin), GPIO_MODE_INPUT);

    // Actively drive X+/Y-
    gpio_set_level(static_cast<gpio_num_t>(xpPin), 0);
    gpio_set_level(static_cast<gpio_num_t>(ymPin), 1);
    gpio_set_direction(static_cast<gpio_num_t>(xpPin), GPIO_MODE_OUTPUT);
    gpio_set_direction(static_cast<gpio_num_t>(ymPin), GPIO_MODE_OUTPUT);

    // Wait for voltage to settle
    TimerDelaySpinUs(5);

    // Measure voltage at X-
    const float ratioZ1 = ADCManager::getInstance().readIOVoltageRatio(xmPin, ioVoltageMv);

    TimerDelaySpinUs(5);

    // Measure voltage at Y+
    const float ratioZ2 = ADCManager::getInstance().readIOVoltageRatio(ypPin, ioVoltageMv);

    //LogInfo("ratioX: %.3f, ratioZ1: %.3f, ratioZ2: %.3f", ratioX, ratioZ1, ratioZ2);

    return 1.0f - ((1.0f - ratioX) * (ratioZ2/ratioZ1 - 1.0f));
#else
    return 0;
#endif
}

}

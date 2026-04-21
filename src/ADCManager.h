#pragma once

#include "Globals.h"

#ifdef ESP_PLATFORM
#include <esp_adc/adc_oneshot.h>
#endif

#include <unordered_map>


namespace MINTGGGameEngine
{


class ADCManager
{
private:
#ifdef ESP_PLATFORM
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

    bool setupAnalogPin(uint8_t pin);
    void destroyAnalogPin(uint8_t pin);
    uint32_t readRaw(uint8_t pin);

    uint32_t getMaxRawValue();

private:
    ADCManager();

#ifdef ESP_PLATFORM
    adc_oneshot_unit_handle_t setupADCInst(adc_unit_t unit);
#endif

private:
#ifdef ESP_PLATFORM
    std::unordered_map<adc_unit_t, ADCUnitConfig> adcUnitConfigs;
    std::unordered_map<uint8_t, ADCPinConfig> adcPinConfigs;
#endif
};


}

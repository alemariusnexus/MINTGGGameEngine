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

    bool setupAnalogPin(uint8_t pin);
    void destroyAnalogPin(uint8_t pin);
    uint32_t readRaw(uint8_t pin);

    uint32_t getMaxRawValue();

private:
    ADCManager();

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    adc_oneshot_unit_handle_t setupADCInst(adc_unit_t unit);
#endif

private:
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    std::unordered_map<adc_unit_t, ADCUnitConfig> adcUnitConfigs;
    std::unordered_map<uint8_t, ADCPinConfig> adcPinConfigs;
#endif
};


}

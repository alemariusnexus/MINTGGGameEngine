#include "ADCManager.h"


namespace MINTGGGameEngine
{


ADCManager& ADCManager::getInstance()
{
    static ADCManager inst;
    return inst;
}

ADCManager::ADCManager()
{
}

bool ADCManager::setupAnalogPin(uint8_t pin)
{
#ifdef ARDUINO
    // Nothing to do
#elif defined(ESP_PLATFORM)
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
#endif

    return true;
}

void ADCManager::destroyAnalogPin(uint8_t pin)
{
    // Nothing to do
}

uint32_t ADCManager::readRaw(uint8_t pin)
{
#ifdef ARDUINO
    return analogRead(pin);
#elif defined(ESP_PLATFORM)
    auto it = adcPinConfigs.find(pin);
    if (it == adcPinConfigs.end()) {
        return 0;
    }

    int rawValue;
    if (adc_oneshot_read(it->second.adcHandle, it->second.adcChannel, &rawValue) != ESP_OK) {
        return 0;
    }
    return rawValue;
#endif
}


#ifdef ESP_PLATFORM

adc_oneshot_unit_handle_t ADCManager::setupADCInst(adc_unit_t unit)
{
    auto it = adcUnitConfigs.find(unit);
    if (it != adcUnitConfigs.end()) {
        return it->second.adcHandle;
    }

    ADCUnitConfig unitCfg;
    adc_oneshot_unit_init_cfg_t cfg = {
        .unit_id = unit,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    if (adc_oneshot_new_unit(&cfg, &unitCfg.adcHandle) != ESP_OK) {
        return nullptr;
    }

    adcUnitConfigs[unit] = unitCfg;

    return unitCfg.adcHandle;
}

#endif

uint32_t ADCManager::getMaxRawValue()
{
    // TODO: Neither Arduino nor ESP-IDF seem to have a way to query this. For now we'll
    //  assume 12 bit width. Find a better way!
    return 4095;
}


}

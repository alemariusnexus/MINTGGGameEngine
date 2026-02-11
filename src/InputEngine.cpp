#include "InputEngine.h"

#include <set>


namespace MINTGGGameEngine
{

void _InputEngineTaskMain(void* params)
{
    ((InputEngine*) params)->inputTaskMain();
}


bool InputEngine::begin(uint8_t debounceCount)
{
    this->debounceCount = debounceCount;
    
    inputMtx = xSemaphoreCreateMutex();
    
    BaseType_t res = xTaskCreate(&_InputEngineTaskMain, "InputTask", 4096,
            this, 1, &inputTask);
    if (res != pdPASS) {
        Serial.println("ERROR: Unable to create InputTask.");
        return false;
    }
    
    return true;
}

void InputEngine::inputTaskMain()
{
    std::vector<ButtonDef*> stateChangedButtons;
	
	std::vector<unsigned int> pinNums;
	std::vector<uint8_t> pinValues;
    
    while (true) {
		xSemaphoreTake(inputMtx, portMAX_DELAY);
		
		for (auto& pair : buttonsByDevice) {
			GPIODevice* dev = pair.first;
			
			// Collect pin numbers for buttons
			pinNums.clear();
			for (ButtonDef* def : pair.second) {
				pinNums.push_back(def->pin);
			}
			pinValues.resize(pinNums.size());
			
			// Read all at once
			dev->readPins(pinNums.data(), pinValues.data(), pinNums.size());
			
			// Store raw state
			size_t pinIdx = 0;
			for (ButtonDef* def : pair.second) {
				def->rawState = pinValues[pinIdx];
				pinIdx++;
			}
		}
        
        // Apply all button states (including debouncing)
        for (auto it = buttons.begin() ; it != buttons.end() ; ++it) {
            ButtonDef* def = it->second;
			
			bool pressed;
			if ((def->flags & PinFlagsActiveLow) != 0) {
				pressed = (def->rawState == 0);
			} else {
				pressed = (def->rawState != 0);
			}
			if (debounceButton(def, pressed)) {
				stateChangedButtons.push_back(def);
			}
        }
        
        // Read all axis values
        float analogMaxValueFloat = getAnalogReadMaxValue();
        for (auto it = axes.begin() ; it != axes.end() ; ++it) {
            AxisDef* def = it->second;
            float adcT = analogRead(def->pin) / analogMaxValueFloat;
            
            float minVal;
            float maxVal;
            float finalFactor;
            if (def->minValue < def->maxValue) {
                finalFactor = 1.0f;
                minVal = def->minValue;
                maxVal = def->maxValue;
            } else {
                finalFactor = -1.0f;
                minVal = def->maxValue;
                maxVal = def->minValue;
            }
            
            float value;
            float distFromNeutral = fabs(adcT - def->neutralValue);
            if (distFromNeutral > def->neutralWidth) {
                if (adcT > def->neutralValue) {
                    value = finalFactor * (distFromNeutral - def->neutralWidth)
                            / (maxVal - def->neutralValue - def->neutralWidth);
                } else {
                    value = -finalFactor * (distFromNeutral - def->neutralWidth)
                            / (def->neutralValue - minVal - def->neutralWidth);
                }
            } else {
                value = 0.0f;
            }
            
            def->rawValue = adcT;
            def->value = value;
        }
        
        xSemaphoreGive(inputMtx);
        
        // Scan for activated button combos
        for (ButtonCombo* combo : buttonCombos) {
            bool allPressed = true;
            for (const std::string& cid : combo->ids) {
                if (!isButtonPressed(cid)) {
                    allPressed = false;
                    break;
                }
            }
            if (allPressed) {
                // Check if one of the combo buttons was just pressed
                bool justPressed = false;
                for (ButtonDef* changedDef : stateChangedButtons) {
                    if (changedDef->pressed  &&  combo->ids.find(changedDef->id) != combo->ids.end()) {
                        justPressed = true;
                        break;
                    }
                }
                if (justPressed) {
                    combo->cb();
                }
            }
        }
        
        stateChangedButtons.clear();
        
        vTaskDelay(1);
    }
}

bool InputEngine::defineButton (
		const std::string& id,
		unsigned int pin,
		GPIODevice& gpioDevice,
		int flags
) {
    if (getButtonDef(id)) {
        return false;
    }
    
    xSemaphoreTake(inputMtx, portMAX_DELAY);
    
    ButtonDef* def = new ButtonDef(id, pin, &gpioDevice, flags);
	buttonIDs.push_back(id);
    buttons[id] = def;
	buttonsByDevice[&gpioDevice].push_back(def);
    
    int pmFlags = INPUT;
    if ((flags & PinFlagsPullup) != 0) {
        pmFlags = INPUT_PULLUP;
    }
    if ((flags & PinFlagsPulldown) != 0) {
        pmFlags = INPUT_PULLDOWN;
    }
	
	gpioDevice.setPinMode(pin, pmFlags);
    
    xSemaphoreGive(inputMtx);
    
    return true;
}

bool InputEngine::undefineButton(const std::string& id)
{
    auto it = buttons.find(id);
    if (it == buttons.end()) {
        return false;
    }
	
	ButtonDef* def = it->second;
	auto& devButtons = buttonsByDevice[def->dev];
    
    xSemaphoreTake(inputMtx, portMAX_DELAY);
	buttonIDs.erase (
			std::find(buttonIDs.begin(), buttonIDs.end(), id));
	devButtons.erase (
			std::find(devButtons.begin(), devButtons.end(), def));
    delete def;
    buttons.erase(it);
    xSemaphoreGive(inputMtx);
    
    return true;
}

std::vector<std::string> InputEngine::getButtonIDs() const
{
	return buttonIDs;
}

bool InputEngine::defineAxis (
        const std::string& id, uint8_t pin,
        float minValue, float maxValue,
        float neutralValue, float neutralWidth
) {
    if (getAxisDef(id)) {
        return false;
    }
    
    xSemaphoreTake(inputMtx, portMAX_DELAY);
	
	axisIDs.push_back(id);
    
    AxisDef* def = new AxisDef(id, pin);
    def->minValue = minValue;
    def->maxValue = maxValue;
    def->neutralValue = neutralValue;
    def->neutralWidth = neutralWidth;
    axes[id] = def;
    
    xSemaphoreGive(inputMtx);
    
    return true;
}

bool InputEngine::undefineAxis(const std::string& id)
{
    auto it = axes.find(id);
    if (it == axes.end()) {
        return false;
    }
    
    xSemaphoreTake(inputMtx, portMAX_DELAY);
	axisIDs.erase (
			std::find(axisIDs.begin(), axisIDs.end(), id));
    delete it->second;
    axes.erase(it);
    xSemaphoreGive(inputMtx);
    
    return true;
}

std::vector<std::string> InputEngine::getAxisIDs() const
{
	return axisIDs;
}

bool InputEngine::hasButton(const std::string& id)
{
    return getButtonDef(id) != nullptr;
}

bool InputEngine::isButtonPressed(const std::string& id)
{
    ButtonDef* def = getButtonDef(id);
    if (!def) {
        return false;
    }
    return def->pressed;
}

void InputEngine::defineButtonCombo(const std::unordered_set<std::string>& ids, ButtonComboCb cb)
{
    xSemaphoreTake(inputMtx, portMAX_DELAY);
    
    ButtonCombo* combo = new ButtonCombo(ids, cb);
    buttonCombos.push_back(combo);
    
    xSemaphoreGive(inputMtx);
}

bool InputEngine::hasAxis(const std::string& id)
{
    return getAxisDef(id) != nullptr;
}

float InputEngine::getAxis(const std::string& id)
{
    AxisDef* def = getAxisDef(id);
    return def ? def->value : 0.0f;
}

float InputEngine::getAxisRaw(const std::string& id)
{
    AxisDef* def = getAxisDef(id);
    return def ? def->rawValue : 0.0f;
}

InputEngine::ButtonDef* InputEngine::getButtonDef(const std::string& id)
{
    auto it = buttons.find(id);
    return it != buttons.end() ? it->second : nullptr;
}

InputEngine::AxisDef* InputEngine::getAxisDef(const std::string& id)
{
    auto it = axes.find(id);
    return it != axes.end() ? it->second : nullptr;
}

bool InputEngine::debounceButton(ButtonDef* def, bool pressed)
{
    if (pressed == def->pressed) {
        return false;
    }
    if (def->debounceCount++ >= debounceCount) {
        def->pressed = pressed;
        def->debounceCount = 0;
        return true;
    }
    return false;
}

uint16_t InputEngine::getAnalogReadMaxValue() const
{
    // TODO: This assumes the ESP32 ADC resolution of 12 bits. Arduino doesn't
    //  have a function to query it...
    return 4095;
}

}

#include "InputEngine.h"


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
    
    while (true) {
        // Fetch all MCP23009 states from I2C
        for (MCP23009Device* dev : mcp23009Devices) {
            dev->lastState = dev->mcp.read8();
        }
        
        xSemaphoreTake(inputMtx, portMAX_DELAY);
        
        // Apply all button press states (including debouncing)
        for (auto it = buttons.begin() ; it != buttons.end() ; ++it) {
            ButtonDef* def = it->second;
            
            if (def->type == ButtonType::Regular) {
                bool pressed = (digitalRead(def->regular.pin) == ((def->flags & PinFlagsActiveLow) != 0 ? LOW : HIGH));
                if (debounceButton(def, pressed)) {
                    stateChangedButtons.push_back(def);
                }
            } else if (def->type == ButtonType::MCP23009) {
                uint8_t singleState = def->mcp23009.dev->lastState & (1 << def->mcp23009.pin);
                bool pressed = ((def->flags & PinFlagsActiveLow) != 0) ? (singleState == 0) : (singleState != 0);
                if (debounceButton(def, pressed)) {
                    stateChangedButtons.push_back(def);
                }
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

bool InputEngine::defineButton(const std::string& id, uint8_t pin, int flags)
{
    if (getButtonDef(id)) {
        return false;
    }
    
    xSemaphoreTake(inputMtx, portMAX_DELAY);
    
    ButtonDef* def = new ButtonDef(id, ButtonType::Regular, flags);
    def->regular.pin = pin;
    buttons[id] = def;
    
    int pmFlags = INPUT;
    if ((flags & PinFlagsPullup) != 0) {
        pmFlags = INPUT_PULLUP;
    }
    if ((flags & PinFlagsPulldown) != 0) {
        pmFlags = INPUT_PULLDOWN;
    }
    pinMode(pin, pmFlags);
    
    xSemaphoreGive(inputMtx);
    
    return true;
}

bool InputEngine::defineButtonMCP23009(const std::string& id, uint8_t pin, int flags, uint8_t i2cAddr)
{
    if (getButtonDef(id)) {
        return false;
    }
    
    xSemaphoreTake(inputMtx, portMAX_DELAY);
    
    MCP23009Device* dev = registerMCP23009(i2cAddr);
    
    ButtonDef* def = new ButtonDef(id, ButtonType::MCP23009, flags);
    def->mcp23009.dev = dev;
    def->mcp23009.pin = pin;
    buttons[id] = def;
    
    int pmFlags = INPUT;
    if ((flags & PinFlagsPullup) != 0) {
        pmFlags = INPUT_PULLUP;
    }
    if ((flags & PinFlagsPulldown) != 0) {
        pmFlags = INPUT_PULLDOWN;
    }
    
    dev->mcp.pinMode1(pin, pmFlags);
    
    xSemaphoreGive(inputMtx);
    
    return true;
}

bool InputEngine::undefineButton(const std::string& id)
{
    auto it = buttons.find(id);
    if (it == buttons.end()) {
        return false;
    }
    
    xSemaphoreTake(inputMtx, portMAX_DELAY);
    delete it->second;
    buttons.erase(it);
    xSemaphoreGive(inputMtx);
    
    return true;
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
    delete it->second;
    axes.erase(it);
    xSemaphoreGive(inputMtx);
    
    return true;
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

InputEngine::MCP23009Device* InputEngine::registerMCP23009(uint8_t addr)
{
    for (MCP23009Device* dev : mcp23009Devices) {
        if (dev->mcp.getAddress() == addr) {
            return dev;
        }
    }
    MCP23009Device* dev = new MCP23009Device(addr);
    mcp23009Devices.push_back(dev);
    dev->mcp.begin();
    return dev;
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

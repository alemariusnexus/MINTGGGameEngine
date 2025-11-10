#pragma once

#include "Globals.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <MCP23008.h>


namespace MINTGGGameEngine
{

/**
 * \brief Handles all game inputs.
 *
 * Use Game.input() to receive the active input engine.
 *
 * Currently, this class only handles buttons. Two types of buttons are
 * currently supported. Buttons can be active-high or active-low, and can
 * optionally have a pull-up or pull-down resistor enabled.
 *
 * Regular buttons are defined by defineButton(). They are directly connected
 * to the microcontroller running the game code.
 *
 * MCP23009 buttons are connected to an external MCP23009 IO expander via I2C.
 *
 * Each button is identified by a unique string ID.
 *
 * To check if a button is currently pressed, you can use isButtonPressed().
 *
 * To be notified of when a given combination of buttons (or even a single
 * button) is pressed, you can use defineButtonCombo().
 */
class InputEngine
{
    friend void _InputEngineTaskMain(void* params);
    
public:
    /**
     * \brief Configuration flags for button pins.
     */
    enum PinFlags
    {
        PinFlagsActiveHigh  = 0,    ///< The button is active-high (i.e. HIGH when pressed).
        PinFlagsActiveLow   = 0x01, ///< The button is active-low (i.e. LOW when pressed).
        PinFlagsPullup      = 0x02, ///< The internal pull-up resistor is enabled.
        PinFlagsPulldown    = 0x04  ///< The internal pull-down resistor is enabled.
    };
    
    /**
     * \brief Callback function for when a button combination is pressed.
     */
    typedef void (*ButtonComboCb)();
    
private:
    enum class ButtonType
    {
        Regular,
        MCP23009
    };

    struct MCP23009Device
    {
        MCP23009Device(uint8_t i2cAddr) : mcp(i2cAddr) {}
        MCP23008 mcp;
        uint8_t lastState;
    };
    
    struct ButtonDef
    {
        ButtonDef(const std::string& id, ButtonType type, int flags) : id(id), type(type), flags(flags), pressed(false), debounceCount(0) {}
        
        std::string id;
        ButtonType type;
        int flags;
        bool pressed;
        uint8_t debounceCount;
        
        union {
            struct {
                uint8_t pin;
            } regular;
            struct {
                MCP23009Device* dev;
                uint8_t pin;
            } mcp23009;
        };
    };
    
    struct ButtonCombo
    {
        ButtonCombo(const std::unordered_set<std::string>& ids, ButtonComboCb cb) : ids(ids), cb(cb) {}
        std::unordered_set<std::string> ids;
        ButtonComboCb cb;
    };
    
public:
    /**
     * \brief Create a new engine.
     *
     * Users should **not** call this method directly.
     *
     * Note that begin() must still be called to initialize it.
     */
    InputEngine() : debounceCount(0) {}
    
    /**
     * \brief Initialize the input engine.
     *
     * \param debounceCount The number of milliseconds used for debouncing
     *      changes in the button states.
     * \return true if successful, false otherwise.
     */
    bool begin(uint8_t debounceCount = 10);
    
    
    /// \name Defining Buttons
    ///@{
    
    /**
     * \brief Define a button that's directly connected to the microcontroller.
     *
     * \param id An arbitrary unique ID for the button. This is used later to
     *      identify this button.
     * \param pin The pin to which the button is connected.
     * \param flags Configuration flags. Default is active-low with pull-up.
     * \return true if successful, false otherwise.
     */
    bool defineButton(const std::string& id, uint8_t pin, int flags = PinFlagsActiveLow | PinFlagsPullup);
    
    /**
     * \brief Define a button that's connected to an external MCP23009 via I2C.
     *
     * Multiple buttons can be defined on the same MCP23009, and multiple
     * MCP23009 devices can be used as long as they have different I2C addresses
     * and are on the same default I2C bus.
     *
     * \param id An arbitrary unique ID for the button. This is used later to
     *      identify this button.
     * \param pin The pin to which the button is connected. For the MCP23009,
     *      this is a number between 0 and 7.
     * \param flags Configuration flags. Default is active-low with pull-up.
     * \param i2cAddr The I2C address of the MCP23009 device. The default value
     *      assumes the ADDR pin to be connected directly to GND.
     * \return true if successful, false otherwise.
     */
    bool defineButtonMCP23009(const std::string& id, uint8_t pin, int flags = PinFlagsActiveLow | PinFlagsPullup, uint8_t i2cAddr = 0x20);
    
    ///@}
    
    
    /// \name Querying Button State
    ///@{
    
    /**
     * \brief Check if the given button is currently pressed.
     *
     * Note that buttons are checked with debouncing in a separate task, so this
     * method deliberately provides a delayed view of the button.
     *
     * \param id The button ID.
     * \return true if pressed, false otherwise.
     */
    bool isButtonPressed(const std::string& id);
    
    /**
     * \brief Defines a button combination for which to watch out.
     *
     * The given callback function will automatically be called whenever the
     * given combination of buttons is pressed. If the buttons are kept pressed,
     * the callback will **not** be called again, until at least one of the
     * buttons is released and then pressed again.
     *
     * \param ids The set of button IDs for the combo. It is valid to provide a
     *      single button ID here.
     * \param cb The callback function.
     */
    void defineButtonCombo(const std::unordered_set<std::string>& ids, ButtonComboCb cb);
    
    ///@}

private:
    void inputTaskMain();
    
    MCP23009Device* registerMCP23009(uint8_t addr);
    
    ButtonDef* getButtonDef(const std::string& id);
    
    bool debounceButton(ButtonDef* def, bool pressed);

private:
    TaskHandle_t inputTask;
    SemaphoreHandle_t inputMtx;
    
    std::vector<MCP23009Device*> mcp23009Devices;
    
    std::unordered_map<std::string, ButtonDef*> buttons;
    
    std::vector<ButtonCombo*> buttonCombos;
    
    uint8_t debounceCount;
};

}

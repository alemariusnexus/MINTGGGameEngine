#pragma once

#include "../Globals.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../platform/GPIODevice.h"
#include "../platform/GPIODeviceNative.h"


namespace MINTGGGameEngine
{

/**
 * \brief Handles all game inputs.
 *
 * Use Game.input() to receive the active input engine.
 *
 * Currently, this class only handles buttons and analog axes.
 *
 * \section sec_buttons Buttons
 *
 * Two types of buttons are currently supported. Buttons can be active-high or
 * active-low, and can optionally have a pull-up or pull-down resistor enabled.
 *
 * Regular buttons are defined by defineButton(). They are directly connected
 * to the microcontroller running the game code.
 *
 * MCP2300X buttons are connected to an external MCP23008/MCP23009 IO expander
 * via I2C.
 *
 * Each button is identified by a unique string ID.
 *
 * To check if a button is currently pressed, you can use isButtonPressed().
 *
 * To be notified of when a given combination of buttons (or even a single
 * button) is pressed, you can use defineButtonCombo().
 *
 * \section sec_axes Analog Axes
 *
 * An analog axis maps an external analog signal to the range [-1.0, 1.0].
 * The most common example of analog axes is an analog joystick: A 2D analog
 * joystick consists of two axes: One for the x direction, and one for the y
 * direction. See defineAxis() for configuration details.
 *
 * The value of an axis can be read with getAxis(). You can also use
 * getAxisRaw() to get the raw ADC value of the axis before processing, which
 * might be useful for joystick calibration.
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
    typedef std::function<void()> ButtonComboCb;
    //typedef void (*ButtonComboCb)();
    
private:
    struct ButtonDef
    {
        ButtonDef(const std::string& id, unsigned int pin, GPIODevice* dev, int flags) : id(id), pin(pin), dev(dev), flags(flags), rawState(0), pressed(false), debounceCount(0) {}
        
        std::string id;
		unsigned int pin;
        GPIODevice* dev;
        int flags;
        
		uint8_t rawState;
        bool pressed;
        uint8_t debounceCount;
    };
    
    struct AxisDef
    {
        AxisDef(const std::string& id, uint8_t pin) : id(id), pin(pin), minValue(0.0f), maxValue(1.0f),
                neutralValue(0.5f), neutralWidth(0.1f), rawValue(0.5f), value(0.5f) {}
        
        std::string id;
        uint8_t pin;
        float minValue;
        float maxValue;
        float neutralValue;
        float neutralWidth;
        
        float rawValue;
        float value;
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
     * \brief Define a button.
     *
     * \param id An arbitrary unique ID for the button. This is used later to
     *      identify this button.
     * \param pin The pin to which the button is connected.
	 * \param gpioDevice The GPIO device on which the button is connected. Uses
	 *		the default on-board GPIOs by default.
	 * \param flags Configuration flags. Default is active-low with pull-up.
     * \return true if successful, false otherwise.
     */
    bool defineButton (
			const std::string& id,
			unsigned int pin,
			GPIODevice& gpioDevice = GPIODeviceNative::getInstance(),
			int flags = PinFlagsActiveLow | PinFlagsPullup
			);
    
    /**
     * \brief Remove a previously defined button.
     *
     * \param id The button ID.
     * \return true if successfully undefined, false if button not found.
     */
    bool undefineButton(const std::string& id);
	
	/**
	 * \brief Return a list of all defined button IDs.
	 *
	 * \return List of button IDs.
	 */
	std::vector<std::string> getButtonIDs() const;
    
    ///@}
    
    
    /// \name Defining Analog Axes
    ///@{
    
    /**
     * \brief Defines an analog axis (e.g. for a joystick).
     *
     * Each axis value is provided by an analog signal, which is linearly mapped
     * to the range [-1.0, 1.0], with 0.0 being the neutral position (e.g. a
     * joystick axis in middle position).
     * A typical 2D joystick consists of two axes: one for the x direction, and
     * one for the y direction.
     *
     * The raw analog input is first mapped to the range [0.0, 1.0]. The result
     * of this mapping is what the configuration values of this function
     * reference (i.e. they are independent of the actual ADC range and
     * resolution).
     *
     * If minValue > maxValue, the axis will be inverted.
     *
     * \param id An arbitrary unique ID for the axis. This is used later to
     *      identify this axis.
     * \param pin The pin from which the analog signal is read. Must be an
     *      ADC-capable pin.
     * \param minValue Raw ADC value at which the axis should yield -1.0.
     * \param maxValue Raw ADC value at which the axis should yield 1.0.
     * \param neutralValue Raw ADC value at which the axis should yield 0.0.
     * \param neutralWidth Range around the neutral value at which the axis
     *      should still yield 0.0. This is useful to avoid slight drifting if
     *      e.g. a joystick doesn't center itself on an exact value.
     * \return true if successful, false otherwise.
     */
    bool defineAxis (
            const std::string& id, uint8_t pin,
            float minValue = 0.0f, float maxValue = 1.0f,
            float neutralValue = 0.5f, float neutralWidth = 0.1f
            );
    
    /**
     * \brief Remove a previously defined axis.
     *
     * \param id The axis ID.
     * \return true if successfully undefined, false if axis not found.
     */
    bool undefineAxis(const std::string& id);
	
	/**
	 * \brief Return a list of all defined axis IDs.
	 *
	 * \return List of axis IDs.
	 */
	std::vector<std::string> getAxisIDs() const;
    
    ///@}
    
    
    /// \name Querying Button State
    ///@{
    
    /**
     * \brief Check whether a button is defined.
     *
     * \param id The button ID.
     * \return true if defined, false otherwise.
     */
    bool hasButton(const std::string& id);
    
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
    
    
    /// \name Querying Analog Axis State
    ///@{
    
    /**
     * \brief Check whether an axis is defined.
     *
     * \param id The axis ID.
     * \return true if defined, false otherwise.
     */
    bool hasAxis(const std::string& id);
    
    /**
     * \brief Get the current value of the given axis.
     *
     * \param id The axis ID.
     * \return The axis value, in range [-1.0, 1.0].
     */
    float getAxis(const std::string& id);
    
    /**
     * \brief Get the raw value of the given axis.
     *
     * This returns the raw ADC value mapped to range [0.0, 1.0], before any
     * calculations involving min, max or neutral position.
     *
     * \param id The axis ID.
     * \param The raw axis value, in range [0.0, 1.0]
     */
    float getAxisRaw(const std::string& id);
    
    ///@}

private:
    void inputTaskMain();
    
    ButtonDef* getButtonDef(const std::string& id);
    AxisDef* getAxisDef(const std::string& id);
    
    bool debounceButton(ButtonDef* def, bool pressed);

private:
    TaskHandle_t inputTask;
    SemaphoreHandle_t inputMtx;
    
	std::vector<std::string> buttonIDs;
    std::unordered_map<std::string, ButtonDef*> buttons;
	std::unordered_map<GPIODevice*, std::vector<ButtonDef*>> buttonsByDevice;
	
	std::vector<std::string> axisIDs;
    std::unordered_map<std::string, AxisDef*> axes;
    
    std::vector<ButtonCombo*> buttonCombos;
    
    uint8_t debounceCount;
};

}

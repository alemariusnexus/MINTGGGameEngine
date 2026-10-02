#pragma once

namespace MINTGGGameEngine
{

/**

\page Input


\section input_overview Overview

Input in the engine is handled by the \ref InputEngine class, which must be accessed through \ref Game::input().

Currently, the input engine supports the following input devices only:

- Buttons: This includes both push buttons and binary switches
- Analog axes: This usually means joysticks, but can be any other input device that outputs an analog signal readable
  by a device's ADC.

Input devices must be defined once, usually at game setup, and their state can later be checked at any time. Input
devices are monitored in a separate task in the engine, so they should be handled correctly even if the game "lags".

The following sections will give a general overview for how to work with the input engine, but you should look at
the documentation for \ref InputEngine for more details.


\section input_buttons Buttons

A button is a binary input device: It is either pressed or released. This terminology is borrowed from push buttons,
which are the simplest supported types. Nevertheless, binary switches (which are flipped instead of having to be
actively held down) are also considered buttons for the input engine, and are also considered pressed or released
depending on their current flip direction.

Simple buttons that are directly connected to the microcontroller can be defined like this:

\code{.cpp}
    game.input().defineButton("left", 16); // "left" button on pin 16 (active-low, pull-up)
    game.input().defineButton("right", 17); // "right" button on pin 16 (active-low, pull-up)
\endcode

Each button has a unique ID that you can choose freely. This ID is later used to identify the button when querying its
state. Note that by default, buttons are considered active-low (i.e. pressed when their pin has a low voltage) with
an internal weak pull-up resistor enabled. If your button is active-high, you can use the following:

\code{.cpp}
    // "left" button on pin 16 (active-high, pull-down)
    game.input().defineButton("left", 16,
            GPIODeviceNative::getInstance(),
            InputEngine::PinFlagsActiveHigh | InputEngine::PinFlagsPulldown);
\endcode

Once defined, you can check whether a specific button is currently pressed or not:

\code{.cpp}
    if (game.input().isButtonPressed("left")) {
        // "left" is pressed
    }
    if (game.input().isButtonPressed("right")) {
        // "right" is pressed
    }
\endcode

As noted above, the input engine actually reads button states continuously in a separate task (many times a second) and
then saves their current state internally, so the code above will always show a **slightly delayed** view of the button.
This is by design, as it makes querying a button state very quick (isButtonPressed() itself does not do any electrical
measurement), allows features like automatic debouncing of buttons, and allows for automatic callbacks whenever a button
state changes.

The method isButtonPressed() will return true while the button is held, which might be the case for several frames
if it is held down for a while. This can be a problem if you want to do something exactly once for each press of a
button. For this reason, there is a separate method that returns true only during the first frame during which a
button gets pressed:

\code{.cpp}
    // Somewhere in the game loop (on every frame)
    if (game.input().isButtonPressedThisFrame("shoot")) {
        // Shoot button was freshly pressed this frame -> maybe shoot a projectile now.
    }
\endcode

Code like this can alternatively be implemented using button combos with only one button, as described in the following
subsection.

\subsection input_buttons_callbacks Callbacks and Button Combos

The input engine can automatically recognize when certain combinations of buttons are pressed together (including a
combination of just one button), and can automatically call a user-defined function whenever that happens. Here's
an example, where the function onUltraBlast() will automatically be called once whenever the buttons "a", "b" and
"start" are pressed at the same time:

\code{.cpp}
    // On the global level:
    void onUltraBlast() {
        game.despawnObjects(game.getGameObjectsWithTag(TagEnemy)); // Despawn all enemies (BOOM!)
    }

    ...

    // Somewhere during game setup (assumes "a", "b" and "start" buttons have been defined):
    game.input().defineButtonCombo({"a", "b", "start"}, onUltraBlast); // Combo: a+b+start
\endcode


\section input_axes Analog Axes

An analog axis is a device that delivers an analog voltage that can be measured by the microcontroller's
analog-to-digital converter (ADC). Such an axis is usually a device that can move continuously between some defined
start and end point, like a slider.

The most common example for analog axes is a simple 2D joystick. Such a joystick consists of two independent axes:
x axis and y axis. It can be defined like this:

\code{.cpp}
    game.input().defineAxis("x", 32); // x axis connected to ADC pin 32
    game.input().defineAxis("y", 33); // y axis connected to ADC pin 33
\endcode

It's also possible to invert an axis by swapping the minimum and maximum values like so:

\code{.cpp}
    game.input().defineAxis("x", 32, 1.0, 0.0); // inverted x axis
\endcode

More configuration options for analog axes exist, e.g. for defining dead zones. See \ref InputEngine::defineAxis() for
details.

For each axis, the input engine converts the external analog signal linearly to a real number between -1.0 and 1.0.
This value can be read using \ref InputEngine::getAxis(). The following example shows how to move a player object
based on the values of two joystick analog axes:

\code{.cpp}
    player.move(game.input().getAxis("x"), game.input().getAxis("y"));
\endcode

Note again that the actual axis values are measured in a separate task, so the values read through getAxis() will
be slightly delayed.


\section input_gpiodevice Using External GPIO Devices

By default when using \ref InputEngine::defineButton(), the button is assumed to be connected directly to the main
microcontroller's native GPIO port ("the microcontroller pins"). However, the input engine supports buttons connected
to arbitrary GPIO devices supported by the engine. See the \ref GPIODevice class for details on such devices.

For example, the following code shows how to define a button that is connected to an external MCP2300X GPIO expander:

\code{.cpp}
    // Somewhere in the global scope
    GPIODeviceMCP2300X ioExpander(16, 17); // GPIO expander I2C interface connected to pins 16 (SCL) and 17 (SDA)

    // During game setup
    ioExpander.begin(); // Setup the GPIO expander
    game.input().defineButton("up", 0, ioExpander);
    game.input().defineButton("down", 1, ioExpander);
    game.input().defineButton("left", 2, ioExpander);
    game.input().defineButton("right", 3, ioExpander);
\endcode

The \ref GPIODevice to be used is passed directly to defineButton(). Pin numbers in this case are the IO port numbers
of the GPIO expander.

*/

}
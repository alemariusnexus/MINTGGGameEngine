#pragma once

namespace MINTGGGameEngine
{

/**

\page Core Concepts


\section core_overview Overview

This page will briefly introduce some of the most basic, core concepts on which the engine is built. For details, you
should check the various subpages on specific engine components, as well as the documentation for individual classes.


\subsection core_overview_scope What The Engine Is And Isn't

This engine was created as a relatively simple game engine specifically for certain kinds of higher-end
microcontrollers and embedded systems. Several design decisions were made with this in mind, and it is important to
keep this in mind when thinking about what kinds of games can be implemented with it.

Here are some things that the engine is or isn't designed for. The engine is ...

- ... targeted mainly at **higher-end microcontrollers**. Much better alternatives exist for desktop or mobile games (e.g.
  Unity, Godot). It also has no chance of running on smaller microcontrollers like the ATmegas used in most common
  Arduino boards.
- ... mainly designed for **educational use**. It does not claim to provide the most features of any game engine, nor
  does it aim to be the fastest. It is designed with the goal of simple things being simple to do. Advanced
  features may or may not be supported, and may or may not be well documented.
- ... aimed at a **specific set of hardware**. While an attempt is made to make some portions of the engine
  hardware-agnostic and provide a means to implement drivers for different kinds of hardware, its internal flexibility
  is limited, and currently only a small set of actual devices (microcontrollers, screens, input/output devices etc.)
  are actually supported.


\section core_platforms Supported Platforms

The engine is intended to run on a single higher-end microcontroller. Many internal parts of it are very dependent on
the specific kind of microcontroller used. Currently, only the following platforms are supported:

- **ESP32** (the "original" one)
- **ESP32C3**
- Other ESP32 variants may or may not work, but they haven't been tested.
- **Desktop**: The desktop port is considered experimental, and is mostly tested on Windows, but Linux and macOS
  should generally work, too.

Other microcontrollers (including the various ATmega- or megaAVR-based Arduinos) are **not** supported.

As far as development platforms go, only the following are currently supported:

- <a href="https://www.arduino.cc/">Arduino</a> IDE: This engine can be used as a regular Arduino library. Even so, it
  makes direct use of many ESP-IDF functions that the ESP32 port for Arduino is itself based on. That's why it
  does **not** work with the various other Arduino-based platforms.
- <a href="https://docs.espressif.com/projects/esp-idf/en/stable/esp32/index.html">ESP-IDF</a>: The engine can be used
  as a component in an ESP-IDF project. It does **not** require the Arduino compatibility layer for this. The engine
  itself is actually mainly developed in <a href="https://www.jetbrains.com/clion/">CLion</a>, using ESP-IDF directly
  without any Arduino components. Currently, ESP-IDF version 6.1 is targeted (earlier or later versions may or may not
  work).
- <a href="https://www.qt.io/">Qt</a>: The desktop port of the engine requires the Qt toolkit, currently at least
  version 6.8. On Windows, you need the MinGW version. The Visual Studio version will not work.


\section core_project The Starter Project

To create a new game, it is best to use
<a href="https://github.com/alemariusnexus/MINTGGGameEngine_MyGame">the starter project</a> as a base. It contains all
the files necessary to setup the engine, and contains all code necessary for an empty game. How you open it depends on
the platform you're working on:

- For Arduino IDE, simply open the file main/main.ino in Arduino IDE. Note that this file is mostly empty and only
  exists as a placeholder for Arduino. The main game code is still in main.cpp.
- For ESP-IDF, the root directory of the starter project is a valid ESP-IDF project. Make sure you have the latest
  version of the engine downloaded in the components subdirectory. You can then build the game just like any other
  ESP-IDF project (using the idf.py script). A guide for setting this up with IDEs like CLion will be created
  shortly. Due to more difficult setup, this option is currently **for advanced users only**.
- For Qt (desktop port), you can open the root directory of the starter project as a CMake project. Make sure to
  configure the paths to Qt in CMake, and use the same version of MinGW that was used to compile Qt itself.

The main file you will be using is main/main.cpp, which will contain most of your own game code. As your game gets
more complex, you might take a look at some of the other files in the main directory. Here's a quick overview:

- **main.cpp**: The main file for your game code. It is explained in more detail below.
- **main.h**: Main header file. Might be useful to declare global variables that you want to access in other files, if
  your game becomes complex enough for you to create additional source files later.
- **main.ino**: Just a placeholder file for opening the project in Arduino IDE. You should not put any code here.
- **engine.cpp**: Contains the setup code for starting the engine. You can edit it if the default engine configuration
  is not suitable for you. If you update to a new engine version, this file might need to be adapted to it.
- **defines.h**: Contains some configuration options for the engine, especially things like display and pin
  configurations. To switch between different configurations, you can create more defines_*.h files, and include
  them in the main defines.h.
- **CMakeLists.txt**: This file is ignored in Arduino IDE, but contains information about which drivers to include
  and which source files to build for other platforms like ESP-IDF and desktop.


\subsection core_project_main The Main File

To write your own game, you should edit the file main/main.cpp. It should contain the actual game code, and defines
functions that are called by the engine to make the game run. Here is a brief overview of the functions in this file:

- **gameSetup()**: This function is called exactly once by the engine when the game starts (after the engine itself
  finishes its setup in engine.cpp). You might want to create the player, define input devices, load bitmaps and
  audio files, and do other setup code here.
- **gameLoop()**: This function is called once every frame by the engine, to handle the continuous game logic. It will
  be called multiple times per second, depending on the currently configured FPS, and on how long the frame processing
  takes. You can do things like move GameObjects, check for button inputs or spawn new enemies at certain times here.
  The function gets a parameter called **dt** (delta time), which is the number of seconds that have passed since the
  last frame (useful to create framerate-independent logic).
- **postDraw()**: This function gets called once every frame, after all drawing for the frame has finished. It can be
  used to draw additional things like a UI or an overlay on top of the screen. Most games don't need to use this
  function.
- **onCollision()**: This function will be called whenever the engine detects that two GameObjects collide with each
  other. You can use this e.g. to make the player lose health when they touch an enemy, or to despawn an enemy when it
  collides with a bullet shot by a player.

*/

}
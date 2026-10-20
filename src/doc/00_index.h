#pragma once

namespace MINTGGGameEngine
{

/**
 * \mainpage MINTGGGameEngine - A Simple 2D %Game %Engine for Microcontrollers
 *
 *
 * \section index_overview Introduction
 *
 * %MINTGGGameEngine is a simple C++ 2D game engine, mainly intended for larger
 * microcontrollers, with support for simple peripheral components like small
 * color displays, buttons, joysticks or piezo speakers.
 *
 * It was first developed for ESP32 microcontrollers, but has been ported to
 * a few other platforms as well. Although designed to be somewhat extensible,
 * it only supports certain kinds of chips and peripherals right now. Its main
 * focus is on simplicity, with an API that's relatively easy to use for
 * beginners. It does not claim to be the most portable game engine, nor the
 * fastest or the one with the most features.
 *
 * The engine is developed by me, David Lerch, as a teacher for my student
 * courses on microcontrollers at the Berufliche Schulen Groß-Gerau and the
 * MINT-Zentrum Groß-Gerau in Germany.
 *
 *
 * \section index_setup Getting Started
 *
 * To get started, you first have to set up your system for development. A detailed
 * guide for all supported platforms can be found in \ref setup
 *
 * Once this setup is done, you can try following the tutorial for creating a
 * simple game here: \ref tutorial_simple
 *
 * After that, you can get more information about the core concepts and most
 * important classes of the engine in \ref core, and then continue with the other
 * documentation chapters depending on what you need for your game.
 *
 *
* \section index_features Features
 *
 * Here's a broad overview of the engine's features:
 *  - Spawning, positioning and movement of **GameObjects** in a 2D scene
 *  - **Rendering of 2D sprites** (circles, rectangles, bitmaps) on a simple
 *    color screen
 *  - **Collision detection** and ray casting with circles and axis-aligned
 *    rectangles as basic shapes
 *  - **Audio playback** for simple single-channel, single-voice sound output
 *    on a piezo speaker
 *  - **Input handling**, with support for buttons, switches and analog
 *    axes (e.g. joysticks).
 *
 * For more details on the features, as well as supported peripheral devices,
 * see the other sections of this documentation.
 *
 *
 * \section index_platforms Platforms
 *
 * A description of the platforms the engine runs on can be found on a dedicated
 * page: \ref platforms
 *
 *
 * \section index_links Links
 *
 * This documentation is for the engine itself. You can find the source code of
 * [the engine on Github](https://github.com/alemariusnexus/MINTGGGameEngine).
 *
 * To create a new game, you can use the
 * [skeleton project](https://github.com/alemariusnexus/MINTGGGameEngine_MyGame), which
 * provides the basic structure for an empty game. There is also an example
 * [Shoot 'em up](https://github.com/alemariusnexus/MINTGGGameEngine_ShootEmUp)
 * that was developed as a proof-of-concept, and a few additional
 * [Demo Projects](https://github.com/alemariusnexus/MINTGGGameEngine_Demos)
 * showing off various featuers of the engine.
 */

}

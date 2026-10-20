#pragma once

namespace MINTGGGameEngine
{

/**
 * \page platforms Platforms
 *
 *
 * \section plaforms_overview Overview
 *
 * This page is an overview of some of the important features, limitation and special considerations
 * necessary when running the engine on the different supported platforms. It is **not** a guide for
 * how to get the engine or a guide to work on these platforms. For that, take a look at \ref setup
 * instead.
 *
 *
 * \section platforms_arduino Arduino
 *
 * \subsection platforms_arduino_issues Known Issues
 *
 * Arduino is definitely the *odd one out* among the supported platforms, in that it must frequently
 * be handled as a special case in certain parts of the engine. While the Arduino IDE and its
 * associated system are great for beginners, they are unfortunately **extremely lacking when it comes
 * to providing advanced features**, or following proper software development standards - which is in
 * part by design. That is why not all engine features will work when developing for Arduino, although
 * most still do.
 *
 * The biggest disappointment of the Arduino ecosystem is its *build system*, or more specifically,
 * its **lack of a build system**. While the vast majority of modern C++ software development now
 * uses CMake as the core of its build system, or is at least somewhat compatible with it, Arduino
 * has absolutely no support for it. Even worse, Arduino's build system can hardly even be called
 * one. Arduino just compiles all C++ files it finds in the sketch or library directories
 * unconditionally with default settings and calls it a day. This is extremely primitive, and leads
 * to certain limitations mentioned below.
 *
 * The following is a non-exhaustive list of issues with Arduino:
 *
 *  - There is currently no **support for uploading the _data_ directory** to a device. This means
 *    that unless you support an SD card in your hardware, you currently have to upload any files like
 *    bitmaps or MIDIs for your game by **converting them to C/C++ code first**. There are third-party
 *    plugins for Arduino that *can* upload the *data* directory, but none of them are particularly
 *    satisfactory, and none of them are specifically supported by the engine.
 *  - Arduino usually **lacks behind a few versions when it comes to ESP-IDF**. There is currently
 *    nothing major that the engine needs which these older versions don't also provide, but that
 *    may change in the future.
 *
 *
 * \section platforms_espidf ESP-IDF
 *
 * \subsection platforms_espidf_issues Known Issues
 *
 * **Nothing. ESP-IDF is perfect.**
 *
 * That's of course not true. But ESP-IDF is the main platform targeted by the engine, so it's
 * the baseline for what the engine does, and there are currently no major features that are
 * provided on other platforms but missing for ESP-IDF.
 *
 *
 * \section platforms_desktop Desktop
 *
 * \subsection platforms_desktop_issues Known Issues
 *
 * The desktop port of the engine is mostly intended for quicker development, debugging, and
 * demonstration purposes. While you can use it to develop simple desktop games, be aware that
 * **all the limitations of this engine being intended for microcontrollers still show up in the
 * desktop port.** The engine is not intended to compete with actual desktop game engines
 * like Unity or Godot. There are also some bugs and features that are not currently supported.
 *
 * Here's a non-exhaustive list of the known issues with the desktop port:
 *
 *  - The input system currently **does not support analog axes** on desktop. It would of course
 *    be possible and desirable to support game controllers for them, or to emulate them using
 *    digital controls like keys, but this has simply not been implemented yet.
 *  - The **audio system is current glitchy.** For the desktop port, the piezo speaker must be
 *    "emulated", and since audio isn't the main focus of the engine, this emulation is currently
 *    very primitive and buggy. Tones can sometimes be played longer or shorter than expected,
 *    and fast sequences of tones are not handled correctly.
 *  - Features that require **specific external hardware are of course not supported**. That means
 *    you can't actually attach physical buttons or sensors to your game. Buttons are instead
 *    emulated by keyboard keys. See below for details.
 *
 * \subsection platforms_desktop_config Configuration
 *
 * The desktop port uses the **configuration file _config/desktop.json_** in the app root directory
 * to control various aspects of the hardware emulation necessary for desktops. The available settings
 * are described in the following sections.
 *
 * \subsection platforms_desktop_screens Screen Support
 *
 * The engine allows game developers to configure the physical screen that the game will be shown
 * on. There is of course generally no way to attach these screens to a desktop computer, or it
 * is at least not something that's usually done. For this reason, the desktop port **simply shows
 * the contents of the game screen in a regular window**. It does not matter which kind of screen
 * the game is configured with, or which pins are configured. The engine **will handle all screen
 * types the same way** on desktop.
 *
 * The configured **screen's resolution is still respected**. This means that the entire rendering
 * still happens at the in the game's configured screen resolution. By default however, the image is
 * **automatically scaled up** (or down) to the current window size, preserving the original aspect
 * ratio. This behavior can be changed using the configuration option *screen/transformMode*. The
 * default setting is *fast*, which leads to a very "pixelly" image if the original screen
 * resolution is much lower than the desktop window resolution, but it preserves the original
 * image better. *smooth* can be used to average the pixel values out more, which will lead to
 * a less blocky, but often very blurry image. *none* can be used to completely disable scaling,
 * so the window will show the image in exactly the original resolution, regardless of window
 * size.
 *
 * An additional option *screen/keepAspectRatio* exists. If set to *false*, the image will be
 * **stretched** to fit the window size exactly, regardless of the original aspect ratio.
 *
 * \subsection platforms_desktop_input Input
 *
 * On a microcontroller, input is usually handled by defining buttons and axes and assigning
 * them pin numbers on the chip's **GPIO port** (or an external IO expander) where the physical
 * buttons are connected. This is of course not generally possible on a desktop system. So
 * instead, **buttons are mapped to keyboard keys**: The button is considered pressed if the
 * keyboard key is pressed. The pin number and GPIO device configured by the game code for
 * each button is ignored on desktop.
 *
 * Instead, buttons are mapped to keyboard keys **in the config file under the _keymap_ option**.
 * each entry in the *keymap* maps one keyboard key to one button. Multiple keys can be mapped
 * to the same button, but not vice-versa. The "left value" of each entry (JSON key) defines
 * the keyboard key. The value is interpreted by Qt's
 * [QKeySequence](https://doc.qt.io/qt-6/qkeysequence.html) constructor. Unfortunately, the
 * values it accepts aren't documented very well, so you have to experiment a bit with them.
 * The "right value" (JSON value) is the ID of the corresponding button, as defined by
 * \ref InputEngine::defineButton().
 *
 * As mentioned under \ref platforms_desktop_issues, analog axes are currently not supported
 * on desktop, and will always read as 0.
 *
 * \subsection platforms_desktop_storage Storage
 *
 * The StorageEngine is **fully supported** on desktop.
 *
 * Individual values are saved and loaded using Qt's
 * [QSettings](https://doc.qt.io/qt-6/qsettings.html) class, which uses different backends on
 * each platform: On Windows, it uses the registry. On Linux, it uses a simple configuration
 * file.
 *
 * Both the **internal storage and the SD card support** of the storage engine also exist on
 * the desktop port. Both are simply implemented as **special directories** on the file system.
 * The internal storage will use the *data* directory, and the SD card will use the *sdcard*
 * directory (if they exist). Paths like <i>/storage/player.bmp</i> still work on desktop, and
 * are automatically converted to the corresponding full file system paths.
 *
 * **IMPORTANT**: The conversion from absolute paths like <i>/storage/player.bmp</i> to the
 * real file system paths is handled by \ref StorageEngine::resolvePath(). This method is
 * automatically called by the storage classes (e.g. \ref File, \ref FileReader) and so the
 * conversion is automatic for all APIs of the engine. If you want to pass such a path to
 * an API that is not part of the engine (like the standard C/C++ library), you have to
 * make sure to call this method explicitly.
 */

}

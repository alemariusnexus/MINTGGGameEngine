#pragma once

namespace MINTGGGameEngine
{

/**
 * \page setup Setup
 *
 *
 * \section setup_overview Overview
 *
 * This guide will explain how to set up the development tools, the engine, and
 * the skeleton project on your system, so you can get started with developing
 * your games.
 *
 * This guide will mostly explain the setup for **Windows** computers. The steps are
 * similar for Linux systems, but are not fully spelled out in this guide.
 *
 * First, you have to decide which **platform** you want to use for development.
 * Here are your options:
 *
 *  - <b>\ref setup_arduino</b> - The simplest to setup up **for
 *    beginners**. If you're new to development, or to microcontrollers specifically,
 *    this is the recommended platform. More advanced and experienced developers will
 *    find the Arduino platform quite limiting though, and some engine features might
 *    not work on it.
 *  - <b>\ref setup_espidf</b> - The preferred platform **for serious developers**. The
 *    setup is a bit more involved and can be frustrating for beginners, but once
 *    everything is set up, it gives you much more power over advanced features and
 *    productivity.
 *  - <b>\ref setup_desktop</b> - While this is not the main intended platform for the
 *    engine, it can be useful during development due to faster build times, or if
 *    you don't have the required hardware like the microcontrollers, screens and
 *    buttons yet.
 *
 * **You don't have to settle for one of the platforms forever.** If you stick to the
 * API offered by the engine itself, your code should generally work on all these
 * platforms without changes.
 *
 * The following sections will describe the setup for each of these platforms.
 *
 * More details about the specific platforms, their limitations and special
 * considerations to be aware of are explained in \ref platforms.
 *
 *
 * \section setup_arduino Option A: Setup for Arduino
 *
 * \subsection setup_arduino_ide Installing Arduino IDE
 *
 * First, you have to install the [**Arduino IDE**](https://docs.arduino.cc/software/ide/),
 * which will be where you write your game code, as well as build and upload your
 * code to the hardware.
 *
 * Choose and install the correct version of the Arduino IDE for your system. Arduino
 * IDE 2 is recommended and will be used in this guide, but the older version 1.x
 * should work as well.
 *
 * \subsection setup_arduino_engine Installing the Engine
 *
 * \image html setup_arduino_engine.svg "Downloading the Engine from Github" width=100%
 *
 * Next, you have to install **the engine itself**. The engine can be used directly as an
 * Arduino library, but is currently not included in the Arduino Library Manager, so you
 * must download it manually.
 *
 * To do so, download the full code repository of the engine
 * [**from Github**](https://github.com/alemariusnexus/MINTGGGameEngine). You can click
 * the *Code* button and download it as a ZIP file. There are currently no release
 * versions, so you should just get the latest version of the code from the main
 * branch.
 *
 * **Unzip the code**, and move the folder *MINTGGGameEngine* to your Arduino **libraries
 * directory**. On Windows, this directly is usually found in your home directory under
 * *Documents &rarr; Arduino &rarr; libraries*. To be clear: You should then see the file
 * *Documents &rarr; Arduino &rarr; libraries &rarr; %MINTGGGameEngine &rarr;
 * library.properties*.
 *
 * **IMPORTANT**: When downloading the ZIP file from Github, the unzipped folder
 * will usually have *-main* appended to its name. **You have to make sure to remove
 * that suffix** from the name when putting it into the library directory. The resulting
 * directory has to be called *MINTGGGameEngine* exactly.
 *
 * \subsection setup_arduino_skeleton Getting the Skeleton Project
 *
 * After the engine, you can now setup the **skeleton project**, which gives you all the
 * necessary files for creating a new game from scratch.
 *
 * First, get the full repository of the skeleton project
 * [**from Github**](https://github.com/alemariusnexus/MINTGGGameEngine_MyGame), just like
 * you did for the engine, and unzip it. You can then put the resulting directory
 * *MINTGGGameEngine_MyGame* into your the directory for your Arduino sketches (on
 * Windows, it's usually *Documents &rarr; Arduino*), but you can put it anywhere on your
 * system you want. Then, open is as an Arduino project. There are two ways to do this:
 *
 *  1. By **double-clicking the file** _MINTGGGameEngine_MyGame &rarr; main &rarr;
 *     **main.ino**_. This should automatically start Arduino IDE and open the project.
 *  2. By opening Arduino IDE, choosing ***%File &rarr; Open...***, then selecting the file
 *     main.ino from there.
 *
 * You should then see Arduino IDE open the project, and show you the contents of the
 * first file. That file just contains a comment about being intentionally left blank.
 * To start coding, you should instead choose the file *main.cpp* from the tabs at the
 * top.
 *
 * \subsection setup_arduino_esp32 Installing the ESP32 Board Package
 *
 * \image html setup_arduino_overview.svg "Various steps for configuring the Arduino IDE" width=80%
 *
 * Before building the project, you have to **install the board support** for the ESP32
 * microcontrollers. Open the **Boards Manager** in the IDE (e.g. using one of the buttons
 * in the left side bar, or the Menu *Tools -> Board: XYZ -> Boards Manager...*), and
 * search for *esp32*. Then install the **esp32 by Espressif Systems** package. **Do not
 * use the _Arduino ESP32 Boards by Arduino_** boards. We will use the official Arduino
 * integration by Espressif, which are the original developers of the ESP32 chips.
 *
 * \subsection setup_arduino_build Building and Uploading
 *
 * The skeleton project contains the necessary code for a game that simply does nothing
 * and shows nothing on the screen. You can then **upload it directly** to see if
 * everything works.
 *
 * First, **connect the device** you want to develop for (e.g. an ESP32) to the PC via a
 * USB cable. Then click the big checkbox at the top of Arduino IDE that says ***Select
 * Board***. and select your board from it. You may have to search for the exact name of
 * the board and select the USB port, or Arduino IDE might recognize the board automatically.
 * If no device is recognized at all, check that appears in the Windows Device Manager under
 * *Ports (COM & LPT)*. Otherwise, you might have to install a driver for it first, the
 * procedure for which varies depending on your board.
 *
 * Then, you can click the <b><i>Upload</i> button</b> left of the *Select board* box (arrow
 * pointing to the right). This will compile the game code, and upload it to the board. This
 * may take a while, especially on the first upload. If you see a message like ***Hard
 * resetting via RTS pin...*** at the end, that means it worked, and you can now start
 * developing your game. If you get an error message, you will have to try fixing it first.
 *
 * Whenever you write new code and want to test it on the device, you have to <b>use the
 * <i>Upload</i> button again</b> to build and transfer your game to the device.
 *
 *
 * \section setup_espidf Option B: Setup for ESP-IDF
 *
 * \subsection setup_espidf_idf Installing ESP-IDF
 *
 * The first step is to
 * [**install ESP-IDF itself**](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html).
 * ESP-IDF is the official development framework for the ESP32 family of chips, and even the
 * Arduino version builds on top of it. **Please follow the guide under that link for installing
 * ESP-IDF, as this guide will not explain every step of it**. Nevertheless, here are some
 * general tips for the installation:
 *
 *  - The easiest way is to use the **ESP-IDF Installation Manager (EIM)**, as explained in the
 *    guide. I recommend doing the *custom installation* to be able to select a version of
 *    ESP-IDF, and which chips to install. Most other settings can be left at their default
 *    unless mentioned here.
 *  - You should use **at least ESP-IDF v6.1**, which the engine is currently tested with. Newer
 *    or older versions may or may not work.
 *  - Select all the **ESP32 families** you want to develop for. Selecting more than you need
 *    can't hurt, other than wasting space on your hard drive.
 *  - When the installer asks for an installation directory, you can leave it at the default,
 *    or choose any directory you like. But you should **remember the path** you chose here, as
 *    you will need it in the following steps.
 *
 * \subsection setup_espidf_skeleton Getting the Engine and Skeleton Project
 *
 * Next, you should download
 * [**the skeleton project**](https://github.com/alemariusnexus/MINTGGGameEngine_MyGame) and the engine
 * itself from Github. There are multiple ways to do this, but this guide will describe two
 * them: Downloading **via the Git website**, and downloading **via Git on the command line**.
 *
 * \subsubsection setup_espidf_skeleton_gitweb Option 1: Download from Github Website
 *
 * \image html setup_arduino_engine.svg "Downloading the Engine from Github" width=100%
 *
 * The easiest way is to download both the skeleton project and the engine directly from the
 * Github website. This approach is the easiest, as it doesn't require installing additional
 * software or using command line tools. The downside is that whenever you want to upgrade to a
 * new engine version, you have to **manually download and unpack the engine again.** Also, you
 * can't commit changes to the engine yourself to the public Git repository, if that's something
 * you might want to do in the future.
 *
 * First, **download the skeleton project's repository** from the Github link above (e.g. by
 * clicking _Code &rarr; Download ZIP_), and unzip it anywhere on your system.
 *
 * Then, **do the same for the engine**, which you can also get
 * [from Github](https://github.com/alemariusnexus/MINTGGGameEngine). Make sure that **the
 * engine's directory is called _MINTGGGameEngine_**, without any prefix like *-main*. Finally,
 * move the engine's directory **into the skeleton project's _components_ directory**. To be clear,
 * you should then have the file _MINTGGGameEngine_MyGame &rarr; components &rarr; %MINTGGGameEngine
 * &rarr; CMakeLists.txt_. The engine itself is an ESP-IDF component, and the components
 * directory is the easiest way for the skeleton project to find the component.
 *
 * \subsubsection setup_espidf_skeleton_gitcli Option 2: Using Git from the Command Line
 *
 * The more professional way is to download the project properly using the version control
 * software [**Git**](https://git-scm.com/). This makes it more convenient to update to new versions
 * of the engine. It also allows you to contribute changes to the engine itself (if approved),
 * and you can host your own game code on sites like Github too, if you want (but don't have to).
 *
 * Using Git from the command line can be a bit daunting for beginners, and this guide will not
 * go into detail on how Git or the command line works. First, **install Git** from the link above.
 * Then, **open a command line** (for Windows, Git itself should install a *Git CMD* or *Git Bash*
 * that you can use), and change into the directory where you want your game project to be installed.
 * Then, run the following Git command:
 *
 * \code{.sh}
 * git clone --recursive https://github.com/alemariusnexus/MINTGGGameEngine_MyGame.git
 * \endcode
 *
 * This command will **setup your own local Git repository**, and fetch the content of both the
 * skeleton project and the engine into it. It should create the directory
 * *MINTGGGameEngine_MyGame*, which you can use for your own game. The engine itself is in the
 * subdirectory components &rarr; %MINTGGGameEngine, and is included as a Git submodule.
 *
 * \subsection setup_espidf_build Building and Uploading
 *
 * You can now start developing your own code, starting with the file %MINTGGGameEngine &rarr; main
 * &rarr; main.cpp. With ESP-IDF, there are many options for which editor or IDE to use for
 * development. This section will show the basic steps for building and uploading the game **from
 * the command line using the ESP-IDF build system**. Alternatively, you can use **any good C++ IDE
 * that supports CMake projects**, and the following section will explain how to use **CLion** for
 * development.
 *
 * To get started with the ESP-IDF build system, you should read the official
 * [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html)
 * for ESP-IDF. The skeleton project is a simple ESP-IDF project, and the engine is an ESP-IDF
 * component. The basic sequence to get started from the command line is the following. Make
 * sure to execute them from a console that has the ESP-IDF environment set up correctly (e.g.
 * the ESP-IDF PowerShell that EIM installs) otherwise they will fail.
 *
 * First, change into the MINTGGGameEngine_MyGame directory. Then set the chip target you want
 * to build for:
 *
 * \code{.sh}
 * idf.py set-target esp32
 * \endcode
 *
 * Then build the project:
 *
 * \code{.sh}
 * idf.py build
 * \endcode
 *
 * And flash it, i.e. upload it to the device. You have to connect your device using the USB
 * cable first, of course:
 *
 * \code{.sh}
 * idf.py flash
 * \endcode
 *
 * For subsequent uploads, you can also use *idf.py app-flash* instead for quicker uploads, as
 * long as only your code changes, and not any files in the *data* directory that you upload to
 * the device using e.g. SPIFFS.
 *
 * To see your game's log outputs, which is useful for debugging when things don't work like
 * they should:
 *
 * \code{.sh}
 * idf.py monitor
 * \endcode
 *
 * **IMPORTANT**: The ESP-IDF monitor is unfortunately quite awkward when it comes to closing it.
 * Sometimes, it reacts to inputs like CTRL-C or CTRL-D, but usually the sequence CTRL-T, CTRL-X
 * is needed instead, sometimes multiple times. Occasionally, it refuses to close even with this
 * sequence. In that case, the easiest way to close it is via the Task manager (look for a Python
 * script to terminate). For some keyboard layouts, other sequences might be necessary.
 *
 * For more details on the ESP-IDF build system, see the official ESP-IDF documentation by
 * Espressif.
 *
 * \subsection setup_espidf_clion Setting Up CLion
 *
 * One good option for development is the IDE [**CLion**](https://www.jetbrains.com/clion/). It is
 * available under a free license for non-commercial use, but be aware that you still need to
 * **register an account** with Jetbrains to use it, and **using it for commercial development
 * usually requires a paid license**. If this is unacceptable to you, can can instead look into
 * alternatives like [Eclipse IDE for C/C++](https://eclipseide.org/) or
 * [Visual Studio Code](https://code.visualstudio.com/), but this guide will not explain how to
 * set them up.
 *
 * First, **download and install CLion** normally. Before opening a game project, a few things must
 * be configured in CLion's global settings, which are explained in the following subsections.
 *
 * \subsubsection setup_espidf_clion_toolchain Configuring the ESP-IDF Toolchain
 *
 * \image html setup_espidf_clion_toolchain.svg "Setup for the Toolchain in CLion" width=80%
 *
 * You have to configure a toolchain, which tells CLion which compilers and other general settings
 * to use for your ESP-IDF projects. In the CLion settings, go to ***Build, Execution, Deployment
 * &rarr; Toolchains***. Click the *+* sign above the toolchain list and choose *System*. Then you
 * need to setup some paths on the right.
 *
 * Give your toolchain a name, e.g. *ESP32-Xtensa*. Then click *Add environment*, and **enter the
 * _Environment file_**. This is necessary to set up various environment variables, which is what
 * e.g. the *IDF Powershell* does automatically. The environment file can be found in the tools
 * directory of your ESP-IDF installation. It should be something like the following on Windows
 * (but make sure to choose the correct path for ***your*** system):
 *
 * \code
 * C:\Espressif\tools\Microsoft.v6.1.PowerShell_profile.ps1
 * \endcode
 *
 * **IMPORTANT**: Many older ESP-IDF installation guides instead recommend you to enter the path
 * to the export.bat/export.sh scripts in the IDF installation path (instead of the tool path).
 * **This should not be done**, and can lead to various errors pertaining to missing environment
 * variables at build time.
 *
 * Next, you have to configure the **path for the _C Compiler_ and the _C++ Compiler_**. Those can
 * also be found in the ESP-IDF tools directory. Depending on which family of ESP32 you want to
 * develop for, you have to choose **either the Xtensa or the RISC-V compiler**. Older chips like
 * the ESP32 or the ESP32-S3 have an Xtensa processor, while newer chips like the ESP32C3 or the
 * ESP32P4 have a RISC-V processor. Here are some example values for the Xtensa compiler (again,
 * make sure to adjust the paths on your system):
 *
 * \code
 * C:\Espressif\tools\xtensa-esp-elf\esp-15.2.0_20251204\xtensa-esp-elf\bin\xtensa-esp32-elf-gcc.exe
 * C:\Espressif\tools\xtensa-esp-elf\esp-15.2.0_20251204\xtensa-esp-elf\bin\xtensa-esp32-elf-g++.exe
 * \endcode
 *
 * The C compiler will generally end in *-gcc*, and the C++ compiler in *-g++*. The other options
 * for the toolchain can usually be left at their default values.
 *
 * \subsubsection setup_espidf_clion_terminal Optional: Terminal
 *
 * \image html setup_espidf_clion_terminal.svg "Setup for the Terminal in CLion" width=80%
 *
 * If you want to use **CLion's built-in terminal** to run the command-line ESP-IDF tools like
 * *idf.py* (which can be useful to see the game's log messages, or for fixing the build when things
 * go wrong), you have to tell it to set up the ESP-IDF environment as well. You can do this in the
 * CLion settings under ***Tools &rarr; Terminal &rarr; Application Settings &rarr; Shell path***.
 * For Windows, enter the following command there (which includes the path you set up during
 * \ref setup_espidf_clion_toolchain for the environment file. Adjust it for your system):
 *
 * \code
 * powershell.exe -NoExit -ExecutionPolicy Bypass -NoProfile -Command "& {. 'C:\Espressif\tools\Microsoft.v6.1.PowerShell_profile.ps1'}"
 * \endcode
 *
 * \subsubsection setup_espidf_clion_project Setting Up the Project
 *
 * \image html setup_espidf_clion_project.svg "Project CMake Setup in CLion" width=80%
 *
 * Now that the global CLion setup is done, you can **open the skeleton project**. In CLion, just
 * use *File &rarr; Open*, and select the directory MINTGGGameEngine_MyGame. CLion might show you
 * the CMake configuration dialog automatically. If not, open it now in the CLion settings under
 * ***Build, Execution, Deployment &rarr; CMake***. Add a new profile using the *+* button, then
 * configure it as follows.
 *
 * Choose any name you want for it, e.g. *ESP32-Debug*. For now, you can set the *Build type* to
 * *Debug*. Under *Toolchain*, **choose the one you set up for ESP-IDF** in the previous steps. Under
 * *CMake options*, enter the following line to define **which ESP32 chip family you want to build
 * the project for**:
 *
 * \code
 * -DIDF_TARGET=esp32
 * \endcode
 *
 * If you use a different chip, adjust the value accordingly (e.g. *esp32c3*, *esp32p4* or *esp32s3*).
 * You can leave the value for the *Build directory* at the default, but once you create more than
 * one CMake profile, you have to give each of them a different directory here. Naming these
 * directories like *build-esp32-debug* is recommended, but not required.
 *
 * The other options can usually be left at their default values.
 *
 * \subsubsection setup_espidf_clion_build Building the Project
 *
 * \image html setup_espidf_clion_build.svg "Important UI elements in CLion" width=80%
 *
 * The setup should be done now. CLion should automatically start the *Configure* step for the
 * CMake profile you just set up. Some warnings or even errors might be printed, but as long as the
 * CMake output ends with lines like *Generating done* and *Build files have been written*, it
 * should be fine. Otherwise, some settings from the previous step are probably missing or wrong.
 *
 * In CLion, you **use the _Build_ button** (a hammer symbol in the top, to the right) for both
 * building and uploading your game to the chip under ESP-IDF. **Do not use the regular run button**,
 * which is intended for running applications on the current desktop system.
 *
 * To the left of the *Build* button, you have to **configure the _Build Target_**, which decides
 * exactly what the *Build* button will do. There are a ton of values, almost all of which are
 * internal and should not be used directly. The three interesting values for ESP-IDF you should use
 * here are **app**, **flash** and **app-flash**:
 *
 *  - **app** will **build** the project, but not upload it to the chip. Useful during development when
 *    you want to check if there are build errors, but don't want to test the game on the chip yet.
 *    This corresponds to the *idf.py build* command.
 *  - **flash** will **build the project, then upload the code and any data** from the *data* directory
 *    to the currently connected chip. This corresponds to the *idf.py flash* command.
 *  - **app-flash** works like *flash*, but **only uploads the code**, not any other data. This is
 *    faster than doing a full flash, and is thus recommended for speedy development unless you have
 *    changed other data since the last full flash.
 *
 * This concludes the main CLion setup. You can now **open the file _MINTGGGameEngine_MyGame &rarr;
 * main &rarr; main.cpp_**, and start writing your game code there. Then use the *Build* button
 * with the appropriate target to build and flash (upload) your game to the device.
 *
 * The following sections contain some info about additional features of CLion you might want to use,
 * or fixes for certain issues that might pop up.
 *
 * \subsubsection setup_espidf_clion_build_misc_monitor Optional: IDF Monitor and IDF Command Line Tools
 *
 * Sometimes, you might want to **directly run the ESP-IDF command-line tools** on your project. You
 * can of course still do this in the normal ESP-IDF shell installed by EIM, but it can be done more
 * comfortably from within CLion.
 *
 * <b>Make sure you followed the steps in \ref setup_espidf_clion_terminal first</b>, otherwise the
 * ESP-IDF tool will not work. Then, **open a CLion terminal** (button on the bottom left). You can
 * enter IDF commands directly. Some useful ones will be briefly explained here.
 *
 * Sometimes, it can be useful to **build the project from the command line**, especially on the first
 * build if you get an error from CLion, or if something about the configuration changes. You can
 * do this as follows, specifying the CMake build directory manually:
 *
 * \code
 * idf.py -B build-esp32-debug build
 * \endcode
 *
 * To **monitor the log outputs** of your game (which is useful for debugging when something doesn't
 * work as it should), you can use:
 *
 * \code
 * idf.py -B build-esp32-debug monitor.
 * \endcode
 *
 * **IMPORTANT**: See the comment about the IDF monitor under \ref setup_espidf_build.
 *
 *
 * \section setup_desktop Option C: Setup for Desktop
 *
 * \subsection setup_desktop_qt Installing Qt
 *
 * To run on desktop platforms (e.g. Windows and Linux), the engine uses the
 * [**Qt framework**](https://www.qt.io/). Currently, it requires at least Qt version 6.8. To install
 * Qt, the easiest way is to download the **online installer for the Qt Community Edition**.
 * Unfortunately, it requires you to create an account to install. The alternative is to build it from
 * source, which is not trivial and thus not recommended, even for moderately skilled developers.
 *
 * Here are the important things to keep in mind when installing Qt:
 *
 *  1. You will need **at least Qt 6.8**. On Windows, you specifically need **the MinGW version**. The
 *     MSVC or LLVM versions are currently **not** supported by the engine, but might be in the future.
 *  2. Under ***Additional Libraries***, make sure to include ***Qt Multimedia***, which the engine
 *     needs for audio output (and builds will currently fail without it, even if you don't use audio in
 *     your game).
 *  3. Under ***Qt for Development***, make sure to include ***Build Tools &rarr; MinGW*** in the version
 *     that the Qt version you chose was built for. These are the compilers Qt itself was built with, and
 *     you **must** use the same ones (in the **same version!**) to build your game, or building might
 *     fail.
 *
 * Make sure to **remember which directory you installed Qt in**. You will need it later.
 *
 * \subsection setup_desktop_skeleton Getting the Engine and Skeleton Project
 *
 * Next, you have to <b>download
 * [the engine](https://github.com/alemariusnexus/MINTGGGameEngine) and the
 * [skeleton project](https://github.com/alemariusnexus/MINTGGGameEngine_MyGame)</b>. The steps here are
 * the same as for ESP-IDF, so please <b>see \ref setup_espidf_skeleton</b> for how to download them, then
 * continue with the next section here.
 *
 * \subsection setup_desktop_clion Setting up CLion
 *
 * \subsubsection setup_desktop_clion_build Build Configuration
 *
 * Like with ESP-IDF, there are many options for editors and IDEs for the desktop platform. The setup
 * will be similar to the one described for the ESP-IDF platform: The engine and the skeleton project
 * are both regular CMake projects.
 *
 * The following sections will again use **CLion for the IDE**. Because the steps are very similar to
 * the ones for ESP-IDF, they will not be repeated here. Instead, you should <b>follow the guide at
 * \ref setup_espidf_clion</b>, but keep in mind the differences for getting it to work with Qt on the
 * desktop.
 *
 * Here are the **important differences** from the setup described for ESP-IDF:
 *
 *  1. On Windows, when you create the toolchain, choose <b><i>MinGW</i> instead of <i>System</i></b> for the
 *     toolchain type.
 *  2. You **don't need an environment file** for the desktop platform.
 *  3. The <b><i>C Compiler</i> and <i>C++ Compiler</i></b> can be found in the Qt installation directory. The exact
 *     path depends on your Qt and MinGW version, but should look something like *Tools &rarr;
 *     mingw1310_64 &rarr; bin &rarr; gcc.exe* and *g++.exe*, respectively. If these files don't exist,
 *     you probably forgot to include the MinGW tools during Qt installation.
 *  4. You don't need to setup anything special for the CLion terminal.
 *  5. In the ***CMake configuration***, again choose the **toolchain** you created for Qt-MinGW.
 *  6. In the CMake configuration, you **don't need the _-DIDF_TARGET=esp32_ definition** in the *CMake
 *     options*. Instead, you have to provide the path to the core CMake files of your Qt installation.
 *     <b>Use the following for your _CMake options_</b>, adjusted for your system again:
 *     <i>-DQt6_DIR:PATH=C:\Qt\6.12.0\mingw_64\lib\cmake\Qt6</i> (note that this is **not** the path to Qt's
 *     installation root directory, but to a specific directory within it).
 *
 * Then to build your game, you again **use the _Build_** button, but the interesting *Build target* will
 * be *MINTGGGameEngine_MyGame* instead of *app*. Because the game will run on your desktop, there's no
 * need for targets like *flash* or *app-flash*.
 *
 * \subsubsection setup_desktop_clion_run Run Configuration
 *
 * \image html setup_desktop_clion_run.svg "Run Configuration settings for the desktop platform" width=80%
 *
 * Unlike for ESP-IDF, there is one additional step you have to take after the CMake configuration but
 * before running your program. You have to **edit the program's _Run configuration_** to allow it to find
 * certain files that it's looking for. Without this, it can either crash right when it's starting, or
 * mostly work, but not find data that your game stores in its *data* directory, or its configuration
 * files.
 *
 * To do this, **click the _Build target_ box next to the _Build_ button**, and choose *Edit
 * configurations...*. There, you have to configure two things:
 *
 *  1. Under ***Program arguments***, you have to put *-d ..*. This tells the game where to find its *app
 *     root directory*, which is the directory where it looks for things like the *data* and *config*
 *     folder.
 *  2. More importantly, under ***Environment variables***, you have to change the value of the PATH
 *     variable that the system uses to find libraries that the program depends on. In this case, that
 *     means the Qt libraries. This step **should only be necessary on Windows**, where the program will
 *     immediately crash without any output if you leave it out.
 *     The value for the *Environment variables* should look like this, again adjusting to your Qt
 *     installation directory:
 *     <i>PATH=C:\Qt\6.12.0\mingw_64\bin</i>
 *  3. This is optional, but while you're at it, you can check the box that says ***Emulate terminal in
 *     the output console***, which is needed to correctly display the colored log outputs from your game
 *     in CLion's console.
 *
 * Finally, to run your game, you now <b>use the regular <i>Run</i> button</b> (the green triangle) instead of the
 * *Build* button.
 *
 *
 * \section setup_epilog Next Steps
 *
 * Once you've set up your development environment, the engine and the skeleton project, you can start
 * coding your first game. For this, you can take a look at the tutorial (\ref tutorial_simple), or
 * start learning about the core concepts of the engine (\ref core).
 */

}

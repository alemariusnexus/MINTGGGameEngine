# This file is part of MINTGGGameEngine.
#
# Copyright 2025-2026 David Lerch
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

include(${CMAKE_CURRENT_LIST_DIR}/cmake/MINTGGGameEngine.cmake)

MGG_ComponentPrologue()

list(APPEND SRCS

	3rdparty/eMIDI/src/helpers.c
	3rdparty/eMIDI/src/midifile.c

    addons/mipidsi/rpi_tc358762/ScreenRPiTC358762.cpp

    audio/internal/emidi_hal_mintgggameengine.cpp
	audio/AudioClip.cpp
	audio/AudioEngine.cpp
	audio/MIDILoader.cpp

    core/DefaultEngine.cpp
    core/Engine.cpp
	core/Game.cpp
	core/GameObject.cpp
	core/Startup.cpp

    graphics/screen/drivers/mipidcs/AbstractMIPIDCSScreen.cpp
    graphics/screen/drivers/mipidcs/ScreenILI9341.cpp
    graphics/screen/drivers/mipidcs/ScreenST7735.cpp
    graphics/screen/drivers/mipidsi/AbstractMIPIDSIScreen.cpp
	graphics/screen/drivers/mipidsi/ScreenST7701.cpp
	graphics/screen/drivers/mipidsi/ScreenWaveshare4_3_DSI_TOUCH_A.cpp
    graphics/screen/drivers/ScreenNull.cpp
    graphics/screen/BufferedScreen.cpp
    graphics/screen/Screen.cpp
    graphics/surface/BitmapDrawSurface.cpp
    graphics/surface/DrawSurface.cpp
    graphics/surface/MemDrawSurface.cpp
	graphics/Bitmap.cpp
    graphics/BitmapRenderer.cpp
	graphics/Color.cpp
	graphics/Font.cpp
	graphics/ImageLoader.cpp
	graphics/Sprite.cpp
	graphics/Text.cpp

	input/InputEngine.cpp
	input/InputTouchEvent.cpp
	input/ResTouchScreenInputDevice.cpp
	input/TouchInputDevice.cpp

	network/NetworkEngine.cpp

	physics/Collider.cpp
	physics/GameObjectCollision.cpp
	physics/GravitySimulator.cpp

    platform/desktop/MainWindow.cpp
	platform/i2c/I2CBusDevice.cpp
	platform/i2c/I2CBusMaster.cpp
	platform/spi/SPIBusDevice.cpp
	platform/spi/SPIBusMaster.cpp
	platform/ADCManager.cpp
	platform/GPIODevice.cpp
	platform/GPIODeviceMCP230XX.cpp
	platform/GPIODeviceNative.cpp
	platform/HardwareManager.cpp
	platform/MCP230XXDevice.cpp
	platform/PlatformManager.cpp
	platform/ResTouchScreen.cpp

    storage/BufferedReader.cpp
	storage/File.cpp
	storage/FileReader.cpp
	storage/MemReader.cpp
	storage/Reader.cpp
	storage/StorageEngine.cpp

    util/EngineThread.cpp
    util/GameObjectStreamer.cpp
	util/Log.cpp
	util/MathUtils.cpp
	util/RayCastResult.cpp
    util/Util.cpp
	util/Vec2.cpp
	util/Vec3.cpp
	util/WorkerTask.cpp
	)

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/src/3rdparty/eMIDI/src/midifile.c")
    list(APPEND SRCS
        3rdparty/eMIDI/src/helpers.c
        3rdparty/eMIDI/src/midifile.c
        )
else()
    message(STATUS "eMidi not found. MIDI file handling will be disabled.")
endif()

foreach(_SRCFILE ${SRCS})
	list(APPEND SRCS_ABS "src/${_SRCFILE}")
endforeach()


if(MGG_PLATFORM STREQUAL ESPIDF)
    list(APPEND SRCS_ESPIDF
		3rdparty/esp_lcd_st7701/esp_lcd_st7701.c
		3rdparty/esp_lcd_st7701/esp_lcd_st7701_mipi.c
		3rdparty/esp_lcd_st7701/esp_lcd_st7701_rgb.c
		3rdparty/esp_lcd_touch/esp_lcd_touch.c
		3rdparty/esp_lcd_touch_gt911/esp_lcd_touch_gt911.c
    )
    foreach(_SRCFILE ${SRCS_ESPIDF})
        list(APPEND SRCS_ESPIDF_ABS "src/${_SRCFILE}")
    endforeach()

    idf_component_register(
        SRCS
            ${SRCS_ABS}
            ${SRCS_ESPIDF_ABS}
        INCLUDE_DIRS
            src
			src/3rdparty/esp_lcd_st7701
			src/3rdparty/esp_lcd_touch
			src/3rdparty/esp_lcd_touch_gt911
        REQUIRES
            esp_adc
            esp_driver_gpio
            esp_driver_gptimer
            esp_driver_i2c
            esp_driver_ledc
            esp_driver_sdmmc
            esp_driver_spi
			esp_hal_i2s
			esp_hal_lcd
            esp_http_client
            esp_lcd
            esp_wifi
            fatfs
			hal
            nvs_flash
            spiffs

            esp_mm
            esp_driver_ppa

            main
    )

    # Disable some annoying and unhelpful warnings
    target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wno-missing-field-initializers
        -Wno-parentheses                        # For eMidi
        -Wno-unused-function                    # For eMidi
        )
    target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_20)
elseif(MGG_PLATFORM STREQUAL DESKTOP)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)
    qt_standard_project_setup()

    qt_add_library(MINTGGGameEngine STATIC ${SRCS_ABS})
    target_include_directories(MINTGGGameEngine PUBLIC "${CMAKE_SOURCE_DIR}/main")
    target_compile_features(MINTGGGameEngine PUBLIC cxx_std_20)
    target_link_libraries(MINTGGGameEngine PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Multimedia)
else()
    message(SEND_ERROR "Unsupported platform for engine!")
endif()

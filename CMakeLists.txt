include(${CMAKE_CURRENT_LIST_DIR}/cmake/MINTGGGameEngine.cmake)

MINTGGGameEngine_ComponentPrologue()

list(APPEND SRCS

    3rdparty/eMIDI/src/helpers.c
    3rdparty/eMIDI/src/midifile.c

    audio/internal/emidi_hal_mintgggameengine.cpp
	audio/AudioClip.cpp
	audio/AudioEngine.cpp
	audio/MIDILoader.cpp

    core/DefaultEngine.cpp
    core/Engine.cpp
	core/Game.cpp
	core/GameObject.cpp

    graphics/screen/drivers/AbstractMIPIScreen.cpp
    graphics/screen/drivers/ScreenILI9341.cpp
    graphics/screen/drivers/ScreenNull.cpp
    graphics/screen/drivers/ScreenST7735.cpp
    graphics/screen/BufferedScreen.cpp
    graphics/screen/Screen.cpp
    graphics/surface/DrawSurface.cpp
    graphics/surface/MemDrawSurface.cpp
	graphics/Bitmap.cpp
	graphics/Color.cpp
	graphics/Font.cpp
	graphics/ImageLoader.cpp
	graphics/Sprite.cpp
	graphics/Text.cpp

	input/InputEngine.cpp

	network/NetworkEngine.cpp

	physics/Collider.cpp
	physics/GameObjectCollision.cpp
	physics/GravitySimulator.cpp

    platform/desktop/MainWindow.cpp
	platform/ADCManager.cpp
	platform/GPIODevice.cpp
	platform/GPIODeviceMCP2300X.cpp
	platform/GPIODeviceNative.cpp
	platform/MCP2300XDevice.cpp

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
	util/WorkerTask.cpp
	)

foreach(_SRCFILE ${SRCS})
	list(APPEND SRCS_ABS "src/${_SRCFILE}")
endforeach()


if(MINTGGGAMEENGINE_PLATFORM STREQUAL ESPIDF)
    idf_component_register(
        SRCS
            ${SRCS_ABS}
        INCLUDE_DIRS
            src
        REQUIRES
            esp_adc
            esp_driver_gpio
            esp_driver_gptimer
            esp_driver_i2c
            esp_driver_ledc
            esp_driver_sdmmc
            esp_driver_spi
            esp_http_client
            esp_lcd
            esp_wifi
            fatfs
            nvs_flash
            spiffs
    )

    # Disable some annoying and unhelpful warnings
    target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wno-missing-field-initializers
        -Wno-parentheses                        # For eMidi
        -Wno-unused-function                    # For eMidi
        )
elseif(MINTGGGAMEENGINE_PLATFORM STREQUAL DESKTOP)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)
    qt_standard_project_setup()

    qt_add_library(MINTGGGameEngine STATIC ${SRCS_ABS})
    target_compile_features(MINTGGGameEngine PUBLIC cxx_std_20)
    target_link_libraries(MINTGGGameEngine PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Multimedia)
endif()

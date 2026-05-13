list(APPEND SRCS

	audio/AudioClip.cpp
	audio/AudioEngine.cpp

    core/DefaultEngine.cpp
	core/Game.cpp
	core/GameObject.cpp

	graphics/Bitmap.cpp
	graphics/Color.cpp
	graphics/ImageLoader.cpp
	graphics/Screen.cpp
	graphics/ScreenHAGL.cpp
	graphics/ScreenNull.cpp
	graphics/ScreenST7735.cpp
	graphics/Sprite.cpp
	graphics/Text.cpp

	input/InputEngine.cpp

	network/NetworkEngine.cpp

	physics/Collider.cpp
	physics/GameObjectCollision.cpp
	physics/GravitySimulator.cpp

	platform/ADCManager.cpp
	platform/GPIODevice.cpp
	platform/GPIODeviceMCP2300X.cpp
	platform/GPIODeviceNative.cpp
	platform/MCP2300XDevice.cpp

    storage/BufferedReader.cpp
	storage/File.cpp
	storage/StorageEngine.cpp

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

idf_component_register(
    SRCS
		${SRCS_ABS}
    INCLUDE_DIRS
		src
    REQUIRES
		driver
		esp_adc
		esp_http_client
        esp_wifi
		fatfs
		hagl
		hagl_hal
		nvs_flash
		spiffs
)

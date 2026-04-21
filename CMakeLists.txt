list(APPEND SRCS

	ADCManager.cpp
	AudioClip.cpp
	AudioEngine.cpp
	Bitmap.cpp
	Collider.cpp
	Color.cpp
	GPIODevice.cpp
	GPIODeviceMCP2300X.cpp
	GPIODeviceNative.cpp
	Game.cpp
	GameObject.cpp
	GameObjectCollision.cpp
	GravitySimulator.cpp
	InputEngine.cpp
	Log.cpp
	MathUtils.cpp
    MCP2300XDevice.cpp
	RayCastResult.cpp
	Screen.cpp
	ScreenNull.cpp
	ScreenST7735_Arduino.cpp
    ScreenST7735_ESPIDF.cpp
	Sprite.cpp
	Text.cpp
    Util.cpp
	Vec2.cpp
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
)

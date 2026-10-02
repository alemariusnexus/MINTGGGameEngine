#pragma once

#include "Globals.h"

#include "audio/AudioClip.h"
#include "audio/AudioEngine.h"
#include "audio/MIDILoader.h"

#include "core/DefaultEngine.h"
#include "core/Engine.h"
#include "core/Game.h"
#include "core/GameObject.h"

#include "graphics/screen/drivers/AbstractMIPIScreen.h"
#include "graphics/screen/drivers/ScreenILI9341.h"
#include "graphics/screen/drivers/ScreenNull.h"
#include "graphics/screen/drivers/ScreenST7735.h"
#include "graphics/screen/BufferedScreen.h"
#include "graphics/screen/Screen.h"
#include "graphics/surface/DrawSurface.h"
#include "graphics/surface/MemDrawSurface.h"
#include "graphics/Bitmap.h"
#include "graphics/Color.h"
#include "graphics/Font.h"
#include "graphics/ImageLoader.h"
#include "graphics/Sprite.h"
#include "graphics/Text.h"

#include "input/InputEngine.h"

#include "network/NetworkEngine.h"

#include "physics/Collider.h"
#include "physics/GameObjectCollision.h"
#include "physics/GravitySimulator.h"

#include "platform/desktop/MainWindow.h"
#include "platform/ADCManager.h"
#include "platform/GPIODevice.h"
#include "platform/GPIODeviceMCP2300X.h"
#include "platform/GPIODeviceNative.h"
#include "platform/MCP2300XDevice.h"

#include "storage/BufferedReader.h"
#include "storage/File.h"
#include "storage/FileReader.h"
#include "storage/MemReader.h"
#include "storage/Reader.h"
#include "storage/StorageEngine.h"

#include "util/EngineThread.h"
#include "util/GameObjectStreamer.h"
#include "util/Log.h"
#include "util/MathUtils.h"
#include "util/RayCastResult.h"
#include "util/Util.h"
#include "util/Vec2.h"
#include "util/WorkerTask.h"


namespace MINTGGGameEngine
{
};

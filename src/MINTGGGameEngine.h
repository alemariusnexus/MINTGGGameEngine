/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Globals.h"

#include "addons/mipidsi/rpi_tc358762/ScreenRPiTC358762.h"

#include "audio/AudioClip.h"
#include "audio/AudioEngine.h"
#include "audio/MIDILoader.h"

#include "core/DefaultEngine.h"
#include "core/Engine.h"
#include "core/Game.h"
#include "core/GameObject.h"
#include "core/Startup.h"

#include "graphics/screen/drivers/mipidcs/AbstractMIPIDCSScreen.h"
#include "graphics/screen/drivers/mipidcs/ScreenILI9341.h"
#include "graphics/screen/drivers/mipidcs/ScreenST7735.h"
#include "graphics/screen/drivers/mipidsi/AbstractMIPIDSIScreen.h"
#include "graphics/screen/drivers/ScreenNull.h"
#include "graphics/screen/BufferedScreen.h"
#include "graphics/screen/Screen.h"
#include "graphics/surface/BitmapDrawSurface.h"
#include "graphics/surface/DrawSurface.h"
#include "graphics/surface/MemDrawSurface.h"
#include "graphics/Bitmap.h"
#include "graphics/BitmapRenderer.h"
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
#include "platform/PlatformManager.h"

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


#if MGG_ENABLE_DEFAULT_USING_STATEMENTS
    using namespace MINTGGGameEngine;
    using namespace std::string_literals;
    using namespace std;
#endif


namespace MINTGGGameEngine
{
};

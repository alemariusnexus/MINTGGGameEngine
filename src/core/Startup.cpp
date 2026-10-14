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

#include "Startup.h"

#include "../util/Log.h"

LOG_USE_TAG("Startup")


namespace MINTGGGameEngine
{

#if MGG_STARTUP_CODE_TYPE == MGG_STARTUP_CODE_TYPE_FULL

DefaultEngine engine;
Game game;


static void EngineSetup(int argc = 0, char** argv = nullptr)
{
    engine.earlySetup();

    DefaultEngine::SetupConfig cfg;
    DefaultEngine::initConfig(cfg);
    cfg.game            = &game;
    engine.setup(&cfg, argc, argv);

    game.setCollisionCallback(&onCollision);

    LogInfo("Running gameSetup()...");
    gameSetup();
    LogInfo("Finished gameSetup().");

    LogInfo("Running main loop...");
}

static bool EngineLoop()
{
    return engine.doFrame(&gameLoop, &postDraw);
}

static void EngineShutdown()
{
    engine.shutdown();
}

#endif

}


#if MGG_STARTUP_CODE_TYPE == MGG_STARTUP_CODE_TYPE_FULL  ||  MGG_STARTUP_CODE_TYPE == MGG_STARTUP_CODE_TYPE_PLATFORM

using namespace MINTGGGameEngine;

#ifdef MINTGGGAMEENGINE_PORT_ARDUINO

void setup()
{
    EngineSetup();
}
void loop()
{
    if (!EngineLoop()) {
        EngineShutdown();
        for (;;) {}
    }
}

#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)

extern "C" {
void app_main()
{
    EngineSetup();
    while (EngineLoop()) {}
    EngineShutdown();
}
}

#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)

extern "C" {
int main(int argc, char** argv)
{
    EngineSetup(argc, argv);
    while (EngineLoop()) {}
    EngineShutdown();
    return 0;
}
}

#endif

#endif

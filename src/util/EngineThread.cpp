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

#include "EngineThread.h"

#include "Util.h"


namespace MINTGGGameEngine
{

EngineThread::EngineThread(const std::string& name)
    : name(name), stopRequested(false)
{
}

EngineThread::~EngineThread()
{
}

bool EngineThread::start(const MainFunc& main, size_t stackSize, unsigned int priority)
{
    mainFunc = main;

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    try {
        thread = std::thread([this] { staticMain(this); });
        return true;
    } catch (std::exception& ex) {
        return false;
    }
#else
    // NOTE: For ESP-IDF specifically, it's actually the size in bytes, NOT in
    // words (which differs from vanilla FreeRTOS).
    const size_t stackSizeActual = stackSize;
    BaseType_t res = xTaskCreate(&EngineThread::staticMain, name.c_str(), stackSizeActual,
            this, priority, &task);
    if (res != pdPASS) {
        return false;
    }
    return true;
#endif
}

void EngineThread::stopAndWait()
{
    stopRequested = true;
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    thread.join();
#endif
    while (stopRequested) {
        DelayTaskMs(1);
    }
}

bool EngineThread::isStopRequested() const
{
    return stopRequested;
}

void EngineThread::main()
{
    mainFunc();

    stopRequested = false;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    vTaskDelete(nullptr);
#endif
}

void EngineThread::staticMain(void* thread)
{
    static_cast<EngineThread*>(thread)->main();
}

}

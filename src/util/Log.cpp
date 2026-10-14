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

#include "Log.h"

#include "Util.h"

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
#   define LOG_USE_CHRONO
#endif

#ifdef LOG_USE_CHRONO
#   include <chrono>
#endif


namespace MINTGGGameEngine
{


#if !defined(MINTGGGAMEENGINE_PORT_ESPIDF)  ||  defined(MINTGGGAMEENGINE_PORT_ARDUINO)

bool LogMessageBegin(const char* tag, int level)
{
#ifdef LOG_USE_CHRONO
    const auto now = std::chrono::system_clock::now();
    const auto timeStr = std::format("{:%Y-%m-%d %H:%M:%S}", now);
    const char* timeCStr = timeStr.c_str();
#else
    char timeCStr[32];
    const timer_mstick_t now = TimerGetTickcountMs();
    sprintf(timeCStr, "%lu", static_cast<long unsigned>(now));
#endif

    const char* levelStr = "???";
    switch (level) {
    case LOG_LEVEL_ERROR: levelStr = "ERR"; break;
    case LOG_LEVEL_WARNING: levelStr = "WRN"; break;
    case LOG_LEVEL_INFO: levelStr = "INF"; break;
    case LOG_LEVEL_DEBUG: levelStr = "DBG"; break;
    case LOG_LEVEL_VERBOSE: levelStr = "VRB"; break;
    }
#if defined(MINTGGGAMEENGINE_PORT_ARDUINO)
    Serial.print("[");
    Serial.print(levelStr);
    Serial.print("] (");
    Serial.print(timeCStr);
    Serial.print(") ");
    Serial.print(tag);
    Serial.print(" - ");
#else
    printf("[%s] (%s) %s - ", levelStr, timeCStr, tag);
#endif
    return true;
}

#endif


}

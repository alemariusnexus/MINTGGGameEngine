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

#include "../Globals.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <freertos/FreeRTOS.h>
#   include <freertos/semphr.h>
#endif

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
// TODO: ESP32C3 supports it, but we haven't benchmarked it yet, and we're currently getting messages likes "peripheral
//  0 is already used by another channel".
#   if /*defined(CONFIG_IDF_TARGET_ESP32C3)  ||*/  defined(CONFIG_IDF_TARGET_ESP32P4)
#       define MINTGGGAMEENGINE_ESP_ASYNC_MEMCPY_SUPPORTED
#   endif
#endif

#ifdef MINTGGGAMEENGINE_ESP_ASYNC_MEMCPY_SUPPORTED
#   include <esp_async_memcpy.h>
#endif

namespace MINTGGGameEngine
{

class PlatformManager
{
public:
    enum AllocFlags
    {
        ALLOC_FLAG_PREFER_INTERNAL  = 0x01,
        ALLOC_FLAG_PREFER_EXTERNAL  = 0x02,
        ALLOC_FLAG_BITMAP           = 0x04,
        ALLOC_FLAG_UNALIGNED_ACCESS = 0x08
    };

public:
    static PlatformManager& getInstance();

public:
    bool setup();

    void shutdown();

    void fastMemcpy(void* dest, const void* src, size_t size, bool* wasHWAccelerated = nullptr);

    void* allocDataBuffer(size_t size, uint32_t flags);

private:
    bool setupFastMemcpy();
    void shutdownFastMemcpy();

#ifdef MINTGGGAMEENGINE_ESP_ASYNC_MEMCPY_SUPPORTED
    bool fastMemcpyAsyncMemcpy(void* dest, const void* src, size_t size);
#endif

private:
#ifdef MINTGGGAMEENGINE_ESP_ASYNC_MEMCPY_SUPPORTED
    async_memcpy_handle_t asyncMemcpyHandle;
    SemaphoreHandle_t asyncMemcpySem;
#endif
};

}
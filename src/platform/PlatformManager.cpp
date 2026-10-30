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

#include "PlatformManager.h"

#include <algorithm>

#include "../util/Log.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <esp_cache.h>
#   include <esp_err.h>
#endif

#ifdef MINTGGGAMEENGINE_CHIP_ESP32P4
#   define MINTGGGAMEENGINE_ESP_AXI_GDMA_SUPPORTED
#endif

LOG_USE_TAG("PlatformManager")


namespace MINTGGGameEngine
{


#ifdef MINTGGGAMEENGINE_ESP_ASYNC_MEMCPY_SUPPORTED
static bool ESPAsyncMemcpyCb(async_memcpy_handle_t asyncMemcpyHandle, async_memcpy_event_t* evt, void* args)
{
    auto sem = static_cast<SemaphoreHandle_t>(args);
    BaseType_t hpTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sem, &hpTaskWoken);
    return hpTaskWoken == pdTRUE;
}
#endif



PlatformManager& PlatformManager::getInstance()
{
    static PlatformManager inst;
    return inst;
}

bool PlatformManager::setup()
{
    if (!setupFastMemcpy()) {
        return false;
    }

    return true;
}

void PlatformManager::shutdown()
{
    shutdownFastMemcpy();
}

bool PlatformManager::setupFastMemcpy()
{
#ifdef MINTGGGAMEENGINE_ESP_ASYNC_MEMCPY_SUPPORTED
    asyncMemcpySem = xSemaphoreCreateBinary();

    async_memcpy_config_t cfg = ASYNC_MEMCPY_DEFAULT_CONFIG();
    //cfg.dma_burst_size = 128;
    cfg.dma_burst_size = 64;
#ifdef MINTGGGAMEENGINE_ESP_AXI_GDMA_SUPPORTED
    esp_err_t err = esp_async_memcpy_install_gdma_axi(&cfg, &asyncMemcpyHandle);
#else
    esp_err_t err = esp_async_memcpy_install(&cfg, &asyncMemcpyHandle);
#endif
    if (err != ESP_OK) {
        LogError("Error intalling ESP async memcpy: %s", esp_err_to_name(err));
        return false;
    }
#endif

    return true;
}

void PlatformManager::shutdownFastMemcpy()
{
#ifdef MINTGGGAMEENGINE_ESP_ASYNC_MEMCPY_SUPPORTED
    esp_async_memcpy_uninstall(asyncMemcpyHandle);

    vSemaphoreDelete(asyncMemcpySem);
#endif
}

void PlatformManager::fastMemcpy(void* dest, const void* src, size_t size, bool* wasHWAccelerated)
{
#ifdef MINTGGGAMEENGINE_ESP_ASYNC_MEMCPY_SUPPORTED
    // TODO: Benchmark and adjust this value, or find better heuristics.
    if (size >= 64*1024) {
        if (fastMemcpyAsyncMemcpy(dest, src, size)) {
            if (wasHWAccelerated) {
                *wasHWAccelerated = true;
            }
            return;
        }
    }
#endif

    memcpy(dest, src, size);

    if (wasHWAccelerated) {
        *wasHWAccelerated = false;
    }
}

void* PlatformManager::allocDataBuffer(size_t size, uint32_t flags)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    const size_t autoExtMemSizeThreshold = 16*1024;
    const size_t autoBitmapDMAAlignmentThreshold = 64*1024;

    uint32_t heapCaps = 0;
    size_t alignment = 0;

    const bool haveSPIRAM = (heap_caps_get_total_size(MALLOC_CAP_SPIRAM) != 0);

    if (haveSPIRAM) {
        if ((flags & ALLOC_FLAG_PREFER_INTERNAL) != 0) {
            heapCaps |= MALLOC_CAP_INTERNAL;
        } else if ((flags & ALLOC_FLAG_PREFER_EXTERNAL) != 0) {
            heapCaps |= MALLOC_CAP_SPIRAM;
        } else if (size >= autoExtMemSizeThreshold) {
            heapCaps |= MALLOC_CAP_SPIRAM;
        }
    } else {
        // No SPIRAM available. Don't try allocating there in the first place (which avoids our failed alloc hook
        // being called in such cases, logging an error message).
        heapCaps |= MALLOC_CAP_INTERNAL;
    }

    if ((flags & ALLOC_FLAG_BITMAP) != 0) {
        if (size >= autoBitmapDMAAlignmentThreshold) {
            heapCaps |= MALLOC_CAP_CACHE_ALIGNED;
            alignment = 128; // Very conservative for all kinds of DMAs
        }
    }
    if ((flags & ALLOC_FLAG_UNALIGNED_ACCESS)) {
        heapCaps |= MALLOC_CAP_8BIT;
    }

    if (heapCaps != 0) {
        void* buf = (alignment != 0) ? heap_caps_aligned_alloc(alignment, size, heapCaps) : heap_caps_malloc(size, heapCaps);

        if (!buf  &&  haveSPIRAM) {
            // Try again in the other type of memory
            if ((heapCaps & MALLOC_CAP_SPIRAM) != 0) {
                heapCaps = (heapCaps & ~MALLOC_CAP_SPIRAM) | MALLOC_CAP_INTERNAL;
                buf = (alignment != 0) ? heap_caps_aligned_alloc(alignment, size, heapCaps) : heap_caps_malloc(size, heapCaps);
            } else if ((heapCaps & MALLOC_CAP_INTERNAL) != 0) {
                heapCaps = (heapCaps & ~MALLOC_CAP_INTERNAL) | MALLOC_CAP_SPIRAM;
                buf = (alignment != 0) ? heap_caps_aligned_alloc(alignment, size, heapCaps) : heap_caps_malloc(size, heapCaps);
            }
        }

        if (!buf) {
            // Try again without alignment requirements
            if ((heapCaps & MALLOC_CAP_CACHE_ALIGNED) != 0  ||  alignment != 0) {
                heapCaps &= ~MALLOC_CAP_CACHE_ALIGNED;
                buf = heap_caps_malloc(size, heapCaps);
            }
        }

        if (buf) {
            return buf;
        }
    }

    // Give up and try regular malloc() as a last resort
#endif

    return malloc(size);
}

bool PlatformManager::hasExternalRAM() const
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    return heap_caps_get_total_size(MALLOC_CAP_SPIRAM) != 0;
#else
    return false;
#endif
}


#ifdef MINTGGGAMEENGINE_ESP_ASYNC_MEMCPY_SUPPORTED

bool PlatformManager::fastMemcpyAsyncMemcpy(void* dest, const void* src, size_t size)
{
    auto dest8 = reinterpret_cast<uint8_t*>(dest);
    auto src8 = reinterpret_cast<uint8_t*>(const_cast<void*>(src));

    // ESP-IDF doesn't seem to have a straightforward API to determine the alignment required by the DMA (anymore. It
    // was part of async_memcpy_config_t, but was removed in v6.0). Skimming through the driver code suggests that in
    // most cases, there is no alignment requirement (which is also what the ESP32-P4's TRM states), BUT in some
    // specific configurations, it requires alignment to dma_burst_size due to "hardware limitations". Since we use
    // the maximum value of dma_burst_size=128, let's use that here as a conservative maximum (begrudgingly).
    const size_t dmaAlign = 128;

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
    const size_t srcCacheAlign = esp_cache_get_line_size_by_addr(src8);
    const size_t destCacheAlign = esp_cache_get_line_size_by_addr(dest8);
#else
    const size_t srcCacheAlign = 128;
    const size_t destCacheAlign = 128;
#endif
    const size_t commonAlignment = std::max(std::max(
        srcCacheAlign != 0 ? srcCacheAlign : 1, destCacheAlign != 0 ? destCacheAlign : 1), dmaAlign);

    if (size < commonAlignment) {
        return false;
    }

    const uintptr_t srcMisalignment = reinterpret_cast<uintptr_t>(src8) % commonAlignment;
    const uintptr_t destMisalignment = reinterpret_cast<uintptr_t>(dest8) % commonAlignment;

    if (srcMisalignment != destMisalignment) {
        // No common alignment point
        return false;
    }


    if (srcMisalignment != 0) {
        // Copy first part using regular memcpy
        const size_t headSize = commonAlignment - srcMisalignment;
        memcpy(dest8, src8, headSize);
        src8 += headSize;
        dest8 += headSize;
        size -= headSize;
    }


    const size_t tailSize = size % commonAlignment;
    size -= tailSize;

    esp_err_t err;

    if (size != 0) {
        if (srcCacheAlign != 0) {
            // Sync source to memory, so the DMA sees changes that might still be cached from the CPU
            err = esp_cache_msync(src8, size, ESP_CACHE_MSYNC_FLAG_DIR_C2M);
            if (err != ESP_OK) {
                LogError("Error syncing src cache before esp_async_memcpy(): %s", esp_err_to_name(err));
                return false;
            }
        }
        if (destCacheAlign != 0) {
            // Sync destination to memory as well. Invalidating will ensure that after the DMA finishes, the CPU will
            // have no dirty cache lines left, and will read the new data from memory instead.
            err = esp_cache_msync(dest8, size, ESP_CACHE_MSYNC_FLAG_DIR_C2M | ESP_CACHE_MSYNC_FLAG_INVALIDATE);
            if (err != ESP_OK) {
                LogError("Error syncing dest cache before esp_async_memcpy(): %s", esp_err_to_name(err));
                return false;
            }
        }

        // Copy fully aligned middle part using DMA
        err = esp_async_memcpy(asyncMemcpyHandle, dest8, src8, size, &ESPAsyncMemcpyCb, asyncMemcpySem);
        if (err != ESP_OK) {
            LogError("Error calling esp_async_memcpy(): %s", esp_err_to_name(err));
            return false;
        }
        xSemaphoreTake(asyncMemcpySem, portMAX_DELAY);

        // No need for additional cache synchronization. Only dest8 has changed, and we invalidated all cache lines for
        // it prior to the transfer. As long as the the CPU doesn't read from the buffer while DMA is copying (and it
        // shouldn't), this should be safe.
    }


    if (tailSize != 0) {
        // Copy last part using regular memcpy
        memcpy(dest8 + size, src8 + size, tailSize);
    }


    return true;
}

#endif

}

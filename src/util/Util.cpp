#include "Util.h"

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
#   include <freertos/FreeRTOS.h>

#   include <driver/gptimer.h>
#   include <esp_err.h>
#   include <esp_heap_caps.h>
#   include <soc/soc.h>
#else
#   include <ctime>
#   include <unistd.h>
#endif

#include <cstdlib>


namespace MINTGGGameEngine
{


static bool TimerInitialized = false;

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
static gptimer_handle_t TimerHandle;
#endif


void TimerInit()
{
    if (TimerInitialized) {
        return;
    }

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    gptimer_config_t cfg = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1*1000*1000    // Tick at 1MHz, simply because we currently don't need
                                        // sub-microsecond precision
    };
    gptimer_new_timer(&cfg, &TimerHandle);
    gptimer_set_raw_count(TimerHandle, 0);
    gptimer_enable(TimerHandle);
    gptimer_start(TimerHandle);
#endif

    TimerInitialized = true;
}

void TimerShutdown()
{
    if (!TimerInitialized) {
        return;
    }

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    gptimer_stop(TimerHandle);
    gptimer_disable(TimerHandle);
    gptimer_del_timer(TimerHandle);
#endif

    TimerInitialized = false;
}

timer_mstick_t TimerGetTickcountMs()
{
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    return millis();
#else
    return TimerGetTickcountUs() / 1000;
#endif
}

timer_ustick_t TimerGetTickcountUs()
{
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    return micros();
#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)
    uint64_t tc;
    ESP_ERROR_CHECK(gptimer_get_raw_count(TimerHandle, &tc));
    return static_cast<timer_ustick_t>(tc);
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((timer_ustick_t) (ts.tv_nsec / 1000)) + ((timer_ustick_t) ts.tv_sec)*1000000;
#else
    return 0;
#endif
}

void TimerDelaySpinMs(unsigned int ms)
{
    if (ms == 0) {
        return;
    }
    const timer_mstick_t endTime = TimerGetTickcountMs() + ms;
    while (TimerGetTickcountMs() <= endTime);
}

void TimerDelaySpinUs(unsigned int us)
{
#ifdef MINTGGGAMEENGINE_TIMER_HAVE_US_RESOLUTION
    if (us == 0) {
        return;
    }
    const timer_ustick_t endTime = TimerGetTickcountUs() + us;
    while (TimerGetTickcountUs() <= endTime);
#else
    TimerDelaySpinMs((us+999) / 1000);
#endif
}

void DelayTaskMs(uint32_t delayMs)
{
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    usleep(static_cast<useconds_t>(delayMs) * 1000);
#else
    vTaskDelay((delayMs + portTICK_PERIOD_MS-1) / portTICK_PERIOD_MS);
#endif
}


static bool CheckAdvancedAllocFlags(uint32_t flags)
{
    if (    (flags & ADVANCED_ALLOC_FLAG_EXTERNAL_RAM) != 0
        &&  (flags & ADVANCED_ALLOC_FLAG_INTERNAL_RAM) != 0
    ) {
        // These are mutually exclusive
        return false;
    }
    return true;
}

#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
static uint32_t ESPCalcHeapCapsFromAdvancedAllocFlags(uint32_t flags)
{
    uint32_t caps = 0;
    if ((flags & ADVANCED_ALLOC_FLAG_UNALIGNED_ACCESS) != 0) {
        caps |= MALLOC_CAP_8BIT;
    }
    if ((flags & ADVANCED_ALLOC_FLAG_DMA_ACCESSIBLE) != 0) {
        caps |= MALLOC_CAP_DMA;
    }
    if ((flags & ADVANCED_ALLOC_FLAG_EXTERNAL_RAM) != 0) {
        caps |= MALLOC_CAP_SPIRAM;
    }
    if ((flags & ADVANCED_ALLOC_FLAG_INTERNAL_RAM) != 0) {
        caps |= MALLOC_CAP_INTERNAL;
    }
    return caps;
}
#endif

void* AdvancedAlloc(size_t size, uint32_t flags)
{
    if (!CheckAdvancedAllocFlags(flags)) {
        return nullptr;
    }
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    return heap_caps_malloc(size, ESPCalcHeapCapsFromAdvancedAllocFlags(flags));
#else
    return malloc(size);
#endif
}

void* AdvancedAlignedAlloc(size_t alignment, size_t size, uint32_t flags)
{
    if (!CheckAdvancedAllocFlags(flags)) {
        return nullptr;
    }
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    return heap_caps_aligned_alloc(alignment, size, ESPCalcHeapCapsFromAdvancedAllocFlags(flags));
#elif defined(_WIN32)
    // Yes, we silently allocate non-aligned memory on Windows.
    // Yes, that's terrible.
    // The problem is that MSVC doesn't implement std::aligned_alloc(), because that function is required by the C++
    // standard to work with free(), which Microsoft are too dumb to implement. So instead, they provide
    // _aligned_malloc(), which has to be freed using _aligned_free(). Using free() with it instead can lead to bugs.
    // We don't use _aligned_malloc() because then we would have to force the user to use a specialized free function,
    // but ONLY use that specialized free function for aligned allocations. That would mean requiring caller-defined
    // free functions in basically any code where you could potentially pass in an aligned buffer, and that is just not
    // happening.
    // Fucking Microsoft. Can't make this shit up.
    return malloc(size);
#else
    return std::aligned_alloc(alignment, size);
#endif
}

void AdvancedFree(void* d)
{
#ifdef MINTGGGAMEENGINE_PORT_ESPIDF
    heap_caps_free(d);
#else
    free(d);
#endif
}

void ExtractArduinoPinMode(uint8_t pinMode, bool* output, bool* puEnabled, bool* pdEnabled)
{
    if (output) {
        if (pinMode == INPUT  ||  pinMode == INPUT_PULLDOWN  ||  pinMode == INPUT_PULLUP) {
            *output = false;
        } else {
            *output = true;
        }
    }
    if (puEnabled) {
        if (pinMode == INPUT_PULLUP) {
            *puEnabled = true;
        } else {
            *puEnabled = false;
        }
    }
    if (pdEnabled) {
        if (pinMode == INPUT_PULLDOWN) {
            *pdEnabled = true;
        } else {
            *pdEnabled = false;
        }
    }
}

const char* PixelFormatToString(PixelFormat pixelFormat)
{
    switch (pixelFormat) {
    case PixelFormat::RGB888:       return "RGB888";
    case PixelFormat::BGR888:       return "BGR888";
    case PixelFormat::RGB565:       return "RGB565";
    case PixelFormat::BGR565:       return "BGR565";
    case PixelFormat::RGBX8888:     return "RGBX8888";
    case PixelFormat::XBGR8888:     return "XBGR8888";
    default:                        return "[invalid]";
    }
}

}

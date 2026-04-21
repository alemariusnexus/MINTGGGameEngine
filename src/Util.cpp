#include "util.h"

#include <driver/gptimer.h>
#include <esp_err.h>
#include <soc/soc.h>


namespace MINTGGGameEngine
{


static bool TimerInitialized = false;

#ifdef ESP_PLATFORM
static gptimer_handle_t TimerHandle;
#endif


void TimerInit()
{
    if (TimerInitialized) {
        return;
    }

#ifdef ESP_PLATFORM
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

#ifdef ESP_PLATFORM
    gptimer_stop(TimerHandle);
    gptimer_disable(TimerHandle);
    gptimer_del_timer(TimerHandle);
#endif

    TimerInitialized = false;
}

timer_mstick_t TimerGetTickcountMs()
{
#ifdef ARDUINO
    return millis();
#else
    return TimerGetTickcountUs() / 1000;
#endif
}

timer_ustick_t TimerGetTickcountUs()
{
#ifdef ARDUINO
    return micros();
#elif defined(ESP_PLATFORM)
    uint64_t tc;
    ESP_ERROR_CHECK(gptimer_get_raw_count(TimerHandle, &tc));
    return tc;
#else
#   error Not implemented!
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
        if (pinMode == INPUT_PULLUP  ||  pinMode == OUTPUT_PULLUP) {
            *puEnabled = true;
        } else {
            *puEnabled = false;
        }
    }
    if (pdEnabled) {
        if (pinMode == INPUT_PULLDOWN  ||  pinMode == OUTPUT_PULLDOWN) {
            *pdEnabled = true;
        } else {
            *pdEnabled = false;
        }
    }
}


}

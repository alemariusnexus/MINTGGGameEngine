#pragma once

#include "../Globals.h"


namespace MINTGGGameEngine
{


typedef uint64_t timer_mstick_t;
typedef uint64_t timer_ustick_t;


void TimerInit();
void TimerShutdown();

timer_mstick_t TimerGetTickcountMs();
timer_ustick_t TimerGetTickcountUs();

void ExtractArduinoPinMode(uint8_t pinMode, bool* output, bool* puEnabled, bool* pdEnabled);


}

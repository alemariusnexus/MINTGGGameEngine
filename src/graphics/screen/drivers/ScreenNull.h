#pragma once

#include "../../../Globals.h"

#include <esp_heap_caps.h>

#include "../BufferedScreen.h"


namespace MINTGGGameEngine
{

class ScreenNull : public BufferedScreen
{
public:
    ScreenNull(uint16_t width = 160, uint16_t height = 128);

    bool init() override;
    void commit() override;

private:
    static void _defaultDelete(uint8_t* buf) { heap_caps_free(buf); }
};

}

#pragma once

#include "../../../Globals.h"

#include <esp_heap_caps.h>

#include "../BufferedScreen.h"


namespace MINTGGGameEngine
{

/**
 * \brief A dummy screen driver that does not display its content anywhere.
 *
 * This can be useful for headless applications that don't actually have a screen. It is also used when no screen is
 * found or when another screen's initialization fails for some reason.
 *
 * Although no actual external screen is used, the pixel data is still drawn properly to a framebuffer, and it can be
 * read back normally. This means that even such a null screen will take up memory for the framebuffer, which can
 * be avoided by making the screen a smaller size.
 */
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

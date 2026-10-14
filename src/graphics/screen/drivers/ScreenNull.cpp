#include "ScreenNull.h"

#include "../../../util/Util.h"

LOG_USE_TAG("ScreenNull")


namespace MINTGGGameEngine
{


ScreenNull::ScreenNull(uint16_t width, uint16_t height)
    : BufferedScreen (
        PixelFormat::RGB565,
        ByteOrder::Native,
        AdvancedAlignedAlloc(16, width * height * sizeof(uint16_t),
            ADVANCED_ALLOC_FLAG_UNALIGNED_ACCESS | ADVANCED_ALLOC_FLAG_DMA_ACCESSIBLE | ADVANCED_ALLOC_FLAG_EXTERNAL_RAM),
        width,
        height,
        &AdvancedFree
        )
{
}

ScreenNull::ScreenNull()
    : ScreenNull (
#ifdef MGG_SCREEN_MAIN_WIDTH
        MGG_SCREEN_MAIN_WIDTH,
#else
        160,
#endif
#ifdef MGG_SCREEN_MAIN_HEIGHT
        MGG_SCREEN_MAIN_HEIGHT
#else
        128
#endif
        )
{
}

bool ScreenNull::init()
{
    LogWarning (
        "Initializing null screen (%ux%u). This is a dummy screen, so no image will be visible!",
        getWidth(), getHeight()
        );

    return BufferedScreen::init();
}


}

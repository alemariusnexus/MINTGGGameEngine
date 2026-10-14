#include "AbstractMIPIDCSScreen.h"

#include "../../../../util/Log.h"


LOG_USE_TAG("AbstractMIPIDCSScreen")


namespace MINTGGGameEngine
{


AbstractMIPIDCSScreen::AbstractMIPIDCSScreen (
    PixelFormat pixelFormat, ByteOrder byteOrder,
    uint16_t width, uint16_t height,
    uint16_t offsetX, uint16_t offsetY,
    bool mirrorX, bool mirrorY, bool swapXY,
    bool swapRGBAndBGR,
    uint8_t* staticBuffer
)
    : BufferedScreen (
        pixelFormat, byteOrder,
        staticBuffer
            ? staticBuffer
            : AdvancedAlloc(width * height * CalcPixelFormatBytesPerPixel(pixelFormat),
                ADVANCED_ALLOC_FLAG_UNALIGNED_ACCESS | ADVANCED_ALLOC_FLAG_DMA_ACCESSIBLE),
        width,
        height,
        &AdvancedFree
        ),
      addressMode(0),
      offsetX(offsetX),
      offsetY(offsetY)
{
    if (!fb.getBuffer()) {
        LogError("Error allocating framebuffer (size: %u bytes).", width * height * sizeof(uint16_t));
    }
    if (mirrorX) {
        addressMode |= MIPI_DCS_ADDRESS_MODE_MIRROR_X;
    }
    if (mirrorY) {
        addressMode |= MIPI_DCS_ADDRESS_MODE_MIRROR_Y;
    }
    if (swapXY) {
        addressMode |= MIPI_DCS_ADDRESS_MODE_SWAP_XY;
    }
    if (    (   (   pixelFormat == PixelFormat::BGR565
                ||  pixelFormat == PixelFormat::BGR888
                ||  pixelFormat == PixelFormat::XBGR8888
                )
            &&  !swapRGBAndBGR
            )
        ||  (   (   pixelFormat == PixelFormat::RGB565
                ||  pixelFormat == PixelFormat::RGB888
                ||  pixelFormat == PixelFormat::RGBX8888
                )
            &&  swapRGBAndBGR
            )
    ) {
        addressMode |= MIPI_DCS_ADDRESS_MODE_BGR;
    }
}

AbstractMIPIDCSScreen::~AbstractMIPIDCSScreen()
{
}

bool AbstractMIPIDCSScreen::init()
{
    if (!BufferedScreen::init()) {
        return false;
    }

#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
    if (!mipiInitBus()) {
        return false;
    }

    doHWReset();

    if (!mipiWriteCommandPlusData(MIPI_DCS_SOFT_RESET, nullptr, 0)) {
        return false;
    }
    DelayTaskMs(200);

    uint8_t cmd;

    cmd = addressMode;
    if (!mipiWriteCommandPlusData(MIPI_DCS_SET_ADDRESS_MODE, &cmd, 1)) {
        return false;
    }

    const auto bytesPerPixel = CalcPixelFormatBytesPerPixel(getNativePixelFormat());
    if (bytesPerPixel == 2) {
        cmd = MIPI_DCS_PIXEL_FORMAT_16BIT;
    } else if (bytesPerPixel == 3) {
        cmd = MIPI_DCS_PIXEL_FORMAT_24BIT;
    } else {
        assert(false);
    }
    if (!mipiWriteCommandPlusData(MIPI_DCS_SET_PIXEL_FORMAT, &cmd, 1)) {
        return false;
    }

    if (!mipiWriteCommandPlusData(MIPI_DCS_EXIT_SLEEP_MODE, nullptr, 0)) {
        return false;
    }
    DelayTaskMs(200);

    if (!mipiWriteCommandPlusData(MIPI_DCS_SET_DISPLAY_ON, nullptr, 0)) {
        return false;
    }
    DelayTaskMs(200);
#endif

    return true;
}


bool AbstractMIPIDCSScreen::mipiSetAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t data[4];

    x1 = x1 + offsetX;
    y1 = y1 + offsetY;
    x2 = x2 + offsetX;
    y2 = y2 + offsetY;

    data[0] = x1 >> 8;
    data[1] = x1 & 0xff;
    data[2] = x2 >> 8;
    data[3] = x2 & 0xff;
    if (!mipiWriteCommandPlusData(MIPI_DCS_SET_COLUMN_ADDRESS, data, 4)) {
        return false;
    }

    data[0] = y1 >> 8;
    data[1] = y1 & 0xff;
    data[2] = y2 >> 8;
    data[3] = y2 & 0xff;
    if (!mipiWriteCommandPlusData(MIPI_DCS_SET_PAGE_ADDRESS, data, 4)) {
        return false;
    }

    return true;
}


}

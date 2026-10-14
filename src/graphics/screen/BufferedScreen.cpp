#include "BufferedScreen.h"

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <QImage>
#   include <QVBoxLayout>
#   include <QWidget>

#   include "../../platform/desktop/MainWindow.h"
#endif

#include "../../util/Log.h"

LOG_USE_TAG("BufferedScreen")


namespace MINTGGGameEngine
{


BufferedScreen::BufferedScreen (
    PixelFormat pixelFormat,
    ByteOrder byteOrder,
    void* buffer, uint16_t width, uint16_t height,
    const MemDrawSurface::delete_fn_t& del
)
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    : fb(pixelFormat, byteOrder, buffer, width, height, del)
#else
    : fb(pixelFormat, byteOrder, buffer, width, height, del)
#endif
{
}

BufferedScreen::~BufferedScreen()
{
}

void BufferedScreen::setBuffer(void* buffer, const MemDrawSurface::delete_fn_t& del)
{
    fb.setBuffer(buffer, del);
}

uint16_t BufferedScreen::getWidth() const
{
    return fb.getWidth();
}

uint16_t BufferedScreen::getHeight() const
{
    return fb.getHeight();
}

PixelFormat BufferedScreen::getNativePixelFormat() const
{
    return fb.getNativePixelFormat();
}

ByteOrder BufferedScreen::getNativeByteOrder() const
{
    return fb.getNativeByteOrder();
}

void BufferedScreen::fill(const Color& color, bool honorClipRegion)
{
    fb.fill(color, honorClipRegion);
}

void BufferedScreen::drawPixel(int32_t x, int32_t y, const Color& color)
{
    fb.drawPixel(x, y, color);
}

void BufferedScreen::drawPixelFast(int32_t x, int32_t y, const Color& color)
{
    fb.drawPixelFast(x, y, color);
}

void BufferedScreen::drawPixelRaw(int32_t x, int32_t y, const void* data)
{
    fb.drawPixelRaw(x, y, data);
}

void BufferedScreen::drawPixelFastRaw(int32_t x, int32_t y, const void* data)
{
    fb.drawPixelFastRaw(x, y, data);
}

void BufferedScreen::drawPixelsRaw (
    uint16_t srcX, uint16_t srcY,
    uint16_t srcW, uint16_t srcH,
    uint16_t blockW, uint16_t blockH,
    int32_t destX, int32_t destY,
    const void* data
) {
    fb.drawPixelsRaw(srcX, srcY, srcW, srcH, blockW, blockH, destX, destY, data);
}

void BufferedScreen::drawHLine(int32_t x0, int32_t y0, int32_t w, const Color& color)
{
    fb.drawHLine(x0, y0, w, color);
}

void BufferedScreen::drawVLine(int32_t x0, int32_t y0, int32_t h, const Color& color)
{
    fb.drawVLine(x0, y0, h, color);
}

void BufferedScreen::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Color& color)
{
    fb.drawLine(x0, y0, x1, y1, color);
}

void BufferedScreen::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, const Color& color, bool filled)
{
    fb.drawRect(x, y, w, h, color, filled);
}

void BufferedScreen::drawCircle(int32_t cx, int32_t cy, int32_t r, const Color& color, bool filled)
{
    fb.drawCircle(cx, cy, r, color, filled);
}

void BufferedScreen::drawBitmap (
    int32_t x, int32_t y,
    const Bitmap& bitmap,
    FlipDir flipDir,
    Rotation rotation,
    int16_t scaleX, int16_t scaleY
) {
    fb.drawBitmap(x, y, bitmap, flipDir, rotation, scaleX, scaleY);
}

void BufferedScreen::setClipRegion(int16_t cx, int16_t cy, int16_t cw, int16_t ch)
{
    fb.setClipRegion(cx, cy, cw, ch);
}

void BufferedScreen::getClipRegion(uint16_t* cx, uint16_t* cy, uint16_t* cw, uint16_t* ch)
{
    fb.getClipRegion(cx, cy, cw, ch);
}

Color BufferedScreen::readPixel(int32_t x, int32_t y)
{
    return fb.readPixel(x, y);
}

void BufferedScreen::drawText(const Text& text, int32_t ox, int32_t oy)
{
    fb.drawText(text, ox, oy);
}

bool BufferedScreen::saveScreenshot(const char* path)
{
    return fb.saveScreenshot(path);
}

bool BufferedScreen::init()
{
    if (!fb.getBuffer()) {
        LogInfo("Framebuffer is null!");
        return false;
    }

    return true;
}

void BufferedScreen::shutdown()
{
}

void BufferedScreen::commit()
{
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    MainWindow::instance()->displayFrame (
        fb.getBuffer(),
        getWidth(), getHeight(),
        getNativePixelFormat(), getNativeByteOrder()
        );
#endif
}



}

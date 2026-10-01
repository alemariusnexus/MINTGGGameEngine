#include "BufferedScreen.h"

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <QImage>
#   include <QVBoxLayout>
#   include <QWidget>

#   include "../../platform/desktop/MainWindow.h"
#endif


namespace MINTGGGameEngine
{


BufferedScreen::BufferedScreen (
    uint8_t* buffer, uint16_t width, uint16_t height,
    const MemDrawSurface::delete_fn_t& del,
    bool swapEndianness
)
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    : fb(buffer, width, height, del, false)
#else
    : fb(buffer, width, height, del, swapEndianness)
#endif
{
}

BufferedScreen::~BufferedScreen()
{
}

uint16_t BufferedScreen::getWidth() const
{
    return fb.getWidth();
}

uint16_t BufferedScreen::getHeight() const
{
    return fb.getHeight();
}

void BufferedScreen::fill(const Color& color, bool honorClipRegion)
{
    fb.fill(color, honorClipRegion);
}

void BufferedScreen::drawPixel(int32_t x, int32_t y, const Color& color)
{
    fb.drawPixel(x, y, color);
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

void BufferedScreen::drawBitmap(int32_t x, int32_t y, const Bitmap& bitmap, FlipDir flipDir)
{
    fb.drawBitmap(x, y, bitmap, flipDir);
}

void BufferedScreen::setClipRegion(uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch)
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
    MainWindow::instance()->displayFrame(fb.getBuffer(), getWidth(), getHeight(), QImage::Format_RGB16);
#endif
}



}

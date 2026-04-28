#include "Screen.h"

#include "../storage/File.h"

namespace MINTGGGameEngine
{


bool Screen::saveScreenshot(const char* path)
{
    // TODO: Support writing BMP files (based on extension maybe)
    File f(path);
    if (!f.open(File::WriteOnly)) {
        return false;
    }
    const uint16_t w = getWidth();
    const uint16_t h = getHeight();
    for (uint16_t y = 0 ; y < h ; y++) {
        for (uint16_t x = 0 ; x < w ; x++) {
            uint16_t pixel = readPixel(static_cast<int16_t>(x), static_cast<int16_t>(y)).toRGB565();
            f.write(&pixel, sizeof(pixel));
        }
    }
    f.flush();
    f.close();
    return true;
}


}

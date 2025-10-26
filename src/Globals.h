#pragma once

#include <Arduino.h>

#include <cstdlib>
#include <cstring>


namespace MINTGGGameEngine
{

enum class FlipDir
{
    None = 0,
    Horizontal,
    Vertical,
    Both
};

enum ZOrder
{
    ZOrderBackground    = 4000,
    ZOrderNormal        = 5000,
    ZOrderForeground    = 6000,
    ZOrderOverlay       = 7000
};

}

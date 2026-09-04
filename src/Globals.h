#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>


#ifdef ESP_PLATFORM
#   define MINTGGGAMEENGINE_PORT_ESPIDF
#endif

#ifdef ARDUINO
#   define MINTGGGAMEENGINE_PORT_ARDUINO
#endif


#if defined(CONFIG_IDF_TARGET_ESP32)
#   define MINTGGGAMEENGINE_CHIP_ESP32
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#   define MINTGGGAMEENGINE_CHIP_ESP32C3
#else
#   error This chip is currently not supported by MINTGGGameEngine!
#endif


namespace MINTGGGameEngine
{


typedef int16_t gpionum_t; // Must be signed, because it accepts -1 for "invalid pin"!

/**
 * \brief The direction to flip an object.
 */
enum class FlipDir
{
    None = 0,   ///< Do not flip.
    Horizontal, ///< Flip horizontally (i.e. change direction along x axis).
    Vertical,   ///< Flip horizontally (i.e. change direction along y axis).
    Both        ///< Flip both horizontally and vertically.
};


/**
 * \brief The drawing order (along the imagined z axis).
 *
 * Objects are drawn from lower Z order values to higher Z order values. This
 * means that lower values result in background objects, and higher values in
 * foreground objects. The order is only important for when objects overlap.
 */
enum ZOrder : uint16_t
{
    ZOrderBackground    = 4000, ///< Rendered behind everything else (e.g. for background images).
    ZOrderNormal        = 5000, ///< Rendered as a regular object (e.g. player, enemy, bullet, ...)
    ZOrderForeground    = 6000, ///< Rendered as a foreground object (e.g. for HUD icons)
    ZOrderOverlay       = 7000  ///< Rendered as an overlay object (e.g. for "game over" screens)
};


#ifndef ARDUINO
enum
{
    LOW = 0,
    HIGH = 1
};


enum
{
    INPUT           = 0x00,
    OUTPUT          = 0x01,

    INPUT_PULLDOWN  = 0x02,
    INPUT_PULLUP    = 0x04
};
#endif

}

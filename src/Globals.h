#pragma once

#include <Arduino.h>

#include <cstdlib>
#include <cstring>


namespace MINTGGGameEngine
{

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
enum ZOrder
{
    ZOrderBackground    = 4000, ///< Rendered behind everything else (e.g. for background images).
    ZOrderNormal        = 5000, ///< Rendered as a regular object (e.g. player, enemy, bullet, ...)
    ZOrderForeground    = 6000, ///< Rendered as a foreground object (e.g. for HUD icons)
    ZOrderOverlay       = 7000  ///< Rendered as an overlay object (e.g. for "game over" screens)
};

}

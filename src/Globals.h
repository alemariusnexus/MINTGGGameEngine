/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>

#define MINTGGGAMEENGINE_VERSION_MAJOR      0
#define MINTGGGAMEENGINE_VERSION_MINOR      1
#define MINTGGGAMEENGINE_VERSION_PATCH      0
#define MINTGGGAMEENGINE_VERSION_SUFFIX     ""

#define MINTGGGAMEENGINE_VERSION_MAKE(major, minor, patch) (((major) << 24) | ((minor) << 16) | (patch))

#define MINTGGGAMEENGINE_VERSION MINTGGGAMEENGINE_VERSION_MAKE( \
    MINTGGGAMEENGINE_VERSION_MAJOR, MINTGGGAMEENGINE_VERSION_MINOR, MINTGGGAMEENGINE_VERSION_PATCH)


#define MINTGGGAMEENGINE_VERSION_MAKE_STRING_STR(x) #x
#define MINTGGGAMEENGINE_VERSION_MAKE_STRING(major, minor, patch, suffix) \
    MINTGGGAMEENGINE_VERSION_MAKE_STRING_STR(major) "." \
    MINTGGGAMEENGINE_VERSION_MAKE_STRING_STR(minor) "." \
    MINTGGGAMEENGINE_VERSION_MAKE_STRING_STR(patch) \
    suffix
#define MINTGGGAMEENGINE_VERSION_STRING MINTGGGAMEENGINE_VERSION_MAKE_STRING( \
    MINTGGGAMEENGINE_VERSION_MAJOR, MINTGGGAMEENGINE_VERSION_MINOR, MINTGGGAMEENGINE_VERSION_PATCH, \
    MINTGGGAMEENGINE_VERSION_SUFFIX)


#ifdef ESP_PLATFORM
#   define MINTGGGAMEENGINE_PORT_ESPIDF
#endif

#ifdef ARDUINO
#   define MINTGGGAMEENGINE_PORT_ARDUINO
#endif

#if !defined(MINTGGGAMEENGINE_PORT_ESPIDF)  &&  !defined(MINTGGGAMEENGINE_PORT_ARDUINO)
#   define MINTGGGAMEENGINE_PORT_DESKTOP
#endif


#ifndef MINTGGGAMEENGINE_PORT_DESKTOP
#   if defined(CONFIG_IDF_TARGET_ESP32)
#       define MINTGGGAMEENGINE_CHIP_ESP32
#   elif defined(CONFIG_IDF_TARGET_ESP32C3)
#       define MINTGGGAMEENGINE_CHIP_ESP32C3
#   elif defined(CONFIG_IDF_TARGET_ESP32P4)
#       define MINTGGGAMEENGINE_CHIP_ESP32P4
#   else
#       error This chip is currently not supported by MINTGGGameEngine!
#   endif
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

// TODO: Actually detect it. Currently we're just assuming.
#define MINTGGGAMEENGINE_LITTLE_ENDIAN


#include "config/Config.h"


namespace MINTGGGameEngine
{


typedef int16_t gpionum_t; // Must be signed, because it accepts -1 for "invalid pin"!

/**
 * \brief The direction to flip an object.
 */
enum class FlipDir : uint8_t
{
    // NOTE: It's crucial that these behave like bit flags. Parts of the engine rely on that.
    None        = 0x00,                     ///< Do not flip.
    Horizontal  = 0x01,                     ///< Flip horizontally (i.e. change direction along x axis).
    Vertical    = 0x02,                     ///< Flip horizontally (i.e. change direction along y axis).
    Both        = Horizontal | Vertical     ///< Flip both horizontally and vertically.
};
inline FlipDir operator|(FlipDir lhs, FlipDir rhs)
        { return static_cast<FlipDir>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)); }
inline FlipDir operator&(FlipDir lhs, FlipDir rhs)
        { return static_cast<FlipDir>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)); }
inline bool operator==(FlipDir lhs, uint8_t rhs) { return static_cast<uint8_t>(lhs) == rhs; }
inline bool operator==(uint8_t lhs, FlipDir rhs) { return lhs == static_cast<uint8_t>(rhs); }
inline bool operator!=(FlipDir lhs, uint8_t rhs) { return static_cast<uint8_t>(lhs) != rhs; }
inline bool operator!=(uint8_t lhs, FlipDir rhs) { return lhs != static_cast<uint8_t>(rhs); }

enum class Rotation
{
    Rot0,
    Rot90,
    Rot180,
    Rot270
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


enum class PixelFormat
{
    RGB565,
    BGR565,
    RGB888,
    BGR888,

    RGBX8888,
    XBGR8888
};

enum class AlphaStorage
{
    Bit1
};

enum class ByteOrder
{
    LittleEndian,
    BigEndian,

#ifdef MINTGGGAMEENGINE_LITTLE_ENDIAN
    Native = LittleEndian
#else
    Native = BigEndian
#endif
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

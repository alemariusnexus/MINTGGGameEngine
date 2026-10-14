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

#include "../Globals.h"

#include <memory>

#include "../util/Vec2.h"
#include "Bitmap.h"
#include "Color.h"
#include "screen/Screen.h"


// Enable this to cache the scaled version of all sprite bitmaps (if they have a non-unit scale) instead of scaling the
// bitmap during rendering each frame. This can improve rendering performance quite a lot, especially with many scaled
// bitmap sprites, but comes at the cost of significantly higher memory usage.
// Enabling it currently has the side-effect of better handling of clipping for scaled bitmaps. The renderer currently
// doesn't support "partial" rendering of scaled-up pixels, leading to abrupt cut-offs at the clipping boundaries.
// Because this setting creates intermediate bitmaps, it works around that issue.
// This setting stems from an era when the renderer didn't support bitmap scaling during rendering at all. It is still
// useful as an optimization, but could be removed in the future.
#define MINTGGGAMEENGINE_SPRITE_USE_SCALE_CACHE


namespace MINTGGGameEngine
{

/**
 * \brief The visual representation of a GameObject.
 *
 * Currently, four types of sprites are supported.
 *
 * The Null type is an invisible sprite. This is useful is an object should
 * function purely as an invisible collider.
 *
 * The Rect type is an axis-aligned rectangle with a solid color.
 *
 * The Circle type is a circle with a solid color.
 *
 * The Bitmap type is a rectangular bitmap, i.e. an array of color pixel values.
 *
 * \see Bitmap
 */
class Sprite
{
    friend class GameObject;

public:
    enum class Type
    {
        Null,
        Rect,
        Circle,
        Bitmap
    };

private:
    struct Data
    {
        Data(Type type) : type(type) {}
        ~Data() = default;

        Type type;
        union {
            struct {
                float w;
                float h;
                Color color;
                bool filled;
            } rect;

            struct {
                float r;
                Color color;
                bool filled;
            } circle;

            struct {
#ifdef MINTGGGAMEENGINE_SPRITE_USE_SCALE_CACHE
                float cachedScaleX;
                float cachedScaleY;
#endif
            } bmp;
        };

        // Don't put these in the enum because of it's non-trivial destructor
        Bitmap bitmap;
#ifdef MINTGGGAMEENGINE_SPRITE_USE_SCALE_CACHE
        Bitmap cachedScaledBitmap;
#endif
    };

public:
    static Sprite createRect(float w, float h, const Color& color, bool filled = true);
    static Sprite createCircle(float r, const Color& color, bool filled = true);
    static Sprite createBitmap(const Bitmap& bitmap);

public:
    /**
     * \brief Create a Null sprite, i.e. one that is invisible.
     */
    Sprite() = default;

    /**
     * \brief Create a Null sprite, i.e. one that is invisible.
     */
    Sprite(nullptr_t) {}

    Sprite(const Sprite& other) = default;

    ~Sprite() = default;
    
    /**
     * \brief Return the width of the sprite in pixels.
     */
    float getWidth() const;
    
    /**
     * \brief Return the height of the sprite in pixels.
     */
    float getHeight() const;
    
    /**
     * \brief Return the type of sprite.
     */
    Type getType() const { return d ? d->type : Type::Null; }
    
    /**
     * \brief Return the bitmap behind the sprite.
     *
     * Note that sprites of types other than Bitmap will return an invalid
     * bitmap for this method.
     *
     * \return The bitmap, or an invalid bitmap for non-Bitmap type sprites.
     */
    Bitmap getBitmap() const { return (d  &&  (d->type == Type::Bitmap)) ? d->bitmap : Bitmap(); }

    /**
     * \brief Draw the given sprite on a surface.
     *
     * \param screenPos Position on screen to draw the sprite at. The scale factor will NOT be applied to this.
     */
    void draw (
        DrawSurface& surface,
        const Vec2& screenPos,
        float scaleX, float scaleY,
        Rotation rotation,
        FlipDir flipDir
        ) const;

    Sprite& operator=(const Sprite& other) = default;

private:
    Sprite(Type type) : d(std::make_shared<Data>(type)) {}

#ifdef MINTGGGAMEENGINE_SPRITE_USE_SCALE_CACHE
    void updateScaleCache(float scaleX, float scaleY);
#else
    void updateScaleCache(float scaleX, float scaleY) {}
#endif

private:
    std::shared_ptr<Data> d;
};

}

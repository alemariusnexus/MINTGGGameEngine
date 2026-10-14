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

#include "Sprite.h"

#include <cassert>
#include <cmath>

LOG_USE_TAG("Sprite")


namespace MINTGGGameEngine
{

Sprite Sprite::createRect(float w, float h, const Color& color, bool filled)
{
    Sprite s(Type::Rect);
    s.d->rect.w = w;
    s.d->rect.h = h;
    s.d->rect.color = color;
    s.d->rect.filled = filled;
    return s;
}

Sprite Sprite::createCircle(float r, const Color& color, bool filled)
{
    Sprite s(Type::Circle);
    s.d->circle.r = r;
    s.d->circle.color = color;
    s.d->circle.filled = filled;
    return s;
}

Sprite Sprite::createBitmap(const Bitmap& bitmap)
{
    Sprite s(Type::Bitmap);
#ifdef MINTGGGAMEENGINE_SPRITE_USE_SCALE_CACHE
    s.d->bmp.cachedScaleX = 0.0f;
    s.d->bmp.cachedScaleY = 0.0f;
#endif
    s.d->bitmap = bitmap;
    return s;
}

float Sprite::getWidth() const
{
    if (!d) {
        return 0;
    } else if (d->type == Type::Rect) {
        return d->rect.w;
    } else if (d->type == Type::Circle) {
        return 2*d->circle.r;
    } else if (d->type == Type::Bitmap) {
        return d->bitmap.getWidth();
    }
    assert(false);
    return 0;
}

float Sprite::getHeight() const
{
    if (!d) {
        return 0;
    }
    if (d->type == Type::Rect) {
        return d->rect.h;
    } else if (d->type == Type::Circle) {
        return 2*d->circle.r;
    } else if (d->type == Type::Bitmap) {
        return d->bitmap.getHeight();
    }
    assert(false);
    return 0;
}

void Sprite::draw (
    DrawSurface& surface,
    const Vec2& screenPos,
    float scaleX, float scaleY,
    Rotation rotation,
    FlipDir flipDir
) const {
    if (!d) {
        return;
    }
    if (scaleX == 0.0f  ||  scaleY == 0.0f) {
        return;
    }

    if (d->type == Type::Rect) {
        auto w = static_cast<int32_t>(std::round(d->rect.w * scaleX));
        auto h = static_cast<int32_t>(std::round(d->rect.h * scaleY));
        if (rotation == Rotation::Rot90  ||  rotation == Rotation::Rot270) {
            std::swap(w, h);
        }
        surface.drawRect (
            static_cast<int32_t>(std::round(screenPos.x())), static_cast<int32_t>(std::round(screenPos.y())),
            w, h,
            d->rect.color,
            d->rect.filled
            );
    } else if (d->type == Type::Circle) {
        const float cx = screenPos.x() + d->circle.r*scaleX;
        const float cy = screenPos.y() + d->circle.r*scaleY;
        // TODO: Support scaleX != scaleY (which results in an oval, not a circle)
        surface.drawCircle (
            static_cast<int32_t>(std::round(cx)), static_cast<int32_t>(std::round(cy)),
            static_cast<int32_t>(std::round(d->circle.r*scaleX)),
            d->circle.color,
            d->circle.filled
            );
    } else if (d->type == Type::Bitmap) {
        if (d->bitmap) {
#ifdef MINTGGGAMEENGINE_SPRITE_USE_SCALE_CACHE
            const_cast<Sprite*>(this)->updateScaleCache(scaleX, scaleY);
            if (scaleX == 1.0f  &&  scaleY == 1.0f) {
                surface.drawBitmap (
                    static_cast<int32_t>(std::round(screenPos.x())), static_cast<int32_t>(std::round(screenPos.y())),
                    d->bitmap,
                    flipDir,
                    rotation,
                    1, 1
                    );
            } else {
                surface.drawBitmap (
                    static_cast<int32_t>(std::round(screenPos.x())), static_cast<int32_t>(std::round(screenPos.y())),
                    d->cachedScaledBitmap,
                    flipDir,
                    rotation,
                    1, 1
                    );
            }
#else
            surface.drawBitmap (
                    static_cast<int32_t>(std::round(screenPos.x())), static_cast<int32_t>(std::round(screenPos.y())),
                    d->bitmap,
                    flipDir,
                    rotation,
                    static_cast<int16_t>(std::round(scaleX)), static_cast<int16_t>(std::round(scaleY))
                    );
#endif
        }
    }
}

#ifdef MINTGGGAMEENGINE_SPRITE_USE_SCALE_CACHE
void Sprite::updateScaleCache(float scaleX, float scaleY)
{
    if (!d  ||  d->type != Type::Bitmap  ||  (scaleX == d->bmp.cachedScaleX  &&  scaleY == d->bmp.cachedScaleY)) {
        return;
    }

    // Do this first to potentially free up some memory
    d->cachedScaledBitmap = {};

    d->bmp.cachedScaleX = scaleX;
    d->bmp.cachedScaleY = scaleY;
    if (d->bitmap) {
        d->cachedScaledBitmap = d->bitmap.scaled(scaleX, scaleY);
    } else {
        d->cachedScaledBitmap = {};
    }
}
#endif

}

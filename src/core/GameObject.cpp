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

#include "GameObject.h"

#include <cassert>

#include "Game.h"


namespace MINTGGGameEngine
{

GameObject GameObject::createCircle(float x, float y, float r, const Color& color, bool filled, bool collider)
{
    return GameObject(x, y, Sprite::createCircle(r, color, filled), collider ? Collider::createCircle(0, 0, r) : Collider());
}

GameObject GameObject::createRect(float x, float y, float w, float h, const Color& color, bool filled, bool collider)
{
    if (w < 0) {
        x += w;
        w = -w;
    }
    if (h < 0) {
        y += h;
        h = -h;
    }
    return GameObject(x, y, Sprite::createRect(w, h, color, filled), collider ? Collider::createRect(0, 0, w, h) : Collider());
}

GameObject GameObject::createBitmap(float x, float y, const Bitmap& bitmap, bool collider)
{
    return GameObject(x, y, Sprite::createBitmap(bitmap), collider ? Collider::createRect(0, 0, bitmap.getWidth(), bitmap.getHeight()) : Collider());
}

GameObject GameObject::createColliderCircle(float x, float y, float r)
{
    return GameObject(x, y, Sprite(), Collider::createCircle(0, 0, r));
}

GameObject GameObject::createColliderRect(float x, float y, float w, float h)
{
    return GameObject(x, y, Sprite(), Collider::createRect(0, 0, w, h));
}

GameObject::GameObject(float x, float y, const Sprite& sprite, const Collider& collider)
    : d(std::make_shared<Data>())
{
    d->game = nullptr;
    d->x = x;
    d->y = y;
    d->moveDir = Vec2();
    d->scaleX = 1;
    d->scaleY = 1;
    d->rotation = Rotation::Rot0;
    d->flipDir = FlipDir::None;
    d->sprite = sprite;
    d->collider = collider;
    d->tags = 0;
    d->zOrder = ZOrderNormal;
    d->visible = true;
}

Vec2 GameObject::getCenterPosition(bool useSprite) const
{
    return getPosition() + getSize(useSprite)*0.5f;
}

void GameObject::setCenterPosition(const Vec2& p, bool useSprite)
{
    setPosition(p.x() - getWidth(useSprite)*0.5f, p.y() - getHeight(useSprite)*0.5f);
}

bool GameObject::isLeftOf(const GameObject& other, bool useSprite) const
{
    return getCenterPosition(useSprite).x() < other.getCenterPosition(useSprite).x();
}

bool GameObject::isRightOf(const GameObject& other, bool useSprite) const
{
    return other.isLeftOf(*this, useSprite);
}

bool GameObject::isAbove(const GameObject& other, bool useSprite) const
{
    return getCenterPosition(useSprite).y() < other.getCenterPosition(useSprite).y();
}

bool GameObject::isBelow(const GameObject& other, bool useSprite) const
{
    return other.isAbove(*this, useSprite);
}

void GameObject::setScale(float scaleX, float scaleY)
{
    if (d) {
        d->scaleX = std::max<int16_t>(static_cast<int16_t>(std::round(scaleX)), 1);
        d->scaleY = std::max<int16_t>(static_cast<int16_t>(std::round(scaleY)), 1);
    }
}

void GameObject::setScaleAroundCenter(float scaleX, float scaleY)
{
    const Vec2 center = getCenterPosition();
    setScale(scaleX, scaleY);
    setCenterPosition(center);
}

void GameObject::setRotationAroundCenter(Rotation rotation)
{
    const Vec2 center = getCenterPosition();
    setRotation(rotation);
    setCenterPosition(center);
}

float GameObject::getWidth(bool useSprite) const
{
    if (!d) return 0;
    if (useSprite) {
        return (d->rotation == Rotation::Rot0  ||  d->rotation == Rotation::Rot180)
                ? d->sprite.getWidth() * static_cast<float>(d->scaleX)
                : d->sprite.getHeight() * static_cast<float>(d->scaleY);
    } else {
        return (d->rotation == Rotation::Rot0  ||  d->rotation == Rotation::Rot180)
                ? d->collider.getWidth() * static_cast<float>(d->scaleX)
                : d->collider.getHeight() * static_cast<float>(d->scaleY);
    }
}

float GameObject::getHeight(bool useSprite) const
{
    if (!d) return 0;
    if (useSprite) {
        return (d->rotation == Rotation::Rot0  ||  d->rotation == Rotation::Rot180)
                ? d->sprite.getHeight() * static_cast<float>(d->scaleY)
                : d->sprite.getWidth() * static_cast<float>(d->scaleX);
    } else {
        return (d->rotation == Rotation::Rot0  ||  d->rotation == Rotation::Rot180)
                ? d->collider.getHeight() * static_cast<float>(d->scaleY)
                : d->collider.getWidth() * static_cast<float>(d->scaleX);
    }
}

void GameObject::setZOrder(uint16_t zorder)
{
    if (d) {
        d->zOrder = zorder;
        if (d->game) {
            d->game->notifyGameObjectZOrderChanged(*this);
        }
    }
}

Collider GameObject::getWorldCollider() const
{
    if (!d) return {};
    return getCollider().toWorld(d->x, d->y, d->scaleX, d->scaleY, d->rotation, d->flipDir);
}

void GameObject::draw(DrawSurface& surface, const Vec2& offset, float scaleX, float scaleY) const
{
    if (isVisible()) {
        getSprite().draw (
            surface,
            {d->x*scaleX + offset.x(), d->y*scaleY + offset.y()},
            scaleX * static_cast<float>(d->scaleX), scaleY * static_cast<float>(d->scaleY),
            d->rotation,
            d->flipDir
            );
    }
}

bool GameObject::collides(const GameObject& other, float shrink) const
{
    return getWorldCollider().collides(other.getWorldCollider(), shrink);
}

void GameObject::updateScaleCache(float scaleX, float scaleY)
{
    getSprite().updateScaleCache(scaleX, scaleY);
}

void GameObject::notifySpawned(Game* game) const
{
    if (d) {
        assert(!d->game);
        d->game = game;
    }
}

void GameObject::notifyDespawned(Game* game) const
{
    if (d) {
        assert(d->game == game);
        d->game = nullptr;
    }
}

}

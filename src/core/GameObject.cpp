#include "GameObject.h"


namespace MINTGGGameEngine
{

GameObject GameObject::createCircle(float x, float y, float r, const Color& color, bool filled, bool collider)
{
    return GameObject(x, y, Sprite::createCircle(r, color, filled), collider ? Collider::createCircle(r, r, r) : Collider());
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
    return GameObject(x, y, Sprite(), Collider::createCircle(r, r, r));
}

GameObject GameObject::createColliderRect(float x, float y, float w, float h)
{
    return GameObject(x, y, Sprite(), Collider::createRect(0, 0, w, h));
}

GameObject::GameObject(float x, float y, const Sprite& sprite, const Collider& collider)
    : d(std::make_shared<Data>())
{
    d->x = x;
    d->y = y;
    d->moveDir = Vec2();
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

float GameObject::getWidth(bool useSprite) const
{
    return useSprite ? d->sprite.getWidth() : d->collider.getWidth();
}

float GameObject::getHeight(bool useSprite) const
{
    return useSprite ? d->sprite.getHeight() : d->collider.getHeight();
}

Collider GameObject::getWorldCollider() const
{
    return getCollider().toWorld(getX(), getY(), getFlipDir());
}

void GameObject::draw(Screen& screen, const Vec2& offset) const
{
    if (isVisible()) {
        getSprite().draw(screen, getX()+offset.x(), getY()+offset.y(), getFlipDir());
    }
}

bool GameObject::collides(const GameObject& other, float shrink) const
{
    return getWorldCollider().collides(other.getWorldCollider(), shrink);
}

}

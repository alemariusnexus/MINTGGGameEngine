#include "GameObject.h"


namespace MINTGGGameEngine
{

GameObject GameObject::createCircle(float x, float y, int r, const Color& color, bool filled, bool collider)
{
    return GameObject(x, y, Sprite::createCircle(r, color, filled), collider ? Collider::createCircle(r, r, r) : Collider());
}

GameObject GameObject::createRect(float x, float y, int w, int h, const Color& color, bool filled, bool collider)
{
    return GameObject(x, y, Sprite::createRect(w, h, color, filled), collider ? Collider::createRect(0, 0, w, h) : Collider());
}

GameObject GameObject::createBitmap(float x, float y, const Bitmap& bitmap, bool collider)
{
    return GameObject(x, y, Sprite::createBitmap(bitmap), collider ? Collider::createRect(0, 0, bitmap.getWidth(), bitmap.getHeight()) : Collider());
}

GameObject GameObject::createColliderCircle(float x, float y, int r)
{
    return GameObject(x, y, Sprite(), Collider::createCircle(r, r, r));
}

GameObject GameObject::createColliderRect(float x, float y, int w, int h)
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

Collider GameObject::getWorldCollider() const
{
    return getCollider().toWorld(getX(), getY(), getFlipDir());
}

void GameObject::draw(Screen& screen) const
{
    if (isVisible()) {
        getSprite().draw(screen, getX(), getY(), getFlipDir());
    }
}

bool GameObject::collides(const GameObject& other, float shrink) const
{
    return getWorldCollider().collides(other.getWorldCollider(), shrink);
}

}

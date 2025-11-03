#pragma once

#include "Globals.h"
#include "Bitmap.h"
#include "Collider.h"
#include "Color.h"
#include "Screen.h"
#include "Sprite.h"
#include "Vec2.h"

#include <memory>


namespace MINTGGGameEngine
{

class GameObject
{
private:
    struct Data
    {
        float x;
        float y;
        Vec2 moveDir;
        FlipDir flipDir;
        Sprite sprite;
        Collider collider;
        uint64_t tags;
        uint16_t zOrder; // Higher is in front
        bool visible;
    };

public:
    static GameObject createCircle(float x, float y, int r, const Color& color, bool filled = true, bool collider = true);
    static GameObject createRect(float x, float y, int w, int h, const Color& color, bool filled = true, bool collider = true);
    static GameObject createBitmap(float x, float y, const Bitmap& bitmap, bool collider = true);
    
    static GameObject createColliderCircle(float x, float y, int r);
    static GameObject createColliderRect(float x, float y, int w, int h);

public:
    GameObject(float x = 0.0f, float y = 0.0f, const Sprite& sprite = Sprite(), const Collider& collider = Collider());
    GameObject(const GameObject& other) : d(other.d) {}

    float getX() const { return d->x; }
    float getY() const { return d->y; }
	Vec2 getPosition() const { return Vec2(getX(), getY()); }
    Vec2 getMoveDirection() const { return d->moveDir; }
    FlipDir getFlipDir() const { return d->flipDir; }
    uint16_t getZOrder() const { return d->zOrder; }
    bool isVisible() const { return d->visible; }
    Sprite getSprite() const { return d->sprite; }
    Collider getCollider() const { return d->collider; }
    Collider getWorldCollider() const;

    void setPosition(float x, float y) { d->x = x; d->y = y; }
    void setPosition(const Vec2& p) { setPosition(p.x(), p.y()); }
	void setX(float x) { d->x = x; }
	void setY(float y) { d->y = y; }
    void setMoveDirection(const Vec2& dir) { d->moveDir = dir.normalized(); }
    void setMoveDirection(float x, float y) { setMoveDirection(Vec2(x, y)); }
    void move(float dx, float dy) { setPosition(d->x + dx, d->y + dy); }
    void move(const Vec2& v) { move(v.x(), v.y()); }
    void move(float speed) { move(d->moveDir * speed); }
    void setFlipDir(FlipDir flipDir) { d->flipDir = flipDir; }
    void setZOrder(uint16_t zorder = ZOrderNormal) { d->zOrder = zorder; }
    void setVisible(bool visible) { d->visible = visible; }
    void setSprite(const Sprite& sprite) { d->sprite = sprite; }
    void setCollider(const Collider collider) { d->collider = collider; }

    void draw(Screen& screen) const;

    bool collides(const GameObject& other, float shrink = 0.0f) const;
    
    GameObject& setTag(uint64_t tag) { d->tags |= tag; return *this; }
    GameObject& unsetTag(uint64_t tag) { d->tags &= ~tag; return *this; }
    bool hasTag(uint64_t tag) const { return hasAnyTags(tag); }
    bool hasAllTags(uint64_t tags) const { return (d->tags & tags) == tags; }
    bool hasAnyTags(uint64_t tags) const { return (d->tags & tags) != 0; }

    GameObject& operator=(const GameObject& other) { d = other.d; return *this; };

    bool operator==(const GameObject& other) const { return d == other.d; }
    bool operator!=(const GameObject& other) const { return d != other.d; }
    bool operator<(const GameObject& other) const { return d < other.d; }
    bool operator<=(const GameObject& other) const { return d <= other.d; }
    bool operator>(const GameObject& other) const { return d > other.d; }
    bool operator>=(const GameObject& other) const { return d >= other.d; }

private:
    std::shared_ptr<Data> d;
};

}

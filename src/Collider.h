#pragma once

#include "Globals.h"
#include "Color.h"


namespace MINTGGGameEngine
{

class Screen;

class Collider
{
public:
    enum class Type
    {
        Null,
        Circle,
        Rect
    };

public:
    static Collider createCircle(float cx, float cy, float r)
    {
        Collider c(Type::Circle);
        c.circle.cx = cx;
        c.circle.cy = cy;
        c.circle.r = r;
        return c;
    }

    static Collider createRect(float x, float y, float w, float h)
    {
        Collider c(Type::Rect);
        c.rect.x = x;
        c.rect.y = y;
        c.rect.w = w;
        c.rect.h = h;
        return c;
    }

public:
    Collider() : type(Type::Null) {}
    Collider(const Collider& o);

    Collider toWorld(float px, float py, FlipDir flip) const;
    
    Collider shrunk(float shrink) const;

    bool collides(const Collider& other, float shrink = 0.0f) const;

    void debugDraw(Screen& screen, const Color& color);

    operator bool() const { return type != Type::Null; }

private:
    Collider(Type type) : type(type) {}

    static bool collidesCircleCircle(const Collider& a, const Collider& b);
    static bool collidesRectRect(const Collider& a, const Collider& b);
    static bool collidesCircleRect(const Collider& c, const Collider& r);

private:
    Type type;
    union {
        struct {
            float cx;
            float cy;
            float r;
        } circle;

        struct {
            float x;
            float y;
            float w;
            float h;
        } rect;
    };
};

}

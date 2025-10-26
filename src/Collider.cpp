#include "Collider.h"


#include "Screen.h"




namespace MINTGGGameEngine
{


Collider::Collider(const Collider& o)
    : type(o.type)
{
    if (type == Type::Null) {
            // Nothing to do
    } else if (type == Type::Circle) {
            circle.cx = o.circle.cx;
            circle.cy = o.circle.cy;
            circle.r = o.circle.r;
    } else if (type == Type::Rect) {
        rect.x = o.rect.x;
        rect.y = o.rect.y;
        rect.w = o.rect.w;
        rect.h = o.rect.h;
    } else {
        assert(false);
    }
}


Collider Collider::toWorld(float px, float py, FlipDir flip) const
{
    if (type == Type::Null) {
        return Collider();
    } else if (type == Type::Circle) {
        return Collider::createCircle(circle.cx+px, circle.cy+py, circle.r);
    } else if (type == Type::Rect) {
        return Collider::createRect(rect.x+px, rect.y+py, rect.w, rect.h);
    }
    assert(false);
    return *this;
}


Collider Collider::shrunk(float shrink) const
{
    if (type == Type::Null) {
        return Collider();
    } else if (type == Type::Circle) {
        return Collider::createCircle(circle.cx, circle.cy, circle.r-shrink);
    } else if (type == Type::Rect) {
        return Collider::createRect(rect.x+shrink, rect.y+shrink, rect.w-2*shrink, rect.h-2*shrink);
    }
    assert(false);
    return *this;
}


bool Collider::collides(const Collider& other, float shrink) const
{
    if (type == Type::Null  ||  other.type == Type::Null) {
        return false;
    }

    Collider shrunkThis = (shrink == 0.0f) ? *this : shrunk(shrink);
    Collider shrunkOther = (shrink == 0.0f) ? other : other.shrunk(shrink);
  
    if (type == Type::Circle) {
        if (other.type == Type::Circle) {
            return collidesCircleCircle(shrunkThis, shrunkOther);
        } else if (other.type == Type::Rect) {
            return collidesCircleRect(shrunkThis, shrunkOther);
        }
    } else if (type == Type::Rect) {
        if (other.type == Type::Circle) {
            return collidesCircleRect(shrunkOther, shrunkThis);
        } else if (other.type == Type::Rect) {
            return collidesRectRect(shrunkThis, shrunkOther);
        }
    }
  
    assert(false);
    return false;
}


void Collider::debugDraw(Screen& screen, const Color& color)
{
    if (type == Type::Circle) {
        screen.drawCircle(circle.cx, circle.cy, circle.r, color, false);
    } else if (type == Type::Rect) {
        screen.drawRect(rect.x, rect.y, rect.w, rect.h, color, false);
    }
}


bool Collider::collidesCircleCircle(const Collider& a, const Collider& b)
{
    float dx = b.circle.cx - a.circle.cx;
    float dy = b.circle.cy - a.circle.cy;
    return sqrtf(dx*dx + dy*dy) < a.circle.r+b.circle.r;
}


bool Collider::collidesRectRect(const Collider& a, const Collider& b)
{
    return  a.rect.x < b.rect.x+b.rect.w
        &&  a.rect.x+a.rect.w > b.rect.x
        &&  a.rect.y < b.rect.y+b.rect.h
        &&  a.rect.y+a.rect.h > b.rect.y;
}


bool Collider::collidesCircleRect(const Collider& c, const Collider& r)
{
    float rectX = c.circle.cx;
    float rectY = c.circle.cy;
    if (c.circle.cx < r.rect.x) {
        rectX = r.rect.x;
    } else if (c.circle.cx > r.rect.x+r.rect.w) {
        rectX = r.rect.x+r.rect.w;
    }
    if (c.circle.cy < r.rect.y) {
        rectY = r.rect.y;
    } else if (c.circle.cy > r.rect.y+r.rect.h) {
        rectY = r.rect.y+r.rect.h;
    }
    float dx = c.circle.cx - rectX;
    float dy = c.circle.cy - rectY;
    return sqrtf(dx*dx + dy*dy) < c.circle.r;
}


}

#include "Collider.h"


#include <cmath>

#include "MathUtils.h"
#include "RayCastResult.h"
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


float Collider::getWidth() const
{
    if (type == Type::Circle) {
        return 2*circle.r;
    } else if (type == Type::Rect) {
        return rect.w;
    }
    return 0.0f;
}


float Collider::getHeight() const
{
    if (type == Type::Circle) {
        return 2*circle.r;
    } else if (type == Type::Rect) {
        return rect.h;
    }
    return 0.0f;
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


void Collider::debugDraw(Screen& screen, const Color& color, const Vec2& offset)
{
    if (type == Type::Circle) {
        screen.drawCircle(circle.cx+offset.x(), circle.cy+offset.y(), circle.r, color, false);
    } else if (type == Type::Rect) {
        screen.drawRect(rect.x+offset.x(), rect.y+offset.y(), rect.w, rect.h, color, false);
    }
}


size_t Collider::castRay (
        std::vector<RayCastHit>& hits,
        const Vec2& start, const Vec2& direction, float length
) const {
    RayCastHit hit;
    size_t numHits = 0;
    
    if (type == Type::Circle) {
        
        Vec2 startToCenter = Vec2(circle.cx, circle.cy) - start;
        float distRayFromCenter = startToCenter.length() * sinf(direction.angle(startToCenter));
        if (distRayFromCenter <= circle.r) {
            float tCenter = startToCenter.dot(direction); // Project startToCenter onto direction
            float tEnterExitHalfDist = sqrtf(circle.r*circle.r - distRayFromCenter*distRayFromCenter);
            float tEnter = tCenter-tEnterExitHalfDist;
            float tExit = tCenter+tEnterExitHalfDist;
            
            if (tEnter >= 0.0f  &&  tEnter <= length) {
                hit.entering = true;
                hit.hitPoint = start + direction*tEnter;
                hit.rayOffset = tEnter;
                hits.push_back(hit);
                numHits++;
            }
            if (tExit >= 0.0f  &&  tExit <= length) {
                hit.entering = false;
                hit.hitPoint = start + direction*tExit;
                hit.rayOffset = tExit;
                hits.push_back(hit);
                numHits++;
            }
        }
        
    } else if (type == Type::Rect) {
        
        const Vec2 startToEnd = direction*length;
        const Vec2 end = start + startToEnd;
        
        int numIntersects = 0;
        float intersectTs[2];
        
        int numHits;
        
        // Top
        intersectTs[numIntersects] = IntersectLineSegLineSegSimple (
                start, end,
                Vec2(rect.x, rect.y), Vec2(rect.x+rect.w, rect.y),
                &numHits
                );
        if (numHits > 0) {
            numIntersects++;
        }
        
        // Bottom
        intersectTs[numIntersects] = IntersectLineSegLineSegSimple (
                start, end,
                Vec2(rect.x, rect.y+rect.h), Vec2(rect.x+rect.w, rect.y+rect.h),
                &numHits
                );
        if (numHits > 0) {
            numIntersects++;
        }
        
        if (numIntersects < 2) {
            // Left
            intersectTs[numIntersects] = IntersectLineSegLineSegSimple (
                    start, end,
                    Vec2(rect.x, rect.y), Vec2(rect.x, rect.y+rect.h),
                    &numHits
                    );
            if (numHits > 0) {
                numIntersects++;
            }
        }
        
        if (numIntersects < 2) {
            // Right
            intersectTs[numIntersects] = IntersectLineSegLineSegSimple (
                    start, end,
                    Vec2(rect.x+rect.w, rect.y), Vec2(rect.x+rect.w, rect.y+rect.h),
                    &numHits
                    );
            if (numHits > 0) {
                numIntersects++;
            }
        }
        
        if (numIntersects == 2) {
            // Ray cuts through
            
            if (intersectTs[0] > intersectTs[1]) {
                float tmp = intersectTs[0];
                intersectTs[0] = intersectTs[1];
                intersectTs[1] = tmp;
            }
            
            hit.entering = true;
            hit.hitPoint = start + startToEnd*intersectTs[0];
            hit.rayOffset = intersectTs[0]*length;
            hits.push_back(hit);
            
            hit.entering = false;
            hit.hitPoint = start + startToEnd*intersectTs[1];
            hit.rayOffset = intersectTs[1]*length;
            hits.push_back(hit);
            
            numHits = 2;
        } else if (numIntersects == 1) {
            // Ray only enters or leaves
            
            if (PointLiesInsideAARect(start, rect.x, rect.y, rect.w, rect.h)) {
                hit.entering = false;
            } else {
                hit.entering = true;
            }
            hit.hitPoint = start + startToEnd*intersectTs[0];
            hit.rayOffset = intersectTs[0];
            hits.push_back(hit);
            
            numHits = 1;
        }
        
    }
    
    return numHits;
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

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

#include "Collider.h"

#include <cassert>
#include <cmath>

#include "../util/MathUtils.h"
#include "../util/RayCastResult.h"
#include "../graphics/screen/Screen.h"




namespace MINTGGGameEngine
{


Collider::Collider(const Collider& o)
    : type(o.type)
{
    if (type == Type::Null) {
            // Nothing to do
    } else if (type == Type::Circle) {
            circle.x = o.circle.x;
            circle.y = o.circle.y;
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


Collider& Collider::operator=(const Collider& o)
{
    if (o != *this) {
        type = o.type;
        if (type == Type::Null) {
            // Nothing to do
        } else if (type == Type::Circle) {
            circle.x = o.circle.x;
            circle.y = o.circle.y;
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
    return *this;
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


Collider Collider::toWorld(float px, float py, float scaleX, float scaleY, Rotation rotation, FlipDir flip) const
{
    (void) flip;
    if (type == Type::Null) {
        return {};
    } else if (type == Type::Circle) {
        // TODO: Handle scaleX != scaleY (which results in an oval)
        return Collider::createCircle(circle.x+px, circle.y+py, circle.r*scaleX);
    } else if (type == Type::Rect) {
        if (rotation == Rotation::Rot0  ||  rotation == Rotation::Rot180) {
            return Collider::createRect(rect.x+px, rect.y+py, rect.w*scaleX, rect.h*scaleY);
        } else {
            return Collider::createRect(rect.x+px, rect.y+py, rect.h*scaleY, rect.w*scaleX);
        }
    }
    assert(false);
    return *this;
}


Collider Collider::shrunk(float shrink) const
{
    if (type == Type::Null) {
        return Collider();
    } else if (type == Type::Circle) {
        return Collider::createCircle(circle.x+shrink, circle.y+shrink, circle.r-shrink*0.5f);
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


void Collider::debugDraw(DrawSurface& surface, const Color& color, const Vec2& offset, float scaleX, float scaleY)
{
    if (type == Type::Circle) {
        auto center = getCenterPosition();
        surface.drawCircle (
            // TODO: Support scaleX != scaleY (which would be an oval)
            static_cast<int32_t>(std::round(center.x()*scaleX + offset.x())),
            static_cast<int32_t>(std::round(center.y()*scaleY + offset.y())),
            static_cast<int32_t>(std::round(circle.r * scaleX)),
            color,
            false
            );
    } else if (type == Type::Rect) {
        surface.drawRect (
            static_cast<int32_t>(std::round(rect.x*scaleX + offset.x())),
            static_cast<int32_t>(std::round(rect.y*scaleY + offset.y())),
            static_cast<int32_t>(std::round(rect.w * scaleX)),
            static_cast<int32_t>(std::round(rect.h * scaleY)),
            color,
            false
            );
    }
}


size_t Collider::castRay (
        std::vector<RayCastHit>& hits,
        const Vec2& start, const Vec2& direction, float length,
        const GameObject& gameObject
) const {
    RayCastHit hit;
    size_t numHits = 0;
    
    if (type == Type::Circle) {
        
        const Vec2 startToCenter = getCenterPosition() - start;
        
        const float distRayFromCenterSin = sinf(direction.angle(startToCenter));
        const float distRayFromCenterSq = startToCenter.lengthSq() * distRayFromCenterSin*distRayFromCenterSin;
        const float radiusSq = circle.r*circle.r;
        
        if (distRayFromCenterSq <= radiusSq) {
            const float tCenter = startToCenter.dot(direction); // Project startToCenter onto direction
            const float tEnterExitHalfDist = sqrtf(radiusSq - distRayFromCenterSq);
            const float tEnter = tCenter-tEnterExitHalfDist;
            const float tExit = tCenter+tEnterExitHalfDist;
            
            if (tEnter >= 0.0f  &&  tEnter <= length) {
                hits.emplace_back(gameObject, true, start + direction*tEnter, tEnter);
                numHits++;
            }
            if (tExit >= 0.0f  &&  tExit <= length) {
                hits.emplace_back(gameObject, false, start + direction*tExit, tExit);
                numHits++;
            }
        }
        
    } else if (type == Type::Rect) {
        
        const Vec2 startToEnd = direction*length;
        const Vec2 end = start + startToEnd;
        
        int numIntersects = 0;
        float intersectTs[2];
        
        int numIntersectsSingle;
        
        // Top
        intersectTs[numIntersects] = IntersectLineSegLineSegSimple (
                start, end,
                Vec2(rect.x, rect.y), Vec2(rect.x+rect.w, rect.y),
                &numIntersectsSingle
                );
        if (numIntersectsSingle > 0) {
            numIntersects++;
        }
        
        // Bottom
        intersectTs[numIntersects] = IntersectLineSegLineSegSimple (
                start, end,
                Vec2(rect.x, rect.y+rect.h), Vec2(rect.x+rect.w, rect.y+rect.h),
                &numIntersectsSingle
                );
        if (numIntersectsSingle > 0) {
            numIntersects++;
        }
        
        if (numIntersects < 2) {
            // Left
            intersectTs[numIntersects] = IntersectLineSegLineSegSimple (
                    start, end,
                    Vec2(rect.x, rect.y), Vec2(rect.x, rect.y+rect.h),
                    &numIntersectsSingle
                    );
            if (numIntersectsSingle > 0) {
                numIntersects++;
            }
        }
        
        if (numIntersects < 2) {
            // Right
            intersectTs[numIntersects] = IntersectLineSegLineSegSimple (
                    start, end,
                    Vec2(rect.x+rect.w, rect.y), Vec2(rect.x+rect.w, rect.y+rect.h),
                    &numIntersectsSingle
                    );
            if (numIntersectsSingle > 0) {
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
            
            hits.emplace_back (
                    gameObject, true,
                    start + startToEnd*intersectTs[0],
                    intersectTs[0]*length
                    );
            
            hits.emplace_back (
                    gameObject, false,
                    start + startToEnd*intersectTs[1],
                    intersectTs[1]*length
                    );
            
            numHits = 2;
        } else if (numIntersects == 1) {
            // Ray only enters or leaves
            
            bool entering;
            if (PointLiesInsideAARect(start, rect.x, rect.y, rect.w, rect.h)) {
                entering = false;
            } else {
                entering = true;
            }
            hits.emplace_back (
                    gameObject, entering,
                    start + startToEnd*intersectTs[0],
                    intersectTs[0]*length
                    );
            
            numHits = 1;
        }
        
    }
    
    return numHits;
}


Vec2 Collider::getCenterPosition() const
{
    if (type == Type::Null) {
        return {};
    } else if (type == Type::Circle) {
        [[likely]]
        return {circle.x+circle.r, circle.y+circle.r};
    } else if (type == Type::Rect) {
        return {rect.x+rect.w*0.5f, rect.y+rect.h*0.5f};
    }
    assert(false);
    return {};
}


bool Collider::collidesCircleCircle(const Collider& a, const Collider& b)
{
    Vec2 d = b.getCenterPosition() - a.getCenterPosition();
    float sumR = a.circle.r + b.circle.r;
    return d.lengthSq() < sumR*sumR;
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
    const auto cCenter = c.getCenterPosition();
    float rectX = cCenter.x();
    float rectY = cCenter.y();
    if (cCenter.x() < r.rect.x) {
        rectX = r.rect.x;
    } else if (cCenter.x() > r.rect.x+r.rect.w) {
        rectX = r.rect.x+r.rect.w;
    }
    if (cCenter.y() < r.rect.y) {
        rectY = r.rect.y;
    } else if (cCenter.y() > r.rect.y+r.rect.h) {
        rectY = r.rect.y+r.rect.h;
    }
    Vec2 d(cCenter.x()-rectX, cCenter.y()-rectY);
    return d.lengthSq() < c.circle.r*c.circle.r;
}


}

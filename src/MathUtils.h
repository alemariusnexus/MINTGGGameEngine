#pragma once

#include "Globals.h"

#include "Vec2.h"


namespace MINTGGGameEngine
{


bool PointLiesInsideAARect (
        const Vec2& p,
        float rx, float ry, float rw, float rh
        );

float IntersectLineSegLineSegSimple (
        const Vec2& a1, const Vec2& a2,
        const Vec2& b1, const Vec2& b2,
        int* numHits
        );


}

#include "MathUtils.h"

#include <cmath>


namespace MINTGGGameEngine
{


bool PointLiesInsideAARect (
        const Vec2& p,
        float rx, float ry, float rw, float rh
) {
    if (rw < 0) {
        rx += rw;
        rw = -rw;
    }
    if (rh < 0) {
        ry += rh;
        rh = -rh;
    }
    return  p.x() >= rx
        &&  p.y() >= ry
        &&  p.x() <= rx+rw
        &&  p.y() <= ry+rh;
}

float IntersectLineSegLineSegSimple (
        const Vec2& a1, const Vec2& a2,
        const Vec2& b1, const Vec2& b2,
        int* numHits
) {
    // See: https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line_segment
    
    const float dxA1A2 = a1.x() - a2.x();
    const float dxB1B2 = b1.x() - b2.x();
    const float dyA1A2 = a1.y() - a2.y();
    const float dyB1B2 = b1.y() - b2.y();
    
    const float dxA1B1 = a1.x() - b1.x();
    const float dyA1B1 = a1.y() - b1.y();
    
    const float tuDenom = dxA1A2*dyB1B2 - dyA1A2*dxB1B2;
    
    if (fabsf(tuDenom) < 1e-8f) {
        // Line segments are parallel
        // TODO: Implement a "non-simple" version that can tell the different
        //  parallel cases apart, e.g.:
        //      * Parallel lines but not coincident (0 hits)
        //      * Same line segments (-1 hits, for infinite)
        //      * Partially coincident line segments (-1 hits)
        //      * Coincident lines but no overlap in segments (0 hits)
        if (numHits) *numHits = 0;
        return -1.0f;
    }
    
    const float tNum = dxA1B1*dyB1B2 - dyA1B1*dxB1B2;
    const float t = tNum / tuDenom;
    if (t < 0.0f  ||  t > 1.0f) {
        // No intersection on A
        if (numHits) *numHits = 0;
        return -1.0f;
    }
    
    const float uNum = dxA1A2*dyA1B1 - dyA1A2*dxA1B1;
    const float u = -uNum / tuDenom;
    if (u < 0.0f  ||  u > 1.0f) {
        // No intersection on B
        if (numHits) *numHits = 0;
        return -1.0f;
    }
    
    if (numHits) *numHits = 1;
    return t;
}


}

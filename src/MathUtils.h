#pragma once

#include "Globals.h"

#include "Vec2.h"


namespace MINTGGGameEngine
{


/**
 * \brief Test whether a point lies within an alix-aligned rectangle.
 *
 * \param p The point in question.
 * \param rx Top-left corner x coordinate of rectangle.
 * \param ry Top-left corner y coordinate of rectangle.
 * \param rw Width of rectangle.
 * \param rh Height of rectangle.
 * \param true if inside, false otherwise.
 */
bool PointLiesInsideAARect (
        const Vec2& p,
        float rx, float ry, float rw, float rh
        );

/**
 * \brief Calculate intersection between two line segments (simple version).
 *
 * This works on line segments, i.e. they have a definite start and end point
 * and do NOT extend infinitely in either direction.
 *
 * This function only calculates intersection if the line segments are not
 * parallel, i.e. if exactly one intersection point exists. If they are
 * parallel, zero intersection points will be reported (even if the segments are
 * coincident).
 *
 * \param a1 Start point of first line segment.
 * \param a2 End point of first line segment.
 * \param b1 Start point of second line segment.
 * \param b2 End point of second line segment.
 * \param numHits Output variable. Will be set to the number of intersection
 *      points found (see above).
 * \return Fraction of distance from a1 to a2 at which the intersection point
 *      lies, in range [0,1]. If no intersection exists or the segments overlap,
 *      -1 will be returned instead.
 */
float IntersectLineSegLineSegSimple (
        const Vec2& a1, const Vec2& a2,
        const Vec2& b1, const Vec2& b2,
        int* numHits
        );


}

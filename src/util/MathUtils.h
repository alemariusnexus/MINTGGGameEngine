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

#pragma once

#include "../Globals.h"

#include <algorithm>
#include <cmath>

#include "Vec2.h"


namespace MINTGGGameEngine
{


/**
 * \brief Test whether a point lies within an axis-aligned rectangle.
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

bool IntersectAABoxAABox (
    float ax, float ay, float aw, float ah,
    float bx, float by, float bw, float bh
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

/**
 * Clip a rectangle to the inside of another rectangle.
 *
 * This function takes the input rectangle {(rectXS, rectYS), (rectXE, rectYE)} and cuts off all the parts of it that
 * lie outside the clipping rectangle {(clipXS, clipYS), (clipXE, clipYE)}. The resulting rectangle is written back
 * to the coordinates passed by reference if any part of it remains after clipping.
 *
 * The two points passed for the input rectangle can be any pair of corners that span the rectangle (not necessarily the
 * top-left and bottom-right), but the resulting rectangle will always have (rectXS, rectYS) as the top-left corner, and
 * (rectXE, rectYE) as the bottom-right corner if any part of it remains.
 *
 * Note that the same is not true for the clip rectangle: This function assumes that (clipXS, clipYS) is the top-left
 * corner and (clipXE, clipYE) is the bottom-right corner. Passing other corners results in incorrect results from
 * this function.
 *
 * \tparam NumT Numerical type to work on.
 * \param rectXS x coordinate of first corner of input (and output) rectangle.
 * \param rectYS y coordinate of first corner of input (and output) rectangle.
 * \param rectXE x coordinate of second corner of input (and output) rectangle (exclusive!).
 * \param rectYE y coordinate of second corner of input (and output) rectangle (exclusive!).
 * \param clipXS x coordinate of the top-left corner of the clipping rectangle.
 * \param clipYS y coordinate of the top-left corner of the clipping rectangle.
 * \param clipXE x coordinate of the bottom-right corner of the clipping rectangle (exclusive!).
 * \param clipYE y coordinate of the bottom-right corner of the clipping rectangle (exclusive!).
 * \return true if any part of the input rectangle remains after clipping, false otherwise. If false is returned, the
 *      values of the input/output rectangle remain unchanged.
 */
template <typename NumT>
bool ClipRectToRect (
    NumT& rectXS, NumT& rectYS, NumT& rectXE, NumT& rectYE,
    NumT clipXS, NumT clipYS, NumT clipXE, NumT clipYE
) {
    // Normalize start and end points
    const NumT rXS = std::min(rectXS, rectXE);
    const NumT rYS = std::min(rectYS, rectYE);
    const NumT rXE = std::max(rectXS, rectXE);
    const NumT rYE = std::max(rectYS, rectYE);

    // Check if rect lies fully outside of clip
    if (    rXS >= clipXE       // Rect fully to the right of clip
        ||  rYS >= clipYE       // Rect fully below clip
        ||  rXE <= clipXS       // Rect fully to the left of clip
        ||  rYE <= clipYS       // Rect fully above clip
    ) {
        // No overlap
        return false;
    }

    // Clip rect inside of clip
    rectXS = std::max(rXS, clipXS);
    rectYS = std::max(rYS, clipYS);
    rectXE = std::min(rXE, clipXE);
    rectYE = std::min(rYE, clipYE);

    return true;
}

/**
 * Clip a rectangle to the inside of another rectangle (corner+size version).
 *
 * See ClipRectToRect() for more details. The output rectangle is normalized again, with (rectX, rectY) always being
 * the top-left corner and both width and height always being non-negative if this function returns true.
 *
 * @tparam NumT Numerical type to work on.
 * @param rectX x coordinate of one corner of input (and output) rectangle.
 * @param rectY y coordinate of one corner of input (and output) rectangle.
 * @param rectW width of the input (and output) rectangle. May be negative.
 * @param rectH height of the input (and output) rectangle. May be negative.
 * @param clipX x coordiante of the top-left corner of the clipping rectangle.
 * @param clipY y coordiante of the top-left corner of the clipping rectangle.
 * @param clipW width of the clipping rectangle. Must not be negative.
 * @param clipH height of the clipping rectangle. Must not be negative.
 * @return true if any part of the input rectangle remains after clipping, false otherwise. If false is returned, the
 *      values of the input/output rectangle remain unchanged.
 */
template <typename NumT>
bool ClipRectToRectWH (
    NumT& rectX, NumT& rectY, NumT& rectW, NumT& rectH,
    NumT clipX, NumT clipY, NumT clipW, NumT clipH
) {
    NumT rectXE = rectX + rectW;
    NumT rectYE = rectY + rectH;
    const NumT clipXE = clipX+clipW;
    const NumT clipYE = clipY+clipH;
    if (!ClipRectToRect(rectX, rectY, rectXE, rectYE, clipX, clipY, clipXE, clipYE)) {
        return false;
    }
    rectW = rectXE-rectX;
    rectH = rectYE-rectY;
    return true;
}


}

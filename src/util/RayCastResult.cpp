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

#include "RayCastResult.h"

#include <cmath>



namespace MINTGGGameEngine
{


void RayCastResult::drawDebugRay (
        DrawSurface& surface,
        const Vec2& start, const Vec2& end,
        const Vec2& offset,
        float scaleX, float scaleY
) {
    surface.drawLine (
            static_cast<int32_t>(std::round(start.x()*scaleX + offset.x())),
            static_cast<int32_t>(std::round(start.y()*scaleY + offset.y())),
            static_cast<int32_t>(std::round(end.x()*scaleX + offset.x())),
            static_cast<int32_t>(std::round(end.y()*scaleY + offset.y())),
            Color::fromRGB565(0x681F)
            );
}

void RayCastResult::drawDebug (
    DrawSurface& surface,
    const Vec2& offset,
    float scaleX, float scaleY
) const {
    for (const auto& hit : d->hits) {
        // TODO: Support scaleX != scaleY (which would be an oval)
        surface.drawCircle (
                static_cast<int32_t>(std::round(hit.hitPoint.x()*scaleX + offset.x())),
                static_cast<int32_t>(std::round(hit.hitPoint.y()*scaleY + offset.y())),
                static_cast<int32_t>(2*scaleX),
                Color::fromRGB565(hit.entering ? 0x681F : 0xF81E),
                true
                );
    }
}


}

#include "RayCastResult.h"




namespace MINTGGGameEngine
{


void RayCastResult::drawDebugRay (
        Screen& screen,
        const Vec2& start, const Vec2& end,
        const Vec2& offset
) {
    screen.drawLine (
            (int16_t) (start.x() + offset.x() + 0.5f),
            (int16_t) (start.y() + offset.y() + 0.5f),
            (int16_t) (end.x() + offset.x() + 0.5f),
            (int16_t) (end.y() + offset.y() + 0.5f),
            Color(0x681F)
            );
}

void RayCastResult::drawDebug(Screen& screen, const Vec2& offset) const
{
    for (const auto& hit : d->hits) {
        screen.drawCircle (
                (int16_t) (hit.hitPoint.x() + offset.x() + 0.5f),
                (int16_t) (hit.hitPoint.y() + offset.y() + 0.5f),
                2,
                hit.entering ? Color(0x681F) : Color(0xF81E),
                true
                );
    }
}


}

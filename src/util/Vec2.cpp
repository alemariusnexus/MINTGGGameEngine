#include "Vec2.h"

#include <cmath>


namespace MINTGGGameEngine
{


float Vec2::angle(const Vec2& o) const
{
    return acosf(dot(o) / (length()*o.length()));
}


}

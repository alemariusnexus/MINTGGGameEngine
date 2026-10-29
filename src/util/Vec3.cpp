/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "Vec3.h"

#include <cmath>


namespace MINTGGGameEngine
{


float Vec3::angle(const Vec3& o) const
{
    return acosf(dot(o) / (length()*o.length()));
}


}

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

#include "GravitySimulator.h"


namespace MINTGGGameEngine
{


GravitySimulator::GravitySimulator()
{
    initDefault();
}

GravitySimulator::GravitySimulator(const GameObject& gameObj)
        : gameObj(gameObj)
{
    initDefault();
}

void GravitySimulator::initDefault()
{
    gravity = Vec2(0.0f, 4.0f);
    terminalVelocity = 2.5f;
    velocity = 0.0f;
}

void GravitySimulator::reset()
{
    setVelocity(0.0f);
}

void GravitySimulator::simulate(float dt)
{
    velocity += gravity.length()*dt;
    if (velocity > terminalVelocity) {
        velocity = terminalVelocity;
    }
    gameObj.setPosition(gameObj.getPosition() + gravity.normalized()*velocity);
}


}

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

#include "DefaultEngine.h"
#include "Game.h"


namespace MINTGGGameEngine
{

extern DefaultEngine engine;
extern Game game;

}

extern void gameSetup();
extern void gameLoop(float dt);
extern void onCollision(const MINTGGGameEngine::GameObjectCollision& coll);

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
extern void postDraw(float dt);
extern void onCollision(const MINTGGGameEngine::GameObjectCollision& coll);

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

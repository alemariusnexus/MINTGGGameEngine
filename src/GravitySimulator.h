#pragma once

#include "Globals.h"

#include "GameObject.h"
#include "Vec2.h"


namespace MINTGGGameEngine
{


/**
 * \brief A class that simulates the velocity of a GameObject on which gravity
 *      acts.
 *
 * Useful to implement simple falling or jumping objects.
 *
 * simulate() must be called every frame, with the time that passed since the
 * previous frame.
 */
class GravitySimulator
{
public:
    GravitySimulator();
    GravitySimulator(const GameObject& gameObj);
    
    void setGameObject(const GameObject& gameObj) { this->gameObj = gameObj; }
    GameObject getGameObject() const { return gameObj; }
    
    void setGravity(const Vec2& gravity) { this->gravity = gravity; }
    Vec2 getGravity() const { return gravity; }
    
    void setTerminalVelocity(float velocity) { this->terminalVelocity = velocity; }
    float getTerminalVelocity() const { return terminalVelocity; }
    
    void setVelocity(float velocity) { this->velocity = velocity; }
    float getVelocity() const { return velocity; }
    
    void reset();
    
    void simulate(float dt);
    
private:
    void initDefault();
    
private:
    GameObject gameObj;
    
    Vec2 gravity;
    float terminalVelocity;
    
    float velocity;
};


}

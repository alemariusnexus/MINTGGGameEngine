#pragma once

#include "Globals.h"
#include "GameObject.h"


namespace MINTGGGameEngine
{

class GameObjectCollision
{
public:
    GameObjectCollision(const GameObject& a, const GameObject& b) : a(a), b(b) {}
    GameObjectCollision(const GameObjectCollision& other) : a(other.a), b(other.b) {}
    
    bool isBetween(const GameObject& checkA, const GameObject& checkB) const
    {
        return (a == checkA  &&  b == checkB)  ||  (a == checkB  &&  b == checkA);
    }
    
    bool isInvolved(const GameObject& go) const { return go == a  ||  go == b; }
    bool isTagInvolved(uint64_t tag) const { return a.hasTag(tag)  ||  b.hasTag(tag); }
    
    GameObject getByTag(uint64_t tag) const { return a.hasTag(tag) ? a : b; }
    GameObject getOther(const GameObject& go) const { return go == a ? b : a; }
    GameObject getOtherByTag(uint64_t tag) const { return a.hasTag(tag) ? b : a; }

public:
    GameObject a;
    GameObject b;
};

}

#pragma once

#include "Globals.h"
#include "GameObjectCollision.h"
#include "GameObject.h"
#include "Screen.h"
#include "Vec2.h"

#include <memory>
#include <vector>


namespace MINTGGGameEngine
{

struct RayCastHit
{
    GameObject gameObject;
    bool entering;
    Vec2 hitPoint;
    float rayOffset;
};

/**
 * \brief The central class representing a game.
 */
class RayCastResult
{
private:
    struct Data
    {
        std::vector<RayCastHit> hits;
    };
    
public:
    static void drawDebugRay(Screen& screen, const Vec2& start, const Vec2& end, const Vec2& offset);

public:
    RayCastResult() : d(std::make_shared<Data>()) {}
    RayCastResult(std::vector<RayCastHit>&& hits) : d(std::make_shared<Data>()) { d->hits = std::move(hits); }
    RayCastResult(const RayCastResult& other) : d(other.d) {}
    
    RayCastResult& operator=(const RayCastResult& other) { d = other.d; return *this; }
    
    void drawDebug(Screen& screen, const Vec2& offset) const;
    
    const std::vector<RayCastHit>& getHits() const { return d->hits; }
    std::vector<RayCastHit>& getHits() { return d->hits; }
    
private:
    std::shared_ptr<Data> d;
};

}

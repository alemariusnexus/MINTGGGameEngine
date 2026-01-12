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

/**
 * \brief Represents a single intersection (hit) of a ray with a GameObject.
 */
struct RayCastHit
{
    /**
     * \brief The GameObject that was hit.
     */
    GameObject gameObject;
    
    /**
     * \brief true if the ray entered the GameObject, false if it exited it.
     */
    bool entering;
    
    /**
     * \brief The point at which the intersection occurred.
     */
    Vec2 hitPoint;
    
    /**
     * \brief Distance along the ray at which the intersection occurred, in
     *      range [0, rayLength].
     */
    float rayOffset;
};

/**
 * \brief Result of a ray cast operation.
 *
 * This is the result of calling Game::castRay(). It contains a list of
 * RayCastHit objects, one for each intersection between the ray and a
 * GameObject.
 *
 * \see RayCastHit
 */
class RayCastResult
{
private:
    struct Data
    {
        std::vector<RayCastHit> hits;
    };
    
public:
    /**
     * \brief Draw a ray for debug purposes.
     *
     * This is considered an internal method.
     */
    static void drawDebugRay(Screen& screen, const Vec2& start, const Vec2& end, const Vec2& offset);

public:
    RayCastResult() : d(std::make_shared<Data>()) {}
    RayCastResult(std::vector<RayCastHit>&& hits) : d(std::make_shared<Data>()) { d->hits = std::move(hits); }
    RayCastResult(const RayCastResult& other) : d(other.d) {}
    
    RayCastResult& operator=(const RayCastResult& other) { d = other.d; return *this; }
    
    /**
     * \brief Draw the results for debug purposes.
     *
     * This is considered an internal method.
     */
    void drawDebug(Screen& screen, const Vec2& offset) const;
    
    /**
     * \brief Return the list of hit points (and their info).
     *
     * \see RayCastHit
     */
    const std::vector<RayCastHit>& getHits() const { return d->hits; }
    
    /**
     * \brief Return the list of hit points (and their info).
     *
     * \see RayCastHit
     */
    std::vector<RayCastHit>& getHits() { return d->hits; }
    
private:
    std::shared_ptr<Data> d;
};

}

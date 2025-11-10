#pragma once

#include "Globals.h"
#include "GameObject.h"


namespace MINTGGGameEngine
{


/**
 * \brief Stores information about a collision between two GameObjects.
 *
 * Currently, only the two colliding objects are stored. In the future, this
 * class might provide info on the exact point(s) of collision as well.
 *
 * \see GameObject
 * \see Collider
 */
class GameObjectCollision
{
public:
    /**
     * \brief Create a collision between two objects.
     *
     * \param a The first object involved.
     * \param b The second object involved.
     */
    GameObjectCollision(const GameObject& a, const GameObject& b) : a(a), b(b) {}
    
    /**
     * \brief Copy constructor.
     */
    GameObjectCollision(const GameObjectCollision& other) : a(other.a), b(other.b) {}
    
    
    /**
     * \brief Check whether the collision is between the two given objects.
     *
     * \param checkA The first object.
     * \param checkB The second object.
     * \return true if the collision is between the two objects, false
     *      otherwise.
     */
    bool isBetween(const GameObject& checkA, const GameObject& checkB) const
    {
        return (a == checkA  &&  b == checkB)  ||  (a == checkB  &&  b == checkA);
    }
    
    /**
     * \brief Check if the collision involves the given GameObject.
     *
     * \param go The GameObject to check for collision.
     * \return true if involved, false otherwise.
     */
    bool isInvolved(const GameObject& go) const { return go == a  ||  go == b; }
    
    /**
     * \brief Check if at least one of the involved object has the given tag.
     *
     * \param tag The tag to check for.
     * \return true if the tag is involved, false otherwise
     * \see GameObject.setTag()
     * \see getByTag()
     */
    bool isTagInvolved(uint64_t tag) const { return a.hasTag(tag)  ||  b.hasTag(tag); }
    
    
    /**
     * \brief Get the object involved in the collision with the given tag.
     * 
     * Note that if none of the objects involved has the tag, the result is
     * **undefined**.
     *
     * \param tag The tag to check for.
     * \return The object with the tag.
     */
    GameObject getByTag(uint64_t tag) const { return a.hasTag(tag) ? a : b; }
    
    /**
     * \brief Get the other GameObject involved in the collision.
     * 
     * Note that if the given GameObject is not involved in the collision, the
     * result is **undefined**.
     *
     * \param go One of the two objects involved in the collision.
     * \return The other object involved in the collision.
     */
    GameObject getOther(const GameObject& go) const { return go == a ? b : a; }
    
    /**
     * \brief Get the object involved in the collision without the given tag.
     *
     * Note that if both or none of the involved objects have the tag, the
     * result is **undefined**.
     *
     * \param tag The tag for the other object involved.
     * \return The other object involved in the collision.
     * \see getByTag()
     */
    GameObject getOtherByTag(uint64_t tag) const { return a.hasTag(tag) ? b : a; }

public:
    /**
     * \brief The first object involved in the collision.
     *
     * Note that the order in which the objects are stored (a/b) is undefined.
     *
     * \see b
     */
    GameObject a;
    
    /**
     * \brief The second object involved in the collision.
     *
     * Note that the order in which the objects are stored (a/b) is undefined.
     *
     * \see a
     */
    GameObject b;
};

}
